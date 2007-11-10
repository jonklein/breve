#include "steve.h"
#include "xml.h"

/**
 * \brief Archives an entire simulation to a file.
 */

int stCWriteXMLEngine( brEval args[], brEval *target, brInstance *i ) {
	char *filename = BRSTRING( &args[0] );
	char *path = brOutputPath( i->engine, filename );

	target->set( brXMLWriteSimulationToFile( path, i->engine ) );

	slFree( path );

	return EC_OK;
}

/**
 * \brief Archives an instance to a string.
 */

int brIXMLArchiveObjectToFile( brEval args[], brEval *target, brInstance *i ) {
	char *filename = BRSTRING( &args[1] );
	char *path = brOutputPath( i->engine, filename );

	target->set( brXMLWriteObjectToFile( BRINSTANCE( &args[0] ), path, 0 ) );

	slFree( path );

	return EC_OK;
}

/*!
 * \brief Archives an instance to a string.
 */

int brIXMLArchiveObjectToString( brEval args[], brEval *target, brInstance *i ) {
	slStringStream *stream = slOpenStringStream();

	brXMLWriteObjectToStream( BRINSTANCE( &args[0] ), stream -> fp, 0 );

	target->set( slCloseStringStream( stream ) );

	return EC_OK;
}


/**
 * \brief Dearchives and returns an instance from a file.
 */

int stCDearchiveXMLObject( brEval args[], brEval *target, brInstance *i ) {
	char *filename = brFindFile( i->engine, BRSTRING( &args[0] ), NULL );
	brInstance *instance;

	if ( !filename ) {
		slMessage( DEBUG_ALL, "Cannot locate file \"%s\" for object dearchive\n", BRSTRING( &args[0] ) );

		target->set(( brInstance* )NULL );

		return EC_OK;
	}

	instance = brXMLDearchiveObjectFromFile( i->engine, filename );

	if ( !instance ) {
		slMessage( DEBUG_ALL, "error decoding XML message from file\n" );

		target->set(( brInstance* )NULL );

		return EC_ERROR;
	}

	target->set( instance );

	slFree( filename );

	return EC_OK;
}

/**
 * \brief Dearchives and returns an instance from an XML string.
 */

int stCDearchiveXMLObjectFromString( brEval args[], brEval *target, brInstance *i ) {
	brInstance *instance = brXMLDearchiveObjectFromString( i->engine, BRSTRING( &args[0] ) );

	if ( !instance ) {
		slMessage( DEBUG_ALL, "error decoding XML message from string\n" );

		target->set( ( brInstance* )NULL );

		return EC_ERROR;
	}

	target->set( instance );

	return EC_OK;
}

void breveInitXMLFuncs( brNamespace *n ) {
	BRBREVECALL( n, brIXMLArchiveObjectToFile, AT_INT, AT_INSTANCE, AT_STRING );
	BRBREVECALL( n, brIXMLArchiveObjectToString, AT_STRING, AT_INSTANCE );

	brNewBreveCall( n, "writeXMLEngine", stCWriteXMLEngine, AT_INT, AT_STRING, 0 );
	brNewBreveCall( n, "dearchiveXMLObject", stCDearchiveXMLObject, AT_INSTANCE, AT_STRING, 0 );
	brNewBreveCall( n, "dearchiveXMLObjectFromString", stCDearchiveXMLObjectFromString, AT_INSTANCE, AT_STRING, 0 );
}
