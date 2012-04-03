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
#include <stdlib.h>

#include "log.h"

int 	conf_debug;
char	conf_tunnel[25] = "ethernet";
int	conf_tunneldevice;

struct kwvar {
	char *	kw;
	void *	var;
	enum vartype { Vint, Vstring, Vbool, Vchar, Vdouble} type;
};

static const struct kwvar keywords[] = {
	{"Debug", &conf_debug, Vbool},
	{"Tunnel", &conf_tunnel, Vstring},
	{"TunnelDevice", &conf_tunneldevice, Vint},
	{NULL, NULL, Vint}
};

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
		log_info("%s: %s -> %s", kvp->kw, kvp->var, buf);
		(void)strlcpy(kvp->var, buf, sizeof kvp->var);
		return p;
	} else {
		log_errx(1, "invalid string value");
		return NULL;
	}
	return NULL;
}

static char *
parse_bool(char *p, struct kwvar *kvp) {
	char *valuestart;
	char buf[25];
	int newval;

	valuestart = p;
	while (isalpha(*p))
		p++;
	if ((*p == '\0' || isspace(*p) || *p == '#') && valuestart != p) {
		(void)memset(buf, '\0', sizeof buf);
		(void)strncpy(buf, valuestart, (size_t)(p - valuestart));
		if (strcasecmp(buf, "yes") == 0) {
			log_info("%s: %d -> %d",
			    kvp->kw, *(int *)kvp->var, 1);
			*(int *)kvp->var = 1;
		} else {
			log_info("%s: %d -> %d",
			    kvp->kw, *(int *)kvp->var, 0);
			*(int *)kvp->var = 0;
		}
		return p;
	} else {
		log_errx(1, "invalid boolean value");
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
		log_info("%s: %d -> %d", kvp->kw, *(int *)kvp->var, newval);
		*(int *)kvp->var = newval;
		return p;
	} else {
		log_errx(1, "invalid integer value");
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
		log_errx(1, "type not implemented");
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
		log_errx(1 ,"expected variable name");
		return NULL;
	}

	/* match the configuration variable name */
	(void)strncpy(varname, word, (size_t)(endword - word));

	for (kvp = (struct kwvar *)keywords; kvp->kw; kvp++) 
		if (strcmp(kvp->kw, varname) == 0)
			break;

	if (kvp->kw == NULL) {
		log_errx(1, "unrecognised variable");
		return NULL;
	}

	/* skip whitespace */
	while (isblank(*p))
		p++;

	if (*p++ != '=') {
		log_errx(1, "expected `='");
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

