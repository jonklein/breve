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

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "slBrevePluginAPI.h"

struct slURLWriteData {
	char *string;
	int maxSize;
	int size;
};


typedef struct slURLWriteData slURLWriteData;

size_t slURLWriteFunction(void *ptr, size_t size, size_t nmemb, void *stream);
int slUGetURL(brEval args[], brEval *target, void *i);

void slInitURLFunctions(void *n) {
    brNewBreveCall(n, "getURL", slUGetURL, AT_STRING, AT_STRING, 0);
}

size_t slURLWriteFunction(void *ptr, size_t size, size_t nmemb, void *stream) {
	slURLWriteData *data = stream;

	if(data->maxSize <= data->size + size * nmemb) {
		data->maxSize = data->size + (size * nmemb) + 1;
		data->string = slRealloc(data->string, data->maxSize);
	}

	memmove(&data->string[data->size], ptr, size * nmemb);

	data->size += size * nmemb;
}

int slUGetURL(brEval args[], brEval *target, void *i) {
	CURL *handle;
	CURLcode result;
	slURLWriteData data;

	data.string = slMalloc(1024);
	data.maxSize = 1024;
	data.size = 0;

	handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, BRSTRING(&args[0]));
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, slURLWriteFunction);
	curl_easy_setopt(handle, CURLOPT_FILE, &data);
	result = curl_easy_perform(handle);

	BRSTRING(target) = data.string;
    
	return EC_OK;
}
