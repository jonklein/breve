#include "kernel.h"

#ifdef HAVE_LIBPERL

#include "perlInterface.h"

void *brPerlSetController( void *inSelf, void *inArgs ) {
	slMessage(DEBUG_ALL,"Test setting the controller.\n");
/*	PyObject *moduleObject, *pythonInstance;

	if ( !PyArg_ParseTuple( inArgs, "OO", &moduleObject, &pythonInstance) ) return NULL;

	PyObject *engineObject = PyObject_GetAttrString( moduleObject, "breveEngine" );
	PyObject *instanceObject = PyObject_GetAttrString( pythonInstance, "breveInstance" );

	brEngine *engine = (brEngine*)PyCObject_AsVoidPtr( engineObject );
	brInstance *instance = (brInstance*)PyCObject_AsVoidPtr( instanceObject );

        brEngineSetController( engine, instance );


	Py_DECREF( engineObject );
	Py_DECREF( instanceObject );

	Py_INCREF( Py_None );
	return Py_None;*/
	return NULL;
}

void blahblah() {
	printf("blahblah() called.\n");
}

#endif
