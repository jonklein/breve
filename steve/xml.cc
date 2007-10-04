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

#define XMLPutSpaces(count, file)	{ int m; for(m=0;m<(count);m++) fputc(' ', (file)); }
#define XML_INDENT_SPACES	1

/**
 * \brief Assigns indices to the instances in the engine.
 *
 * In preparation for an archiving event, this method assigns indices to all instances
 * in the engine.
 */

int brXMLAssignIndices( brEngine *e, std::map< brInstance*, int > &_indexMap ) {
	unsigned int n;

	for ( n = 0; n < e->instances.size(); n++ ) 
		_indexMap[ e->instances[ n ] ] = n;

	for ( n = 0; n < e->instancesToAdd.size(); n++ ) 
		_indexMap[ e->instancesToAdd[ n ] ] = n + e->instances.size();

	return n;
}

int brXMLWriteObjectToFile( brInstance *i, char *filename, int isDataObject ) {
	FILE *file;
	int r;

	if ( !( file = fopen( filename, "w" ) ) ) {
		slMessage( DEBUG_ALL, "error opening file \"%s\" for archive of instance %p\n", filename, i );
		return -1;
	}

	r = brXMLWriteObjectToStream( i, file, isDataObject );

	fclose( file );

	return r;
}

/**
 * \brief Exports an instance to a FILE stream as XML.
 */

int brXMLWriteObjectToStream( brInstance *inInstance, FILE *file, int isDataObject ) {
	brXMLArchiveRecord record;
	int spaces = 0;

	brXMLAssignIndices( inInstance->engine, record._indexMap );

	fprintf( file, "<?xml version=\"1.0\"?>\n" );
	fprintf( file, "<!DOCTYPE steveObject SYSTEM \"steveObject.dtd\">\n" );

	if ( !isDataObject ) 
		fprintf( file, "<instance_archive archiveIndex=\"%d\">\n", record._indexMap[ inInstance ] );
	else 
		fprintf( file, "<data_instance_archive>\n" );

	spaces += XML_INDENT_SPACES;

	brXMLWriteObject( &record, file, inInstance, spaces, isDataObject );

	spaces -= XML_INDENT_SPACES;

	if ( !isDataObject ) 
		fprintf( file, "</instance_archive>\n" );
	else 
		fprintf( file, "</data_instance_archive>\n" );

	return 0;
}

/**
 * \brief Writes an XML archive of a simulation to a file.
 */

int brXMLWriteSimulationToFile( char *filename, brEngine *e ) {
	FILE *file;

	if ( !( file = fopen( filename, "w" ) ) ) {
		slMessage( DEBUG_ALL, "error opening file \"%s\" for archive of simulation\n", filename );
		return -1;
	}

	brXMLWriteSimulationToStream( file, e );

	fclose( file );

	return 0;
}

/*!
 * \brief Writes an XML archive of a simulation to a FILE stream.
 */

int brXMLWriteSimulationToStream( FILE *file, brEngine *e ) {
	int spaces = 0;
	brXMLArchiveRecord record;

	brXMLAssignIndices( e, record._indexMap );

	fprintf( file, "<?xml version=\"1.0\"?>\n" );
	fprintf( file, "<!DOCTYPE steveEngine SYSTEM \"steveEngine.dtd\">\n" );

	fprintf( file, "<engine controllerIndex=\"%d\">\n", record._indexMap[ brEngineGetController( e ) ] );

	spaces += XML_INDENT_SPACES;

	std::vector< brInstance* >::iterator bi;

	for ( bi = e->instances.begin(); bi != e->instances.end(); bi++ )
		brXMLWriteObject( &record, file, *bi, spaces, 0 );

	spaces -= XML_INDENT_SPACES;

	fprintf( file, "</engine>\n" );

	return 0;
}

// Temporary function declaration to assist in conversion of code to breveObjectAPI and away
// from steve specific code.

void stXMLWriteObjectVariables( brXMLArchiveRecord *inRecord, FILE *inFile, stInstance *inInstance, int inSpaces );

int brXMLWriteObject( brXMLArchiveRecord *record, FILE *file, brInstance *inInstance, int spaces, int isDataObject ) {
	if ( record->_written.find( inInstance ) != record->_written.end() ) 
		return 0;

	record->_written.insert( inInstance );

	if ( inInstance->status != AS_ACTIVE ) {
		slMessage( DEBUG_ALL, "warning: requested archive of freed instance %p of class %s, skipping...\n", inInstance, inInstance->object->name );
		return -1;
	}

	// make sure the dependencies are archived first

	std::set< brInstance*, brInstanceCompare>::iterator ii;

	for ( ii = inInstance->_dependencies.begin(); ii != inInstance->_dependencies.end(); ii++ ) {
		brXMLWriteObject( record, file, *ii, spaces, isDataObject );
	}

	// if we're not writing data only, call the archive method

	if ( !isDataObject ) {
		brEval result;

		int r = brMethodCallByName( inInstance, "archive", &result );

		if ( r != EC_OK || BRINT( &result ) != 1 ) {
			slMessage( DEBUG_ALL, "archive of \"%s\" instance (%p) failed [%d]: \"archive\" method did not execute successfully\n", inInstance->object->name, inInstance, BRINT( &result ) );
			return -1;
		}
	}

	XMLPutSpaces( spaces, file );

	if ( isDataObject ) {
		fprintf( file, "<data_instance class=\"%s\" index=\"%d\" typesignature=\"%ld\">\n", inInstance->object->name, record->_indexMap[ inInstance ], inInstance->object->type->_typeSignature );
	} else {
		fprintf( file, "<instance class=\"%s\" index=\"%d\" typesignature=\"%ld\">\n", inInstance->object->name, record->_indexMap[ inInstance ], inInstance->object->type->_typeSignature );
	}

	spaces += XML_INDENT_SPACES;

	// write out the observers

	if ( !isDataObject ) {
		for( unsigned int n = 0; n < inInstance->observers.size(); n++ ) {
			brObserver *obs = inInstance->observers[ n ];

			XMLPutSpaces( spaces, file );
			fprintf( file, "<observers>\n" );
			spaces += XML_INDENT_SPACES;

			int index;

			if ( obs->instance && obs->instance->status == AS_ACTIVE ) 
				index = record->_indexMap[ obs->instance ];
			else
				index = -1;

			XMLPutSpaces( spaces, file );

			fprintf( file, "<object index=\"%d\"/>\n", index );
			XMLPutSpaces( spaces, file );
			fprintf( file, "<notification name=\"%s\"/>\n", obs->notification );
			XMLPutSpaces( spaces, file );
			fprintf( file, "<method name=\"%s\"/>\n", obs->method->name );

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

		std::set< brInstance*, brInstanceCompare>::iterator ii;

		for ( ii = inInstance->_dependencies.begin(); ii != inInstance->_dependencies.end(); ii++ ) {
			brInstance *dep = *ii;
			int index = -1;

			if ( dep && dep->status == AS_ACTIVE ) 
				index = record->_indexMap[ dep ];

			XMLPutSpaces( spaces, file );

			fprintf( file, "<object index=\"%d\"/>\n", index );
		}

		spaces -= XML_INDENT_SPACES;

		XMLPutSpaces( spaces, file );
		fprintf( file, "</dependencies>\n" );
	}

	// stores the variables of this instance (& the parents)

	XMLPutSpaces( spaces, file );

	fprintf( file, "<instancedata>\n" );

	spaces += XML_INDENT_SPACES;

	if( inInstance->object->type->_typeSignature == STEVE_TYPE_SIGNATURE )
		stXMLWriteObjectVariables( record, file, (stInstance*)inInstance -> userData, spaces );
	else {
		slMessage( DEBUG_ALL, "Warning: object encoding and decoding not fully implemented for non-steve objects.  Dearchiving of this object will likely fail.\n" );

		char *encoding = brInstanceEncodeToString( inInstance->engine, inInstance );

		if( encoding ) {
			fprintf( file, encoding );
			slFree( encoding );
		}
	}
		
	spaces -= XML_INDENT_SPACES;

	XMLPutSpaces( spaces, file );
	fprintf( file, "</instancedata>\n" );

	spaces -= XML_INDENT_SPACES;

	XMLPutSpaces( spaces, file );

	if ( isDataObject ) fprintf( file, "</data_instance>\n" );
	else fprintf( file, "</instance>\n" );

	return 0;
}

void stXMLWriteObjectVariables( brXMLArchiveRecord *inRecord, FILE *inFP, stInstance *inInstance, int inSpaces ) {
	stObject *o = inInstance->type;
	
	std::string result;

	while ( o ) {
		std::map< std::string, stVar* >::iterator vi;

		XMLPutSpaces( inSpaces, inFP );
		fprintf( inFP, "<class name=\"%s\" version=\"%f\">\n", o->name.c_str(), o->version );
		inSpaces += XML_INDENT_SPACES;

		for ( vi = o->variables.begin(); vi != o->variables.end(); vi++ ) 
			stXMLVariablePrint( inRecord, inFP, vi->second, inInstance, inSpaces );

		inSpaces -= XML_INDENT_SPACES;

		XMLPutSpaces( inSpaces, inFP );
		fprintf( inFP , "</class>\n" );

		o = o->super;
	}

}
int stXMLVariablePrint( brXMLArchiveRecord *record, FILE *file, stVar *variable, stInstance *i, int spaces ) {
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

int stXMLPrintList( brXMLArchiveRecord *record, FILE *file, const char *name, brEvalListHead *theHead, int spaces ) {
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

int stXMLPrintHash( brXMLArchiveRecord *record, FILE *file, const char *name, brEvalHash *hash, int spaces ) {
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

int stXMLPrintEval( brXMLArchiveRecord *record, FILE *file, const char *name, brEval *target, int spaces ) {
	stInstance *i = NULL;
	int index = -1;
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

			encoded = brXMLEncodeString( BRSTRING( target ) );

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
			if ( BRINSTANCE( target ) ) 
				i = ( stInstance* )BRINSTANCE( target )->userData;

			if ( i && i->status == AS_ACTIVE ) 
				index = record->_indexMap[ BRINSTANCE( target ) ] ;

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


struct brXMLState {
	std::vector< brXMLDOMElement* >		_stack;
};

brXMLDOMElement::~brXMLDOMElement() {
	for( unsigned int n = 0; n < _children.size(); n++ ) 
		delete _children[ n ];
}

const std::string *brXMLDOMElement::getAttr( const char *inAttr ) {
	std::string key( inAttr );
	
	return &_attrs[ key ];
}

std::string brXMLDOMElement::toXMLString( int inDepth ) {
	std::string xml;
	
	for( int s = 0; s < inDepth * 2; s++ ) xml += " ";

	xml += "<" + _name;
	
	std::map< std::string, std::string >::iterator ai;
	
	for( ai = _attrs.begin(); ai != _attrs.end(); ai++ ) {
		xml += " " + ai->first + "=\"" + ai->second + "\"";
	}

	xml += ">\n";

	if( _cdata.size() > 0 )
		xml += _cdata + "\n";
	
	for( unsigned int n = 0; n < _children.size(); n++ ) {
		
		xml += _children[ n ]->toXMLString( inDepth + 1 );

	}

	for( int s = 0; s < inDepth * 2; s++ ) xml += " ";
	
	xml += "</" + _name + ">\n";
	
	return xml;
}

void brXMLStartElementHandler( void *inUserData, const XML_Char *inName, const XML_Char **inAttrs ) {
	brXMLState *state = (brXMLState*)inUserData;

	brXMLDOMElement *element = new brXMLDOMElement( (char*)inName );	

	int n = 0;

	while( inAttrs[ n ] ) {
		std::string attr( inAttrs[ n ] ), value( inAttrs[ n + 1 ] );

		element->_attrs[ attr ] = value;
		
		n += 2;
	}

	state->_stack.push_back( element );
}

void brXMLEndElementHandler( void *inUserData, const XML_Char *inName ) {
	brXMLState *state = (brXMLState*)inUserData;

	brXMLDOMElement *element = state->_stack.back();

	if( state->_stack.size() > 1 ) {
		state->_stack.pop_back();
		state->_stack.back()->_children.push_back( element );
	}
}

void brXMLCharacterDataHandler( void *inUserData, const XML_Char *inData, int inLen ) {
	brXMLState *state = (brXMLState*)inUserData;

	brXMLDOMElement *element = state->_stack.back();

	for( int n = 0; n < inLen; n++ ) 
		element->_cdata += inData[ n ];
}

std::vector< brXMLDOMElement* > brXMLDOMElement::getElementsByName( const char *inName ) {
	std::vector< brXMLDOMElement* > matches;

	if( _name == inName ) 
		matches.push_back( this );

	for( unsigned int n = 0; n < _children.size(); n++ ) {
		std::vector< brXMLDOMElement* > childmatches;
		childmatches = _children[ n ]->getElementsByName( inName );

		for( unsigned int m = 0; m < childmatches.size(); m++ ) 
			matches.push_back( childmatches[ m ] );
	}

	return matches;
}

brXMLDOMElement* brXMLDOMElement::getChildByName( const char *inName ) {
	for( unsigned int n = 0; n < _children.size(); n++ ) {
		if( _children[ n ]->_name == inName ) 
			return _children[ n ];
	}

	return NULL;
}

brXMLDOMElement *brXMLParse( char *inBuffer ) {
	XML_Parser parser;
	brXMLState dom;

	parser = XML_ParserCreate( NULL );

	XML_SetStartElementHandler( parser, brXMLStartElementHandler );
	XML_SetEndElementHandler( parser, brXMLEndElementHandler );
	XML_SetCharacterDataHandler( parser, brXMLCharacterDataHandler );
	XML_SetUserData( parser, &dom );

	if ( !XML_Parse( parser, inBuffer, strlen( inBuffer ), 1 ) ) {
		slMessage( DEBUG_ALL, "Error loading simulation from buffer:\n" );
		stPrintXMLError( parser );
	}

	XML_ParserFree( parser );

	return dom._stack[ 0 ];
}

int stXMLReadObjectFromString( stInstance *i, char *buffer ) {
	brXMLParserState parserState;
	int result = 0;

	brXMLDOMElement *dom = brXMLParse( buffer );
	std::vector< brXMLDOMElement* > matches = dom->getElementsByName( "instance" ); 

	parserState.engine = i->type->engine;
	parserState.error = 0;

	if( brXMLPrepareInstanceMap( dom, &parserState ) != 0 )
		return EC_ERROR;

	for( unsigned int n = 0; n < matches.size(); n++ ) {
		const std::string *instindex = matches[ n ]->getAttr( "index" );
		
		if( !instindex ) {
			slMessage( DEBUG_ALL, "Error decoding XML object from string: could not locate instance index\n" );	
			return EC_ERROR;	
		}
		
		int ind = atoi( instindex->c_str() );
			
		if( brXMLDecodeInstance( &parserState, matches[ n ], parserState._indexToInstanceMap[ ind ] ) != EC_OK ) 
			return EC_ERROR;
	}

	return result;
}

brInstance *brXMLDearchiveObjectFromFile( brEngine *e, char *filename ) {
	char *buffer;
	brInstance *i;

	buffer = slUtilReadFile( filename );

	if ( !buffer ) {
		slMessage( DEBUG_ALL, "error opening file \"%s\" for dearchive of instance\n", filename );
		return NULL;
	}

	i = brXMLDearchiveObjectFromString( e, buffer );

	slFree( buffer );

	return i;
}

brInstance *brXMLDearchiveObjectFromStream( brEngine *e, FILE *stream ) {
	char *buffer;
	brInstance *i;

	buffer = slUtilReadStream( stream );

	if ( !buffer ) {
		slMessage( DEBUG_ALL, "error opening stream for dearchive of instance\n" );
		return NULL;
	}

	i = brXMLDearchiveObjectFromString( e, buffer );

	slFree( buffer );

	return i;
}



brInstance *brXMLDearchiveObjectFromString( brEngine *e, char *buffer ) {
	brXMLParserState parserState;

	// Parse the DOM tree

	brXMLDOMElement *dom = brXMLParse( buffer );
	
	std::vector< brXMLDOMElement* > archive = dom->getElementsByName( "instance_archive" ); 
	
	if( archive.size() != 1 ) {
		slMessage( DEBUG_ALL, "Error decoding XML from string: expected 1 instance_archive, found %d\n", archive.size() );	
		return NULL;
	}
	
	const std::string *index = archive[ 0 ]->getAttr( "archiveIndex" );
	
	if( !index ) {
		slMessage( DEBUG_ALL, "Error decoding XML from string: could not locate archived instance index\n" );	
		return NULL;	
	}
	
	int archivedIndex = atoi( index->c_str() );
	
	
	//
	// Dearchive the individual elements
	//
	
	std::vector< brXMLDOMElement* > matches = dom->getElementsByName( "instance" ); 

	parserState.engine = e;
	parserState.error = 0;

	if( brXMLPrepareInstanceMap( dom, &parserState ) != 0 )
		return NULL;

	for( unsigned int n = 0; n < matches.size(); n++ ) {
		const std::string *instindex = matches[ n ]->getAttr( "index" );
		
		if( !instindex ) {
			slMessage( DEBUG_ALL, "Error decoding XML from string: could not locate instance index\n", archive.size() );	
			return NULL;	
		}
		
		int i = atoi( instindex->c_str() );
			
		if( brXMLDecodeInstance( &parserState, matches[ n ], parserState._indexToInstanceMap[ i ] ) != EC_OK ) 
			return NULL;
	}

	brXMLRunDearchiveMethods( &parserState );

	return parserState._indexToInstanceMap[ archivedIndex ];
}





int brXMLInitSimulationFromFile( brEngine *e, char *filename ) {
	char *buffer;
	int result = 0;

	buffer = slUtilReadFile( filename );

	if ( !buffer ) {
		slMessage( DEBUG_ALL, "Error loading archived simulation file: could not open \"%s\" (%s)\n", filename, strerror( errno ) );
		return -1;
	}

	result = brXMLInitSimulationFromString( e, buffer );

	slFree( buffer );
	return result;
}

int brXMLInitSimulationFromStream( brEngine *e, FILE *stream ) {
	char *buffer;
	int result = 0;

	buffer = slUtilReadStream( stream );

	if ( !buffer ) {
		slMessage( DEBUG_ALL, "Error loading archived simulation stream\n" );
		return -1;
	}

	result = brXMLInitSimulationFromString( e, buffer );

	slFree( buffer );
	return result;
}

int brXMLInitSimulationFromString( brEngine *e, char *buffer ) {
	brXMLParserState parserState;
	int result = 0;

	brXMLDOMElement *dom = brXMLParse( buffer );

	std::vector< brXMLDOMElement* > engine = dom->getElementsByName( "engine" ); 

	if( engine.size() != 1 ) {
		slMessage( DEBUG_ALL, "Error decoding archived XML simulation: could not locate breve engine object\n" );	
		return EC_ERROR;		
	}

	int controllerIndex = atoi( engine[ 0 ]->getAttr( "controllerIndex" )->c_str() );




	std::vector< brXMLDOMElement* > matches = dom->getElementsByName( "instance" ); 

	parserState.engine = e;
	parserState.error = 0;

	if( brXMLPrepareInstanceMap( dom, &parserState ) != 0 )
		return EC_ERROR;

	for( unsigned int n = 0; n < matches.size(); n++ ) {
		const std::string *instindex = matches[ n ]->getAttr( "index" );
		
		if( !instindex ) {
			slMessage( DEBUG_ALL, "Error decoding archived XML simulation: could not locate instance index\n" );	
			return -1;	
		}
		
		int i = atoi( instindex->c_str() );
			
		if( brXMLDecodeInstance( &parserState, matches[ n ], parserState._indexToInstanceMap[ i ] ) != EC_OK ) 
			return EC_ERROR;
	}


	brEngineSetController( e, parserState._indexToInstanceMap[ controllerIndex ] );

	if ( brXMLRunDearchiveMethods( &parserState ) ) {
		slMessage( DEBUG_ALL, "Error decoding archived XML simulation: dearchive method failed\n" );
		result = -1;
	}

	return result;
}

/**
 * \brief Runs dearchive for the list of instances.
 */

int brXMLRunDearchiveMethods( brXMLParserState *s ) {
	int r;
	brEval result;

	for ( unsigned int n = 0; n < s->_dearchiveOrder.size(); n++ ) {
		brInstance *instance = s->_dearchiveOrder[ n ];

		if ( instance ) {

			r = brMethodCallByName( instance, "post-dearchive-set-controller", &result );
			r = brMethodCallByName( instance, "dearchive", &result );

			if ( r != EC_OK || BRINT( &result ) != 1 ) {
				slMessage( DEBUG_ALL, "dearchive of instance %p (%s) failed: \"dearchive\" method did not execute successfully\n", instance, instance->object->name );
				return -1;
			}
		}
	}

	return 0;
}

/**
 * \brief Preparse stage for XML dearchiving.
 *
 * In the preparse stage, we go through the archive and create the empty
 * instances.  We do this first so that we can correctly assign instance
 * pointers the second time around.
 */

int brXMLPrepareInstanceMap( brXMLDOMElement *inRoot, brXMLParserState *inState ) {
	std::vector< brXMLDOMElement* > instances = inRoot->getElementsByName( "instance" ); 

	stInstance *i;

	for( unsigned int n = 0; n < instances.size(); n++ ) {
		std::string objectname = instances[ n ]->_attrs[ "class" ];
		int instanceindex = atoi( instances[ n ]->_attrs[ "index" ].c_str() );
		int typeSignature = atoi( instances[ n ]->_attrs[ "typesignature" ].c_str() );


		brObject *object = brObjectFind( inState->engine, objectname.c_str() );

		if ( !object ) {
			slMessage( DEBUG_ALL, "XML archive contains an instance of unknown class \"%s\"\n", objectname.c_str() );
			slMessage( DEBUG_ALL, "Cannot decode XML archive: mismatch between simulation file and XML data\n" );
			inState->error++;
			return -1;
		}

		if( typeSignature != STEVE_TYPE_SIGNATURE ) 
			return EC_ERROR;

		i = stInstanceNew( (stObject*)object->userData );

		i->breveInstance = brEngineAddInstance( inState->engine, object, i );

		inState->_indexToInstanceMap[ instanceindex ] = i->breveInstance;
		
		inState->_dearchiveOrder.push_back( i->breveInstance );
	}

	return 0;
}


void brXMLParseList( brXMLParserState *inState, brXMLDOMElement *inListElement, brEvalListHead *ioList ) {
	// Step through the children of the list element, decoding them and appending them to the list

	for( unsigned int n = 0; n < inListElement->_children.size(); n++ ) {
		brEval member;
		
		brXMLParseEval( inState, inListElement->_children[ n ], &member );
		
		brEvalListInsert( ioList, ioList->_vector.size(), &member );
	}
}


void brXMLParseHash( brXMLParserState *inState, brXMLDOMElement *inHashElement, brEvalHash *ioHash ) {
	// Step through the key/value pairs of the hash element, decoding them and appending them to the list

	std::vector< brXMLDOMElement* > keys   = inHashElement->getElementsByName( "key" );
	std::vector< brXMLDOMElement* > values = inHashElement->getElementsByName( "value" );
	
	for( unsigned int n = 0; n < keys.size(); n++ ) {
		brEval keyEval, valueEval;
		
		brXMLParseEval( inState,   keys[ n ]->_children[ 0 ],   &keyEval );
		brXMLParseEval( inState, values[ n ]->_children[ 0 ], &valueEval );
	
		brEvalHashStore( ioHash, &keyEval, &valueEval );
	}
}


void brXMLParseEval( brXMLParserState *inState, brXMLDOMElement *inElement, brEval *outEval ) {
	int type = brXMLEvalTypeForTagName( inElement->_name.c_str() );

	slMatrix m;
	slVector v;
	char *str;
	int index;
	
	switch( type ) {
		case AT_INSTANCE:
			index = atoi( inElement->getAttr( "index" )->c_str() );

			outEval->set( inState->_indexToInstanceMap[ index ] );
	
			break;

		case AT_ARRAY:
			slMessage( DEBUG_ALL, "Arrays are deprecated and cannot be dearchived in this version of breve\n" );
			break;

		case AT_LIST:
			outEval->set( new brEvalListHead() );
			brXMLParseList( inState, inElement, BRLIST( outEval ) );

			break;

		case AT_HASH:
			outEval->set( new brEvalHash() );
			brXMLParseHash( inState, inElement, BRHASH( outEval ) );

			break;

		case AT_DATA:
			outEval->set( brDataHexDecode( inElement->_cdata.c_str() ) );

			break;

		case AT_POINTER:
			outEval->set( ( void* )NULL );

			break;

		case AT_INT:
			outEval->set( atoi( inElement->_cdata.c_str() ) );

			break;

		case AT_DOUBLE:
			outEval->set( atof( inElement->_cdata.c_str() ) );

			break;

		case AT_STRING:
			str = brXMLDecodeString( inElement->_cdata.c_str() );
			outEval->set( str );

			slFree( str );

			break;

		case AT_VECTOR:
			sscanf( inElement->_cdata.c_str(), "(%lf, %lf, %lf)", &v.x, &v.y, &v.z );
			outEval->set( v );

			break;

		case AT_MATRIX:
			sscanf( inElement->_cdata.c_str(), "[ (%lf, %lf, %lf), (%lf, %lf, %lf), (%lf, %lf, %lf) ]", 
				&m[0][0], &m[0][1], &m[0][2], 
				&m[1][0], &m[1][1], &m[1][2], 
				&m[2][0], &m[2][1], &m[2][2] );

			outEval->set( m );

			break;	
	
		default:
			printf( "Warning: could not parse eval for tag \"%s\"\n", inElement->_name.c_str() );
	}
}


/** 
 * Decodes a breve instance by decoding common breveInstance variables (dependencies, observers, etc), and 
 * then passing along the instance data to the appropriate language frontend handler.
 */

int brXMLDecodeInstance( brXMLParserState *inState, brXMLDOMElement *inInstanceElement, brInstance *outInstance ) {
	unsigned int i;

	brXMLDOMElement *dependencies = inInstanceElement->getChildByName( "dependencies" ); 
	
	if( dependencies ) {
		std::vector< brXMLDOMElement* > dependencyList = dependencies->getElementsByName( "object" );
			
		for( i = 0; i < dependencyList.size(); i++ ) {		
			int dependencyIndex = atoi( dependencyList[ i ]->getAttr( "index" )->c_str() );
		
			brInstance *dependency = inState->_indexToInstanceMap[ dependencyIndex ];

			if( dependency )
				brInstanceAddDependency( outInstance, dependency );
		}
	}
	
	std::vector< brXMLDOMElement* > observers = inInstanceElement->getElementsByName( "observers" ); 

	for( i = 0; i < observers.size(); i++ )
		brXMLDecodeObserver( inState, observers[ i ], outInstance );






	std::vector< brXMLDOMElement* > data = inInstanceElement->getElementsByName( "instancedata" ); 

	// Legacy support for the steve-only "variables" data...

	if( data.size() == 0 ) {
		data = inInstanceElement->getElementsByName( "variables" ); 

	} else {
		
		int typeSignature = 0xffffffff;
		brInstanceDecodeFromString( inState->engine, typeSignature, data[ 0 ]->_cdata.c_str() );

	}

	int typeSignature = atoi( inInstanceElement -> getAttr( "typesignature" )->c_str() );
	
	if( typeSignature != STEVE_TYPE_SIGNATURE ) {
		slMessage( DEBUG_ALL, "Warning: object encoding and decoding not fully implemented for non-steve objects\n" );

		return EC_ERROR;
	}

	return stXMLParseInstanceData( inState, data[ 0 ], (stInstance*)outInstance -> userData );
}

// Decode the "observer" data

void brXMLDecodeObserver( brXMLParserState *inState, brXMLDOMElement *inObserverElement, brInstance *inInstance ) {
	brInstance *observer = NULL;
	const char *notification = NULL, *method = NULL;

	brXMLDOMElement *observerElement     = inObserverElement->getChildByName( "object" );
	brXMLDOMElement *methodElement       = inObserverElement->getChildByName( "method" );
	brXMLDOMElement *notificationElement = inObserverElement->getChildByName( "notification" );
	
	if( observerElement ) {
		int observerIndex = atoi( observerElement->getAttr( "index" )->c_str() );
		observer = inState->_indexToInstanceMap[ observerIndex ];
	}

	if( methodElement )
		method = methodElement->getAttr( "name" )->c_str();


	if( notificationElement )
		notification = notificationElement->getAttr( "name" )->c_str();


	if( observer && notification && method )		
		brInstanceAddObserver( inInstance, observer, notification, method );
}

int brXMLEvalTypeForTagName( const char *name ) {


	if ( !strcasecmp( name, "array" ) ) return AT_ARRAY;
	if ( !strcasecmp( name, "int" ) ) return AT_INT;
	if ( !strcasecmp( name, "float" ) ) return AT_DOUBLE;
	if ( !strcasecmp( name, "list" ) ) return AT_LIST;
	if ( !strcasecmp( name, "hash" ) ) return AT_HASH;
	if ( !strcasecmp( name, "string" ) ) return AT_STRING;
	if ( !strcasecmp( name, "vector" ) ) return AT_VECTOR;
	if ( !strcasecmp( name, "pointer" ) ) return AT_POINTER;
	if ( !strcasecmp( name, "object" ) ) return AT_INSTANCE;
	if ( !strcasecmp( name, "data" ) ) return AT_DATA;
	if ( !strcasecmp( name, "matrix" ) ) return AT_MATRIX;

	slMessage( DEBUG_ALL, "warning: unknown tag \"%s\" in XML parse\n", name );

	return -1;
}

void stPrintXMLError( XML_Parser p ) {
	int line = XML_GetCurrentLineNumber( p );
	int col = XML_GetCurrentColumnNumber( p );

	slMessage( DEBUG_ALL, "Error at line %d (character %d) of breve XML archive file: %s\n", line, col, XML_ErrorString( XML_GetErrorCode( p ) ) );
}

/**
 * \brief Encode a string for XML export.
 *
 * Both the encode and decode XML string functions use hardcoded values
 * to parse out < > & ' and ".  A more general solution might be nice, but
 * whatever.
 * Returns an slMalloc'd string.
 */

char *brXMLEncodeString( const char *string ) {
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

	result[ m ] = 0;

	return result;
}

/**
 * \brief Decodes a string for XML import.
 * 
 * Both the encode and decode XML string functions use hardcoded values
 * to parse out < > & ' and ".  A more general solution might be nice, but
 * whatever.
 *
 * Returns an slMalloc'd string.
 */

char *brXMLDecodeString( const char *string ) {
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

	result[ m ] = 0;

	return result;
}

















/**
 * Dearchive a steve instance by filling in its variables.
 *
 * This method operates on steve objects only
 */

int stXMLParseInstanceData( brXMLParserState *inState, brXMLDOMElement *inInstanceData, stInstance *outInstance ) {

	std::vector< brXMLDOMElement* > classes   = inInstanceData->getElementsByName( "class" );

	// From the base class down to the derived class, we go through and set the variables for each one

	for( unsigned int n = 0; n < classes.size(); n++ ) {
		brXMLDOMElement *cls = classes[ n ];

		stRunInstance ri;

		ri.instance = outInstance;
		ri.type = ( stObject* )( brObjectFind( inState->engine, cls->getAttr( "name" )->c_str() ) ) -> userData;
			
		for( unsigned int e = 0; e < cls->_children.size(); e++ ) {
			brEval eval;
			brXMLDOMElement *variable = cls->_children[ e ];
		
			// Decode the eval and save it to the steve object
		
			brXMLParseEval( inState, variable, &eval );

			stVar *var = stObjectLookupVariable( outInstance->type, variable -> getAttr( "name" ) -> c_str() );

			if( ! var ) {
				slMessage( DEBUG_ALL, "Could not locate matching class variable for XML data\n" );
				slMessage( DEBUG_ALL, "Cannot decode XML archive: mismatch between simulation file and XML data\n" );
				return EC_ERROR;
			}
	
			stSetVariable( &outInstance->variables[ var->offset ], var->type->_type, NULL, &eval, &ri );
		}
	}

	return EC_OK;
}
