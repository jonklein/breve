#include "steve.h"
#include "xml.h"

/*!
	\brief Archives an entire simulation to a file.
*/

int stCWriteXMLEngine( brEval args[], brEval *target, brInstance *i ) {
	char *filename = BRSTRING( &args[0] );
	char *path = brOutputPath( i->engine, filename );

	target->set( brXMLWriteSimulationToFile( path, i->engine ) );

	slFree( path );

	return EC_OK;
}

/*!
	\brief Archives an instance to a string.
*/

int stCArchiveXMLObject( brEval args[], brEval *target, brInstance *i ) {
	char *filename = BRSTRING( &args[1] );
	char *path = brOutputPath( i->engine, filename );

	target->set( brXMLWriteObjectToFile( BRINSTANCE( &args[0] ), path, 0 ) );

	slFree( path );

	return EC_OK;
}

/*!
	\brief Dearchives and returns an instance from a file.
*/

int stCDearchiveXMLObject( brEval args[], brEval *target, brInstance *i ) {
	char *filename = brFindFile( i->engine, BRSTRING( &args[0] ), NULL );
	stInstance *si;

	if ( !filename ) {
		slMessage( DEBUG_ALL, "Cannot locate file \"%s\" for object dearchive\n", BRSTRING( &args[0] ) );

		target->set(( brInstance* )NULL );

		return EC_OK;
	}

	si = stXMLDearchiveObjectFromFile( i->engine, filename );

	if ( !si ) {
		slMessage( DEBUG_ALL, "error decoding XML message from file\n" );

		target->set(( brInstance* )NULL );

		return EC_ERROR;
	}

	target->set( si->breveInstance );

	slFree( filename );

	return EC_OK;
}

/*!
	\brief Dearchives and returns an instance from an XML string.
*/

int stCDearchiveXMLObjectFromString( brEval args[], brEval *target, brInstance *i ) {
	stInstance *si = stXMLDearchiveObjectFromString( i->engine, BRSTRING( &args[0] ) );

	if ( !si ) {
		slMessage( DEBUG_ALL, "error decoding XML message from string\n" );

		target->set(( brInstance* )NULL );

		return EC_ERROR;
	}

	target->set( si->breveInstance );

	return EC_OK;
}

void breveInitXMLFuncs( brNamespace *n ) {
	brNewBreveCall( n, "writeXMLEngine", stCWriteXMLEngine, AT_INT, AT_STRING, 0 );
	brNewBreveCall( n, "archiveXMLObject", stCArchiveXMLObject, AT_INT, AT_INSTANCE, AT_STRING, 0 );
	brNewBreveCall( n, "dearchiveXMLObject", stCDearchiveXMLObject, AT_INSTANCE, AT_STRING, 0 );
	brNewBreveCall( n, "dearchiveXMLObjectFromString", stCDearchiveXMLObjectFromString, AT_INSTANCE, AT_STRING, 0 );
}
