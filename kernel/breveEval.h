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

enum evaluationCodes {
	EC_ERROR_HANDLED = -2,
	EC_ERROR,
	EC_STOP,
	EC_OK
};

enum atomicTypes {
	AT_UNDEFINED = 0,
	AT_NULL,
	AT_INT,
	AT_DOUBLE,
	AT_STRING,
	AT_INSTANCE,
	AT_BRIDGE_INSTANCE,
	AT_POINTER,
	AT_VECTOR,
	AT_MATRIX,
	AT_LIST,
	AT_ARRAY,
	AT_DATA,
	AT_HASH,
	AT_TYPE
};

/*!
	\brief A typed expression in the breve engine.

	The universal currency in the breve engine, this structure
	holds data of any type.  It is used to pass arguments and 
	return values, and is also used to hold all expressions in
	the steve language frontend.
*/
	
struct brEval {
	union {
		double doubleValue;  
		int intValue;
		slVector vectorValue;
		double matrixValue[3][3];
		void *pointerValue;
		char *stringValue;
		brEvalHash *hashValue;
		brData *dataValue;
		brInstance *instanceValue;
		brEvalListHead *listValue;
	} values;

	unsigned char type;
};

#define BRINT(e)		((e)->values.intValue)
#define BRFLOAT(e)		((e)->values.doubleValue)
#define BRDOUBLE(e)		((e)->values.doubleValue)
#define BRSTRING(e)		((e)->values.stringValue)
#define BRVECTOR(e)		((e)->values.vectorValue)
#define BRMATRIX(e)		((e)->values.matrixValue)
#define BRINSTANCE(e)	((e)->values.instanceValue)
#define BRBRIDGEINSTANCE(e)	((e)->values.instanceValue)
#define BRPOINTER(e)	((e)->values.pointerValue)
#define BRDATA(e)		((e)->values.dataValue)
#define BRHASH(e)		((e)->values.hashValue)
#define BRLIST(e)		((e)->values.listValue)

int brEvalCopy(brEval *s, brEval *d);

int brEvalPointer(brEval *s, void **pointer);
