#include <gwdirs.h>

static char* configdir = NULL;
static char* logdir = NULL;
static char* libdir = NULL;
static char* cachedir = NULL;
static char* maxscaledatadir = NULL;
static char* langdir = NULL;
static char* piddir = NULL;

void set_configdir(char* str)
{
    free(configdir);
    configdir = strdup(str);
}
void set_logdir(char* str)
{
    free(logdir);
    logdir = strdup(str);
}
void set_libdir(char* str)
{
    free(libdir);
    libdir = strdup(str);
}
void set_cachedir(char* str)
{
    free(cachedir);
    cachedir = strdup(str);
}
void set_datadir(char* str)
{
    free(maxscaledatadir);
    maxscaledatadir = strdup(str);
}
void set_langdir(char* str)
{
    free(langdir);
    langdir = strdup(str);
}
void set_piddir(char* str)
{
    free(piddir);
    piddir = strdup(str);
}

/**
 * Get the directory with all the modules.
 * @return The module directory
 */
char* get_libdir()
{
    return libdir?libdir:(char*)default_libdir;
}

/**
 * Get the service cache directory
 * @return The path to the cache directory
 */
char* get_cachedir()
{
    return cachedir?cachedir:(char*)default_cachedir;
}


/**
 * Get the service cache directory
 * @return The path to the cache directory
 */
char* get_datadir()
{
    return maxscaledatadir?maxscaledatadir:(char*)default_datadir;
}


char* get_configdir()
{
    return configdir?configdir:(char*)default_configdir;
}

char* get_piddir()
{
    return piddir?piddir:(char*)default_piddir;
}

char* get_logdir()
{
    return logdir?logdir:(char*)default_logdir;
}

char* get_langdir()
{
    return langdir?langdir:(char*)default_langdir;
}
