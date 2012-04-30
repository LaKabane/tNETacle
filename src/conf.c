/*
 * Copyright (c) 2011 Tristan Le Guern <leguern AT medu DOT se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Original copyright notice :
 * David Leonard <d@openbsd.org>, 1999. Public domain.
 */

#include <sys/types.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined Windows
#define strcasecmp _stricmp
#endif

#include "log.h"

extern int 	conf_debug;
char		conf_tunnel[25] = "ethernet";
int		conf_tunneldevice = -1;
char		conf_address[] = "10.0.0.22/255.255.255.0";
char		conf_peer_address[] = "";

struct kwvar {
	char *	kw;
	void *	var;
	enum vartype { Vint, Vstring, Vbool, Vchar, Vdouble} type;
};

static const struct kwvar keywords[] = {
	{"Debug", &conf_debug, Vbool},
	{"Tunnel", &conf_tunnel, Vstring},
	{"TunnelDevice", &conf_tunneldevice, Vint},
	{"Address", &conf_address, Vstring},
	{NULL, NULL, Vint}
};

#if defined Windows
static int
isblank(char c)
{
	return c == ' ' || c == '\n' || c == '\t';
}

/*
** Copied from somewhere in the internet. Because I'm lazy.
** Fabien
*/

static size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';              /* NUL-terminate dst */
		while (*s++) ;
	}
	return(s - src - 1);    /* count does not include NUL */
}

#endif

static char *
parse_string(char *p, struct kwvar *kvp) {
	char *valuestart;
	char buf[25];

	valuestart = p;
	while (isalpha(*p) || *p == '-' || *p == '_')
		p++;
	if ((*p == '\0' || isspace(*p) || *p == '#') && valuestart != p) {
		(void)memset(buf, '\0', sizeof buf);
		(void)strncpy(buf, valuestart, (size_t)(p - valuestart));
		(void)fprintf(stdin, "%s: %s -> %s\n", kvp->kw,
		    (char *)kvp->var, buf);
		(void)strlcpy(kvp->var, buf, sizeof kvp->var);
		return p;
	} else {
		(void)fprintf(stderr, "invalid string value\n");
		return NULL;
	}
	return NULL;
}

static char *
parse_bool(char *p, struct kwvar *kvp) {
	char *valuestart;
	char buf[25];

	valuestart = p;
	while (isalpha(*p))
		p++;
	if ((*p == '\0' || isspace(*p) || *p == '#') && valuestart != p) {
		(void)memset(buf, '\0', sizeof buf);
		(void)strncpy(buf, valuestart, (size_t)(p - valuestart));
		if (strcasecmp(buf, "yes") == 0) {
			(void)fprintf(stdin, "%s: %d -> %d\n",
			    kvp->kw, *(int *)kvp->var, 1);
			*(int *)kvp->var = 1;
		} else {
			(void)fprintf(stdin, "%s: %d -> %d\n",
			    kvp->kw, *(int *)kvp->var, 0);
			*(int *)kvp->var = 0;
		}
		return p;
	} else {
		(void)fprintf(stderr, "invalid boolean value\n");
		return NULL;
	}
	return NULL;
}

static char *
parse_int(char *p, struct kwvar *kvp) {
	char *valuestart, *digitstart;
	char buf[25];
	int newval;

	/* expect a number */
	valuestart = p;
	if (*p == '-') 
		p++;
	digitstart = p;
	while (isdigit(*p))
		p++;
	if ((*p == '\0' || isspace(*p) || *p == '#') && digitstart != p) {
		(void)strncpy(buf, valuestart, (size_t)(p - valuestart));
		newval = atoi(valuestart);
		(void)fprintf(stdin, "%s: %d -> %d\n", kvp->kw,
		    *(int *)kvp->var, newval);
		*(int *)kvp->var = newval;
		return p;
	} else {
		(void)fprintf(stderr, "invalid integer value\n");
		return NULL;
	}
	return NULL;
}

static char *
parse_value(char *p, struct kwvar *kvp) {
	switch (kvp->type) {
	case Vint:
		return parse_int(p, kvp);
	case Vbool:
		return parse_bool(p, kvp);
	case Vstring:
		return parse_string(p, kvp);
	case Vchar:
	case Vdouble:
	default:
		(void)fprintf(stderr, "type not implemented\n");
	}
	return NULL;
}

/* Parse one line of configuration (a key/value pair) */
char *
tnt_parse_line(char *p) {
	char *word;
	char *endword;
	char varname[25];
	struct kwvar *kvp;

	/* skip leading white */
	while (isblank(*p))
		p++;
	/* allow blank lines and comment lines */
	if (*p == '\0')
		return NULL;
	if (*p == '\n')
		return ++p;
	if (*p == '#') {
		while (*p != '\n' && *p != '\0')
			++p;
		++p;
		return p;
	}

	/* walk to the end of the word: */
	word = p;
	if (isalpha(*p) || *p == '_') {
		p++;
		while (isalpha(*p) || isdigit(*p) || *p == '_')
			p++;
	}
	endword = p;

	if (endword == word) {
		(void)fprintf(stderr ,"expected variable name\n");
		return NULL;
	}

	/* match the configuration variable name */
	(void)strncpy(varname, word, (size_t)(endword - word));

	for (kvp = (struct kwvar *)keywords; kvp->kw; kvp++) 
		if (strcmp(kvp->kw, varname) == 0)
			break;

	if (kvp->kw == NULL) {
		(void)fprintf(stderr, "unrecognised variable\n");
		return NULL;
	}

	/* skip whitespace */
	while (isblank(*p))
		p++;

	if (*p++ != '=') {
		(void)fprintf(stderr, "expected `='\n");
		return NULL;
	}

	/* skip whitespace */
	while (isblank(*p))
		p++;

	/* parse the value */
	p = parse_value(p, kvp);
	if (!p) 
		return NULL;

	/* skip trailing whitespace */
	while (isblank(*p))
		p++;

	/* skip trailing comment */
	if (*p != '\0' && *p == '#') {
		while (*p != '\0' && *p != '\n')
			p++;
		if (*p != '\0')
			p++;
	}
	return p;
}

