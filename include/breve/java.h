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

	std::map< std::string, brJavaObject* > objects;

	JavaVM *jvm;
	JNIEnv *env;
};

#ifdef __cplusplus
extern "C" {
#endif
void brJavaInit(brEngine *e);

brJavaInstance *brJavaBootstrapMethodFinder(brJavaObject *);

brJavaMethod *brJavaMethodFind(brJavaBridgeData *, brJavaObject *, char *, unsigned char *, unsigned int);

void brFreeJavaClassData(brJavaObject *data);
void brFreeJavaBridgeData(brJavaBridgeData *data);
void brFreeJavaMethodData(brJavaMethod *data);

brJavaInstance *brJavaInstanceNew(brJavaObject *, brEval **, int);

int brJavaCallMethod(brInstance *, brMethod *, brEval **, brEval *);

brJavaBridgeData *brAttachJavaVM(brEngine *);
void brDetachJavaVM(brJavaBridgeData *);

void brInitJavaFuncs(brNamespace *);

brJavaMethod *brJavaMakeMethod(brJavaBridgeData *, brJavaObject *, char *, unsigned char *, int);
brJavaMethod *brJavaMakeMethodData(char *, jmethodID, char, char *, int);

int brJavaMethodCall(brJavaBridgeData *, brJavaInstance *, brJavaMethod *, jvalue *, brEval *);

void *brJavaMethodFindCallback(void *, char *, unsigned char *, int);
void *brJavaObjectFindCallback(void *, char *);
void *brJavaInstanceNewCallback(void *, brEval **, int);
int brJavaMethodCallCallback(void *, void *, brEval **, brEval *);
int brJavaIsSubclassCallback(void *, void *);
void brJavaInstanceDestroyCallback(void *);

int brEvalToJValue(brJavaBridgeData *, brEval *, jvalue *, char);

brJavaObject *brJavaObjectFind(brJavaBridgeData *bridge, char *name);

char *brReadJavaString(brJavaBridgeData *, jstring);
jstring brMakeJavaString(brJavaBridgeData *, char *);

brInstance *brJavaObjectWrapper(brEngine *, jobject);

char brJTypeForType(unsigned char);
#ifdef __cplusplus
}
#endif
#endif
