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
 * Copyright MariaDB Corporation Ab 2013-2014
 */
#if !defined(LOG_MANAGER_H)
# define LOG_MANAGER_H

typedef struct filewriter_st  filewriter_t;
typedef struct logfile_st     logfile_t;
typedef struct fnames_conf_st fnames_conf_t;
typedef struct logmanager_st  logmanager_t;

typedef enum {
  BB_READY = 0x00,
  BB_FULL,
  BB_CLEARED
} blockbuf_state_t;

typedef enum {
    LOGFILE_ERROR = 1,
    LOGFILE_FIRST = LOGFILE_ERROR,
    LOGFILE_MESSAGE = 2,
    LOGFILE_TRACE = 4,
    LOGFILE_DEBUG = 8,
    LOGFILE_LAST = LOGFILE_DEBUG
} logfile_id_t;


typedef enum { FILEWRITER_INIT, FILEWRITER_RUN, FILEWRITER_DONE }
    filewriter_state_t;

/**
* Thread-specific logging information.
*/
typedef struct log_info_st
{
	size_t li_sesid;
	int    li_enabled_logs;
} log_info_t;
    
#define LE LOGFILE_ERROR
#define LM LOGFILE_MESSAGE
#define LT LOGFILE_TRACE
#define LD LOGFILE_DEBUG

#define LOG_MAY_BE_ENABLED(id) (((lm_enabled_logfiles_bitmask & id) ||	\
				log_ses_count[id] > 0) ? true : false)
/**
 * Execute the given command if specified log is enabled in general or
 * if there is at least one session for whom the log is enabled.
 */
#define LOGIF_MAYBE(id,cmd) if (LOG_MAY_BE_ENABLED(id))	\
	{						\
		cmd;					\
	}
	
/**
 * Execute the given command if specified log is enabled in general or
 * if the log is enabled for the current session.
 */	
#define LOGIF(id,cmd) if (mxs_log_enabed(id))	\
	{					\
		cmd;				\
	}

#if !defined(LOGIF)
#define LOGIF(id,cmd) if (lm_enabled_logfiles_bitmask & id)     \
	{                                                       \
		cmd;                                            \
	}
#endif

/**
 * UNINIT means zeroed memory buffer allocated for the struct.
 * INIT   means that struct members may have values, and memory may
 *        have been allocated. Done function must check and free it.
 * RUN    Struct is valid for run-time checking.
 * DONE   means that possible memory allocations have been released.
 */
typedef enum { UNINIT = 0, INIT, RUN, DONE } flat_obj_state_t; 

EXTERN_C_BLOCK_BEGIN

/**
 * Initialization functions
 */
bool mxs_logmanager_init(int argc, char* argv[]);
void mxs_logmanager_done(void);
void mxs_logmanager_exit(void);
void mxs_log_done(void);

/**
 * Logging functions
 */
int  mxs_log(logfile_id_t id, const char* format, ...);
int  mxs_log_flush(logfile_id_t id, const char* format, ...);

/**
 * Log file management
 */
int  mxs_flush_file(logfile_id_t id);
void mxs_log_sync_all(void);
int  mxs_log_rotate(logfile_id_t id);
int  mxs_log_enable(logfile_id_t id);
int  mxs_log_disable(logfile_id_t id);
void mxs_set_highp(int);
void mxs_enable_syslog(int);
void mxs_enable_maxscalelog(int);
bool mxs_log_enabed(int id);

EXTERN_C_BLOCK_END

const char* get_trace_prefix_default(void);
const char* get_trace_suffix_default(void);
const char* get_msg_prefix_default(void);
const char* get_msg_suffix_default(void);
const char* get_err_prefix_default(void);
const char* get_err_suffix_default(void);
const char* get_logpath_default(void);

#endif /** LOG_MANAGER_H */
