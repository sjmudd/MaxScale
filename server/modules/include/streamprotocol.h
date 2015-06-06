#ifndef _MYSQL_PROTOCOL_H
#define _MYSQL_PROTOCOL_H
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
 * Copyright MariaDB Corporation Ab 2013-2015
 */

/*
 * Revision History
 *
 * Date         Who                     Description
 * 24-03-2015   Markus Makela           Initial implementation

 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <service.h>
#include <router.h>
#include <poll.h>
#include <version.h>
#include <config.h>
#include <gw.h>

#define SPLICE_MAX_BYTES 65535 /*< This is one byte less than the maximum pipe size in Linux versions >= 2.6.11 */
#define PIPE_READ 0
#define PIPE_WRITE 1
#define PIPEPOOL_DEFAULT_SIZE 32

struct dcb;
struct pipepool_t;

typedef struct pipe_t{
  int pipe[2]; /*< The pipe  */
  unsigned long owner; /*< The current thread using this pipe */
  bool in_use;
  struct pipepool_t* pool;
}PIPE;

typedef struct pipepool_t{
  int n_pipes;
  PIPE* pool;
  SPINLOCK lock;
}PIPEPOOL;

/**
 * MySQL Protocol specific state data.
 * 
 * Protocol carries information from client side to backend side, such as 
 * MySQL session command information and history of earlier session commands.
 */
typedef struct {
#if defined(SS_DEBUG)
        skygw_chk_t     protocol_chk_top;
#endif
        PIPEPOOL* pool;
        struct dcb          *owner_dcb;                   /*< The DCB of the socket
        * we are running on */
        SPINLOCK            protocol_lock;
        PIPE* pipe; /*< The active pipe, NULL if no pipe is active */
#if defined(SS_DEBUG)
        skygw_chk_t     protocol_chk_tail;
#endif
} StreamProtocol;

#endif /** _MYSQL_PROTOCOL_H */

static PIPEPOOL* pipepool = NULL;
StreamProtocol* streamprotocol_init(DCB *dcb);
int  stream_do_connect_to_backend(char *host, int port, int* fd);
PIPEPOOL* stream_init_pool();
PIPE* stream_get_pipe(PIPEPOOL* pool);
void stream_return_pipe(PIPE* pipe);