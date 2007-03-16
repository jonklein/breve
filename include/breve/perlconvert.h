#ifndef _PERLCONVERT_H
#define _PERLCONVERT_H

#include "steve.h"

struct stPerlConversionData {
	stPerlConversionData( stObject *inObject ) {
		_indents 	= 0;
		_currentObject 	= inObject;
	}

	int                     _indents;
	stObject                *_currentObject;
};

std::string stPerlConvertObject( stObject *inObject );
std::string stPerlConvertFile( brEngine *inEngine, stSteveData *inSteveData, std::string &inFilename );
std::string stPerlConvertVariableDeclaration( stPerlConversionData *inData, stVar *inVar, bool inInstance );
std::string stPerlConvertMethod( stPerlConversionData *inData, stObject *inObject, stMethod *inMethod );

#include "expression.h"

#endif
