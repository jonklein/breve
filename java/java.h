#include <stdio.h>
#include <stdlib.h>

#include "kernel.h"
#include "util.h"

#ifdef HAVE_LIBJAVA
#include <jni.h>

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

// public String getClassName(Class object)
#define CLASSNAME_NAME			"getClassName"
#define CLASSNAME_SIGNATURE		"(Ljava/lang/Object;)Ljava/lang/String;"

typedef struct brJavaBridgeData brJavaBridgeData;
typedef struct brJavaObject brJavaObject;
typedef struct brJavaMethod brJavaMethod;
typedef struct brJavaInstance brJavaInstance;

struct brJavaObject {
	brJavaBridgeData *bridge;
	jclass object;
};

struct brJavaInstance {
	brJavaObject *object;
	jobject instance;
};

struct brJavaMethod {
	char *name;
	jmethodID method;
	char returnType;
	char argumentTypes[JAVA_MAX_ARGS];
	int argumentCount;
};

struct brJavaBridgeData {
	brObjectType javaObjectType;

	brJavaInstance *methodFinder;

	brJavaMethod *methfindMethod;
	brJavaMethod *methsigMethod;
	brJavaMethod *argtypesMethod;
	brJavaMethod *argcountMethod;
	brJavaMethod *rettypeMethod;
	brJavaMethod *objectnameMethod;

	slHash *objectHash;

	JavaVM *jvm;
	JNIEnv *env;
};

brJavaInstance *brJavaBootstrapMethodFinder(brJavaObject *object);

brJavaMethod *brJavaMethodFind(brJavaBridgeData *bridge, brJavaObject *object, char *name, unsigned char *types, int nargs);

void brFreeJavaClassData(brJavaObject *data);
void brFreeJavaBridgeData(brJavaBridgeData *data);
void brFreeJavaMethodData(brJavaMethod *data);

brJavaInstance *brJavaInstanceNew(brJavaObject *o, brEval **args, int argCount);

int brJavaCallMethod(brInstance *i, brMethod *m, brEval **args, brEval *result);

brJavaBridgeData *brAttachJavaVM(brEngine *e);
void brDetachJavaVM(brJavaBridgeData *bridge);

void brInitJavaFuncs(brNamespace *n);

brJavaMethod *brJavaMakeMethod(brJavaBridgeData *bridge, brJavaObject *object, char *name, unsigned char *types, int nargs);
brJavaMethod *brJavaMakeMethodData(char *name, jmethodID method, char returnType, char *argumentTypes, int nargs);

int brEvalToJValue(brJavaBridgeData *bridge, brEval *e, jvalue *v, char javaType);

brJavaObject *brJavaObjectFind(brJavaBridgeData *bridge, char *name);

char *brReadJavaString(brJavaBridgeData *bridge, jstring string);
jstring brMakeJavaString(brJavaBridgeData *bridge, char *string);

brInstance *brJavaObjectWrapper(brEngine *e, jobject object);

char brJTypeForType(unsigned char breve_type);
#endif
