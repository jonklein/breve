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

brObjectType gJavaObjectType;
brJavaBridgeData *gJavaBridge;

/*!
	\brief the callMethod field of the java brObjectType.
*/

int brJavaMethodCallCallback(brInstance *i, brMethod *m, brEval **args, brEval *result) {
	jvalue jargs[JAVA_MAX_ARGS];
	brJavaMethod *method = m->pointer;
	brJavaInstance *instance = i->pointer;
	int n;

	for(n=0;n<method->argumentCount;n++)
		brEvalToJValue(instance->object->bridge, args[n], &jargs[n], method->argumentTypes[n]);

	return brJavaMethodCall(instance->object->bridge, instance, method, jargs, result);
}

/*!
	\brief the findMethod field of the java brObjectType.
*/

brJavaObject *brJavaObjectFindCallback(brObjectType *type, char *name) {
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

	if(gJavaBridge) brEngineRegisterObjectType(e, &gJavaObjectType);
}
#endif
