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

#ifdef __cplusplus
extern "C" {
#endif
#include <expat.h>
#ifdef __cplusplus
}
#endif

/**
 * \brief Data used when preforming an archive.
 */

struct brXMLArchiveRecord {
	std::set< brInstance*, brInstanceCompare > 	_written;
	brEvalHash					_instanceToIndexMap;
};

/**
 * \brief Data used when parsing an XML archive.
 */

struct brXMLParserState {
	brXMLParserState() {
		engine = NULL;
		error = 0;
	}

	int 				error;
	brEngine*			engine;

	brEvalHash			_indexToInstanceMap;
};

struct brXMLDOMElement {
	brXMLDOMElement( char *inName ) {
		_name = inName;
	}

	~brXMLDOMElement();

	std::vector< brXMLDOMElement* >			_children;
	std::string 					_name;
	std::string 					_cdata;
	std::map< std::string, std::string >		_attrs;

	std::vector< brXMLDOMElement* >			getElementsByName( const char *inStr );
	brXMLDOMElement*				getChildByName( const char *inStr );
	const std::string*				getAttr( const char *inAttr );

	std::string					toXMLString( int inDepth = 0 );
};

int brXMLAssignIndices( brEngine *, std::map< brInstance*, int>&, brEvalHash *inMap );

int brXMLWriteObjectToFile( brInstance *, char *, int );
int brXMLWriteObject( brXMLArchiveRecord *, FILE *, brInstance *, int, int );
int brXMLWriteObjectToStream( brInstance *, FILE *, int );

int brXMLWriteSimulationToFile(char *, brEngine *);
int brXMLWriteSimulationToStream(FILE *, brEngine *);

int brXMLRunDearchiveMethods( brXMLDOMElement *inRoot, brXMLParserState *inState );
int brXMLPrepareInstanceMap( brXMLDOMElement *inRoot, brXMLParserState *inState );

int brXMLDecodeInstance( brXMLParserState *inState, brXMLDOMElement *inInstanceElement, brInstance *inInstance );

void brXMLDecodeObserver( brXMLParserState *inState, brXMLDOMElement *inObserverElement, brInstance *inInstance );

int stXMLReadObjectFromFile(stInstance *i, char *);
int stXMLReadObjectFromStream(stInstance *i, FILE *);
int stXMLReadObjectFromString(stInstance *i, char *);

brInstance *brXMLDearchiveObjectFromFile(brEngine *, char *);
brInstance *brXMLDearchiveObjectFromStream(brEngine *, FILE *);
brInstance *brXMLDearchiveObjectFromString(brEngine *, char *);

int brXMLInitSimulationFromFile(brEngine *, char *);
int brXMLInitSimulationFromStream(brEngine *, FILE *);
int brXMLInitSimulationFromString(brEngine *, char *);

int stXMLVariablePrint(brXMLArchiveRecord *, FILE *, stVar *, stInstance *, int);
int brXMLPrintEval(brXMLArchiveRecord *, FILE *, const char *, brEval *, int);
int brXMLPrintList(brXMLArchiveRecord *, FILE *, const char *, brEvalListHead *, int);
int brXMLPrintHash(brXMLArchiveRecord *, FILE *, const char *, brEvalHash *, int);

int brXMLEvalTypeForTagName( const char * );

void stXMLObjectStartElementHandler(void *, const XML_Char *, const XML_Char **);
void stXMLObjectCharacterDataHandler(void *, const XML_Char *, int);
void stXMLObjectEndElementHandler(void *, const XML_Char *);

XML_Parser stExternalEntityParserCreate(XML_Parser, const XML_Char *, const XML_Char *);

void stPrintXMLError(XML_Parser);

char *brXMLEncodeString( const char * );
char *brXMLDecodeString( const char * );

int stXMLParseInstanceData( brXMLParserState *inState, brXMLDOMElement *inInstanceData, stInstance *outInstance );

void brXMLParseEval( brXMLParserState *inState, brXMLDOMElement *inElement, brEval *outEval );

brXMLDOMElement *brXMLParse( char *inText );
brXMLDOMElement *brXMLParseFile( char *inPath );


