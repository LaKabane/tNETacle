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

#include "coro.h"
#include <event2/util.h>

struct fiber_args;

struct operation
{
    enum {
        NONE = 0,
        READ,
        WRITE,
        ACCEPT,
        RECVFROM,
        SENDTO,
        SEND,
        RECV,
        YIELD,
        FREE
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
    void                *fib_stack;
    size_t              fib_stack_size;
    struct operation    fib_op;
    struct sched        *sched_back_ref;
    struct event        *userland_event;
};

struct sched
{
    struct event_base   *evbase;
    struct coro_context *origin_ctx;
};

struct sched *sched_new(struct event_base *evbase);

void sched_fiber_launch(struct fiber *F);

struct fiber *sched_new_fiber(struct sched *S,
                              coro_func func,
                              intptr_t userptr);

void sched_fiber_exit(struct fiber_args *args, int val);

intptr_t sched_get_userptr(struct fiber_args *args);

intptr_t async_yield(struct fiber_args *S,
                     intptr_t yielded);

intptr_t async_continue(struct fiber_args *A,
                          struct fiber *F,
                          intptr_t data);

ssize_t async_sendto(struct fiber_args *s,
                   int fd,
                   void const *buf,
                   size_t len,
                   int flag,
                   struct sockaddr const *sock,
                   int socklen);

ssize_t async_send(struct fiber_args *s,
                   int fd,
                   void const *buf,
                   size_t len,
                   int flag);

ssize_t async_recv(struct fiber_args *s,
                   int fd,
                   void *buf,
                   size_t len,
                   int flag);


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
