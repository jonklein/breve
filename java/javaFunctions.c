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

#include "javaFunctions.h"

extern brJavaBridgeData *gJavaBridge;

#define USER_CLASSPATH "."

brJavaBridgeData *brAttachJavaVM(brEngine *e) {
	JavaVMInitArgs vm_args;
	JavaVMOption options[2];
	brJavaObject *finderObject;
	jint res;
	jmethodID method;
	char args[JAVA_MAX_ARGS];
	char *optstr;
	char *finder;
	char *classPath;

	brJavaBridgeData *bridge;

	finder = brFindFile(e, "MethodFinder.jar", NULL);

	if(!finder) {
		slMessage(DEBUG_ALL, "Java loading failed: cannot locate the breve Java archive files\n");
		return NULL;
	}

	bridge = slMalloc(sizeof(brJavaBridgeData));

	bridge->objectHash = slNewHash(1027, slHashPointer, slCompPointer);
	bridge->objectHash = slNewHash(1027, slHashString, slCompString);

	classPath = getenv("BREVE_CLASS_PATH");

	if(!classPath) classPath = "";

	optstr = malloc(strlen(finder) + strlen(classPath) + strlen("-Djava.object.path=") + 1024);

	sprintf(optstr, "-Djava.object.path=%s:%s", classPath, finder);
	printf("%s\n", optstr);

	JNI_GetDefaultJavaVMInitArgs(&vm_args);

	options[0].optionString = optstr;
	options[1].optionString = "-verbose:jni,class";
	vm_args.version = JNI_VERSION_1_2;
	vm_args.options = options;
	vm_args.nOptions = 2;
	vm_args.ignoreUnrecognized = JNI_FALSE;

	// Create the Java VM 
	res = JNI_CreateJavaVM(&bridge->jvm, (void**)&bridge->env, &vm_args);

	free(optstr);

	if (res < 0) {
		slMessage(DEBUG_ALL, "Can't create JavaVM\n");
		slFree(bridge);
		return NULL;
	}

	finderObject = brJavaObjectFind(bridge, "MethodFinder");

	if(!finderObject) {
		slMessage(DEBUG_ALL, "Cannot instantiate MethodFinder class\n");
		return NULL;
	}

	bridge->methodFinder = brJavaBootstrapMethodFinder(finderObject);

	if(!bridge->methodFinder) {
		slMessage(DEBUG_ALL, "Cannot instantiate MethodFinder class\n");
		slFree(bridge);
		return NULL;
	}

	// method finder

	args[0] = 'O'; args[1] = 'O'; args[2] = 'I'; args[3] = 'O';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->object->object, METHFIND_NAME, METHFIND_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", METHFIND_NAME);
		return NULL;
	}
	bridge->methfindMethod = brJavaMakeMethodData(METHFIND_NAME, method, 'O', args, 4);

	// method signature finder

	args[0] = 'O';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->object->object, METHSIG_NAME, METHSIG_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", METHSIG_NAME);
		return NULL;
	}
	bridge->methsigMethod = brJavaMakeMethodData(METHSIG_NAME, method, 'T', args, 1);

	args[0] = 'O';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->object->object, RETTYPE_NAME, RETTYPE_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", RETTYPE_NAME);
		return NULL;
	}
	bridge->rettypeMethod = brJavaMakeMethodData(RETTYPE_NAME, method, 'C', args, 2);

	args[0] = 'O'; args[1] = 'I';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->object->object, ARGTYPES_NAME, ARGTYPES_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", ARGTYPES_NAME);
		return NULL;
	}
	bridge->argtypesMethod = brJavaMakeMethodData(ARGTYPES_NAME, method, 'C', args, 2);

	args[0] = 'O';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->object->object, ARGCOUNT_NAME, ARGCOUNT_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", ARGCOUNT_NAME);
		return NULL;
	}
	bridge->argcountMethod = brJavaMakeMethodData(ARGCOUNT_NAME, method, 'I', args, 1);

	args[0] = 'O';
	method = (*bridge->env)->GetMethodID(bridge->env, bridge->methodFinder->object->object, CLASSNAME_NAME, CLASSNAME_SIGNATURE);
	if(!method) {
		slMessage(DEBUG_ALL, "Cannot locate Java method \"%s\" for Java bridge\n", CLASSNAME_NAME);
		return NULL;
	}
	bridge->objectnameMethod = brJavaMakeMethodData(CLASSNAME_NAME, method, 'T', args, 1);

	slFree(finder);

	return bridge;
}

void brDetachJavaVM(brJavaBridgeData *bridge) {
	brFreeJavaBridgeData(bridge);
}

void brFreeJavaClassData(brJavaObject *data) {
	slFree(data);
}

brJavaMethod *brJavaMethodFind(brJavaBridgeData *bridge, brJavaObject *object, char *name, unsigned char *types, int nargs) {
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

	for(count=0;count<nargs;count++) cargTypes[count] = brJTypeForType(types[count]);

	(*bridge->env)->SetCharArrayRegion(bridge->env, array, 0, nargs, cargTypes);

	// find the method with this name and arg counts for this object
	args[0].l = object->object;
	args[1].l = brMakeJavaString(bridge, name);
	args[2].i = nargs;
	args[3].l = array;
	error = brJavaMethodCall(bridge, bridge->methodFinder, bridge->methfindMethod, args, &result);
	methodPtr = BRPOINTER(&result);

	if(error != EC_OK) return NULL;

	// find the proper signature for the method
	args[0].l = methodPtr;
	error = brJavaMethodCall(bridge, bridge->methodFinder, bridge->methsigMethod, args, &result);
	signature = BRSTRING(&result);

	if(error != EC_OK) return NULL;

	methodID = (*bridge->env)->GetMethodID(bridge->env, object->object, name, signature);
	slFree(signature);

	// find the return type for the method
	args[0].l = methodPtr;
	error = brJavaMethodCall(bridge, bridge->methodFinder, bridge->rettypeMethod, args, &result);
	returnType = BRINT(&result);

	if(error != EC_OK) return NULL;

	// fill in the argument types
	for(count=0;count<nargs;count++) {
		args[0].l = methodPtr;
		args[1].i = count;

		error = brJavaMethodCall(bridge, bridge->methodFinder, bridge->argtypesMethod, args, &result);
		argumentTypes[count] = BRINT(&result);

		if(error != EC_OK) return NULL;
	}

	return brJavaMakeMethodData(name, methodID, returnType, argumentTypes, nargs);
}

brJavaMethod *brJavaMakeMethodData(char *name, jmethodID method, char returnType, char *argumentTypes, int nargs) {
	brJavaMethod *data;
	int n;

	data = slMalloc(sizeof(brJavaMethod));

	data->method = method;
	data->argumentCount = nargs;
	data->returnType = returnType;
	data->name = slStrdup(name);

	for(n=0;n<nargs;n++) data->argumentTypes[n] = argumentTypes[n];

	return data;
}

int brJavaMethodCall(brJavaBridgeData *bridge, brJavaInstance *instance, brJavaMethod *method, jvalue *jargs, brEval *result) {
	jvalue returnValue;

	switch(method->returnType) {
		case 'V':
			(*bridge->env)->CallVoidMethodA(bridge->env, instance->instance, method->method, jargs);
			result->type = AT_NULL;
			break;
		case 'I':
			returnValue.i = (*bridge->env)->CallIntMethodA(bridge->env, instance->instance, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.i;
			break;
		case 'J':
			returnValue.j = (*bridge->env)->CallLongMethodA(bridge->env, instance->instance, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.j;
			break;
		case 'C':
			returnValue.c = (*bridge->env)->CallCharMethodA(bridge->env, instance->instance, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.c;
			break;
		case 'B':
			returnValue.b = (*bridge->env)->CallByteMethodA(bridge->env, instance->instance, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.b;
			break;
		case 'Z':
			returnValue.z = (*bridge->env)->CallBooleanMethodA(bridge->env, instance->instance, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.z;
			break;
		case 'S':
			returnValue.s = (*bridge->env)->CallShortMethodA(bridge->env, instance->instance, method->method, jargs);
			result->type = AT_INT;
			BRINT(result) = returnValue.s;
			break;
		case 'F':
			returnValue.f = (*bridge->env)->CallFloatMethodA(bridge->env, instance->instance, method->method, jargs);
			result->type = AT_DOUBLE;
			BRDOUBLE(result) = returnValue.f;
			break;
		case 'D':
			returnValue.d = (*bridge->env)->CallFloatMethodA(bridge->env, instance->instance, method->method, jargs);
			result->type = AT_DOUBLE;
			BRDOUBLE(result) = returnValue.d;
			break;
		case 'T':
			// 'T' is breve's code for a string object
			returnValue.l = (*bridge->env)->CallObjectMethodA(bridge->env, instance->instance, method->method, jargs);
			result->type = AT_STRING;
			BRSTRING(result) = brReadJavaString(bridge, returnValue.l);
			break;
		case 'O':
			// 'O' is breve's code for an object
			returnValue.l = (*bridge->env)->CallObjectMethodA(bridge->env, instance->instance, method->method, jargs);
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

brJavaObject *brJavaObjectFind(brJavaBridgeData *bridge, char *name) {
	brJavaObject *object;

	if(!(object = slDehashData(bridge->objectHash, name))) {
		object = slMalloc(sizeof(brJavaObject));

		object->object = (*bridge->env)->FindClass(bridge->env, name);

		if(!object->object) {
			slMessage(DEBUG_ALL, "Cannot locate Java object \"%s\"\n", name);
			return NULL;
		}

		object->bridge = bridge;
	}

	return object;
}

int brJavaInstanceFree(brInstance *i) {
	brJavaInstance *instance = i->pointer;

	slFree(i);

	return EC_OK;
}

brJavaInstance *brJavaBootstrapMethodFinder(brJavaObject *object) {
	brJavaInstance *instance = slMalloc(sizeof(brJavaInstance));

	jmethodID method = (*object->bridge->env)->GetMethodID(object->bridge->env, object->object, "<init>", "()V"); 

	instance->object = object;
	instance->instance = (*object->bridge->env)->NewObjectA(object->bridge->env, object->object, method, NULL);

	return instance;
}

brJavaInstance *brJavaInstanceNew(brJavaObject *object, brEval **args, int argCount) {
	brJavaInstance *instance;
	jvalue jargs[JAVA_MAX_ARGS];
	brJavaMethod *method;
	unsigned char types[JAVA_MAX_ARGS];
	int n;

	instance = slMalloc(sizeof(brJavaInstance));

	instance->object = object;

	for(n=0;n<argCount;n++) types[n] = args[n]->type;

	method = brJavaMethodFind(object->bridge, instance->object, "<init>", types, argCount);

	for(n=0;n<argCount;n++) {
		if(brEvalToJValue(object->bridge, args[n], &jargs[n], method->argumentTypes[n]) != EC_OK) {
			slMessage(DEBUG_ALL, "Error converting breve type to native Java type\n");
			return NULL;
		}
	}

	instance->instance = (*object->bridge->env)->NewObjectA(object->bridge->env, object->object, method->method, jargs);

	if(!instance->instance) {
		slFree(instance);
		return NULL;
	}

	return instance;
}

int brEvalToJValue(brJavaBridgeData *bridge, brEval *e, jvalue *v, char javaType) {
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
			v->l = brMakeJavaString(bridge, BRSTRING(e));
			break;
		case 'O':
			v->l = ((brJavaInstance*)(BRINSTANCE(e)->pointer))->instance;
			break;
		default:
			return EC_ERROR;
	}

	return EC_OK;
}

char brJTypeForType(unsigned char breveType) {
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

jstring brMakeJavaString(brJavaBridgeData *bridge, char *string) {
	jchar *characters;
	jstring s;
	int n;

	characters = slMalloc(sizeof(jchar) * (strlen(string) + 1));

	for(n=0;n<strlen(string);n++) characters[n] = string[n];

	s = (*bridge->env)->NewString(bridge->env, characters, strlen(string));

	slFree(characters);

	return s;
}

char *brReadJavaString(brJavaBridgeData *bridge, jstring string) {
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

void brFreeJavaMethodData(brJavaMethod *method) {
	slFree(method->name);
	slFree(method);
}

void brFreeJavaBridgeData(brJavaBridgeData *bridge) {
	brFreeJavaMethodData(bridge->methfindMethod);
	brFreeJavaMethodData(bridge->methsigMethod);
	brFreeJavaMethodData(bridge->argtypesMethod);
	brFreeJavaMethodData(bridge->argcountMethod);
	brFreeJavaMethodData(bridge->rettypeMethod);
	brFreeJavaMethodData(bridge->objectnameMethod);
	slFree(bridge->methodFinder);
	slFree(bridge);
}
