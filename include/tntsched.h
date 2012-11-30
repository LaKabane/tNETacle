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

#include "wincompat.h"
#include "coro.h"
#include <event2/util.h>

struct fiber_args;
struct map_fd_evl;

enum sched_operation
{
    NONE = 0,
    READ,
    WRITE,
    ACCEPT,
    RECVFROM,
    SENDTO,
    SEND,
    RECV,
    YIELD,
    FREE,
    EVENT,
};

struct operation
{
    enum sched_operation op_type;
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
    struct event        *yield_event;
    struct map_fd_ev    *map_fe;
    void                (*dtor)(struct fiber *, intptr_t);
    intptr_t            dtor_ctx;
};

struct sched
{
    struct event_base   *evbase;
    struct coro_context *origin_ctx;
};

struct sched *sched_new(struct event_base *evbase);

void sched_delete(struct sched *);

void sched_fiber_launch(struct fiber *F);

struct fiber *sched_new_fiber(struct sched *S,
                              coro_func func,
                              intptr_t userptr);

void sched_fiber_delete(struct fiber *);

void sched_fiber_set_dtor(struct fiber *,
                          void (*dtor)(struct fiber *, intptr_t),
                          intptr_t ctx);

void sched_fiber_exit(struct fiber_args *args, int val);

intptr_t sched_get_userptr(struct fiber_args *args);

struct fiber * sched_get_fiber(struct fiber_args *args);

void async_sleep(struct fiber_args *args,
                 int sec);

int async_event(struct fiber_args *s,
                evutil_socket_t fd,
                short flag);

intptr_t async_yield(struct fiber_args *S,
                     intptr_t yielded);

intptr_t async_continue(struct fiber_args *A,
                          struct fiber *F,
                          intptr_t data);

void async_wake(struct fiber *F,
                intptr_t data);

ssize_t async_sendto(struct fiber_args *s,
                   evutil_socket_t fd,
                   void const *buf,
                   size_t len,
                   int flag,
                   struct sockaddr const *sock,
                   int socklen);

ssize_t async_send(struct fiber_args *s,
                   evutil_socket_t fd,
                   void const *buf,
                   size_t len,
                   int flag);

ssize_t async_read(struct fiber_args *s,
                   evutil_socket_t fd,
                   void *buf,
                   size_t len);

ssize_t async_recv(struct fiber_args *s,
                   evutil_socket_t fd,
                   void *buf,
                   size_t len,
                   int flag);

ssize_t async_write(struct fiber_args *s,
                    evutil_socket_t fd,
                    void const *buf,
                    size_t len);

ssize_t async_recvfrom(struct fiber_args *s,
                   evutil_socket_t fd,
                   char *buf,
                   int len,
                   int flag,
                   struct sockaddr *sock,
                   int *socklen);

evutil_socket_t async_accept(struct fiber_args *s,
                 evutil_socket_t fd,
                 struct sockaddr *sock,
                 int *socklen);


#endif /* end of include guard: SCHED_O6L1KITS */
