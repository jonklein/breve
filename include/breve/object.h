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

/*!
	\brief A steve object type.
*/

#ifdef __cplusplus

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
	char *name;
	brEngine *engine;
	stSteveData *steveData;

	float version;

	unsigned int varSize;
	unsigned int varOffset;

	stObject *super;

	std::map< std::string, stVar* > variables;
	std::map< std::string, std::vector< stMethod* > > methods;

	std::set< stInstance*, stInstanceCompare> allInstances;
};

/*!
	\brief A steve object instance.

	An instance in the steve language.
*/

struct stInstance {
	stObject *type;
	brInstance *breveInstance;

	char *variables;

	// status is the memory status of the object 
	char status;

	bool gc;
	int retainCount;

	slStack *gcStack;

	std::set< stInstance*, stInstanceCompare > dependencies;
	std::set< stInstance*, stInstanceCompare > dependents;
};

/*!
	\brief Holds a steve method.
*/

struct stMethod {
	char *name;

	unsigned char inlined;

	int lineno;
	char *filename;

	std::vector< stKeywordEntry* > keywords;
	std::vector< stExp* > code;
	std::vector< stVar* > variables;

	// how much space does this method need on the stack for inputs and locals? 

	int stackOffset;
};
#endif



/*!
	\brief Holds the type of a variable.

	The type of a variable.  The basic type is the type field.  If the basic
	type is an array, then the arrayType specifies the type contained therein
	and arrayCount indicates the number of elements in the array.

	Finally, if the basic type is an object, the object field may hold a pointer 
	and a name of a specific object type.
*/


struct stVarType {
	char *objectName;
	stObject *objectType;

	unsigned char type;
	unsigned char arrayType;
	int arrayCount;
};

/*!
	\brief Holds a variable definition.

	Contains the name and type of the variable, along with its offset (which 
	may be a local variable offset or a stack offset, depending on the variable.
	It also holds a flag indicating whether the variable is used or not.
*/

struct stVar {
	char *name;
	int offset;

	stVarType *type;

	unsigned char used;
};

/*!
	\brief Stores a steve method-keyword.

	A steve method-keyword is a stVar struct and an associated name.
*/

struct stKeywordEntry {
	char *keyword;
	stVar *var;
	stKeyword *defaultKey;
};

stObject *stObjectNew(brEngine *e, stSteveData *data, char *name, char *alias, stObject *super, float version);
stObject *stObjectFind(brNamespace *n, char *name);

stInstance *stFindInstanceIndex(stInstance *i, int index);
int stEnumerateInstance(stInstance *i, int start);

void stInstanceAddDependency(stInstance *i, stInstance *dependency);
void stInstanceRemoveDependency(stInstance *i, stInstance *dependency);


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

stVarType *stVarTypeNew(unsigned char type, unsigned char atype, int count, char *otype);
stVarType *stVarTypeCopy(stVarType *original);

stVar *stVarNew(char *name, stVarType *type);

void stFreeStVar(stVar *v);

void stFreeKeywordEntry(stKeywordEntry *e);

stVar *stInstanceNewVar(stVar *var, stObject *object);

stMethod *stNewMethod(char *n, std::vector< stKeywordEntry* > *keywords, char *file, int line);
void stFreeMethod(stMethod *meth);
stVar *stMethodAddVar(stVar *var, stMethod *method);

void stMethodAlignStack(stMethod *method);

stKeywordEntry *stNewKeywordEntry(char *stKeyword, stVar *v, brEval *defVal);

int findVariableOffset(stObject *o, char *name);

stVar *stObjectLookupVariable(stObject *ob, char *word);

stKeywordEntry *stFindKeyword(char *keyword, stMethod *m);
stVar *stFindLocal(char *name, stMethod *);

int stUnusedInstanceVarWarning(stObject *);

stMethod *stFindInstanceMethodNoSuper(stObject *, char *, unsigned int);
stMethod *stFindInstanceMethod(stObject *, char *, int, stObject **);
stMethod *stFindInstanceMethodWithArgRange(stObject *, char *, unsigned int, unsigned int, stObject **);
stMethod *stFindInstanceMethodWithMinArgs(stObject *, char *, unsigned int, stObject **);

int stStoreInstanceMethod(stObject *, char *, stMethod *);

void stInstanceRetain(stInstance *);
void stInstanceUnretain(stInstance *);
void stInstanceCollect(stInstance *);

int stIsSubclassOf(stObject *a, stObject *);

void stAddToInstanceLists(stInstance *);
void stRemoveFromInstanceLists(stInstance *);

brInstance *stInstanceCreateAndRegister(stSteveData *d, brEngine *, brObject *);

void stObjectFreeAllInstances(stObject *);
