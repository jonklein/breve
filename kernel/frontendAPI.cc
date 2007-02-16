#include "kernel.h"

#include <libgen.h>

int brLoadSimulation( brEngine *engine, const char *code, const char *file ) {
	int result = EC_OK;

	result = brLoadFile( engine, code, file );

	if( result == EC_OK && !engine->controller ) {
		slMessage( DEBUG_ALL, "No controller object has been define for simulation \"%s\"\n", file );
		return EC_ERROR;
	}

	return result;
}

int brLoadFile( brEngine *engine, const char *code, const char *file ) {
	char *extension = slFileExtension( file );

	for( unsigned int n = 0; n < engine->objectTypes.size(); n++ ) {
		brObjectType *type = engine->objectTypes[ n ];

		if( type->load && type->canLoad && type->canLoad( type->userData, extension ) ) {
			int r = type->load( engine, type->userData, file, code );

			if( r != EC_OK ) 
				return EC_ERROR;

			// dirname may modify contents?!
			// char *copy = slStrdup( file );
			// brAddSearchPath( engine, dirname( copy ) );
			// slFree( copy );

			return EC_OK;
		}		

	}

	slMessage( DEBUG_ALL, "Could not locate breve language frontend which understands \"%s\" files\n", extension );

	return EC_ERROR;
}

int brLoadSavedSimulation( brEngine *engine, const char *code, const char *file, const char *xmlfile ) {
	char *extension = slFileExtension( file );

	for( unsigned int n = 0; n < engine->objectTypes.size(); n++ ) {
		brObjectType *type = engine->objectTypes[ n ];

		if( type->load && type->canLoad && type->canLoad( type->userData, extension ) ) {
			int r = type->loadWithArchive( engine, type->userData, file, code, xmlfile );

			if( r != EC_OK ) 
				return EC_ERROR;

			return EC_OK;
		}		

	}

	slMessage( DEBUG_ALL, "Could not locate breve language frontend which understands \"%s\" files\n", extension );

	return EC_ERROR;
}
