#include "kernel.h"

int brLoadSimulation( brEngine *engine, const char *code, const char *file ) {
	int result = EC_OK;

	result = brLoadFile( engine, code, file );

	if( result == EC_OK && !engine->controller ) {
		slMessage( DEBUG_ALL, "No controller object has been loaded for file \"%s\"\n", file );
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

			return EC_OK;
		}		

	}

	slMessage( DEBUG_ALL, "Could not locate breve language frontend which understands \"%s\" files\n", extension );

	return EC_ERROR;
}

int brLoadSavedSimulation( brEngine *engine, const char *simcode, const char *simfile, const char *xmlfile ) {
	// return stLoadSavedSimulation( NULL, engine, simcode, simfile, xmlfile );
}
