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

#include "java.h"

#ifdef HAVE_LIBJAVA

/*!
	\brief the callMethod field of the java brObjectType.
*/

int brJavaMethodCallCallback(void *instanceData, void *methodData, brEval **args, brEval *result) {
	jvalue jargs[JAVA_MAX_ARGS];
	brJavaMethod *method = methodData;
	brJavaInstance *instance = instanceData;
	int n;

	for(n=0;n<method->argumentCount;n++)
		brEvalToJValue(instance->object->bridge, args[n], &jargs[n], method->argumentTypes[n]);

	return brJavaMethodCall(instance->object->bridge, instance, method, jargs, result);
}

/*!
	\brief the findMethod field of the java brObjectType.
*/

brJavaObject *brJavaObjectFindCallback(void *typeData, char *name) {
	brJavaBridgeData *bridge = typeData;
	return brJavaObjectFind(bridge, name);
}

/*!
	\brief the findMethod field of the java brObjectType.
*/

brJavaMethod *brJavaMethodFindCallback(void *objectData, char *name, unsigned char *types, int tCount) {
	brJavaObject *object = objectData;
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

brJavaInstance *brJavaInstanceNewCallback(void *objectData, brEval **args, int argCount) {
	return brJavaInstanceNew(objectData, args, argCount);
}

/*!
	\brief the destroyInstance field of the java brObjectType.
*/

void brJavaInstanceDestroyCallback(void *instance) {

}

/*!
	\brief Initializes the Java VM and the Java brObjectType.
*/

void brJavaInit(brEngine *e) {
	brObjectType *javaObjectType;

	javaObjectType = slMalloc(sizeof(brObjectType));

	javaObjectType->callMethod = brJavaMethodCallCallback;
	javaObjectType->findMethod = brJavaMethodFindCallback;
	javaObjectType->findObject = brJavaObjectFindCallback;
	javaObjectType->isSubclass = brJavaIsSubclassCallback;
	javaObjectType->instantiate = brJavaInstanceNewCallback;
	javaObjectType->destroyInstance = brJavaInstanceDestroyCallback;

	javaObjectType->userData = brAttachJavaVM(e);

	if(javaObjectType->userData) brEngineRegisterObjectType(e, javaObjectType);
}

void brJavaFree(brEngine *e) {

}
#endif
