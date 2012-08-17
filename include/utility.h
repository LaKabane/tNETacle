#pragma once
#ifndef UTILITY_UVL99YH6
#define UTILITY_UVL99YH6

struct event_config;

int evutil_have_backend(char const *back);

int evutil_select_backend(struct event_config *evcfg,
                          char const *backend);

#endif /* end of include guard: UTILITY_UVL99YH6 */
