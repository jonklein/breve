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

#include "steve.h"
#include "javaFunctions.h"

#define USER_CLASSPATH "."

stJavaBridgeData *stAttachJavaVM(brEngine *e) {
	JavaVMInitArgs vm_args;
	JavaVMOption options[1];
	jint res;
	jmethodID method;
	char args[JAVA_MAX_ARGS];
	char *optstr;
	char *finder;
	char *env;

	stJavaBridgeData *bridge;

	finder = brFindFile(e, "MethodFinder.jar", NULL);

	if(!finder) {
		slMessage(DEBUG_ALL, "Cannot locate the breve Java archive files\n");
		return NULL;
	}

	bridge = slMalloc(sizeof(stJavaBridgeData));

	bridge->objectHash = slNewHash(1027, slHashPointer, slCompPointer);
	bridge->classHash = slNewHash(1027, slHashString, slCompString);

	env = getenv("BREVE_CLASS_PATH");

	if(!env) env = "";

	optstr = malloc(strlen(finder) + strlen(env) + strlen("-Djava.class.path=") + 1024);

	sprintf(optstr, "-Djava.class.path=%s:%s:%s", finder, env, e->userJavaClassPath);

	JNI_GetDefaultJavaVMInitArgs(&vm_args);

	options[0].optionString = optstr;
	vm_args.version = JNI_VERSION_1_2;
	vm_args.options = options;
	vm_args.nOptions = 1;
	vm_args.ignoreUnrecognized = JNI_FALSE;

	/* Create the Java VM */
	res = JNI_CreateJavaVM(&bridge->jvm, (void**)&bridge->env, &vm_args);

	free(optstr);

	if (res < 0) {
		slMessage(DEBUG_ALL, "Can't create JavaVM\n");
		slFree(bridge);
		return NULL;
	}

	bridge->methodFinder = stNewJavaObject(bridge, "MethodFinder", NULL);

	if(!bridge->methodFinder) {
		slMessage(DEBUG_ALL, "Can't find breve MethodFinder class\n");
		slFree(bridge);
		return NULL;
	}

	// method finder

	args[0] = 'O';
	args[1] = 'O';
	args[2] = 'I';
	args[3] = 'O';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->class->class, METHFIND_NAME, METHFIND_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", METHFIND_NAME);
		return NULL;
	}
	bridge->methfindMethod = stJavaMakeMethodData(METHFIND_NAME, method, 'O', args, 4);

	// method signature finder

	args[0] = 'O';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->class->class, METHSIG_NAME, METHSIG_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", METHSIG_NAME);
		return NULL;
	}
	bridge->methsigMethod = stJavaMakeMethodData(METHSIG_NAME, method, 'T', args, 1);

	args[0] = 'O';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->class->class, RETTYPE_NAME, RETTYPE_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", RETTYPE_NAME);
		return NULL;
	}
	bridge->rettypeMethod = stJavaMakeMethodData(RETTYPE_NAME, method, 'C', args, 2);

	args[0] = 'O';
	args[1] = 'I';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->class->class, ARGTYPES_NAME, ARGTYPES_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", ARGTYPES_NAME);
		return NULL;
	}
	bridge->argtypesMethod = stJavaMakeMethodData(ARGTYPES_NAME, method, 'C', args, 2);

	args[0] = 'O';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->class->class, ARGCOUNT_NAME, ARGCOUNT_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", ARGCOUNT_NAME);
		return NULL;
	}
	bridge->argcountMethod = stJavaMakeMethodData(ARGCOUNT_NAME, method, 'I', args, 1);

	args[0] = 'O';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->class->class, CLASSNAME_NAME, CLASSNAME_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", CLASSNAME_NAME);
		return NULL;
	}
	bridge->classnameMethod = stJavaMakeMethodData(CLASSNAME_NAME, method, 'T', args, 1);

	return bridge;
}

void stDetachJavaVM(stJavaBridgeData *bridge) {
	slList *hashes;

	hashes = slHashValues(bridge->classHash);

	while(hashes) {
		stFreeJavaClassData(hashes->data);
		hashes = hashes->next;
	}

	hashes = slHashKeys(bridge->classHash);

	while(hashes) {
		slFree(hashes->data);
		hashes = hashes->next;
	}

	slFreeHash(bridge->classHash);

	stFreeJavaBridgeData(bridge);
}

void stFreeJavaClassData(stJavaClassData *data) {
	slList *hashes;
	
	hashes = slHashValues(data->methodHash);

	while(hashes) {
		slList *list = hashes->data;

		while(list) {
			stFreeJavaMethodData(list->data);
			list = list->next;
		}

		hashes = hashes->next;
	}

	hashes = slHashKeys(data->methodHash);

	while(hashes) {
		slFree(hashes->data);
		hashes = hashes->next;
	}

	slFreeHash(data->methodHash);

	slFree(data);
}

stJavaMethodData *stJavaMakeMethod(stJavaBridgeData *bridge, stJavaClassData *class, char *name, unsigned char *types, int nargs) {
	char returnType, argumentTypes[JAVA_MAX_ARGS];
	jmethodID methodID;
	int count, error;
	brEval result;
	jvalue args[JAVA_MAX_ARGS];
	char *signature;
	jobject methodPtr;
	static jcharArray array = NULL;
	jchar cargTypes[JAVA_MAX_ARGS];

	if(!array) array = (*bridge->env)->NewCharArray(bridge->env, JAVA_MAX_ARGS);

	for(count=0;count<nargs;count++) cargTypes[count] = stJTypeForType(types[count]);

	(*bridge->env)->SetCharArrayRegion(bridge->env, array, 0, nargs, cargTypes);

	// find the method with this name and arg counts for this class
	args[0].l = class->class;
	args[1].l = stMakeJavaString(bridge, name);
	args[2].i = nargs;
	args[3].l = array;
	error = stJavaCallMethod(bridge, bridge->methodFinder, bridge->methfindMethod, args, &result);
	methodPtr = BRPOINTER(&result);

	if(error != EC_OK) return NULL;

	// find the proper signature for the method
	args[0].l = methodPtr;
	error = stJavaCallMethod(bridge, bridge->methodFinder, bridge->methsigMethod, args, &result);
	signature = BRSTRING(&result);

	if(error != EC_OK) return NULL;

	methodID = (*bridge->env)->GetMethodID(bridge->env, class->class, name, signature);
	slFree(signature);

	// find the return type for the method
	args[0].l = methodPtr;
	error = stJavaCallMethod(bridge, bridge->methodFinder, bridge->rettypeMethod, args, &result);
	returnType = BRINT(&result);

	if(error != EC_OK) return NULL;

	// fill in the argument types
	for(count=0;count<nargs;count++) {
		args[0].l = methodPtr;
		args[1].i = count;

		error = stJavaCallMethod(bridge, bridge->methodFinder, bridge->argtypesMethod, args, &result);
		argumentTypes[count] = BRINT(&result);

		if(error != EC_OK) return NULL;
	}

	return stJavaMakeMethodData(name, methodID, returnType, argumentTypes, nargs);
}

stJavaMethodData *stJavaMakeMethodData(char *name, jmethodID method, char returnType, char *argumentTypes, int nargs) {
	stJavaMethodData *data;
	int n;

	data = slMalloc(sizeof(stJavaMethodData));

	data->method = method;
	data->argumentCount = nargs;
	data->returnType = returnType;
	data->name = slStrdup(name);

	for(n=0;n<nargs;n++) data->argumentTypes[n] = argumentTypes[n];

	return data;
}

int stJavaCallMethod(stJavaBridgeData *bridge, stJavaObjectData *object, stJavaMethodData *method, jvalue *jargs, brEval *result) {
	jvalue returnValue;

	switch(method->returnType) {
		case 'V':
			(*bridge->env)->CallVoidMethodA(bridge->env, object->object, method->method, jargs);
			result->type = AT_NULL;
			break;
		case 'I':
			returnValue.i = (*bridge->env)->CallIntMethodA(bridge->env, object->object, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.i;
			break;
		case 'J':
			returnValue.j = (*bridge->env)->CallLongMethodA(bridge->env, object->object, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.j;
			break;
		case 'C':
			returnValue.c = (*bridge->env)->CallCharMethodA(bridge->env, object->object, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.c;
			break;
		case 'B':
			returnValue.b = (*bridge->env)->CallByteMethodA(bridge->env, object->object, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.b;
			break;
		case 'Z':
			returnValue.z = (*bridge->env)->CallBooleanMethodA(bridge->env, object->object, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.z;
			break;
		case 'S':
			returnValue.s = (*bridge->env)->CallShortMethodA(bridge->env, object->object, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.s;
			break;
		case 'F':
			returnValue.f = (*bridge->env)->CallFloatMethodA(bridge->env, object->object, method->method, jargs);
			result->type = AT_DOUBLE;
			BRDOUBLE(result) = returnValue.f;
			break;
		case 'D':
			returnValue.d = (*bridge->env)->CallFloatMethodA(bridge->env, object->object, method->method, jargs);
			result->type = AT_DOUBLE;
			BRDOUBLE(result) = returnValue.d;
			break;
		case 'T':
			// 'T' is breve's code for a string object
			returnValue.l = (*bridge->env)->CallObjectMethodA(bridge->env, object->object, method->method, jargs);
			result->type = AT_STRING;
			BRSTRING(result) = stReadJavaString(bridge, returnValue.l);
			result->retain = 0;
			break;
		case 'O':
			// 'O' is breve's code for an object
			returnValue.l = (*bridge->env)->CallObjectMethodA(bridge->env, object->object, method->method, jargs);
			result->type = AT_POINTER;
			BRPOINTER(result) = returnValue.l;
			break;
		default:
			slMessage(DEBUG_ALL, "error: undefined Java type (%c) returned from method \"\"\n", method->returnType);
			break;
	}

	// check for exception here
	
	if((*bridge->env)->ExceptionOccurred(bridge->env)) {
		slMessage(DEBUG_ALL, "Exception occured in Java execution of method \"%s\"\n", method->name);
		result->type = AT_NULL;
		return EC_ERROR;			
	}

	return EC_OK;
}

stInstance *stJavaObjectWrapper(brEngine *e, jobject object) {
	stJavaBridgeData *bridge = e->javaBridge;
	stJavaObjectData *objectData;
	stObject *o = stObjectFind(e->objects, JAVA_BRIDGE_CLASS_NAME);
	stInstance *i;
	jvalue jargs[JAVA_MAX_ARGS];
	brEval eval;

	if(!o) {
		slMessage(DEBUG_ALL, "Cannot locate Java bridge class \"%s\"\n", JAVA_BRIDGE_CLASS_NAME);
		return NULL;
	}
	
	if((i = slDehashData(bridge->objectHash, object))) return i;

	jargs[0].l = object;

	if(stJavaCallMethod(bridge, bridge->methodFinder, bridge->classnameMethod, jargs, &eval)) {
		return NULL;
	}

	printf("new object is of class %s\n", BRSTRING(&eval));

	objectData = slMalloc(sizeof(stJavaObjectData));

	objectData->class = stJavaFindClass(bridge, BRSTRING(&eval));
	objectData->object = object;
	
	if(!objectData->class) {
		slFree(object);
		return NULL;
	}
	
	i = stNewInstance(o);

	i->bridgeData = objectData;

	slHashData(bridge->objectHash, object, i);
	
	return i;

}

stJavaMethodData *stJavaFindMethod(stJavaBridgeData *bridge, stJavaClassData *class, char *name, unsigned char *types, int args) {
	slList *methods, *methodStart;
	stJavaMethodData *data;

	methodStart = methods = slDehashData(class->methodHash, name);

	while(methods) {
		// we should be checking argument types here also!

		data = methods->data;

		if(data->argumentCount == args) return data;

		methods = methods->next;
	}

	data = stJavaMakeMethod(bridge, class, name, types, args);

	methods = slListPrepend(methodStart, data);

	// if it hasn't already been hashed...

	if(!methodStart) slHashData(class->methodHash, slStrdup(name), methods);

	return data;
}

stJavaClassData *stJavaFindClass(stJavaBridgeData *bridge, char *name) {
	stJavaClassData *class;

	if(!(class = slDehashData(bridge->classHash, name))) {
		class = slMalloc(sizeof(stJavaClassData));

		class->class = (*bridge->env)->FindClass(bridge->env, name);

		if(!class->class) {
			slMessage(DEBUG_ALL, "Cannot locate Java class \"%s\"\n", name);
			return NULL;
		}

		class->methodHash = slNewHash(127, slHashString, slCompString);

		slHashData(bridge->classHash, slStrdup(name), class);
	}

	return class;
}

int stJNewObject(brEval args[], brEval *target, stInstance *i) {
	stJavaBridgeData *bridge = i->type->engine->javaBridge;
	char *classname = BRSTRING(&args[0]);
	brEvalListHead *head = BRLIST(&args[1]);
	int n = 0;

	for(n=0;n<strlen(classname);n++) if(classname[n] == '.') classname[n] = '/';

	if(!bridge) {
		bridge = i->type->engine->javaBridge = stAttachJavaVM(i->type->engine);
		if(!bridge) {
			slMessage(DEBUG_ALL, "Cannot initialize Java virtual machine\n");
			return EC_ERROR;
		}
	}

	i->bridgeData = stNewJavaObject(bridge, classname, head);

	if(!i->bridgeData) {
		slMessage(DEBUG_ALL, "Failed to instantiate Java class\n");
		return EC_ERROR;
	}

	slHashData(bridge->objectHash, i->bridgeData->object, i);

	return EC_OK;
}

int stJFreeObject(brEval *args, brEval *target, stInstance *i) {
	stJavaObjectData *object = i->bridgeData;

	slFree(object);

	return EC_OK;
}

stJavaObjectData *stNewJavaObject(stJavaBridgeData *bridge, char *name, brEvalListHead *args) {
	stJavaObjectData *object;
	jmethodID constructor;
	jvalue jargs[JAVA_MAX_ARGS];
	stJavaMethodData *methodData;
	brEvalList *list, *start;
	int n = 0, argCount;
	unsigned char types[JAVA_MAX_ARGS];

	if(args) {
		list = args->start;
		argCount = args->count;
	} else {
		list = NULL;
		argCount = 0;
	}

	object = slMalloc(sizeof(stJavaObjectData));

	object->class = stJavaFindClass(bridge, name);

	if(!object->class) return NULL;

	if(argCount != 0) {
		start = list;

		while(start) {
			types[n] = list->eval.type;
			start = start->next;
			n++;
		}

		methodData = stJavaFindMethod(bridge, object->class, "<init>", types, argCount);

		n = 0;

		while(list) {
			if(brEvalToJValue(bridge, &list->eval, &jargs[n], methodData->argumentTypes[n]) != EC_OK) {
				slMessage(DEBUG_ALL, "Error converting breve type to native Java type\n");
				return NULL;
			}

			n++;
			list = list->next;
		}

		constructor = methodData->method;
	} else {
		constructor = (*bridge->env)->GetMethodID(bridge->env, object->class->class, "<init>", "()V");
	}

	object->object = (*bridge->env)->NewObjectA(bridge->env, object->class->class, constructor, jargs);

	if(!object->object) {
		slFree(object);
		return NULL;
	}

	return object;
}

int stJCallMethod(brEval *args, brEval *target, stInstance *i) {
	stJavaBridgeData *bridge = i->type->engine->javaBridge;
	stJavaObjectData *object = i->bridgeData; 
	char *method = BRSTRING(&args[0]);
	brEvalListHead *arguments = BRLIST(&args[1]);
	brEvalList *start, *head = arguments->start;
	jvalue jargs[JAVA_MAX_ARGS];
	stJavaMethodData *methodData;
	int n = 0, result;
	unsigned char types[JAVA_MAX_ARGS];
	

	if(!bridge) {
		bridge = i->type->engine->javaBridge = stAttachJavaVM(i->type->engine);
		if(!bridge) {
			slMessage(DEBUG_ALL, "Cannot initialize Java virtual machine\n");
			return EC_ERROR;
		}
	}

	start = head;

	while(start) {
		types[n] = head->eval.type;
		n++;
		start = start->next;
	}

	methodData = stJavaFindMethod(bridge, object->class, method, types, arguments->count);

	if(!methodData) {
		slMessage(DEBUG_ALL, "Cannot find Java method \"%s\"\n", method);
		return EC_OK;
	}

	n = 0;

	while(head) {
		if(brEvalToJValue(bridge, &head->eval, &jargs[n], methodData->argumentTypes[n]) != EC_OK) {
			slMessage(DEBUG_ALL, "Error converting breve type to native Java type\n");
			return EC_OK;
		}

		n++;
		head = head->next;
	}

	result = stJavaCallMethod(bridge, object, methodData, jargs, target);

	if(methodData->returnType == 'O') {
		BRINSTANCE(target) = stJavaObjectWrapper(i->type->engine, BRPOINTER(target));
		target->type = AT_INSTANCE;
	}

	return result;
}

int brEvalToJValue(stJavaBridgeData *bridge, brEval *e, jvalue *v, char javaType) {
	switch(javaType) {
		case 'V':	
			e->type = AT_NULL;
			break;
		case 'I':
			v->i = BRINT(e);
			break;
		case 'D':
			v->d = BRDOUBLE(e);
			break;
		case 'F':
			v->f = BRDOUBLE(e);
			break;
		case 'T':
			v->l = stMakeJavaString(bridge, BRSTRING(e));
			break;
		case 'O':
			v->l = ((stJavaObjectData*)(BRINSTANCE(e)->bridgeData))->object;
			break;
		default:
			return EC_ERROR;
	}

	return EC_OK;
}

char stJTypeForType(unsigned char breveType) {
	switch(breveType) {
		case AT_NULL:
			return 'V';
			break;
		case AT_INT:
			return 'I';
			break;
		case AT_DOUBLE:
			return 'D';
			break;
		case AT_INSTANCE:
			return 'O';
			break;
		case AT_STRING:
			return 'L';
			break;
		default:
			return 'V';
			break;
	}

}

jstring stMakeJavaString(stJavaBridgeData *bridge, char *string) {
	jchar *characters;
	jstring s;
	int n;

	characters = slMalloc(sizeof(jchar) * (strlen(string) + 1));

	for(n=0;n<strlen(string);n++) characters[n] = string[n];

	s = (*bridge->env)->NewString(bridge->env, characters, strlen(string));

	slFree(characters);

	return s;
}

char *stReadJavaString(stJavaBridgeData *bridge, jstring string) {
	char *result;
	const jchar *characters;
	int n, length;

	length = (*bridge->env)->GetStringLength(bridge->env, string);

	result = slMalloc(length + 1);

	characters = (*bridge->env)->GetStringChars(bridge->env, string, NULL);

	for(n=0;n<length;n++) 
		result[n] = characters[n];

	result[n] = 0;

	(*bridge->env)->ReleaseStringChars(bridge->env, string, characters);

	return result;
}

void stFreeJavaMethodData(stJavaMethodData *method) {
	slFree(method->name);
	slFree(method);
}

void stFreeJavaBridgeData(stJavaBridgeData *bridge) {
	stFreeJavaMethodData(bridge->methfindMethod);
	stFreeJavaMethodData(bridge->methsigMethod);
	stFreeJavaMethodData(bridge->argtypesMethod);
	stFreeJavaMethodData(bridge->argcountMethod);
	stFreeJavaMethodData(bridge->rettypeMethod);
	stFreeJavaMethodData(bridge->classnameMethod);
	slFree(bridge->methodFinder);
	slFree(bridge);
}

void stInitJavaFuncs(stNamespace *n) {
	brNewBreveCall(n, "newJavaObject", stJNewObject, AT_NULL, AT_STRING, AT_LIST, 0);
	brNewBreveCall(n, "freeJavaObject", stJFreeObject, AT_NULL, 0);
	brNewBreveCall(n, "callJavaMethod", stJCallMethod, AT_UNDEFINED, AT_STRING, AT_LIST, 0);
}
