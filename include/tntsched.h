/**
 * Copyright (c) 2012, PICHOT Fabien Paul Leonard <pichot.fabien@gmail.com>
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
**/

#ifndef SCHED_O6L1KITS
#define SCHED_O6L1KITS

#include <event2/util.h>

struct fiber_args;

struct operation
{
    enum {
        NONE = 0,
        READ,
        WRITE,
        ACCEPT,
        RECVFROM
    } op_type;
    intptr_t fd;   /* Used to store the fd if needed */
    intptr_t ret;  
    intptr_t arg1; /* Used to store the first arg, if the 1st is a fd, then we store it twice*/
    intptr_t arg2;
    intptr_t arg3;
    intptr_t arg4;
    intptr_t arg5;
    intptr_t arg6;
};

struct fiber
{
    struct coro_context fib_ctx;
    void *fib_stack;
    size_t fib_stack_size;
    struct operation fib_op;
    struct sched *sched_back_ref;
};

struct sched
{
    struct vector_fiber *fibers;
    struct coro_context base_ctx;
    struct event_base *evbase;
};

struct sched *sched_new(struct event_base *evbase);

void sched_launch(struct sched *S);

struct fiber *sched_new_fiber(struct sched *S,
                              coro_func func,
                              void *userptr);

void sched_dispatch(evutil_socket_t fd,
                    short event,
                    void *ctx);

void *sched_get_userptr(struct fiber_args *args);

int async_recvfrom(struct fiber_args *s,
                   int fd,
                   char *buf,
                   int len,
                   int flag,
                   struct sockaddr *sock,
                   int *socklen);

int async_accept(struct fiber_args *s,
                 int fd,
                 struct sockaddr *sock,
                 int *socklen);


#endif /* end of include guard: SCHED_O6L1KITS */
