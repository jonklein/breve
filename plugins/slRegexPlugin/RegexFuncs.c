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

#include "slBrevePluginAPI.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

int slRSplit(brEval args[], brEval *output, void *instance);
int slRRegex(brEval args[], brEval *output, void *instance);

void slInitRegexFuncs(void *n) {
    brNewBreveCall(n, "split", slRSplit, AT_LIST, AT_STRING, AT_STRING, 0);
    brNewBreveCall(n, "regex", slRRegex, AT_INT, AT_STRING, AT_STRING, 0);
}

/*
    + slRSplit
    = use the BSD regexp functions to implement a perl style "split"
    = function.
*/

int slRSplit(brEval args[], brEval *output, void *instance) {
    regex_t exp;
    int result;
    char *expstr = BRSTRING(&args[0]);
    char *string = BRSTRING(&args[1]);
    regmatch_t matches[1];

    brEval stringEval;

    brEvalListHead *list;

    result = regcomp(&exp, expstr, REG_EXTENDED);

    if(result != 0) {
        /* couldn't compile regular expression */

        slMessage(DEBUG_ALL, "error compiling regular expression \"%s\"", expstr);
        BRLIST(output) = NULL;
        return EC_OK;
    }

    list = brEvalListNew();

    /* keep running the regular expression and spliting out new fields */

    stringEval.type = AT_STRING; 

    while(*string && regexec(&exp, string, 1, matches, 0) != REG_NOMATCH) {
        if(matches[0].rm_eo != 0) {
            /* we've found the beginning of the split field... copy from the start */
            /* of the search string, up to the start pointer. */

            BRSTRING(&stringEval) = slMalloc(matches[0].rm_so + 1);
        
            strncpy(BRSTRING(&stringEval), string, matches[0].rm_so);
            (BRSTRING(&stringEval))[matches[0].rm_so] = 0;

            brEvalListAppend(list, &stringEval);

            /* advance the string pointer to the end of the previously matched expression */

            string += matches[0].rm_eo;
        } else {
            /* if we match a zero length regexp, just move one character */
            /* forward without adding anything */
 
            string++;
        }
    }

    if(*string) {
        /* it's possible that we've eaten up the last regexp match, but that there */
        /* is still text left--we want this to be included as well */

        BRSTRING(&stringEval) = slMalloc(strlen(string) + 1);
        strcpy(BRSTRING(&stringEval), string);
        brEvalListAppend(list, &stringEval);
    }
    
    regfree(&exp);
    
    BRLIST(output) = list;

    return EC_OK;
}

int slRRegex(brEval args[], brEval *output, void *instance) {
    regex_t exp;
    int result;
    char *expstr = BRSTRING(&args[0]);
    char *string = BRSTRING(&args[1]);
    regmatch_t matches[1];

    result = regcomp(&exp, expstr, REG_EXTENDED);

    if(result != 0) {
        /* couldn't compile regular expression */

        slMessage(DEBUG_ALL, "error compiling regular expression \"%s\"", expstr);
        BRLIST(output) = NULL;
        return EC_OK;
    }

    result = regexec(&exp, string, 1, matches, 0);

    if(result == 0) {
        BRINT(output) = 1;
    } else if(result == REG_NOMATCH) {
        BRINT(output) = 0;
    } else {
        BRINT(output) = 0;
        slMessage(DEBUG_ALL, "error in regular expression check");
    }

    regfree(&exp);
    
    return EC_OK;
}
