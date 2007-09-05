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

#ifndef _OBJECT_H
#define _OBJECT_H

/**
 * \brief A steve object type.
 */

#include <vector>
#include <set>
#include <map>
#include <string>

struct stInstanceCompare {
	bool operator()(const stInstance* s1, const stInstance* s2) const {
		return s1 < s2;
	}
};

struct stObject {
	std::string name;
	brEngine *engine;
	stSteveData *steveData;

	float version;

	unsigned int varSize;
	unsigned int varOffset;

	stObject *super;

	std::map< std::string, stVar* > variables;
	std::map< std::string, std::vector< stMethod* > > methods;

	std::set< stInstance*, stInstanceCompare > allInstances;

	std::string 				_file;
	std::string 				_comment;
};

/**
 * A steve object instance.
 * 
 * An instance in the steve language.
 */

struct stInstance {
	stObject *type;
	brInstance *breveInstance;

	char *variables;

	// status is the memory status of the object 
	char status;

	bool gc;
	int retainCount;
};

/**
 * \brief Holds a steve method.
 */

class stMethod {
	public:
		stMethod( const char *n, std::vector< stKeywordEntry* > *k, const char *file, int line );
		~stMethod();

		std::string name;

		bool inlined;

		int lineno;
		std::string filename;
	
		std::vector< stKeywordEntry* > keywords;
		std::vector< stExp* > code;
		std::vector< stVar* > variables;

		// how much space does this method need on the stack for inputs and locals? 

		int stackOffset;

		std::string 				_comment;
};

/*!
	\brief Holds the type of a variable.

	The type of a variable.  The basic type is the type field.  If the basic
	type is an array, then the arrayType specifies the type contained therein
	and arrayCount indicates the number of elements in the array.

	Finally, if the basic type is an object, the object field may hold a pointer 
	and a name of a specific object type.
*/

class stVarType {
	public:
		stVarType( unsigned char type, unsigned char arrayType, int arrayCount, const char *objectType );
		stVarType *copy();

		std::string _objectName;
		stObject *_objectType;

		unsigned char _type;
		unsigned char _arrayType;
		int _arrayCount;
};

/*!
	\brief Holds a variable definition.

	Contains the name and type of the variable, along with its offset (which 
	may be a local variable offset or a stack offset, depending on the variable.
	It also holds a flag indicating whether the variable is used or not.
*/

class stVar {
	public:
		stVar( char *n, stVarType *t );
		~stVar();

		std::string name;
		int offset;

		stVarType *type;

		bool used;
};

/**
 * \brief Stores a steve method-keyword.
 * A steve method-keyword is a stVar struct and an associated name.
 */

struct stKeywordEntry {
	std::string keyword;
	stVar *var;
	stKeyword *defaultKey;
};

stObject *stObjectNew( brEngine *e, stSteveData *data, char *name, char *alias, stObject *super, float version, const char *inFile );
stObject *stObjectFind(brNamespace *n, char *name);

stInstance *stFindInstanceIndex(stInstance *i, int index);
int stEnumerateInstance(stInstance *i, int start);

void stObjectFreeSpace(brNamespace *ns);
void stObjectFree(stObject *o);

int stAddCollisionHandler(stObject *handler, stObject *collider, stMethod *m);

stInstance *stNewControllerInstance(stObject *o, brEngine *e);
stInstance *stInstanceNew(stObject *o);
int stInstanceInit(stInstance *i);

int stMethodTrace(stRunInstance *i, char *name);

void stInstanceFree(stInstance *i);
void stInstanceFreeNoInstanceLists(stInstance *i);
void stInstanceFreeInternals(stInstance *i);

void stFreeStVar(stVar *v);

void stFreeKeywordEntry(stKeywordEntry *e);

stVar *stInstanceNewVar(stVar *var, stObject *object);

stMethod *stNewMethod(char *n, std::vector< stKeywordEntry* > *keywords, char *file, int line);
void stFreeMethod(stMethod *meth);
stVar *stMethodAddVar(stVar *var, stMethod *method);

void stMethodAlignStack(stMethod *method);

stKeywordEntry *stNewKeywordEntry(char *stKeyword, stVar *v, brEval *defVal);

stVar *stObjectLookupVariable( stObject *ob, const char *word );

stKeywordEntry *stFindKeyword( const char *keyword, stMethod *m );
stVar *stFindLocal( const char *name, stMethod * );

int stUnusedInstanceVarWarning(stObject *);

stMethod *stFindInstanceMethodNoSuper(stObject *, const char *, unsigned int);
stMethod *stFindInstanceMethod(stObject *, const char *, int, stObject **);
stMethod *stFindInstanceMethodWithArgRange(stObject *, const char *, unsigned int, unsigned int, stObject **);
stMethod *stFindInstanceMethodWithMinArgs(stObject *, const char *, unsigned int, stObject **);

int stStoreInstanceMethod(stObject *, char *, stMethod *);

void stInstanceRetain(stInstance *);
void stInstanceUnretain(stInstance *);
void stInstanceCollect(stInstance *);

int stIsSubclassOf(stObject *a, stObject *);

void stAddToInstanceLists(stInstance *);
void stRemoveFromInstanceLists(stInstance *);

brInstance *stInstanceCreateAndRegister(stSteveData *d, brEngine *, brObject *);

void stObjectFreeAllInstances(stObject *);

#endif /* _OBJECT_H */
