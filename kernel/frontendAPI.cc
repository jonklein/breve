#include "kernel.h"

int brLoadSimulation( brEngine *engine, const char *code, const char *file ) {
	int result = EC_OK;

	result = brLoadFile( engine, code, file );

	if( result == EC_OK && !engine->controller ) {
		brEvalError( engine, PE_NO_CONTROLLER, "No controller object has been define for simulation \"%s\"\n", file );
		return EC_ERROR;
	}

	return result;
}

int brLoadFile( brEngine *engine, const char *code, const char *file ) {
	char *extension = slFileExtension( file );

	// Add a search path for the directory we're loading from, in case this 
	// file wants to load resources from the same directory.
	// warning: dirname may modify contents -- jerks

	char *dir = slDirname( file );
	brAddSearchPath( engine, dir );
	slFree( dir );

	for( unsigned int n = 0; n < engine->_objectTypes.size(); n++ ) {
		brObjectType *type = engine->_objectTypes[ n ];

		if( type->load && type->canLoad && type->canLoad( type->userData, extension ) ) {
			int r = type->load( engine, type->userData, file, code );

			slFree( extension );

			if( r != EC_OK ) 
				return EC_ERROR;

			return EC_OK;
		}		

	}

	slMessage( DEBUG_ALL, "Could not locate breve language frontend which understands \"%s\" files\n", extension );

	slFree( extension );

	return EC_ERROR;
}

int brLoadSavedSimulation( brEngine *inEngine, const char *code, const char *file, const char *inArchive ) {
	char *extension = slFileExtension( file );

	for( unsigned int n = 0; n < inEngine->_objectTypes.size(); n++ ) {
		brObjectType *type = inEngine->_objectTypes[ n ];

		if( type->load && type->canLoad && type->canLoad( type->userData, extension ) ) {
			int r = type->loadWithArchive( inEngine, type->userData, file, code, inArchive );

			if( r != EC_OK ) 
				return EC_ERROR;

			return EC_OK;
		}		

	}

	slMessage( DEBUG_ALL, "Could not locate breve language frontend which understands \"%s\" files\n", extension );

	return EC_ERROR;
}
