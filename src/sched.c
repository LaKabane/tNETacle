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

#include <errno.h>
#include <assert.h>
#include <event2/event.h>
#include "networking.h"
#include "coro.h"
#include "log.h"
#include "tntsched.h"

struct fiber_args
{
    struct fiber *fib;
    struct coro_context *base_ctx;
    void *userptr;
};

#define VECTOR_TYPE struct fiber*
#define VECTOR_PREFIX fiber
#define VECTOR_TYPE_SCALAR
#include "vector.h"

int async_recvfrom(struct fiber_args *s,
                   int fd,
                   char *buf,
                   int len,
                   int flag,
                   struct sockaddr *sock,
                   int *socklen)
{
    int res;

    res = recvfrom(fd, buf, len, flag, sock, socklen);
    if (res == -1)
    {
        if (EVUTIL_SOCKET_ERROR() == EAGAIN)
        /* There is no pending data, transfert the exe stream*/
        {
            s->fib->fib_op.op_type = RECVFROM;
            s->fib->fib_op.fd = fd;
            s->fib->fib_op.arg1 = (intptr_t)fd;
            s->fib->fib_op.arg2 = (intptr_t)buf;
            s->fib->fib_op.arg3 = (intptr_t)len;
            s->fib->fib_op.arg4 = (intptr_t)flag;
            s->fib->fib_op.arg5 = (intptr_t)sock;
            s->fib->fib_op.arg6 = (intptr_t)socklen;
            coro_transfer(&s->fib->fib_ctx, s->base_ctx);
            return s->fib->fib_op.ret;
        }
    }
    return res;
}

int async_accept(struct fiber_args *s,
                 int fd,
                 struct sockaddr *sock,
                 int *socklen)
{
    int res_fd;

    res_fd = accept(fd, sock, socklen);
    if (res_fd == -1)
    {
        if (EVUTIL_SOCKET_ERROR() == EAGAIN)
        /* There is no one to wait for */
        {
            s->fib->fib_op.op_type = ACCEPT;
            s->fib->fib_op.fd = (intptr_t)fd;
            s->fib->fib_op.arg1 = (intptr_t)fd;
            s->fib->fib_op.arg2 = (intptr_t)sock;
            s->fib->fib_op.arg3 = (intptr_t)socklen;
            coro_transfer(&s->fib->fib_ctx, s->base_ctx);
            return s->fib->fib_op.ret;
        }
    }
    return res_fd;
}

void *sched_get_userptr(struct fiber_args *args)
{
    return args->userptr;
}

struct sched *sched_new(struct event_base *evbase)
{
    struct sched *sched;

    sched = malloc(sizeof(*sched));
    memset(sched, 0, sizeof(*sched));
    sched->fibers = v_fiber_new();
    sched->evbase = evbase;
    return sched;
}

void sched_dispatch(evutil_socket_t fd, short event, void *ctx)
{
    struct fiber *fib = (struct fiber *)ctx;
    struct sched *S = fib->sched_back_ref;

    if (event & EV_READ)
    {
        assert(fd == fib->fib_op.fd);
        switch (fib->fib_op.op_type)
        {
            case ACCEPT:
                {
                    fib->fib_op.ret = accept(fib->fib_op.arg1,
                                             (struct sockaddr *)fib->fib_op.arg2,
                                             (socklen_t *)fib->fib_op.arg3);
                    break;
                }
            case RECVFROM:
                {
                    fib->fib_op.ret = recvfrom(fib->fib_op.arg1,
                                               (void *)fib->fib_op.arg2,
                                               (size_t)fib->fib_op.arg3,
                                               (int)fib->fib_op.arg4,
                                               (struct sockaddr *)fib->fib_op.arg5,
                                               (socklen_t *)fib->fib_op.arg6);
                    break;
                }
            case READ:
            case WRITE:
            case NONE:
            default:
                    log_warnx("[sched] syscall not implemented");
        }
    }
    coro_transfer(&S->base_ctx, &fib->fib_ctx);
}

void sched_launch(struct sched *S)
{
    unsigned int i;
    struct event_base *evbase = S->evbase;

    coro_create(&S->base_ctx, NULL, NULL, NULL, 0);
    for (i = 0;
         i < S->fibers->size;
         ++i)
    {
        struct fiber *F = v_fiber_at(S->fibers, i);

        coro_transfer(&S->base_ctx, &F->fib_ctx);
        switch (F->fib_op.op_type)
        {
            case RECVFROM:
            case ACCEPT:
            case READ:
                {
                    struct event *ev;

                    ev = event_new(evbase,
                                   F->fib_op.fd,
                                   EV_READ | EV_PERSIST,
                                   sched_dispatch,
                                   F);
                    event_add(ev, NULL);
                }
            default:
            case NONE:
                continue;
        }
    }
}

struct fiber *sched_new_fiber(struct sched *S, coro_func func, void *userptr)
{
    struct fiber_args *args;
    struct fiber *new_fiber;
    void *stack_space;
        
    args = malloc(sizeof(*args));
    new_fiber = malloc(sizeof(struct fiber));
    stack_space = malloc(1 << 16);

    if (stack_space != NULL
        && new_fiber != NULL
        && args != NULL)
    {
        args->userptr = userptr;
        args->fib = new_fiber;
        args->base_ctx = &S->base_ctx;
        new_fiber->fib_stack_size = 1 << 16;
        new_fiber->fib_stack = stack_space;
        new_fiber->fib_op.op_type = NONE;
        new_fiber->sched_back_ref = S;
        coro_create(&new_fiber->fib_ctx, func, args, stack_space, 1 << 16);
        v_fiber_push(S->fibers, new_fiber);
        return new_fiber;
    }
    free(stack_space);
    free(new_fiber);
    free(args);
    return NULL;
}
