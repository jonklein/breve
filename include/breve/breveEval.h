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

#ifndef _BREVEEVAL_H
#define _BREVEEVAL_H

#include "util.h"
#include <string>
#include <set>

enum evaluationCodes {
	EC_ERROR_HANDLED = -2,
	EC_ERROR,
	EC_STOP,
	EC_OK
};

enum atomicTypes {
	AT_INVALID = 0,
	AT_NULL,
	AT_INT,
	AT_DOUBLE,
	AT_STRING,
	AT_INSTANCE,
	AT_POINTER,
	AT_VECTOR,
	AT_MATRIX,
	AT_LIST,
	AT_ARRAY,
	AT_DATA,
	AT_HASH,
	AT_TYPE,
	AT_UNDEFINED
};

extern char *brAtomicTypeStrings[];

/*!
	\brief A typed expression in the breve engine.

	The universal currency in the breve engine, this structure
	holds data of any type.  It is used to pass arguments and 
	return values, and is also used to hold all expressions in
	the steve language frontend.
*/

extern "C" {

DLLEXPORT void stGCUnretainAndCollectPointer(void *pointer, int type);
DLLEXPORT void stGCRetainPointer(void *pointer, int type);

}

class brEvalObject {
	public:
		brEvalObject() { _retainCount = 0; } ;

		virtual ~brEvalObject() { };

		int _retainCount;

		void retain() { _retainCount++; }
		void unretain() { _retainCount--; }

		void collect() { if( _retainCount < 1 ) delete this; }
};

class brEval {
	public:
		brEval() { _type = AT_NULL; _values.pointerValue = NULL; }

		brEval( const brEval& inOther );

		~brEval() { collect(); }

		inline void collect() {
			if ( _type == AT_NULL || _type == AT_INT || _type == AT_MATRIX || _type == AT_VECTOR || _type == AT_DOUBLE ) return;
			stGCUnretainAndCollectPointer( _values.pointerValue, _type );
		}

		inline void retain() {
			if ( _type == AT_NULL || _type == AT_INT || _type == AT_MATRIX || _type == AT_VECTOR || _type == AT_DOUBLE ) return;
			stGCRetainPointer( _values.pointerValue, _type );
		}


		void clear() { collect(); _type = AT_NULL; }

		inline unsigned char type() { return _type; } 

		inline void set( const double d )    { collect(); _values.doubleValue = d;                _type = AT_DOUBLE;   }
		inline void set( const int i )       { collect(); _values.intValue = i;                   _type = AT_INT;      }
		inline void set( const long i )      { collect(); _values.intValue = i;                   _type = AT_INT;      }
		inline void set( const slVector &v ) { collect(); slVectorCopy(&v, &_values.vectorValue); _type = AT_VECTOR;   }
		inline void set( const slMatrix &m ) { collect(); slMatrixCopy(m, _values.matrixValue);   _type = AT_MATRIX;   }
		inline void set( const char *s )     { collect(); _values.stringValue = slStrdup( s );      _type = AT_STRING;   }
		inline void set( const std::string &s )     { collect(); _values.stringValue = slStrdup( s.c_str() );      _type = AT_STRING;   }
		inline void set( void *p )           { collect(); _values.pointerValue = p;               _type = AT_POINTER;  retain(); }
		inline void set( brEvalHash *h )     { collect(); _values.hashValue = h;                  _type = AT_HASH;     retain(); }
		inline void set( brData *d )         { collect(); _values.dataValue = d;                  _type = AT_DATA;     retain(); }
		inline void set( brInstance *i )     { collect(); _values.instanceValue = i;              _type = AT_INSTANCE; retain(); }
		inline void set( brEvalListHead *l ) { collect(); _values.listValue = l;                  _type = AT_LIST;     retain(); }

		inline int             &getInt()      { return _values.intValue;      }
		inline double          &getDouble()   { return _values.doubleValue;   }
		inline slVector       &getVector()    { return _values.vectorValue;  }
		inline slMatrix       &getMatrix()    { return _values.matrixValue;  } 
		inline void           *&getPointer()  { return _values.pointerValue;  }
		inline char           *&getString()   { return _values.stringValue;   }
		inline brEvalHash     *&getHash()     { return _values.hashValue;     }
		inline brData         *&getData()     { return _values.dataValue;     }
		inline brInstance     *&getInstance() { return _values.instanceValue; }
		inline brEvalListHead *&getList()     { return _values.listValue;     } 

	private:
		union {
			double doubleValue;  
			int intValue;
			slVector vectorValue;
			slMatrix matrixValue;
			void *pointerValue;
			char *stringValue;
			brEvalHash *hashValue;
			brData *dataValue;
			brInstance *instanceValue;
			brEvalListHead *listValue;
		} _values;

		unsigned char _type;
};

//	bool operator<(brEval*);


#define BRINT(e)		( (e)->getInt()      )
#define BRFLOAT(e)		( (e)->getDouble()   )
#define BRDOUBLE(e)		( (e)->getDouble()   )
#define BRSTRING(e)		( (e)->getString()   )
#define BRVECTOR(e)		( (e)->getVector()   )
#define BRMATRIX(e)		( (e)->getMatrix()   )
#define BRINSTANCE(e)	( (e)->getInstance() )
#define BRPOINTER(e)	( (e)->getPointer()  )
#define BRDATA(e)		( (e)->getData()     )
#define BRHASH(e)		( (e)->getHash()     )
#define BRLIST(e)		( (e)->getList()     )

int brEvalCopy( const brEval *source, brEval *dest );
char *brObjectDescription( brInstance *i );
char *brFormatEvaluation( brEval *e, brInstance *i );
char *brFormatEvaluationWithSeenList( brEval *e, brInstance *i, std::set< brEvalListHead* >& seen );

#endif /* _BREVEEVAL_H */
