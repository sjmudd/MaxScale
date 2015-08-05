/*
 * This file is distributed as part of the MariaDB Corporation MaxScale.  It is free
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
 * Copyright MariaDB Corporation Ab 2015
 */

#include <my_config.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <router.h>
#include <readwritesplit.h>

#include <mysql.h>
#include <skygw_utils.h>
#include <log_manager.h>
#include <query_classifier.h>
#include <dcb.h>
#include <spinlock.h>
#include <modinfo.h>
#include <modutil.h>
#include <mysql_client_server_protocol.h>

/**
 * @file rws_common.c Common read/write splitting functions
 */

int calculate_weights(ROUTER_INSTANCE* router, SERVICE* service)
{
    char* weightby;
    /*
     * If server weighting has been defined calculate the percentage
     * of load that will be sent to each server. This is only used for
     * calculating the least connections, either globally or within a
     * service, or the number of current operations on a server.
     */
    if ((weightby = serviceGetWeightingParameter(service)) != NULL)
    {
	int 	n, total = 0;
	BACKEND	*backend;
	char* param;

	for (n = 0; router->servers[n]; n++)
	{
	    backend = router->servers[n];
	    if((param = serverGetParameter(backend->backend_server, weightby)) == NULL)
	    {
		skygw_log_write(LE,"Server %s is missing the weighting parameter %s.",
			 backend->backend_server->unique_name,
			 weightby);
		continue;
	    }
	    total += atoi(param);
	}
	if (total <= 0)
	{
	    LOGIF(LE, (skygw_log_write(LOGFILE_ERROR,
				     "WARNING: Weighting Parameter for service '%s' "
		    "will be ignored as no servers have valid values "
		    "for the parameter '%s'.\n",
				     service->name, weightby)));
	}
	else
	{
	    for (n = 0; router->servers[n]; n++)
	    {
		long perc;
		long wght;

		backend = router->servers[n];
		if((param = serverGetParameter(backend->backend_server,weightby)) == NULL ||
		 (wght = atoi(param)) <= 0)
		{
		    backend->weight = 1;
		    skygw_log_write(
			    LOGFILE_ERROR,
			     "Server '%s' has no valid value "
			    "for weighting parameter '%s', "
			    "no queries will be routed to "
			    "this server.\n",
			     router->servers[n]->backend_server->unique_name,
			     weightby);
		}
		else
		{
		    perc = (wght*1000) / total;

		    if (perc <= 0 && wght != 0)
		    {
			perc = 1;
		    }
		    backend->weight = perc;
		}
	    }
	}
    }
    return 0;
}

int allocate_backends(ROUTER_INSTANCE *router,SERVICE *service)
{
    SERVER_REF* sref;
    int i, nservers;
    /** Calculate number of servers */
    sref = service->servers;
    nservers = 0;

    while (sref != NULL)
    {
	nservers++;
	sref=sref->next;
    }

    /** Free old memory if this is a reallocation of servers */
    if(router->servers)
    {
	for(i = 0;router->servers[i];i++)
	{
	    free(router->servers[i]);
	}
	free(router->servers);
    }

    router->servers = (BACKEND **)calloc(nservers + 1, sizeof(BACKEND *));

    if (router->servers == NULL)
    {
	return -1;
    }
    /**
     * Create an array of the backend servers in the router structure to
     * maintain a count of the number of connections to each
     * backend server.
     */

    sref = service->servers;
    nservers= 0;

    while (sref != NULL) {
	if ((router->servers[nservers] = malloc(sizeof(BACKEND))) == NULL)
	{
	    /** clean up */
	    for (i = 0; i < nservers; i++) {
		free(router->servers[i]);
	    }
	    router->servers[0] = NULL;
	    return -1;
	}
	router->servers[nservers]->backend_server = sref->server;
	router->servers[nservers]->backend_conn_count = 0;
	router->servers[nservers]->be_valid = false;
	router->servers[nservers]->weight = 1000;
#if defined(SS_DEBUG)
	router->servers[nservers]->be_chk_top = CHK_NUM_BACKEND;
	router->servers[nservers]->be_chk_tail = CHK_NUM_BACKEND;
#endif
	nservers += 1;
	sref = sref->next;
    }
    router->servers[nservers] = NULL;
    return 0;
}
