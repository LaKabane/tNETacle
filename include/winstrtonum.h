#ifndef _WINSTRTONUM_
# define _WINSTRTONUM_

long long
strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp);

#endif