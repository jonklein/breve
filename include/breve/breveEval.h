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

#include "slutil.h"
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
	AT_UNDEFINED,
	AT_NULL,
	AT_INT,
	AT_POINTER,
	AT_TYPE,
	AT_DOUBLE,
	AT_VECTOR,
	AT_MATRIX,
	AT_ARRAY,

	AT_STRING,
	AT_INSTANCE,
	AT_LIST,
	AT_DATA,
	AT_HASH
};

extern const char *brAtomicTypeStrings[];

extern "C" {

DLLEXPORT void stGCUnretainAndCollectPointer(void *pointer, int type);
DLLEXPORT void stGCRetainPointer(void *pointer, int type);

}

/**
 * A simple reference counted base class used for different brEval types.
 */

class DLLEXPORT brEvalObject {
	public:
		brEvalObject();

		virtual ~brEvalObject() = 0;

		int _retainCount;

		void retain();
		void unretain();

		void collect();
};

/**
 * A typed expression in the breve engine.
 *
 * The universal currency in the breve engine, this structure
 * holds data of any type.  It is used to pass arguments and 
 * return values, and is also used to hold all expressions in
 * the steve language frontend.
 */

class DLLEXPORT brEval {
	public:
		brEval() { _type = AT_NULL; _values.pointerValue = NULL; _needsCollect = false; }

		brEval( const brEval& inOther );

		~brEval();

		brEval &operator=( const brEval &inOther );

		bool checkNaNInf();

		inline void collect() {
			if( _needsCollect ) {
				stGCUnretainAndCollectPointer( _values.pointerValue, _type );
				_needsCollect = false;
			}
		}

		inline void retain() {
			stGCRetainPointer( _values.pointerValue, _type );

			_needsCollect = true;
		}


		void clear() { collect(); _type = AT_NULL; _values.pointerValue = 0; }

		inline unsigned char type() const { return _type; } 

		inline void set( const double d )    { collect(); _values.doubleValue = d;                _type = AT_DOUBLE; _needsCollect = false;  }
		inline void set( const int i )       { collect(); _values.intValue = i;                   _type = AT_INT; _needsCollect = false;      }
		inline void set( const long i )      { collect(); _values.intValue = i;                   _type = AT_INT; _needsCollect = false;      }
		inline void set( const slVector &v ) { collect(); slVectorCopy(&v, &_values.vectorValue); _type = AT_VECTOR; _needsCollect = false;   }
		inline void set( const slMatrix &m ) { collect(); slMatrixCopy(m, _values.matrixValue);   _type = AT_MATRIX; _needsCollect = false;   }
		inline void set( const char *s )     { collect(); _values.stringValue = slStrdup( s );      _type = AT_STRING; _needsCollect = true; }
		inline void set( const std::string &s )     { collect(); _values.stringValue = slStrdup( s.c_str() );      _type = AT_STRING; _needsCollect = true; }
		inline void set( void *p )           { collect(); _values.pointerValue = p;               _type = AT_POINTER;  retain(); }
		inline void set( brEvalHash *h )     { collect(); _values.hashValue = h;                  _type = AT_HASH;     retain(); }
		inline void set( brData *d )         { collect(); _values.dataValue = d;                  _type = AT_DATA;     retain(); }
		inline void set( brInstance *i )     { collect(); _values.instanceValue = i;              _type = AT_INSTANCE; retain(); }
		inline void set( brEvalListHead *l ) { collect(); _values.listValue = l;                  _type = AT_LIST;     retain(); }

		inline int			getInt()      const { return _values.intValue;      }
		inline double		getDouble()   const { return _values.doubleValue;   }
		inline slVector		&getVector()  		{ return _values.vectorValue;  }
		inline slMatrix		&getMatrix()    	{ return _values.matrixValue;  } 
		inline void			*getPointer()  const { return _values.pointerValue;  }
		inline char			*getString()   const { return _values.stringValue;   }
		inline brEvalHash	*getHash()     const { return _values.hashValue;     }
		inline brData		*getData()     const { return _values.dataValue;     }
		inline brInstance	*getInstance() const { return _values.instanceValue; }
		inline brEvalListHead	*getList()     const { return _values.listValue;     } 

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
		bool _needsCollect;
};

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

int brEvalCopy( const brEval *inSrc, brEval *outDst );
char *brObjectDescription( brInstance *inInstance );
char *brFormatEvaluation( brEval *inEval, brInstance *inInstance );
char *brFormatEvaluationWithSeenList( brEval *inEval, brInstance *inInstance, std::set< brEvalListHead* >& inSeen );

#endif /* _BREVEEVAL_H */
