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

#define SDO_MAGIC_NUMBER        0x49301044
#define SDO_VERSION             1

struct brDataObjectHeader {
    int varSize;
    int nameLength;
    int version;
    int magic;
};

typedef struct brDataObjectHeader brDataObjectHeader;

void breveInitSteveDataObjectFuncs(brNamespace *n);

int stDDataCheckVariables(brEval *args, brEval *result, brInstance *i);
int stDDataCopyObject(brEval *args, brEval *result, brInstance *i);
int stDDataWriteObject(brEval *args, brEval *result, brInstance *i);
int stDDataWriteObjectWithDialog(brEval *args, brEval *result, brInstance *i);
int stDDataReadObject(brEval *args, brEval *result, brInstance *i);
int stDDataReadObjectWithDialog(brEval *args, brEval *result, brInstance *i);

int stCSimpleCrossover(brEval *args, brEval *target, brInstance *i);

/* not callback functions */

int stWriteObject(stInstance *i, char *filename);
int stReadObject(stInstance *i, char *filename);

char *stPackObject(stInstance *i, int *length);
int stUnpackObject(stInstance *i, char *buffer, int length);

int stReadObjectFromStream(stInstance *i, FILE *fp);
