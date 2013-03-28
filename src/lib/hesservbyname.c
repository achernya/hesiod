/*
 * Copyright (c) 2013, Alex Chernyakhovsky <achernya@mit.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>

#include "hesiod.h"

// strcasecmp is sufficiently strange that it's better to define our
// own case-insensitive string comparison function
static int
hesiod_strcasecmp(const char* first, const char* second);

struct servent *
hesiod_getservbyname(void *context, const char *name,
		     const char *proto) {
    char **list, **iterator;
    char service[256], protocol[256];
    int port, retval;
    struct servent *result = NULL;
    list = hesiod_resolve(context, name, "service");

    if (list == NULL) {
	// No results
	return NULL;
    }

    for (iterator = list; (*iterator != NULL); iterator++) {
	retval = sscanf(*iterator, "%256s %256s %d", service, protocol, &port);
	if (retval != 3) {
	    // Malformed record, skip it for now
	    continue;
	}
	if (hesiod_strcasecmp(protocol, proto) != 0) {
	    // Protocol doesn't match
	    continue;
	}
	// Found the entry! Do memory allocations first.
	result = (struct servent *) malloc(sizeof(struct servent));
	if (result != NULL) {
	    result->s_name =
		(char *) malloc(sizeof(char) * (strlen(service) + 1));
	    result->s_proto =
		(char *) malloc(sizeof(char) * (strlen(protocol) + 1));
	    result->s_aliases = (char **) malloc(sizeof(char*) * 1);
	}
	// Make sure we got valid buffers from malloc
	if ((result == NULL) || (result->s_name == NULL) ||
	    (result->s_proto == NULL) || (result->s_aliases == NULL)) {
	    // Set errno to note we ran out of memory, as subsequent
	    // malloc() calls may have clobbered it. Go ahead an try
	    // to delete the structure we allocated.
	    hesiod_free_servent(context, result);
	    errno = ENOMEM;
	    return NULL;
	}

	// Perform the assignments
	strcpy(result->s_name, service);
	strcpy(result->s_proto, protocol);
	result->s_port = htons(port);
	*result->s_aliases = NULL;

	break;
    }

    hesiod_free_list(context, list);
    return result;
}

void
hesiod_free_servent(void *context, struct servent *serv) {
    if (serv == NULL) {
	return;
    }
    if (serv->s_name != NULL) {
	free(serv->s_name);
    }
    if (serv->s_proto != NULL) {
	free(serv->s_proto);
    }
    if (serv->s_aliases != NULL) {
	free(serv->s_aliases);
    }
    free(serv);
}

static int
hesiod_strcasecmp(const char* first, const char* second) {
    while ((*first) && (*second) &&
	   (tolower(*first) == tolower(*second))) {
	first++;
	second++;
    }
    return tolower(*first) - tolower(*second);
}
