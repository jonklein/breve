/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

#include <curl/curl.h>
#include <string.h>

#include "slBrevePluginAPI.h"

struct slURLWriteData {
	char *string;
	size_t maxSize, size;
};

static size_t slURLWrite(void *, size_t, size_t, struct slURLWriteData *);
static int slUGetURL(brEval [], brEval *, void *);

DLLEXPORT void slInitURLFunctions(void *n) {
    brNewBreveCall(n, "getURL", slUGetURL, AT_STRING, AT_STRING, 0);
}

size_t slURLWrite(void *p, size_t s, size_t n, struct slURLWriteData *data) {
	const size_t len = s * n;
	const size_t newSize = data->size + len;

	if (data->maxSize <= newSize) {
		data->maxSize = newSize + 1;
		data->string = slRealloc(data->string, data->maxSize);
	}
	memcpy(&data->string[data->size], p, len);
	data->size = newSize;

	return len;
}

int slUGetURL(brEval args[], brEval *target, void *i) {
	struct slURLWriteData data;
	CURL *handle;

	data.maxSize = 4096;
	data.size = 0;
	data.string = slMalloc(data.maxSize);

	handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, BRSTRING(&args[0]));
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, slURLWrite);
	curl_easy_setopt(handle, CURLOPT_FILE, &data);
	curl_easy_perform(handle);
	curl_easy_cleanup(handle);

	data.string[data.size] = '\0';
	BRSTRING(target) = data.string;
    
	return EC_OK;
}
