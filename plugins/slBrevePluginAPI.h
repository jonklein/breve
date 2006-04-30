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
#include <string.h>
#include <vector>

// The breve Simulation Environment plugin API version 2.4
//
// Documentation on using the breve plugin API is included in
// the documentation distributed with breve.

// These are deprecated symbol names, included here for backwards compatability 

#define stNewBreveCall brNewBreveCall
#define stNewSteveCall brNewBreveCall
#define stEval brEval
#define stHash brEvalHash
#define stEvalList brEvalList
#define stEvalListHead brEvalListHead
#define stNewEvalList() 	(new brEvalListHead())
#define brEvalListNew() 	(new brEvalListHead())
#define stPluginFindFile brPluginFindFile

#define STINT		BRINT
#define STFLOAT		BRFLOAT
#define STDOUBLE	BRDOUBLE
#define STSTRING	BRSTRING
#define STVECTOR	BRVECTOR
#define STMATRIX	BRMATRIX
#define STINSTANCE	BRINSTANCE
#define STPOINTER	BRPOINTER
#define STDATA		BRDATA
#define STHASH		BRHASH
#define STLIST		BRLIST

#include <stdio.h>
#include <stdlib.h>

/* Plugin entry-point functions should be declared as DLLEXPORT */

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WINDOWS)
#ifdef __cplusplus
#define DLLEXPORT extern "C" __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllexport)
#endif
#elif defined(__cplusplus)
#define DLLEXPORT extern "C"
#else
#define DLLEXPORT extern
#endif

typedef struct slVector slVector;
typedef struct brEval brEval;
typedef struct brEvalListHead brEvalListHead;
typedef struct brEvalList brEvalList;
typedef struct brEvalHash brEvalHash;
typedef struct brData brData;
typedef struct stInstance stInstance;
typedef struct brInstance brInstance;

// These entries appear in the brEval type field. 

enum atomicTypes {
    AT_INVALID = 0,
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
    AT_TYPE,
    AT_UNDEFINED
};

struct slVector {
	double x, y, z;
};

typedef double slMatrix[3][3];

#ifdef __cplusplus
extern "C" {
#endif

void stGCUnretainAndCollectPointer(void *pointer, int type);
void stGCRetainPointer(void *pointer, int type);

class brEval {
	public:
		brEval() { _type = AT_NULL; }

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
		inline void set( const slVector &v ) { collect(); memcpy(&_values.vectorValue, &v, sizeof( slVector ) ); _type = AT_VECTOR;   }
		inline void set( const slMatrix &m ) { collect(); memcpy( _values.matrixValue, m, sizeof( slMatrix ) );   _type = AT_MATRIX;   }
		inline void set( const char *s )     { collect(); _values.stringValue = strdup(s);         _type = AT_STRING;   }
		inline void set( void *p )           { collect(); _values.pointerValue = p;               _type = AT_POINTER;  retain(); }
		inline void set( brEvalHash *h )     { collect(); _values.hashValue = h;                  _type = AT_HASH;     retain(); }
		inline void set( brData *d )         { collect(); _values.dataValue = d;                  _type = AT_DATA;     retain(); }
		inline void set( brInstance *i )     { collect(); _values.instanceValue = i;              _type = AT_INSTANCE; retain(); }
		inline void set( brEvalListHead *l ) { collect(); _values.listValue = l;                  _type = AT_LIST;     retain(); }

		inline int             &getInt()      { return _values.intValue;      }
		inline double          &getDouble()   { return _values.doubleValue;   }
		inline slVector        &getVector()   { return _values.vectorValue;  }
		inline slMatrix        &getMatrix()   { return _values.matrixValue;  }
		inline void*           &getPointer()  { return _values.pointerValue;  }
		inline char*           &getString()   { return _values.stringValue;   }
		inline brEvalHash*     &getHash()     { return _values.hashValue;     }
		inline brData*         &getData()     { return _values.dataValue;     }
		inline brInstance*     &getInstance() { return _values.instanceValue; }
		inline brEvalListHead* &getList()     { return _values.listValue;     }

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

class brEvalObject {
	public:
		brEvalObject();

		virtual ~brEvalObject();

		int _retainCount;

		void retain();
		void unretain();

		void collect();
};

class brEvalListHead: public brEvalObject {
    public:
        brEvalListHead();
        ~brEvalListHead();

        std::vector< brEval* > _vector;

        inline std::vector< brEval* > &getVector() { return _vector; }
};

class brData: public brEvalObject {
    public:
        brData( void *inData, int inLength );
        ~brData();

        unsigned char *data;
        int length;
};

brData *brDataNew(void *data, int length);
void brDataFree(brData *data);

/* 
	The following functions are used to create & edit evaluation lists.
	You can create brEvalLists to be returned by your functions.
	Don't worry about freeing brEvalLists, this is done by the engine.
*/

int brEvalListInsert( brEvalListHead *head, int index, brEval *value );

#define brEvalListAppend( a, eval ) brEvalListInsert( (a), (a)->_vector.size(), (eval) )

/* Use these macros to treat brEval pointers as specific types. */

#define BRINT(e)		( (e)->getInt() )
#define BRFLOAT(e)		( (e)->getDouble() )
#define BRDOUBLE(e)		( (e)->getDouble() )
#define BRSTRING(e)		( (e)->getString() )
#define BRVECTOR(e)		( (e)->getVector() )
#define BRMATRIX(e)		( (e)->getMatrix() )
#define BRINSTANCE(e)	( (e)->getInstance() )
#define BRPOINTER(e)	( (e)->getPointer() )
#define BRDATA(e)		( (e)->getData() )
#define BRHASH(e)		( (e)->getHash() )
#define BRLIST(e)		( (e)->getList() )

#define EC_ERROR -1
#define EC_OK 1

int brNewBreveCall(void *n, char *name, int (*call)(brEval argumentArray[],
	brEval *returnValue, void *callingInstance), int rtype, ...);

	/*
	 * The brMethodCallByNameWithArgs() function calls the breve method
	 * named by the string _name_ with _argcount_ number of arguments
	 * in the array pointed to by _args_ and stores the return value of the
	 * method (if any) in the brEval pointed to by _result_.
	 *
	 * The function returns EC_OK if successful.
	 * If an error occurs while calling the specified method, EC_ERROR is
	 * returned and the simulation is halted.
	 */

int brMethodCallByNameWithArgs(void *instance, char *name, brEval **args, int count, brEval *result);

	/*
	 * The slMessage() function prints a formatted error message to the
	 * breve console. The _level_ argument must always be DEBUG_ALL.
	 * The _fmt_ specification (and associated arguments) may be any format
	 * allowed by printf(3) or a simple string.
	 */

#define DEBUG_ALL	0

void slMessage(int level, const char *fmt, ...);

	/*
	 * Macros for obsolete functions.
	 */

#define slMalloc(n) calloc(1,n)
#define slRealloc(p,n) realloc(p,n)
#define slFree(p) free(p)

	/*
	 * The slStrdup() function allocates space for a copy of the string
	 * pointed to by _s_, copies the string, and a returns a pointer
	 * to the copied string. The returned pointer may subsequently be
	 * used as an argument to the slFree() and slRealloc() functions.
	 *
	 * If the string cannot be copied, NULL is returned.
	 */

char *slStrdup(const char *s);

	/*
	 * The slGetLogFilePointer() function returns a FILE pointer
	 * referring to the breve output log, typically stderr.
	 */

FILE *slGetLogFilePointer(void *callingInstance);

	/*
	 * The brPluginFindFile() function finds a file in the engine's
	 * search path and returns the pathname to that file.
	 *
	 * The returned pointer, if it is not NULL, should be deallocated
	 * with slFree() when it is no longer needed by the plugin.
	 */

char *brPluginFindFile(char *file, void *callingInstance);

#ifdef __cplusplus
}
#endif
