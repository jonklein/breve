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

struct stObject {
	char *name;
	brEngine *engine;

	float version;

	int varSize;
	int varOffset;

	stObject *super;

	brNamespace *keywords;

	slList *variableList;
	slList *allInstances;
};

/*!
	\brief A steve object instance.

	An instance in the steve language.
*/

struct stInstance {
	stObject *type;
	brInstance *breveInstance;

	pthread_mutex_t lock;

	unsigned int index;

	char *variables;

	// status is the memory status of the object 
	char status;

	unsigned char gc;
	int retainCount;

	slStack *gcStack;
};

/*!
	\brief Holds a steve method.
*/

struct stMethod {
	char *name;

	unsigned char inlined;

	// = 1 if keywords need garbage collection 
	unsigned char gcKeywords;

	// = 1 if variables need garbage collection 
	unsigned char gcVariables;

	int lineno;
	char *filename;

	slArray *keywords;
	slArray *code;

	slList *variables;

	// how much space does this method need on the stack for inputs and locals? 

	int stackOffset;
};

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
	brEval *defaultValue;
};

stObject *stObjectNew(brEngine *e, stSteveData *data, char *name, char *alias, stObject *super, float version);
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

stVarType *stVarTypeNew(unsigned char type, unsigned char atype, int count, char *otype);
stVarType *stVarTypeCopy(stVarType *original);

stVar *stVarNew(char *name, stVarType *type);

void stFreeStVar(stVar *v);

void stFreeKeywordEntry(stKeywordEntry *e);
void stFreeKeywordEntryArray(slArray *a);

brNamespaceSymbol *stInstanceNewVar(stVar *var, stObject *object);

stMethod *stNewMethod(char *n, slArray *keywords, char *file, int line);
void stFreeMethod(stMethod *meth);
slList *stMethodAddVar(stVar *var, stMethod *method);

void stMethodAlignStack(stMethod *method);

stKeywordEntry *stNewKeywordEntry(char *stKeyword, stVar *v, brEval *defVal);

int findVariableOffset(stObject *o, char *name);

brNamespaceSymbol *stObjectLookup(stObject *o, char *word, unsigned char type);

stKeywordEntry *stFindKeyword(char *keyword, stMethod *m);
stVar *stFindLocal(char *name, stMethod *m);

int stUnusedInstanceVarWarning(stObject *o);

stMethod *stFindInstanceMethod(stObject *o, char *word, int nArgs, stObject **oo);
stMethod *stFindInstanceMethodNoSuper(stObject *o, char *word, int nArgs);
stMethod *stFindInstanceMethodWithArgRange(stObject *o, char *word, int minArgs, int maxArgs, stObject **oo);

int stStoreInstanceMethod(stObject *o, char *word, stMethod *method);

void stInstanceRetain(stInstance *i);
void stInstanceUnretain(stInstance *i);
void stInstanceCollect(stInstance *i);

int stIsSubclassOf(stObject *a, stObject *o);

void stAddToInstanceLists(stInstance *i);
void stRemoveFromInstanceLists(stInstance *i);

stInstance *stInstanceCreateAndRegister(brEngine *e, brObject *class);

void stObjectFreeAllInstances(stObject *o);
