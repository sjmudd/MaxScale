/*
 * This file is distributed as part of MaxScale by MariaDB Corporation.  It is free
 * software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation,
 * version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright MariaDB Corporation Ab 2014
 */
#include <stdio.h>
#include <gwdirs.h>
#include <filter.h>
#include <modinfo.h>
#include <modutil.h>
#include <skygw_utils.h>
#include <log_manager.h>
#include <string.h>
#include <hint.h>
#include <hashtable.h>
#include <query_classifier.h>

#include "server.h"

/** Defined in log_manager.cc */
extern int            lm_enabled_logfiles_bitmask;
extern size_t         log_ses_count[];
extern __thread log_info_t tls_log_info;

/**
 * @file canonfilter.c - 
 * @verbatim

 * Date		Who		Description
 * 20/07/2015	Markus Mäkelä	Initial implementation
 * @endverbatim
 */

MODULE_INFO 	info = {
    MODULE_API_FILTER,
    MODULE_ALPHA_RELEASE,
    FILTER_VERSION,
    "A canonical query filter"
};

static char *version_str = "V1.0.0";

static	FILTER	*createInstance(char **options, FILTER_PARAMETER **params);
static	void	*newSession(FILTER *instance, SESSION *session);
static	void 	closeSession(FILTER *instance, void *session);
static	void 	freeSession(FILTER *instance, void *session);
static	void	setDownstream(FILTER *instance, void *fsession, DOWNSTREAM *downstream);
static	int	routeQuery(FILTER *instance, void *fsession, GWBUF *queue);
static	void	diagnostic(FILTER *instance, void *fsession, DCB *dcb);


static FILTER_OBJECT MyObject = {
    createInstance,
    newSession,
    closeSession,
    freeSession,
    setDownstream,
    NULL,		// No Upstream requirement
    routeQuery,
    NULL,
    diagnostic,
};

enum{
    CANON_SESSION_RUNNING,
    CANON_SESSION_STOPPING,
    CANON_SESSION_FREE
};

/**
 * Instance structure
 */
typedef struct {
    HASHTABLE      *hash; /*< Contains all canonical queries */
    char* rule_output; /*< Rules are written to this file */
    char* rule_input; /*< Initial rules are read from this file */
    time_t last_sync; /*< Last time the hashtable was written to file*/
    double sync_interval;
} CANON_INSTANCE;

/**
 * The session structuee for this regex filter
 */
typedef struct {
    DOWNSTREAM down; /*< The downstream filter */
    int state; /*< Session state */
} CANON_SESSION;

void read_query_hash(CANON_INSTANCE *instance);
void persist_query_hash(CANON_INSTANCE *instance);

#define DEFAULT_SYNC_INTERVAL 15
/**
 * Implementation of the mandatory version entry point
 *
 * @return version string of the module
 */
char *
version()
{
    return version_str;
}

/**
 * The module initialisation routine, called when the module
 * is first loaded.
 */
void
ModuleInit()
{
}

/**
 * The module entry point routine. It is this routine that
 * must populate the structure that is referred to as the
 * "module object", this is a structure with the set of
 * external entry points for this module.
 *
 * @return The module object
 */
FILTER_OBJECT *
GetModuleObject()
{
    return &MyObject;
}

/**
 * Create an instance of the filter for a particular service
 * within MaxScale.
 *
 * @param options	The options for this filter
 * @param params	The array of name/value pair parameters for the filter
 *
 * @return The instance data for this new instance
 */
static	FILTER	*
createInstance(char **options, FILTER_PARAMETER **params)
{
    CANON_INSTANCE	*my_instance;

    if ((my_instance = calloc(1, sizeof(CANON_INSTANCE))) != NULL)
    {
	my_instance->hash = hashtable_alloc(25,simple_str_hash,strcasecmp);
	my_instance->rule_output = NULL;
	my_instance->rule_input = NULL;
	my_instance->sync_interval = DEFAULT_SYNC_INTERVAL;
	hashtable_memory_fns(my_instance->hash,
			 (HASHMEMORYFN)strdup,
			 (HASHMEMORYFN)strdup,
			 (HASHMEMORYFN)free,
			 (HASHMEMORYFN)free);
	for(int i = 0;params[i];i++)
	{
	    if(strcmp(params[i]->name,"input") == 0)
	    {
		my_instance->rule_input = strdup(params[i]->value);
	    }
	    else if(strcmp(params[i]->name,"output") == 0)
	    {
		my_instance->rule_output = strdup(params[i]->value);
	    }
	    else if(strcmp(params[i]->name,"interval") == 0)
	    {
		my_instance->sync_interval = strtof(params[i]->value,NULL);
	    }
	}

	if(my_instance->rule_output == NULL)
	{
	    int size = strlen(get_datadir()) + strlen("/canonfilter.output") + 1;
	    my_instance->rule_output = malloc(sizeof(char*)*size);
	    strcpy(my_instance->rule_output,get_datadir());
	    strcat(my_instance->rule_output,"/canonfilter.output");
	}

	if(my_instance->rule_input)
	{
	    read_query_hash(my_instance);
	}

	my_instance->last_sync = time(NULL);
    }
    return (FILTER *)my_instance;
}

/**
 * Associate a new session with this instance of the filter.
 *
 * @param instance	The filter instance data
 * @param session	The session itself
 * @return Session specific data for this session
 */
static	void	*
newSession(FILTER *instance, SESSION *session)
{
    CANON_INSTANCE	*my_instance = (CANON_INSTANCE *)instance;
    CANON_SESSION	*my_session;

    if ((my_session = calloc(1, sizeof(CANON_SESSION))) != NULL)
    {
	my_session->state = CANON_SESSION_RUNNING;
    }

    return my_session;
}

/**
 * Close a session with the filter, this is the mechanism
 * by which a filter may cleanup data structure etc.
 *
 * @param instance	The filter instance data
 * @param session	The session being closed
 */
static	void 	
closeSession(FILTER *instance, void *session)
{
    CANON_SESSION *my_session = (CANON_SESSION *)session;
    CANON_INSTANCE *my_instance = (CANON_INSTANCE *)instance;
    my_session->state = CANON_SESSION_STOPPING;
    if(difftime(time(NULL),my_instance->last_sync) > my_instance->sync_interval)
    {
	my_instance->last_sync = time(NULL);
	persist_query_hash(my_instance);
    }
}

/**
 * Free the memory associated with this filter session.
 *
 * @param instance	The filter instance data
 * @param session	The session being closed
 */
static void
freeSession(FILTER *instance, void *session)
{
    CANON_SESSION *my_session = (CANON_SESSION *)session;
    my_session->state = CANON_SESSION_FREE;
    free(my_session);
    return;
}

/**
 * Set the downstream component for this filter.
 *
 * @param instance	The filter instance data
 * @param session	The session being closed
 * @param downstream	The downstream filter or router
 */
static void
setDownstream(FILTER *instance, void *session, DOWNSTREAM *downstream)
{
    CANON_SESSION	*my_session = (CANON_SESSION *)session;
    my_session->down = *downstream;
}

/**
 * The routeQuery entry point. This is passed the query buffer
 * to which the filter should be applied. Once applied the
 * query should normally be passed to the downstream component
 * (filter or router) in the filter chain.
 *
 * If the regular expressed configured in the match parameter of the
 * filter definition matches the SQL text then add the hint
 * "Route to named server" with the name defined in the server parameter
 *
 * @param instance	The filter instance data
 * @param session	The filter session
 * @param queue		The query data
 */
static	int	
routeQuery(FILTER *instance, void *session, GWBUF *queue)
{
    CANON_INSTANCE	*my_instance = (CANON_INSTANCE *)instance;
    CANON_SESSION	*my_session = (CANON_SESSION *)session;
    char			*canon,*value;

    if (modutil_is_SQL(queue))
    {
	if(!query_is_parsed(queue))
	    parse_query(queue);

	if((canon = skygw_get_canonical(queue)) != NULL)
	{
	    if((value = hashtable_fetch(my_instance->hash,canon)) != NULL &&
	     server_find_by_unique_name(value) != NULL)
	    {
		skygw_log_write(LT,"Adding routing hint to server %s",value);
		queue->hint = hint_create_route(queue->hint,HINT_ROUTE_TO_NAMED_SERVER,value);
	    }
	    else
	    {
		hashtable_add(my_instance->hash,canon,"");
	    }
	    free(canon);
	}
    }

    return my_session->down.routeQuery(my_session->down.instance,
				       my_session->down.session, queue);
}

/**
 * Diagnostics routine
 *
 * If fsession is NULL then print diagnostics on the filter
 * instance as a whole, otherwise print diagnostics for the
 * particular session.
 *
 * @param	instance	The filter instance
 * @param	fsession	Filter session, may be NULL
 * @param	dcb		The DCB for diagnostic output
 */
static	void
diagnostic(FILTER *instance, void *fsession, DCB *dcb)
{
    CANON_INSTANCE	*my_instance = (CANON_INSTANCE *)instance;
    CANON_SESSION	*my_session = (CANON_SESSION *)fsession;
    HASHITERATOR* iter = hashtable_iterator(my_instance->hash);
    char *key,*value;

    dcb_printf(dcb,"-----------------------------\n"
	    "Canonical queries:\n");
    while((key = (char*)hashtable_next(iter)) != NULL)
    {
	dcb_printf(dcb,"%s",key);
	if((value = hashtable_fetch(my_instance->hash,key)) != NULL)
	{
	    	dcb_printf(dcb,"=%s",value);
	}
	dcb_printf(dcb,"\n");
    }
	dcb_printf(dcb,"-----------------------------\n");
}

void read_query_hash(CANON_INSTANCE *instance)
{
    char *buffer,*tok,*rule,*saved;
    FILE *file;
    long size;

    if((file = fopen(instance->rule_input,"r")) == NULL)
	return;

    fseek(file,0,SEEK_END);
    size = ftell(file);
    rewind(file);
    if((buffer = malloc(sizeof(char)*(size + 1))) == NULL)
    {
	fclose(file);
	return;
    }

    int bytes = fread(buffer,sizeof(char),size,file);
    tok = strtok_r(buffer,"\n",&saved);
    while(tok)
    {
	char *tmpsaved,*tmptok = strdup(tok);
	rule = strtok_r(tmptok,"|",&tmpsaved);

	if(rule)
	{
	    char* server = strtok_r(NULL,"|",&tmpsaved);
	    if(server)
	    {
		hashtable_add(instance->hash,rule,server);
	    }
	}

	free(tmptok);
	tok = strtok_r(NULL,"\n",&saved);
    }
    free(buffer);
    fclose(file);
}

void persist_query_hash(CANON_INSTANCE *instance)
{
    HASHITERATOR* iter = hashtable_iterator(instance->hash);
    char *key,*value;
    FILE *file;

    file = fopen(instance->rule_output,"w+");

    while((key = (char*)hashtable_next(iter)) != NULL)
    {
	fprintf(file,"%s|",key);
	if((value = hashtable_fetch(instance->hash,key)) != NULL)
	{
	    	fprintf(file,"%s",value);
	}
	fprintf(file,"\n");
    }
    fclose(file);
}