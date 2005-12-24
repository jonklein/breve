/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000 - 2004 Jonathan Klein                                  *
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

/**
 *	A record for the steve garbage collector
 */

struct stGCRecord {
	stGCRecord() { pointer = NULL; type = AT_NULL; }

	stGCRecord( void *p, int t ) { pointer = p; type = t; };

	void *pointer;
	int type;
};

void stGCRetain(brEval *eval);
void stGCRetainPointer(void *pointer, int type);

void stGCUnretain(brEval *eval);
void stGCUnretainPointer(void *pointer, int type);

void stGCCollect(brEval *eval);
void stGCCollectPointer(void *pointer, int type);

void stGCUnretainAndCollect(brEval *eval);
void stGCUnretainAndCollectPointer(void *pointer, int type);

void stGCMark(stInstance *instance, brEval *collect);
inline void stGCMarkPointer(stInstance *instance, void *pointer, int type);

void stGCUnmark(stInstance *instance, brEval *collect);

void stGCCollectStack(slStack *gcStack);

void brEvalListRetain(brEvalListHead *h);
void brEvalListUnretain(brEvalListHead *h);
void brEvalListCollect(brEvalListHead *h);

void brEvalHashRetain(brEvalHash *h);
void brEvalHashUnretain(brEvalHash *h);
void brEvalHashCollect(brEvalHash *h);

void brEvalHashFreeGC(brEvalHash *h);
void brEvalListFreeGC(brEvalListHead *h);
