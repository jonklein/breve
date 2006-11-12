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

#include "steve.h"
#include "xml.h"
#include "expression.h"
#include "evaluation.h"

enum states {
    XP_ENGINE = 1,
    XP_INSTANCE_ARCHIVE,
    XP_INSTANCE,
    XP_DATA_INSTANCE,
    XP_DATA_INSTANCE_ARCHIVE,
    XP_CLASS,
    XP_VARIABLES,
    XP_DEPENDENCIES,
    XP_OBSERVERS,
    XP_NOTIFICATION,
    XP_METHOD,
    XP_INDEX,

    XP_ARRAY,

    XP_INT,
    XP_DOUBLE,
    XP_LIST,
    XP_HASH,
    XP_STRING,
    XP_VECTOR,
    XP_POINTER,
    XP_OBJECT,
    XP_MATRIX,
    XP_DATA,

    XP_KEY,
    XP_VALUE
};

#define XMLPutSpaces(count, file)	{ int m; for(m=0;m<(count);m++) fputc(' ', (file)); }
#define XML_INDENT_SPACES	1

/*!
	\brief Assigns indices to the instances in the engine.

	In preparation for an archiving event, this method assigns indices to all instances
	in the engine.
*/

int stXMLAssignIndices( brEngine *e, std::map< stInstance*, int > &instanceToIndexMap ) {
	unsigned int n;

	for ( n = 0; n < e->instances.size(); n++ ) instanceToIndexMap[( stInstance* )e->instances[n]->userData ] = n;

	for ( n = 0; n < e->instancesToAdd.size(); n++ ) instanceToIndexMap[( stInstance* )e->instancesToAdd[n]->userData ] = n + e->instances.size();

	return n;
}

int stXMLWriteObjectToFile( stInstance *i, char *filename, int isDataObject ) {
	FILE *file;
	int r;

	if ( !( file = fopen( filename, "w" ) ) ) {
		slMessage( DEBUG_ALL, "error opening file \"%s\" for archive of instance %p\n", filename, i );
		return -1;
	}

	r = stXMLWriteObjectToStream( i, file, isDataObject );

	fclose( file );

	return r;
}

/*!
	\brief Exports an instance to a FILE stream as XML.
*/

int stXMLWriteObjectToStream( stInstance *i, FILE *file, int isDataObject ) {
	stXMLArchiveRecord record;
	int spaces = 0;

	stXMLAssignIndices( i->type->engine, record.instanceToIndexMap );

	fprintf( file, "<?xml version=\"1.0\"?>\n" );
	fprintf( file, "<!DOCTYPE steveObject SYSTEM \"steveObject.dtd\">\n" );

	if ( !isDataObject ) fprintf( file, "<instance_archive archiveIndex=\"%d\">\n", record.instanceToIndexMap[ i] );
	else fprintf( file, "<data_instance_archive>\n" );

	spaces += XML_INDENT_SPACES;

	stXMLWriteObject( &record, file, i, spaces, isDataObject );

	spaces -= XML_INDENT_SPACES;

	if ( !isDataObject ) fprintf( file, "</instance_archive>\n" );
	else fprintf( file, "</data_instance_archive>\n" );

	return 0;
}

/*!
	\brief Writes an XML archive of a simulation to a file.
*/

int stXMLWriteSimulationToFile( char *filename, brEngine *e ) {
	FILE *file;

	if ( !( file = fopen( filename, "w" ) ) ) {
		slMessage( DEBUG_ALL, "error opening file \"%s\" for archive of simulation\n", filename );
		return -1;
	}

	stXMLWriteSimulationToStream( file, e );

	fclose( file );

	return 0;
}

/*!
	\brief Writes an XML archive of a simulation to a FILE stream.
*/

int stXMLWriteSimulationToStream( FILE *file, brEngine *e ) {
	int spaces = 0;
	stXMLArchiveRecord record;
	stInstance *controller;

	controller = ( stInstance* )brEngineGetController( e )->userData;

	stXMLAssignIndices( e, record.instanceToIndexMap );

	fprintf( file, "<?xml version=\"1.0\"?>\n" );
	fprintf( file, "<!DOCTYPE steveEngine SYSTEM \"steveEngine.dtd\">\n" );

	fprintf( file, "<engine controllerIndex=\"%d\">\n", record.instanceToIndexMap[ controller] );
	spaces += XML_INDENT_SPACES;

	std::vector<brInstance*>::iterator bi;

	for ( bi = e->instances.begin(); bi != e->instances.end(); bi++ )
		stXMLWriteObject( &record, file, ( stInstance* )( *bi )->userData, spaces, 0 );

	spaces -= XML_INDENT_SPACES;

	fprintf( file, "</engine>\n" );

	return 0;
}

int stXMLWriteObject( stXMLArchiveRecord *record, FILE *file, stInstance *i, int spaces, int isDataObject ) {
	slList *list;
	stObject *o;
	brEval result;
	int r;

	if ( record->written.find( i ) != record->written.end() ) return 0;

	if ( i->status != AS_ACTIVE ) {
		slMessage( DEBUG_ALL, "warning: requested archive of freed instance %p of class %s, skipping...\n", i, i->type->name.c_str() );
		return -1;
	}

	o = i->type;

	record->written.insert( i );

	// make sure the dependencies are archived first

	std::set< stInstance*, stInstanceCompare>::iterator ii;

	for ( ii = i->dependencies.begin(); ii != i->dependencies.end(); ii++ ) {
		stXMLWriteObject( record, file, *ii, spaces, isDataObject );
	}

	// if we're not writing data only, call the archive method

	if ( !isDataObject ) {
		stRunInstance ri;

		ri.instance = i;
		ri.type = i->type;

		r = stCallMethodByName( &ri, "archive", &result );

		if ( r != EC_OK || BRINT( &result ) != 1 ) {
			slMessage( DEBUG_ALL, "archive of instance %p (%s) failed [%d]\n", i, i->type->name.c_str(), BRINT( &result ) );
			return -1;
		}
	}

	XMLPutSpaces( spaces, file );

	if ( isDataObject ) {
		fprintf( file, "<data_instance class=\"%s\" index=\"%d\">\n", o->name.c_str(), record->instanceToIndexMap[ i] );
	} else {
		fprintf( file, "<instance class=\"%s\" index=\"%d\">\n", o->name.c_str(), record->instanceToIndexMap[ i] );
	}

	spaces += XML_INDENT_SPACES;

	// write out the observers

	if ( !isDataObject ) {

		list = i->breveInstance->observers;

		while ( list ) {
			XMLPutSpaces( spaces, file );
			fprintf( file, "<observers>\n" );
			spaces += XML_INDENT_SPACES;

			brObserver *obs = ( brObserver* )list->data;
			int index;

			if ( obs->instance && obs->instance->status == AS_ACTIVE ) {
				index = record->instanceToIndexMap[( stInstance* )obs->instance->userData];
			} else {
				index = -1;
			}

			XMLPutSpaces( spaces, file );

			fprintf( file, "<object index=\"%d\"/>\n", index );
			XMLPutSpaces( spaces, file );
			fprintf( file, "<notification name=\"%s\"/>\n", obs->notification );
			XMLPutSpaces( spaces, file );
			fprintf( file, "<method name=\"%s\"/>\n", obs->method->name );

			list = list->next;

			spaces -= XML_INDENT_SPACES;
			XMLPutSpaces( spaces, file );
			fprintf( file, "</observers>\n" );
		}
	}

	// write out the dependencies

	if ( !isDataObject ) {
		XMLPutSpaces( spaces, file );
		fprintf( file, "<dependencies>\n" );
		spaces += XML_INDENT_SPACES;

		std::set< stInstance*, stInstanceCompare>::iterator ii;

		for ( ii = i->dependencies.begin(); ii != i->dependencies.end(); ii++ ) {
			stInstance *dep = *ii;
			int index;

			if ( dep && dep->status == AS_ACTIVE ) index = record->instanceToIndexMap[ dep];
			else index = -1;

			XMLPutSpaces( spaces, file );

			fprintf( file, "<object index=\"%d\"/>\n", index );
		}

		spaces -= XML_INDENT_SPACES;

		XMLPutSpaces( spaces, file );
		fprintf( file, "</dependencies>\n" );
	}

	// stores the variables of this instance (& the parents)

	XMLPutSpaces( spaces, file );

	fprintf( file, "<variables>\n" );

	spaces += XML_INDENT_SPACES;

	while ( o ) {
		std::map< std::string, stVar* >::iterator vi;

		XMLPutSpaces( spaces, file );
		fprintf( file, "<class name=\"%s\" version=\"%f\">\n", o->name.c_str(), o->version );
		spaces += XML_INDENT_SPACES;

		for ( vi = o->variables.begin(); vi != o->variables.end(); vi++ ) {
			stXMLVariablePrint( record, file, vi->second, i, spaces );
		}

		spaces -= XML_INDENT_SPACES;

		XMLPutSpaces( spaces, file );
		fprintf( file, "</class>\n" );

		o = o->super;
	}

	spaces -= XML_INDENT_SPACES;

	XMLPutSpaces( spaces, file );
	fprintf( file, "</variables>\n" );

	spaces -= XML_INDENT_SPACES;

	XMLPutSpaces( spaces, file );

	if ( isDataObject ) fprintf( file, "</data_instance>\n" );
	else fprintf( file, "</instance>\n" );

	return 0;
}

int stXMLVariablePrint( stXMLArchiveRecord *record, FILE *file, stVar *variable, stInstance *i, int spaces ) {
	brEval target;
	stRunInstance ri;

	ri.instance = i;
	ri.type = i->type;

	if ( variable->type->_type == AT_ARRAY ) {
		int n, typeSize;

		XMLPutSpaces( spaces, file );
		fprintf( file, "<array name=\"%s\">\n", variable->name.c_str() );
		spaces += XML_INDENT_SPACES;

		typeSize = stSizeofAtomic( variable->type->_arrayType );

		for ( n = 0;n < variable->type->_arrayCount;n++ ) {
			stLoadVariable( &i->variables[variable->offset + n * typeSize], variable->type->_arrayType, &target, &ri );
			stXMLPrintEval( record, file, "", &target, spaces );
		}

		spaces -= XML_INDENT_SPACES;

		XMLPutSpaces( spaces, file );
		fprintf( file, "</array>\n" );

		return 0;
	}

	stLoadVariable( &i->variables[variable->offset], variable->type->_type, &target, &ri );

	stXMLPrintEval( record, file, variable->name.c_str(), &target, spaces );

	return 0;
}

int stXMLPrintList( stXMLArchiveRecord *record, FILE *file, const char *name, brEvalListHead *theHead, int spaces ) {
	XMLPutSpaces( spaces, file );
	fprintf( file, "<list name=\"%s\">\n", name );
	spaces += XML_INDENT_SPACES;

	std::vector< brEval >::iterator li;

	for ( li = theHead->_vector.begin(); li != theHead->_vector.end(); li++ ) {
		stXMLPrintEval( record, file, "", &(*li), spaces );
	}

	spaces -= XML_INDENT_SPACES;

	XMLPutSpaces( spaces, file );
	fprintf( file, "</list>\n" );

	return 0;
}

int stXMLPrintHash( stXMLArchiveRecord *record, FILE *file, const char *name, brEvalHash *hash, int spaces ) {
	brEvalListHead *keys;

	keys = brEvalHashKeys( hash );

	XMLPutSpaces( spaces, file );
	fprintf( file, "<hash name=\"%s\">\n", name );
	spaces += XML_INDENT_SPACES;

	std::vector< brEval >::iterator li;

	for ( li = keys->_vector.begin(); li != keys->_vector.end(); li++ ) {
		brEval newEval, value;

		brEvalHashLookup( hash, &(*li), &value );

		XMLPutSpaces( spaces, file );
		fprintf( file, "<key>\n" );
		spaces += XML_INDENT_SPACES;

		brEvalCopy( &(*li), &newEval );
		stXMLPrintEval( record, file, "", &newEval, spaces );

		spaces -= XML_INDENT_SPACES;
		XMLPutSpaces( spaces, file );
		fprintf( file, "</key>\n" );

		XMLPutSpaces( spaces, file );
		fprintf( file, "<value>\n" );
		spaces += XML_INDENT_SPACES;

		brEvalCopy( &value, &newEval );
		stXMLPrintEval( record, file, "", &newEval, spaces );

		spaces -= XML_INDENT_SPACES;
		XMLPutSpaces( spaces, file );
		fprintf( file, "</value>\n" );
	}

	spaces -= XML_INDENT_SPACES;

	XMLPutSpaces( spaces, file );
	fprintf( file, "</hash>\n" );

	return 0;
}

int stXMLPrintEval( stXMLArchiveRecord *record, FILE *file, const char *name, brEval *target, int spaces ) {
	stInstance *i;
	int index;
	char *data, *encoded;

	switch ( target->type() ) {

		case AT_LIST:
			return stXMLPrintList( record, file, name, BRLIST( target ), spaces );

			break;

		case AT_HASH:
			return stXMLPrintHash( record, file, name, BRHASH( target ), spaces );

			break;

		case AT_DATA:
			data = brDataHexEncode(( brData* )BRDATA( target ) );

			XMLPutSpaces( spaces, file );

			fprintf( file, "<data name=\"%s\">%s</data>\n", name, data );

			slFree( data );

			break;

		case AT_INT:
			XMLPutSpaces( spaces, file );

			fprintf( file, "<int name=\"%s\">%d</int>\n", name, BRINT( target ) );

			break;

		case AT_DOUBLE:
			XMLPutSpaces( spaces, file );

			fprintf( file, "<float name=\"%s\">%.16g</float>\n", name, BRDOUBLE( target ) );

			break;

		case AT_STRING:
			XMLPutSpaces( spaces, file );

			encoded = stXMLEncodeString( BRSTRING( target ) );

			fprintf( file, "<string name=\"%s\">%s</string>\n", name, encoded );

			delete[] encoded;

			break;

		case AT_VECTOR:
			XMLPutSpaces( spaces, file );

			fprintf( file, "<vector name=\"%s\">(%.16g, %.16g, %.16g)</vector>\n", name, BRVECTOR( target ).x, BRVECTOR( target ).y, BRVECTOR( target ).z );

			break;

		case AT_MATRIX:
			XMLPutSpaces( spaces, file );

			fprintf( file, "<matrix name=\"%s\">[ (%.16g, %.16g, %.16g), (%.16g, %.16g, %.16g), (%.16g, %.16g, %.16g) ]</matrix>\n", name, BRMATRIX( target )[0][0], BRMATRIX( target )[0][1], BRMATRIX( target )[0][2], BRMATRIX( target )[1][0], BRMATRIX( target )[1][1], BRMATRIX( target )[1][2], BRMATRIX( target )[2][0], BRMATRIX( target )[2][1], BRMATRIX( target )[2][2] );

			break;

		case AT_POINTER:
			XMLPutSpaces( spaces, file );

			fprintf( file, "<pointer name=\"%s\"/>\n", name );

			break;

		case AT_INSTANCE:
			if ( BRINSTANCE( target ) ) i = ( stInstance* )BRINSTANCE( target )->userData;
			else i = NULL;

			if ( i && i->status == AS_ACTIVE ) index = record->instanceToIndexMap[ i] ;
			else index = -1;

			XMLPutSpaces( spaces, file );

			fprintf( file, "<object name=\"%s\" index=\"%d\"/>\n", name, index );

			break;

		default:
			slMessage( DEBUG_ALL, "warning: unknown atomic type (%d) while writing XML object\n", target->type() );

			break;
	}

	return 0;
}

#undef XMLPutSpaces

int stXMLReadObjectFromFile( stInstance *i, char *filename ) {
	char *buffer;
	int result = 0;

	buffer = slUtilReadFile( filename );

	if ( !buffer ) {
		slMessage( DEBUG_ALL, "error opening file \"%s\" for dearchive of instance\n", filename, i );
		return -1;
	}

	result = stXMLReadObjectFromString( i, buffer );

	if ( result == -1 ) {
		slMessage( DEBUG_ALL, "error reading file \"%s\" for dearchive of instance\n", filename, i );
		return -1;
	}

	slFree( buffer );

	return result;
}

int stXMLReadObjectFromStream( stInstance *i, FILE *file ) {
	char *buffer;
	int result = 0;
	buffer = slUtilReadStream( file );

	if ( !buffer ) {
		slMessage( DEBUG_ALL, "error opening stream for dearchive of instance\n", i );
		return -1;
	}

	result = stXMLReadObjectFromString( i, buffer );

	if ( result == -1 ) {
		slMessage( DEBUG_ALL, "error reading stream  for dearchive of instance\n", i );
		return -1;
	}

	slFree( buffer );

	return result;
}

int stXMLReadObjectFromString( stInstance *i, char *buffer ) {
	XML_Parser parser;
	stXMLParserState parserState;
	int result = 0;

	parser = XML_ParserCreate( NULL );

	parserState.engine = i->type->engine;
	parserState.currentInstance = i;
	parserState.mode = PARSE_INSTANCE;
	parserState.error = 0;

	/* preparse */

	XML_SetStartElementHandler( parser, stXMLPreparseStartElementHandler );
	XML_SetEndElementHandler( parser, NULL );
	XML_SetCharacterDataHandler( parser, NULL );
	XML_SetUserData( parser, &parserState );

	if ( parserState.error ) {
		slMessage( DEBUG_ALL, "Error loading simulation from buffer\n" );
		result = -1;
	}

	if ( !XML_Parse( parser, buffer, strlen( buffer ), 1 ) ) {
		slMessage( DEBUG_ALL, "Error loading simulation from buffer:\n" );
		stPrintXMLError( parser );
		result = -1;
	}

	XML_ParserFree( parser );

	/* do the real parse stage */

	parserState.currentInstance = i;
	parser = XML_ParserCreate( NULL );
	XML_SetStartElementHandler( parser, stXMLObjectStartElementHandler );
	XML_SetEndElementHandler( parser, stXMLObjectEndElementHandler );
	XML_SetCharacterDataHandler( parser, stXMLObjectCharacterDataHandler );
	XML_SetUserData( parser, &parserState );

	if ( !XML_Parse( parser, buffer, strlen( buffer ), 1 ) ) {
		stPrintXMLError( parser );
		result = -1;
	}

	if ( parserState.error ) {
		result = -1;
	}

	XML_ParserFree( parser );

	return result;
}

stInstance *stXMLDearchiveObjectFromFile( brEngine *e, char *filename ) {
	char *buffer;
	stInstance *i;

	buffer = slUtilReadFile( filename );

	if ( !buffer ) {
		slMessage( DEBUG_ALL, "error opening file \"%s\" for dearchive of instance\n", filename );
		return NULL;
	}

	i = stXMLDearchiveObjectFromString( e, buffer );

	slFree( buffer );

	return i;
}

stInstance *stXMLDearchiveObjectFromStream( brEngine *e, FILE *stream ) {
	char *buffer;
	stInstance *i;

	buffer = slUtilReadStream( stream );

	if ( !buffer ) {
		slMessage( DEBUG_ALL, "error opening stream for dearchive of instance\n" );
		return NULL;
	}

	i = stXMLDearchiveObjectFromString( e, buffer );

	slFree( buffer );

	return i;
}

stInstance *stXMLDearchiveObjectFromString( brEngine *e, char *buffer ) {
	XML_Parser parser;
	stXMLParserState parserState;
	stInstance *dearchivedInstance;
	int result = 0;

	parser = XML_ParserCreate( NULL );

	parserState.engine = e;
	parserState.mode = PARSE_INSTANCE;
	parserState.error = 0;

	XML_SetParamEntityParsing( parser, XML_PARAM_ENTITY_PARSING_ALWAYS );
	// XML_SetExternalEntityRefHandler(parser, stExternalEntityParserCreate);

	// do the preparse stage

	XML_SetStartElementHandler( parser, stXMLPreparseStartElementHandler );
	XML_SetEndElementHandler( parser, NULL );
	XML_SetCharacterDataHandler( parser, NULL );
	XML_SetUserData( parser, &parserState );

	if ( parserState.error ) {
		slMessage( DEBUG_ALL, "Error loading simulation from string\n" );
		result = -1;
	}

	if ( !XML_Parse( parser, buffer, strlen( buffer ), 1 ) ) {
		stPrintXMLError( parser );
		result = -1;
	}

	XML_ParserFree( parser );

	/* do the real parse stage */

	parser = XML_ParserCreate( NULL );
	XML_SetStartElementHandler( parser, stXMLObjectStartElementHandler );
	XML_SetEndElementHandler( parser, stXMLObjectEndElementHandler );
	XML_SetCharacterDataHandler( parser, stXMLObjectCharacterDataHandler );
	XML_SetUserData( parser, &parserState );

	if ( !XML_Parse( parser, buffer, strlen( buffer ), 1 ) ) {
		stPrintXMLError( parser );
		result = -1;
	}

	dearchivedInstance = parserState.indexToInstanceMap[ parserState.archiveIndex];

	result = stXMLRunDearchiveMethods( &parserState );

	if ( parserState.error ) result = -1;

	XML_ParserFree( parser );

	if ( result == -1 ) return NULL;
	else return dearchivedInstance;
}

int stXMLInitSimulationFromFile( brEngine *e, char *filename ) {
	char *buffer;
	int result = 0;

	buffer = slUtilReadFile( filename );

	if ( !buffer ) {
		slMessage( DEBUG_ALL, "Error loading archived simulation file: could not open \"%s\" (%s)\n", filename, strerror( errno ) );
		return -1;
	}

	result = stXMLInitSimulationFromString( e, buffer );

	slFree( buffer );
	return result;
}

int stXMLInitSimulationFromStream( brEngine *e, FILE *stream ) {
	char *buffer;
	int result = 0;

	buffer = slUtilReadStream( stream );

	if ( !buffer ) {
		slMessage( DEBUG_ALL, "Error loading archived simulation stream\n" );
		return -1;
	}

	result = stXMLInitSimulationFromString( e, buffer );

	slFree( buffer );
	return result;
}

int stXMLInitSimulationFromString( brEngine *e, char *buffer ) {
	XML_Parser parser;
	stXMLParserState parserState;
	int result = 0;

	if ( !buffer ) {
		slMessage( DEBUG_ALL, "Error loading archived simulation file: could not open buffer\n" );
		return -1;
	}

	parser = XML_ParserCreate( NULL );

	parserState.engine = e;
	parserState.mode = PARSE_ENGINE;
	parserState.error = 0;

	/* do the preparse stage */

	XML_SetStartElementHandler( parser, stXMLPreparseStartElementHandler );
	XML_SetEndElementHandler( parser, NULL );
	XML_SetCharacterDataHandler( parser, NULL );
	XML_SetUserData( parser, &parserState );

	if ( parserState.error ) {
		slMessage( DEBUG_ALL, "Error loading archived simulation\n" );
		result = -1;
	}

	if ( !XML_Parse( parser, buffer, strlen( buffer ), 1 ) ) {
		slMessage( DEBUG_ALL, "Error loading archived simulation\n" );
		stPrintXMLError( parser );
		result = -1;
	}

	XML_ParserFree( parser );

	/* do the real parse stage */

	parser = XML_ParserCreate( NULL );
	XML_SetStartElementHandler( parser, stXMLObjectStartElementHandler );
	XML_SetEndElementHandler( parser, stXMLObjectEndElementHandler );
	XML_SetCharacterDataHandler( parser, stXMLObjectCharacterDataHandler );
	XML_SetUserData( parser, &parserState );

	if ( !XML_Parse( parser, buffer, strlen( buffer ), 1 ) ) {
		slMessage( DEBUG_ALL, "Error loading archived simulation: parsing failed\n" );
		stPrintXMLError( parser );
		result = -1;
	}

	if ( stXMLRunDearchiveMethods( &parserState ) ) {
		slMessage( DEBUG_ALL, "Error loading archived simulation: dearchive commands failed\n" );
		result = -1;
	}

	if ( parserState.error ) {
		slMessage( DEBUG_ALL, "Error loading archived simulation: parsing failed\n" );
		result = -1;
	}

	XML_ParserFree( parser );

	return result;
}

/*!
	\brief Runs dearchive for the list of instances.
*/

int stXMLRunDearchiveMethods( stXMLParserState *s ) {
	int r;
	brEval result;
	std::map< int, stInstance* >::iterator ii;

	for ( unsigned int n = 0; n < s->dearchiveOrder.size(); n++ ) {
		stRunInstance ri;

		ri.instance = s->dearchiveOrder[ n ];

		if ( ri.instance ) {
			ri.type = ri.instance->type;

			r = stCallMethodByName( &ri, "post-dearchive-set-controller", &result );
			r = stCallMethodByName( &ri, "dearchive", &result );

			if ( r != EC_OK || BRINT( &result ) != 1 ) {
				slMessage( DEBUG_ALL, "dearchive of instance %p (%s) failed\n", ri.instance, ri.instance->type->name.c_str() );
				return -1;
			}
		}
	}

	return 0;
}

/*!
	\brief Preparse stage for XML dearchiving.

	In the preparse stage, we go through the archive and create the empty
	instances.  We do this first so that we can correctly assign instance
	pointers the second time around.
*/

void stXMLPreparseStartElementHandler( void *data, const XML_Char *attname, const XML_Char **atts ) {
	int state;
	int n = 0;
	int index = -1;

	const char *objectName = NULL;

	stXMLParserState *parserState = ( stXMLParserState* )data;

	state = stXMLStateForElement(( char* )attname );

	while ( atts[n] ) {
		if ( !strcasecmp( atts[n], "class" ) ) objectName = atts[n + 1];

		if ( !strcasecmp( atts[n], "index" ) ) index = atoi( atts[n + 1] );

		n += 2;
	}

	if ( state == XP_INSTANCE || state == XP_DATA_INSTANCE ) {
		stInstance *i;

		if ( parserState->mode != PARSE_DATA_INSTANCE ) {
			brObject *object = brObjectFind( parserState->engine, ( char* )objectName );

			if ( !object ) {
				slMessage( DEBUG_ALL, "archive contains an instance of unknown class \"%s\"\n", objectName );
				slMessage( DEBUG_ALL, "mismatch between simulation file and XML archive\n", objectName );
				parserState->error++;
				return;
			}

			i = stInstanceNew(( stObject* )object->userData );

			i->breveInstance = brEngineAddInstance( parserState->engine, object, i );
		} else i = parserState->currentInstance;

		parserState->indexToInstanceMap[ index] = i;
	}
}

void stXMLObjectStartElementHandler( void *userData, const XML_Char *name, const XML_Char **atts ) {

	int n = 0;
	stXMLStackEntry *state;

	const char *objectName = NULL;

	int controllerIndex = 0, index = 0, archiveIndex = 0;

	stInstance *steveInstance;

	brObject *object;

	stXMLParserState *parserState = ( stXMLParserState* )userData;

	if ( parserState->error != 0 ) return;

	state = new stXMLStackEntry;

	state->state = stXMLStateForElement(( char* )name );

	state->name = NULL;

	state->string = ( char* )slMalloc( 1 );

	state->string[ 0 ] = 0;

	parserState->stateStack.push_back( state );

	while ( atts[n] ) {
		if ( !strcasecmp( atts[n], "name" ) ) state->name = slStrdup(( char* )atts[n + 1] );

		if ( !strcasecmp( atts[n], "class" ) ) objectName = atts[n + 1];

		if ( !strcasecmp( atts[n], "controllerIndex" ) ) controllerIndex = atoi( atts[n + 1] );

		if ( !strcasecmp( atts[n], "archiveIndex" ) ) archiveIndex = atoi( atts[n + 1] );

		if ( !strcasecmp( atts[n], "index" ) ) index = atoi( atts[n + 1] );

		n += 2;
	}

	switch ( state->state ) {

		case XP_ENGINE:
			parserState->controllerIndex = controllerIndex;

			break;

		case XP_INSTANCE_ARCHIVE:
			parserState->archiveIndex = archiveIndex;

			break;

		case XP_INSTANCE:
			parserState->currentInstance = parserState->indexToInstanceMap[ index];

			if ( !parserState->currentInstance ) slMessage( DEBUG_ALL, "warning: cannot locate instance #%d\n", index );

			parserState->dearchiveOrder.push_back( parserState->currentInstance );

			break;

		case XP_ARRAY:
			state->variable = stObjectLookupVariable( parserState->currentObject, state->name );

			if ( state->variable ) {
				state->arrayIndex = 0;
			} else {
				slMessage( DEBUG_ALL, "archive contains unknown variable \"%s\" for class \"%s\"\n", state->name, parserState->currentInstance->type->name.c_str() );
				slMessage( DEBUG_ALL, "mismatch between simulation file and XML archive\n", objectName );
				parserState->error++;
			}

			break;

		case XP_LIST:
			state->eval.set( new brEvalListHead() );

			break;

		case XP_HASH:
			state->eval.set( new brEvalHash() );

			break;

		case XP_OBJECT:
			// we don't see the character data for this one

			steveInstance = parserState->indexToInstanceMap[ index];

			if ( steveInstance ) {
				state->eval.set( steveInstance->breveInstance );
			} else {
				state->eval.set(( brInstance* )NULL );
			}

			break;

		case XP_POINTER:
			state->eval.set(( void* )NULL );

			break;

		case XP_CLASS:
			object = brObjectFind( parserState->engine, state->name );

			if ( object ) parserState->currentObject = ( stObject* )object->userData;
			else parserState->currentObject = NULL;

			break;

		case XP_NOTIFICATION:
			state->eval.set( state->name );

			break;

		case XP_METHOD:
			state->eval.set( state->name );

			break;

		case XP_DEPENDENCIES:
			state->eval.set(( brInstance* )NULL );

			break;
	}
}

/*!
	\brief Read the character data for a given XML tag.

	Many states do not use this character data.  Since this function
	may be called multiple times, we have to continually reallocate
	and append data.
*/

void stXMLObjectCharacterDataHandler( void *userData, const XML_Char *data, int len ) {
	stXMLParserState *parserState = ( stXMLParserState* )userData;

	stXMLStackEntry *e;
	int oldLen;

	if ( parserState->error != 0 ) return;

	e = parserState->stateStack.back();

	// these data do not expect character data

	if ( e->state == XP_ENGINE ) return;

	if ( e->state == XP_INSTANCE ) return;

	if ( e->state == XP_INSTANCE_ARCHIVE ) return;

	if ( e->state == XP_DATA_INSTANCE ) return;

	if ( e->state == XP_DATA_INSTANCE_ARCHIVE ) return;

	if ( e->state == XP_OBSERVERS ) return;

	if ( e->state == XP_CLASS ) return;

	if ( e->state == XP_VARIABLES ) return;

	if ( e->state == XP_LIST ) return;

	if ( e->state == XP_HASH ) return;

	if ( e->state == XP_ARRAY ) return;

	if ( e->state == XP_KEY ) return;

	if ( e->state == XP_VALUE ) return;

	if ( e->state == XP_DEPENDENCIES ) return;

	// allocate or reallocate the string data.

	oldLen = 0;

	if ( !e->string ) {
		e->string = ( char* )slMalloc( len + 1 );
	} else {
		oldLen = strlen( e->string );
		e->string = ( char* )slRealloc( e->string, oldLen + len + 1 );
	}

	// copy in the new data

	strncpy( &e->string[oldLen], data, len );

	e->string[oldLen + len] = 0;
}

void stXMLObjectEndElementHandler( void *userData, const XML_Char *name ) {

	stXMLParserState *parserState = ( stXMLParserState* )userData;
	stXMLStackEntry *state = NULL, *lastState = NULL;
	stRunInstance ri;
	stVar *var;

	if ( parserState->error != 0 ) return;

	lastState = state = parserState->stateStack.back();

	// finish up the current state by parsing the character string

	if ( state->string ) {
		slVector v;
		slMatrix m;
		char *str;

		switch ( state->state ) {

			case XP_INSTANCE:
				if ( parserState->currentInstance == parserState->indexToInstanceMap[ parserState->controllerIndex ] )
					brEngineSetController( parserState->engine, parserState->currentInstance->breveInstance );

				state->eval.set( parserState->currentInstance );

				break;

			case XP_DATA:
				state->eval.set( brDataHexDecode( state->string ) );

				break;

			case XP_POINTER:
				state->eval.set(( void* )NULL );

				break;

			case XP_INT:
				state->eval.set( atoi( state->string ) );

				break;

			case XP_DOUBLE:
				state->eval.set( atof( state->string ) );

				break;

			case XP_STRING:
				str = stXMLDecodeString( state->string );

				state->eval.set( str );

				slFree( str );

				break;

			case XP_VECTOR:
				sscanf( state->string, "(%lf, %lf, %lf)", &v.x, &v.y, &v.z );

				state->eval.set( v );

				break;

			case XP_MATRIX:
				sscanf( state->string, "[ (%lf, %lf, %lf), (%lf, %lf, %lf), (%lf, %lf, %lf) ]", &m[0][0], &m[0][1], &m[0][2], &m[1][0], &m[1][1], &m[1][2], &m[2][0], &m[2][1], &m[2][2] );

				state->eval.set( m );

				break;


			case XP_OBSERVERS:
				if ( BRINSTANCE( &state->eval ) && BRSTRING( &state->key ) && BRSTRING( &state->value ) )
					brInstanceAddObserver( parserState->currentInstance->breveInstance, BRINSTANCE( &state->eval ), BRSTRING( &state->key ), BRSTRING( &state->value ) );

				break;
		}

		slFree( state->string );
	}

	parserState->stateStack.pop_back();

	if ( parserState->stateStack.size() != 0 ) state = parserState->stateStack.back();
	else state = NULL;

	if ( state ) {
		switch ( state->state ) {

			case XP_INSTANCE:
			if ( parserState->mode != PARSE_DATA_INSTANCE ) {}

				break;

			case XP_CLASS:
				if ( lastState->state != XP_ARRAY ) {
					stInstance *i = parserState->currentInstance;
					stVar *var;

					var = stObjectLookupVariable( i->type, lastState->name );

					if ( var && lastState->eval.type() ) {
						stRunInstance ri;

						ri.instance = parserState->currentInstance;
						ri.type = ri.instance->type;

						stSetVariable( &parserState->currentInstance->variables[var->offset], var->type->_type, NULL, &lastState->eval, &ri );
					}
				}

				break;

			case XP_KEY:

			case XP_VALUE:
				brEvalCopy( &lastState->eval, &state->eval );

				break;

			case XP_HASH:
				if ( lastState->state == XP_KEY ) {
					brEvalCopy( &lastState->eval, &state->key );
				} else if ( lastState->state == XP_VALUE ) {
					brEvalHashStore( BRHASH( &state->eval ), &state->key, &lastState->eval );
				}

				break;

			case XP_LIST:
				brEvalListInsert( BRLIST( &state->eval ), BRLIST( &state->eval )->_vector.size(), &lastState->eval );

				break;

			case XP_ARRAY:
				var = state->variable;

				ri.instance = parserState->currentInstance;

				ri.type = ri.instance->type;

				stSetVariable( &parserState->currentInstance->variables[var->offset + state->arrayIndex * stSizeofAtomic( var->type->_arrayType )], var->type->_arrayType, NULL, &lastState->eval, &ri );

				state->arrayIndex++;

				break;

			case XP_OBSERVERS:
				if ( lastState->state == XP_NOTIFICATION ) {
					brEvalCopy( &lastState->eval, &state->key );
				}

				if ( lastState->state == XP_METHOD ) {
					brEvalCopy( &lastState->eval, &state->value );
				}

				if ( lastState->state == XP_OBJECT ) {
					brEvalCopy( &lastState->eval, &state->eval );
				}

				break;

			case XP_DEPENDENCIES:
				if ( BRINSTANCE( &lastState->eval ) ) stInstanceAddDependency( parserState->currentInstance, ( stInstance* )BRINSTANCE( &lastState->eval )->userData );

				break;
		}
	}

	if ( lastState->name ) slFree( lastState->name );

	delete lastState;
}

int stXMLStateForElement( char *name ) {

	if ( !strcasecmp( name, "engine" ) ) return XP_ENGINE;

	if ( !strcasecmp( name, "instance" ) ) return XP_INSTANCE;

	if ( !strcasecmp( name, "instance_archive" ) ) return XP_INSTANCE_ARCHIVE;

	if ( !strcasecmp( name, "data_instance" ) ) return XP_DATA_INSTANCE;

	if ( !strcasecmp( name, "data_instance_archive" ) ) return XP_DATA_INSTANCE_ARCHIVE;

	if ( !strcasecmp( name, "observers" ) ) return XP_OBSERVERS;

	if ( !strcasecmp( name, "class" ) ) return XP_CLASS;

	if ( !strcasecmp( name, "variables" ) ) return XP_VARIABLES;

	if ( !strcasecmp( name, "dependencies" ) ) return XP_DEPENDENCIES;

	if ( !strcasecmp( name, "index" ) ) return XP_INDEX;

	if ( !strcasecmp( name, "notification" ) ) return XP_NOTIFICATION;

	if ( !strcasecmp( name, "method" ) ) return XP_METHOD;

	if ( !strcasecmp( name, "array" ) ) return XP_ARRAY;

	if ( !strcasecmp( name, "int" ) ) return XP_INT;

	if ( !strcasecmp( name, "float" ) ) return XP_DOUBLE;

	if ( !strcasecmp( name, "list" ) ) return XP_LIST;

	if ( !strcasecmp( name, "hash" ) ) return XP_HASH;

	if ( !strcasecmp( name, "string" ) ) return XP_STRING;

	if ( !strcasecmp( name, "vector" ) ) return XP_VECTOR;

	if ( !strcasecmp( name, "pointer" ) ) return XP_POINTER;

	if ( !strcasecmp( name, "object" ) ) return XP_OBJECT;

	if ( !strcasecmp( name, "data" ) ) return XP_DATA;

	if ( !strcasecmp( name, "matrix" ) ) return XP_MATRIX;

	if ( !strcasecmp( name, "key" ) ) return XP_KEY;

	if ( !strcasecmp( name, "value" ) ) return XP_VALUE;

	slMessage( DEBUG_ALL, "warning: unknown tag \"%s\" in XML parse\n", name );

	return -1;
}

XML_Parser stExternalEntityParserCreate( XML_Parser p, const XML_Char *context, const XML_Char *encoding ) {
	return XML_ParserCreate( NULL );
}

void stPrintXMLError( XML_Parser p ) {

	int line = XML_GetCurrentLineNumber( p );
	int col = XML_GetCurrentColumnNumber( p );

	slMessage( DEBUG_ALL, "Error at line %d (character %d) of saved simulation file: %s\n", line, col, XML_ErrorString( XML_GetErrorCode( p ) ) );
}

/*!
	\brief Encode a string for XML export.

	Both the encode and decode XML string functions use hardcoded values
	to parse out < > & ' and ".  A more general solution might be nice, but
	whatever.

	Returns an slMalloc'd string.
*/

char *stXMLEncodeString( char *string ) {
	char *result;
	int n, m;
	int size = 0;

	/* first calculate the size of the new string */

	n = 0;

	while ( string[n] != 0 ) {
		switch ( string[n] ) {

			case '<':

			case '>':
				size += 4; /* &lt; and &gt; */

				break;

			case '&':
				size += 5; /* &amp; */

				break;

			case '\'':

			case '\"':
				size += 6; /* &apos; and &quot; */

				break;

			default:
				size++;
		}

		n++;
	}

	result = new char[size + 1];

	n = 0;
	m = 0;

	while ( string[n] != 0 ) {
		switch ( string[n] ) {

			case '<':
				strcpy( &result[m], "&lt;" );

				m += 4;

				break;

			case '>':
				strcpy( &result[m], "&gt;" );

				m += 4;

				break;

			case '&':
				strcpy( &result[m], "&amps;" );

				m += 5;

				break;

			case '\'':
				strcpy( &result[m], "&apos;" );

				m += 6;

				break;

			case '\"':
				strcpy( &result[m], "&quot;" );

				m += 6;

				break;

			default:
				result[m] = string[n];

				m++;
		}

		n++;
	}

	result[m] = 0;

	return result;
}

/*!
	\brief Decodes a string for XML import.

	Both the encode and decode XML string functions use hardcoded values
	to parse out < > & ' and ".  A more general solution might be nice, but
	whatever.

	Returns an slMalloc'd string.
*/

char *stXMLDecodeString( char *string ) {
	char *result;
	int n, m;

	result = ( char* )slMalloc( strlen( string ) + 1 );

	n = 0;
	m = 0;

	while ( string[n] != 0 ) {
		if ( string[n] == '&' ) {
			if ( !strcmp( &string[n], "&gt;" ) ) {
				result[m] = '>';
				n += 4;
			} else if ( !strcmp( &string[n], "&lt;" ) ) {
				result[m] = '<';
				n += 4;
			} else if ( !strcmp( &string[n], "&amp;" ) ) {
				result[m] = '&';
				n += 5;
			} else if ( !strcmp( &string[n], "&apos;" ) ) {
				result[m] = '\'';
				n += 6;
			} else if ( !strcmp( &string[n], "&quot;" ) ) {
				result[m] = '\"';
				n += 6;
			} else {
				slMessage( DEBUG_ALL, "warning: stray '&' in XML file\n" );
				result[m] = ' ';
				n++;
			}

			m++;
		} else {
			result[m] = string[n];

			m++;
			n++;
		}
	}

	result[m] = 0;

	return result;
}
