#include "javaFunctions.h"

brObjectType gJavaObjectType;
brJavaBridgeData *gJavaBridge;

/*!
	\brief the callMethod field of the java brObjectType.
*/

int brJavaMethodCallCallback(brInstance *i, brMethod *m, brEval **args, brEval *result) {
	jvalue jargs[JAVA_MAX_ARGS];
	brJavaMethod *method = m->pointer;
	brJavaInstance *instance = i->pointer;

	return brJavaMethodCall(instance->object->bridge, instance, method, jargs, result);
}

/*!
	\brief the findMethod field of the java brObjectType.
*/

brJavaMethod *brJavaObjectFindCallback(brObjectType *type, char *name) {
	brJavaBridgeData *bridge = type->data;
	return brJavaObjectFind(bridge, name);
}

/*!
	\brief the findMethod field of the java brObjectType.
*/

brJavaMethod *brJavaMethodFindCallback(brObject *o, char *name, unsigned char *types, int tCount) {
	brJavaObject *object = o->pointer;
	return brJavaMethodFind(object->bridge, object, name, types, tCount);
}

/*!
	\brief the isSubclass field of the java brObjectType.
*/

brJavaMethod *brJavaIsSubclassCallback(brObject *class1, brObject *class2) {
	return 0;
}

/*!
	\brief the instantiate field of the java brObjectType.
*/

brJavaInstance *brJavaInstanceNewCallback(brObject *object, brEval **args, int argCount) {
	return brJavaInstanceNew(object->pointer, args, argCount);
}

/*!
	\brief the destroyInstance field of the java brObjectType.
*/

void brJavaInstanceDestroyCallback(brObject *object, brEval **args, int argCount) {

}

/*!
	\brief Initializes the Java VM and the Java brObjectType.
*/

void brJavaInit(brEngine *e) {
	gJavaObjectType.callMethod = brJavaMethodCallCallback;
	gJavaObjectType.findMethod = brJavaMethodFindCallback;
	gJavaObjectType.findObject = brJavaObjectFindCallback;
	gJavaObjectType.isSubclass = brJavaIsSubclassCallback;
	gJavaObjectType.instantiate = brJavaInstanceNewCallback;
	gJavaObjectType.destroyInstance = brJavaInstanceDestroyCallback;

	gJavaBridge = brAttachJavaVM(e);

	gJavaObjectType.data = gJavaBridge;

	brEngineRegisterObjectType(e, &gJavaObjectType);
}
