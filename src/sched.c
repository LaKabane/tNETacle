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

struct rw_events
{
    struct event *r_event;
    struct event *w_event;
};

#define MAP_KEY_TYPE evutil_socket_t
#define MAP_VALUE_TYPE struct rw_events
#define MAP_PREFIX fd_ev
#include "map.h"

void sched_dispatch(evutil_socket_t fd, short event, void *ctx);

struct fiber_args
{
    struct fiber *fib;
    intptr_t userptr;
};

#define VECTOR_TYPE struct fiber*
#define VECTOR_PREFIX fiber
#define VECTOR_TYPE_SCALAR
#include "vector.h"

struct rw_events *get_events(struct fiber_args *s, evutil_socket_t fd, short event)
{
    struct rw_events *t;

    t = m_fd_ev_find(s->fib->map_fe, fd);
    if (t == NULL)
    {
        struct rw_events ev;

        ev.w_event = NULL;
        ev.r_event = NULL;
        if (event == EV_READ)
        {
            ev.r_event = event_new(s->fib->sched_back_ref->evbase,
                                   fd,
                                   EV_READ,
                                   sched_dispatch,
                                   s->fib);
        }
        else if (event == EV_WRITE)
        {
            ev.w_event = event_new(s->fib->sched_back_ref->evbase,
                                   fd,
                                   EV_WRITE,
                                   sched_dispatch,
                                   s->fib);
        }
        return m_fd_ev_insert(s->fib->map_fe, fd, ev);
    }
    else
    {
        if ((event == EV_READ && t->r_event != NULL)
            || (event == EV_WRITE && t->w_event != NULL))
            return t;
        else
        {
            if (event == EV_READ)
            {
                t->r_event = event_new(s->fib->sched_back_ref->evbase,
                                       fd,
                                       EV_READ,
                                       sched_dispatch,
                                       s->fib);
            }
            else if (event == EV_WRITE)
            {
                t->w_event = event_new(s->fib->sched_back_ref->evbase,
                                       fd,
                                       EV_WRITE,
                                       sched_dispatch,
                                       s->fib);
            }
        }
        return t;
    }
    return NULL;
}

void sched_fiber_set_dtor(struct fiber *f,
                          void (*dtor)(struct fiber *, intptr_t),
                          intptr_t ctx)
{
    f->dtor = dtor;
    f->dtor_ctx = ctx;
}

void sched_fiber_exit(struct fiber_args *s,
                      int val)
{
    coro_context *src;
    coro_context *dst;

    src = &s->fib->fib_ctx;
    dst = s->fib->sched_back_ref->origin_ctx;

    m_fd_ev_delete(s->fib->map_fe);
    if (s->fib->yield_event != NULL)
        event_free(s->fib->yield_event);

    s->fib->fib_op.op_type = FREE;
    s->fib->fib_op.arg1 = val;

    if (s->fib->dtor != NULL)
        s->fib->dtor(s->fib, s->fib->dtor_ctx);
    free(s);
    coro_transfer(src, dst);
}

int async_event(struct fiber_args *s,
                evutil_socket_t fd,
                short flag)
{
    struct coro_context *origin = s->fib->sched_back_ref->origin_ctx;
    struct rw_events *it;

    if (flag & EV_READ)
    {
        it = get_events(s, fd, EV_READ);
        /* XXX SIGSEGV it == NULL */
        event_add(it->r_event, NULL);
    }
    if (flag & EV_WRITE)
    {
        it = get_events(s, fd, EV_WRITE);
        /* XXX SIGSEGV it == NULL */
        event_add(it->w_event, NULL);
    }
    s->fib->fib_op.op_type = EVENT;
    s->fib->fib_op.ret = 0;
    coro_transfer(&s->fib->fib_ctx, origin);
    return (int)s->fib->fib_op.ret;
}

ssize_t async_recvfrom(struct fiber_args *s,
                   evutil_socket_t fd,
                   char *buf,
                   int len,
                   int flag,
                   struct sockaddr *sock,
                   socklen_t *socklen)
{
    struct coro_context *origin = s->fib->sched_back_ref->origin_ctx;
    struct rw_events *it;

    it = get_events(s, fd, EV_READ);
    /* XXX SIGSEGV (it == NULL) */
    event_add(it->r_event, NULL);
    s->fib->fib_op.op_type = RECVFROM;
    s->fib->fib_op.fd = fd;
    s->fib->fib_op.arg1 = (intptr_t)fd;
    s->fib->fib_op.arg2 = (intptr_t)buf;
    s->fib->fib_op.arg3 = (intptr_t)len;
    s->fib->fib_op.arg4 = (intptr_t)flag;
    s->fib->fib_op.arg5 = (intptr_t)sock;
    s->fib->fib_op.arg6 = (intptr_t)socklen;
    coro_transfer(&s->fib->fib_ctx, origin);
    return (ssize_t)s->fib->fib_op.ret;
}


evutil_socket_t async_accept(struct fiber_args *s,
                 evutil_socket_t fd,
                 struct sockaddr *sock,
                 socklen_t *socklen)
{
    struct coro_context *origin = s->fib->sched_back_ref->origin_ctx;
    struct rw_events *it;

    it = get_events(s, fd, EV_READ);
    /* XXX SIGSEGV (it == NULL) */
    event_add(it->r_event, NULL);
    s->fib->fib_op.op_type = ACCEPT;
    s->fib->fib_op.fd = (intptr_t)fd;
    s->fib->fib_op.arg1 = (intptr_t)fd;
    s->fib->fib_op.arg2 = (intptr_t)sock;
    s->fib->fib_op.arg3 = (intptr_t)socklen;
    coro_transfer(&s->fib->fib_ctx, origin);
    evutil_make_socket_nonblocking(s->fib->fib_op.ret);
    return (evutil_socket_t)s->fib->fib_op.ret;
}

void    async_sleep(struct fiber_args *s,
                    int time)
{
    struct timeval tv;
    struct coro_context *origin = s->fib->sched_back_ref->origin_ctx;

    tv.tv_sec = time;
    tv.tv_usec = 0;
    event_add(s->fib->yield_event, &tv);
    coro_transfer(&s->fib->fib_ctx, origin);
}

ssize_t async_recv(struct fiber_args *s,
                   evutil_socket_t fd,
                   void *buf,
                   size_t len,
                   int flags)
{
    struct coro_context *origin = s->fib->sched_back_ref->origin_ctx;
    struct rw_events *it;

    it = get_events(s, fd, EV_READ);
    /* XXX SIGSEGV (it == NULL) */
    event_add(it->r_event, NULL);
    s->fib->fib_op.op_type = RECV;
    s->fib->fib_op.fd = (intptr_t)fd;
    s->fib->fib_op.arg1 = (intptr_t)fd;
    s->fib->fib_op.arg2 = (intptr_t)buf;
    s->fib->fib_op.arg3 = (intptr_t)len;
    s->fib->fib_op.arg4 = (intptr_t)flags;
    coro_transfer(&s->fib->fib_ctx, origin);
    return (ssize_t)s->fib->fib_op.ret;
}

ssize_t async_send(struct fiber_args *s,
                   evutil_socket_t fd,
                   void const *buf,
                   size_t len,
                   int flags)
{
    struct coro_context *origin = s->fib->sched_back_ref->origin_ctx;
    struct rw_events *it;

    it = get_events(s, fd, EV_WRITE);
    /* XXX SIGSEGV (it == NULL) */
    event_add(it->w_event, NULL);
    s->fib->fib_op.op_type = SEND;
    s->fib->fib_op.fd = (intptr_t)fd;
    s->fib->fib_op.arg1 = (intptr_t)fd;
    s->fib->fib_op.arg2 = (intptr_t)buf;
    s->fib->fib_op.arg3 = (intptr_t)len;
    s->fib->fib_op.arg4 = (intptr_t)flags;
    coro_transfer(&s->fib->fib_ctx, origin);
    return (ssize_t)s->fib->fib_op.ret;
}

ssize_t async_read(struct fiber_args *s,
                   evutil_socket_t fd,
                   void *buf,
                   size_t len)
{
    struct coro_context *origin = s->fib->sched_back_ref->origin_ctx;
    struct rw_events *it;

    it = get_events(s, fd, EV_READ);
    /* XXX SIGSEGV (it == NULL) */
    event_add(it->r_event, NULL);
    s->fib->fib_op.op_type = READ;
    s->fib->fib_op.fd = (intptr_t)fd;
    s->fib->fib_op.arg1 = (intptr_t)fd;
    s->fib->fib_op.arg2 = (intptr_t)buf;
    s->fib->fib_op.arg3 = (intptr_t)len;
    coro_transfer(&s->fib->fib_ctx, origin);
    return (ssize_t)s->fib->fib_op.ret;
}

ssize_t async_write(struct fiber_args *s,
                    evutil_socket_t fd,
                    void const *buf,
                    size_t len)
{
    struct coro_context *origin = s->fib->sched_back_ref->origin_ctx;
    struct rw_events *it;

    it = get_events(s, fd, EV_WRITE);
    /* XXX SIGSEGV (it == NULL) */
    event_add(it->w_event, NULL);
    s->fib->fib_op.op_type = WRITE;
    s->fib->fib_op.fd = (intptr_t)fd;
    s->fib->fib_op.arg1 = (intptr_t)fd;
    s->fib->fib_op.arg2 = (intptr_t)buf;
    s->fib->fib_op.arg3 = (intptr_t)len;
    coro_transfer(&s->fib->fib_ctx, origin);
    return (ssize_t)s->fib->fib_op.ret;
}

ssize_t async_sendto(struct fiber_args *s,
                     evutil_socket_t fd,
                     void const *buf,
                     size_t len,
                     int flags,
                     struct sockaddr const *dst,
                     socklen_t socklen)
{
    struct coro_context *origin = s->fib->sched_back_ref->origin_ctx;
    struct rw_events *it;

    it = get_events(s, fd, EV_WRITE);
    event_add(it->w_event, NULL);// XXX Check if (it && it->w_event)
    s->fib->fib_op.op_type = SENDTO;
    s->fib->fib_op.fd = (intptr_t)fd;
    s->fib->fib_op.arg1 = (intptr_t)fd;
    s->fib->fib_op.arg2 = (intptr_t)buf;
    s->fib->fib_op.arg3 = (intptr_t)len;
    s->fib->fib_op.arg4 = (intptr_t)flags;
    s->fib->fib_op.arg5 = (intptr_t)dst;
    s->fib->fib_op.arg6 = (intptr_t)socklen;
    coro_transfer(&s->fib->fib_ctx, origin);
    return (ssize_t)s->fib->fib_op.ret;
}

intptr_t async_yield(struct fiber_args *s,
                     intptr_t yielded)
{
    struct coro_context *origin = s->fib->sched_back_ref->origin_ctx;

    s->fib->fib_op.op_type = YIELD;
    s->fib->fib_op.arg1 = yielded;
    coro_transfer(&s->fib->fib_ctx, origin);
    return s->fib->fib_op.ret;
}

intptr_t async_continue(struct fiber_args *A,
                        struct fiber *F,
                        intptr_t data)
{
    struct coro_context *origin = A->fib->sched_back_ref->origin_ctx;

    A->fib->fib_op.op_type = YIELD;
    F->fib_op.fd = (intptr_t)A->fib;
    F->fib_op.ret = data;
    event_active(F->yield_event,
                 EV_TIMEOUT, 0);
    coro_transfer(&A->fib->fib_ctx, origin);
    return F->fib_op.arg1;
}

void async_wake(struct fiber *F,
                intptr_t data)
{
    (void)data;
    if (event_pending(F->yield_event, EV_TIMEOUT, NULL) == 0)
    {
        event_active(F->yield_event,
                     EV_TIMEOUT,
                     /* Unused */0);
    }
}


intptr_t sched_get_userptr(struct fiber_args *args)
{
    return args->userptr;
}

struct fiber *
sched_get_fiber(struct fiber_args *args)
{
    return args->fib;
}

struct sched *sched_new(struct event_base *evbase)
{
    struct sched *sched;

    sched = malloc(sizeof(*sched));
    memset(sched, 0, sizeof(*sched));
    sched->evbase = evbase;
    sched->origin_ctx = NULL;
    return sched;
}

void sched_dispatch(evutil_socket_t fd, short event, void *ctx)
{
    struct fiber *fib = (struct fiber *)ctx;
    struct sched *S = fib->sched_back_ref;
    struct coro_context cctx;

    (void)fd;
    coro_create(&cctx, NULL, NULL, NULL, 0);
    if (event & EV_READ)
    {
        switch (fib->fib_op.op_type)
        {
            case ACCEPT:
                {
                    fib->fib_op.ret = accept(fib->fib_op.arg1,
                                             (struct sockaddr *)fib->fib_op.arg2,
                                             (socklen_t *)fib->fib_op.arg3);
                    S->origin_ctx = &cctx;
                    coro_transfer(&cctx, &fib->fib_ctx);
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
                    S->origin_ctx = &cctx;
                    coro_transfer(&cctx, &fib->fib_ctx);
                    break;
                }
            case RECV:
                {
                    fib->fib_op.ret = recv(fib->fib_op.arg1,
                                           (void *)fib->fib_op.arg2,
                                           (size_t)fib->fib_op.arg3,
                                           (int)fib->fib_op.arg4);
                    S->origin_ctx = &cctx;
                    coro_transfer(&cctx, &fib->fib_ctx);
                    break;
                }
            case READ:
                {
                    fib->fib_op.ret = read(fib->fib_op.arg1,
                                           (void *)fib->fib_op.arg2,
                                           (size_t)fib->fib_op.arg3);
                    S->origin_ctx = &cctx;
                    coro_transfer(&cctx, &fib->fib_ctx);
                    break;
                }
            case EVENT:
                {
                    fib->fib_op.ret |= EV_READ;
                    S->origin_ctx = &cctx;
                    coro_transfer(&cctx, &fib->fib_ctx);
                    break;
                }
            default:
                log_warnx("[sched] syscall not implemented");
        }
    }
    else if (event & EV_WRITE)
    {
        switch (fib->fib_op.op_type)
        {
            case SEND:
                {
                    fib->fib_op.ret = send(fib->fib_op.arg1,
                                           (void *)fib->fib_op.arg2,
                                           (size_t)fib->fib_op.arg3,
                                           (int)fib->fib_op.arg4);
                    S->origin_ctx = &cctx;
                    coro_transfer(&cctx, &fib->fib_ctx);
                    break;
                }
            case SENDTO:
                {
                    fib->fib_op.ret = sendto(fib->fib_op.arg1,
                                             (void *)fib->fib_op.arg2,
                                             (size_t)fib->fib_op.arg3,
                                             (int)fib->fib_op.arg4,
                                             (struct sockaddr *)fib->fib_op.arg5,
                                             (int)fib->fib_op.arg6);
                    S->origin_ctx = &cctx;
                    coro_transfer(&cctx, &fib->fib_ctx);
                    break;
                }
            case WRITE:
                {
                    fib->fib_op.ret = write(fib->fib_op.arg1,
                                            (void const *)fib->fib_op.arg2,
                                            (size_t)fib->fib_op.arg3);
                    S->origin_ctx = &cctx;
                    coro_transfer(&cctx, &fib->fib_ctx);
                    break;
                }
            case EVENT:
                {
                    fib->fib_op.ret |= EV_WRITE;
                    S->origin_ctx = &cctx;
                    coro_transfer(&cctx, &fib->fib_ctx);
                    break;
                }
            default:
                break;
        }
    }
    else if (event & EV_TIMEOUT)
    {
        switch (fib->fib_op.op_type)
        {
            case YIELD:
                {
                    S->origin_ctx = &cctx;
                    coro_transfer(&cctx, &fib->fib_ctx);
                    break;
                }
            case EVENT:
                {
                    fib->fib_op.ret |= EV_TIMEOUT;
                    S->origin_ctx = &cctx;
                    coro_transfer(&cctx, &fib->fib_ctx);
                }
            default:
                break;
        }
    }
    if (fib->fib_op.op_type == FREE)
    {
        free(fib->fib_stack);
    }
}

void sched_fiber_launch(struct fiber *F)
{
    struct sched *S = F->sched_back_ref;
    struct coro_context ctx;
    struct coro_context *save;

    save = S->origin_ctx;
    S->origin_ctx = &ctx;

    coro_create(&ctx, NULL, NULL, NULL, 0);
    coro_transfer(&ctx, &F->fib_ctx);

    S->origin_ctx = save;

    if (F->fib_op.op_type == FREE)
    {
        free(F->fib_stack);
        free(F);
    }
}

struct fiber *sched_new_fiber(struct sched *S,
                              coro_func func,
                              intptr_t userptr)
{
    struct fiber_args *args;
    struct fiber *new_fiber;
    void *stack_space;
    size_t stack_size = 1 << 15;
        
    args = malloc(sizeof(struct fiber_args));
    new_fiber = malloc(sizeof(struct fiber));
    stack_space = malloc(stack_size);

    memset(new_fiber, 0, sizeof(struct fiber));
    memset(args, 0, sizeof(struct fiber_args));
    if (stack_space != NULL
        && new_fiber != NULL
        && args != NULL)
    {
        args->fib = new_fiber;
        args->userptr = userptr;
        args->fib->map_fe = m_fd_ev_new();

        new_fiber->fib_stack_size = stack_size;
        new_fiber->fib_stack = stack_space;
        new_fiber->fib_op.op_type = NONE;
        new_fiber->sched_back_ref = S;
        new_fiber->dtor = NULL;
        new_fiber->dtor_ctx = 0;
        new_fiber->yield_event = event_new(S->evbase,
                                           -1,
                                           0,
                                           sched_dispatch,
                                           new_fiber); 
        event_add(new_fiber->yield_event, NULL);
        coro_create(&new_fiber->fib_ctx, func, args, stack_space, stack_size);
        return new_fiber;
    }
    free(stack_space);
    free(new_fiber);
    free(args);
    return NULL;
}

void
sched_delete(struct sched *S)
{
    /* Nothing to do here yet */
    /* Remember __not__ to free S->evbase !! */
    (void)S;
}

void
sched_fiber_delete(struct fiber *F)
{
    m_fd_ev_delete(F->map_fe);
    free(F->fib_stack);
}
