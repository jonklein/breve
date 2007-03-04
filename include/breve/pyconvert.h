#ifndef _PYCONVERT_H
#define _PYCONVERT_H

#include "steve.h"

struct stPyConversionData {
	stPyConversionData( stObject *inObject ) {
		_indents 	= 0;
		_currentObject 	= inObject;
	}

	int                     _indents;
	stObject                *_currentObject;
};

std::string stPyConvertObject( stObject *inObject );
std::string stPyConvertFile( brEngine *inEngine, stSteveData *inSteveData, std::string &inFilename );
std::string stPyConvertVariableDeclaration( stPyConversionData *inData, stVar *inVar, bool inInstance );
std::string stPyConvertMethod( stPyConversionData *inData, stObject *inObject, stMethod *inMethod );

#include "expression.h"

#endif
