#include <stdio.h>
#include <stdlib.h>

#include <jni.h>

#include "kernel.h"

#define JAVA_MAX_ARGS	32

#define JAVA_BRIDGE_CLASS_NAME	"JavaObject"

// public Method findMethod(Class type, String name, int arguments);
#define METHFIND_NAME			"findMethod"
#define METHFIND_SIGNATURE		"(Ljava/lang/Class;Ljava/lang/String;I[C)Ljava/lang/Object;"

// public String getMethodSignature(Method method)
#define METHSIG_NAME			"getSignature"
#define METHSIG_SIGNATURE		"(Ljava/lang/Object;)Ljava/lang/String;"

// public char getReturnType(Method method)
#define RETTYPE_NAME			"getReturnType"
#define RETTYPE_SIGNATURE		"(Ljava/lang/Object;)C"

// public char getArgumentTypeAtIndex(Method method, int index)
#define ARGTYPES_NAME			"getArgumentTypeAtIndex"
#define ARGTYPES_SIGNATURE		"(Ljava/lang/Object;I)C"

// public int getArgumentCount(Method method)
#define ARGCOUNT_NAME			"getArgumentCount"
#define ARGCOUNT_SIGNATURE		"(Ljava/lang/Object;)I"

// public String getClassName(Class class)
#define CLASSNAME_NAME			"getClassName"
#define CLASSNAME_SIGNATURE		"(Ljava/lang/Object;)Ljava/lang/String;"

struct stJavaClassData {
	jclass class;
	slHash *methodHash;
};

typedef struct stJavaClassData stJavaClassData;

struct stJavaObjectData {
	stJavaClassData *class;
	jobject object;
};

typedef struct stJavaObjectData stJavaObjectData;

struct stJavaMethodData {
	char *name;
	jmethodID method;
	char returnType;
	char argumentTypes[JAVA_MAX_ARGS];
	int argumentCount;
};

typedef struct stJavaMethodData stJavaMethodData;

struct stJavaBridgeData {
	stJavaObjectData *methodFinder;

	stJavaMethodData *methfindMethod;
	stJavaMethodData *methsigMethod;
	stJavaMethodData *argtypesMethod;
	stJavaMethodData *argcountMethod;
	stJavaMethodData *rettypeMethod;
	stJavaMethodData *classnameMethod;

	slHash *objectHash;
	slHash *classHash;

	JavaVM *jvm;
	JNIEnv *env;
};

typedef struct stJavaBridgeData stJavaBridgeData;

void stFreeJavaClassData(stJavaClassData *data);
void stFreeJavaBridgeData(stJavaBridgeData *data);
void stFreeJavaMethodData(stJavaMethodData *data);

stJavaObjectData *stNewJavaObject(stJavaBridgeData *bridge, char *name, brEvalListHead *args);
int stJavaCallMethod(stJavaBridgeData *bridge, stJavaObjectData *object, stJavaMethodData *method, jvalue *jargs, brEval *result);

stJavaBridgeData *stAttachJavaVM(brEngine *e);
void stDetachJavaVM(stJavaBridgeData *bridge);

void brInitJavaFuncs(brNamespace *n);

stJavaMethodData *stJavaFindMethod(stJavaBridgeData *bridge, stJavaClassData *class, char *name, unsigned char *types, int nargs);
stJavaMethodData *stJavaMakeMethod(stJavaBridgeData *bridge, stJavaClassData *class, char *name, unsigned char *types, int nargs);
stJavaMethodData *stJavaMakeMethodData(char *name, jmethodID method, char returnType, char *argumentTypes, int nargs);

int brEvalToJValue(stJavaBridgeData *bridge, brEval *e, jvalue *v, char javaType);

stJavaClassData *stJavaFindClass(stJavaBridgeData *bridge, char *name);

char *stReadJavaString(stJavaBridgeData *bridge, jstring string);
jstring stMakeJavaString(stJavaBridgeData *bridge, char *string);

brInstance *stJavaObjectWrapper(brEngine *e, jobject object);

char stJTypeForType(unsigned char breve_type);
