#include <event2/event.h>
#include <string.h>

#include "utility.h"

int
evutil_have_backend(char const *back)
{
    int i;
    int backend_found = 0;
    const char **methods = event_get_supported_methods();

    for (i = 0; methods[i] != NULL; ++i)
    {
        if (strcmp(methods[i], back) == 0)
        {
            backend_found = 1;
            continue;
        }
    }
    return backend_found;
}

int
evutil_select_backend(struct event_config *evcfg, char const *back)
{
    int i;
    int backend_found = 0;

    if (evutil_have_backend(back))
    {
        const char **methods = event_get_supported_methods();

        for (i = 0; methods[i] != NULL; ++i)
        {
            if (strcmp(methods[i], back) == 0)
            {
                backend_found = 1;
                continue;
            }
            event_config_avoid_method(evcfg, methods[i]);
        }
    }
    return backend_found;
}
