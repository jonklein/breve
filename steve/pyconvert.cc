
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

#define UNIMPLEMENTED			std::string( "*** UNIMPLEMENTED ***" )
#define ADDTABS( data, string )		for( int __t= 0; __t < (data)->_indents; __t++ ) string += "\t"

#include "pyconvert.h"

std::string stPyConvertFile( stSteveData *inSteveData, std::string &inFilename ) {
	std::string result;
	unsigned int n;

	result += "\nimport breve\n\n";

	for( n = 0; n < inSteveData->_paths[ inFilename ].size(); n++ ) {
		
	}

	for( n = 0; n < inSteveData->_includes[ inFilename ].size(); n++ ) {
		
	}

	for( n = 0; n < inSteveData->objects.size(); n++ ) {
		stObject *object = inSteveData->objects[ n ];

		if( object->_file == inFilename ) {
			result += stPyConvertObject( object );
		}
	}

	return result;
}

std::string stPyConvertSymbol( std::string &inValue ) {
	std::string result;
	unsigned int n = 0, m = 0;
	bool upper = false;

	// Hard code this one troublesome symbol -- it's a reserved 
	// keyword in Python, but not steve

	if( inValue == "is" )
		return std::string( "isA" );

	for( n = 0; n < inValue.size(); n++, m++ ) {
		if( inValue[ n ] == '-' ) {
			m--;
			upper = true;
		} else {

			if( upper ) {
				result += toupper( inValue[ n ] );
			} else {
				result += inValue[ n ];
			}

			upper = false;
		}
	}

	return result;
}

std::string stPyConvertObject( stObject *inObject ) {
	stPyConversionData conversionData( inObject );

	conversionData._indents++;

	std::string result = "";
	std::map< std::string, std::vector< stMethod* > >::iterator mi;
	std::map< std::string, stVar* >::iterator vi;

	std::string supername = "";

	if( inObject->super ) {
		supername = "breve." + stPyConvertSymbol( inObject->super->name );
		result = "class " + stPyConvertSymbol( inObject->name ) + "( " + supername + " ):\n";
	} else {
		result = "class " + stPyConvertSymbol( inObject->name ) + ":\n";
	}

	ADDTABS( &conversionData, result );
	result += "'''" + inObject->_comment + "'''\n\n";

	if( supername.size() != 0 ) {
		ADDTABS( &conversionData, result );
		result += "def __init__( self ):\n";
		conversionData._indents++;
		ADDTABS( &conversionData, result );
		result += supername + ".__init__( self )\n";

		for( vi = inObject->variables.begin(); vi != inObject->variables.end(); vi++ ) { 
			stVar *var = vi->second;

			ADDTABS( &conversionData, result );
			result += stPyConvertVariableDeclaration( &conversionData, var, 1 );
		}

		if( inObject->methods[ "init" ].size() != 0 ) {
			ADDTABS( &conversionData, result );
			result += stPyConvertSymbol( inObject->name ) + ".init( self )\n"; 
		}

		result += "\n";

		conversionData._indents--;
	}

	for( mi = inObject->methods.begin(); mi != inObject->methods.end(); mi++ ) { 
		std::vector< stMethod* > &methodlist = mi->second;

		for( unsigned int m = 0; m < methodlist.size(); m++ ) { 
			result += stPyConvertMethod( &conversionData, inObject, methodlist[ m ] );
		}
	}

	conversionData._indents--;

	result += "\nbreve." + stPyConvertSymbol( inObject->name ) + " = " + stPyConvertSymbol( inObject->name ) + "\n\n";

	return result;
}

std::string stPyConvertVariableDeclaration( stPyConversionData *inData, stVar *inVar, bool inInstance ) {
	std::string result;

	if( inInstance )
		result += "self.";

	result += stPyConvertSymbol( inVar->name );

	switch( inVar->type->_type ) {
		case AT_INT:
		case AT_DOUBLE:
			result += " = 0\n";
			break;
		case AT_POINTER:
		case AT_DATA:
		case AT_INSTANCE:
			result += " = None\n";
			break;
		case AT_VECTOR:
			result += " = breve.vector()\n";
			break;
		case AT_MATRIX:
			result += " = breve.matrix()\n";
			break;
		case AT_STRING:
			result += " = ''\n";
			break;
		case AT_LIST:
			result += " = []\n";
			break;
		case AT_HASH:
			result += " = {}\n";
			break;
		default:
			return UNIMPLEMENTED;
			break;
	}

	return result;
}

std::string stPyConvertMethod( stPyConversionData *inData, stObject *inObject, stMethod *inMethod ) {
	std::string result;

	ADDTABS( inData, result );
	result += "def " + stPyConvertSymbol( inMethod->name ) + "( self";


	for( unsigned int n = 0; n < inMethod->keywords.size(); n++ ) {
		result += ", " + stPyConvertSymbol( inMethod->keywords[ n ]->var->name );
	}

	result += " ):\n";

	inData->_indents++;

	ADDTABS( inData, result );
	result += "'''" + inMethod->_comment + "'''\n\n";

	std::vector< stVar* >::iterator vi;
	for( vi = inMethod->variables.begin(); vi != inMethod->variables.end(); vi++ ) { 
		stVar *var = *vi;

		ADDTABS( inData, result );
		result += stPyConvertVariableDeclaration( inData, var, 0 );
	}

	result += "\n";

	for( unsigned int c = 0; c < inMethod->code.size(); c++ ) {
		ADDTABS( inData, result );
		result += inMethod->code[ c ]->toPython( inData ) + "\n";
	}

	inData->_indents--;

	result += "\n";


	return result;
}


std::string stSuperExp::toPython( stPyConversionData *inData ) {
	std::string result;

	if( inData->_currentObject->super ) {
		result = "breve." + stPyConvertSymbol( inData->_currentObject->super->name );
	}

	return result;
}

std::string stRandomExp::toPython( stPyConversionData *inData ) {
	std::string result;

	result = "breve.randomExpression( " + expression->toPython( inData ) + " )";

	return result;
}

std::string stInstanceExp::toPython( stPyConversionData *inData ) {
	std::string result;

	result = "breve.createInstances( breve." + stPyConvertSymbol( name ) + ", " + count->toPython( inData ) + " )";

	return result;
}

std::string stCodeArrayExp::toPython( stPyConversionData *inData ) {
	std::string result;

	for( unsigned int n = 0; n < expressions.size(); n++ ) {
		ADDTABS( inData, result );
		result += expressions[ n ]->toPython( inData ) + "\n";
	}

	return result;
}

std::string stDieExp::toPython( stPyConversionData *inData ) {
	return "raise Exception( " + expression->toPython( inData ) + " )\n";
}

std::string stListExp::toPython( stPyConversionData *inData ) {
	std::string result;

	result += "[";

	for( unsigned int n = 0; n < expressions.size(); n++ ) {
		result += " " + expressions[ n ]->toPython( inData );

		if( n != expressions.size() - 1 )
			result += ",";
		else
			result += " ";
	}

	result += "]";
	
	return result;
}

std::string stSelfExp::toPython( stPyConversionData *inData ) {
	return std::string( "self" );
}

std::string stCCallExp::toPython( stPyConversionData *inData ) {
	std::string result;

	result = "breve.breveInternalFunctionFinder." + stPyConvertSymbol( _function->_name ) + "( self";

	for( unsigned int n = 0; n < _arguments.size(); n++ ) {
		result += ", " + _arguments[ n ]->toPython( inData );

		if( n == _arguments.size() - 1 )
			result += " ";
	}

	result += ")";

	return result;
}

std::string stPrintExp::toPython( stPyConversionData *inData ) {
	std::string result;
	result = "print";

	for( unsigned int n = 0; n < expressions.size(); n++ ) {
		result += " " + expressions[ n ]->toPython( inData );

		if( n != expressions.size() - 1 )
			result += ",";
	}

	return result;
}

std::string stUnaryExp::toPython( stPyConversionData *inData ) {
	std::string result;

	switch( op ) {
		case UT_MINUS:
			result = "-";
			break;
		case UT_NOT:
			result = "not ";
			break;
	}

	return "( " + result + expression->toPython( inData ) + " )";
}

std::string stWhileExp::toPython( stPyConversionData *inData ) {
	std::string result;

	result = "while " + cond->toPython( inData ) + ":\n";

	inData->_indents++;

	if( code->type != ET_CODE_ARRAY )
		ADDTABS( inData, result );
	
	result += code->toPython( inData ) + "\n";
	inData->_indents--;

	return std::string( "" );
}

std::string stAssignExp::toPython( stPyConversionData *inData ) {
	std::string result = "";

	if( !_local )
		result += "self.";

	result += stPyConvertSymbol( _word ) + " = " + _rvalue->toPython( inData );

	return result;
}

std::string stBinaryExp::toPython( stPyConversionData *inData ) {
	std::string leftstr, rightstr, opstr;
	
	switch( op ) {
		case BT_ADD:
			opstr = " + ";
			break;
		case BT_SUB:
			opstr = " - ";
			break;
		case BT_MUL:
			opstr = " * ";
			break;
		case BT_DIV:
			opstr = " / ";
			break;
		case BT_MOD:
			opstr = " % ";
			break;
		case BT_POW:
			opstr = " ** ";
			break;
		case BT_EQ:
			opstr = " == ";
			break;
		case BT_NE:
			opstr = " != ";
			break;
		case BT_GT:
			opstr = " > ";
			break;
		case BT_GE:
			opstr = " >= ";
			break;
		case BT_LT:
			opstr = " < ";
			break;
		case BT_LE:
			opstr = " <= ";
			break;
		case BT_LAND:
			opstr = " and ";
			break;
		case BT_LOR:
			opstr = " or ";
			break;
		default:
			opstr = "[ unknown operator ]";
			break;
	}

	leftstr = left->toPython( inData );
	rightstr = right->toPython( inData );

	return "( " + leftstr +  opstr + rightstr + " )";
}

std::string stLengthExp::toPython( stPyConversionData *inData ) {
	std::string result;

	result = "breve.length( " + expression->toPython( inData ) + " )";

	return result;
}

std::string stMatrixExp::toPython( stPyConversionData *inData ) {
	std::string result;

	result = "breve.matrix( " ;

	for( unsigned int n = 0; n < 9; n++ ) {
		result += " " + expressions[ n ]->toPython( inData );

		if( n < 8 )
			result += ",";
		else
			result += " ";
	}

	result += ")";

	return result;
}

std::string stMethodExp::toPython( stPyConversionData *inData ) {
	std::string result;
	std::vector< stKeyword > positioned;

	result = objectExp->toPython( inData ) + "." + stPyConvertSymbol( methodName ) + "(";

	if( objectExp->type == ET_SUPER ) 
		result += " self,";

	for( unsigned int n = 0; n < arguments.size(); n++ ) {
		result += " " + arguments[ n ]->value->toPython( inData );

		if( n < arguments.size() - 1 )
			result += ",";
		else
			result += " ";
	}

	result += ")";

	return result;
}

std::string stReturnExp::toPython( stPyConversionData *inData ) {
	std::string result;

	result = "return";
	
	if( expression )
		result += " " + expression->toPython( inData );
	
	result += "\n";

	return result;
}

std::string stVectorExp::toPython( stPyConversionData *inData ) {
	return "breve.vector( " + _x->toPython( inData ) + ", " + _y->toPython( inData ) + ", " + _z->toPython( inData ) + " )";
}

std::string stForeachExp::toPython( stPyConversionData *inData ) {
	std::string result;
	std::string var;

	if( assignment->_local )
		var = assignment->_word;
	else 
		var = "self." + assignment->_word;

	result = "for " + var + " in " + list->toPython( inData ) + ":\n";

	inData->_indents++;
	if( code->type != ET_CODE_ARRAY ) 
		ADDTABS( inData, result );
	
	result += code->toPython( inData ) + "\n";
	inData->_indents--;

	return result;
}

std::string stCopyListExp::toPython( stPyConversionData *inData ) {
	return std::string( "" );
}

std::string stListIndexExp::toPython( stPyConversionData *inData ) {
	std::string result;
	
	result = listExp->toPython( inData ) + "[ " + indexExp->toPython( inData ) + " ]";

	return result;
}

std::string stArrayIndexExp::toPython( stPyConversionData *inData ) {
	return UNIMPLEMENTED;
}

std::string stListInsertExp::toPython( stPyConversionData *inData ) {
	std::string result;

	if( index ) {
		result = listExp->toPython( inData ) + "[ " + index->toPython( inData ) + " ] = " + exp->toPython( inData );
	} else {
		result = listExp->toPython( inData ) + ".append( " + exp->toPython( inData ) + " )";
	}

	return result;
}

std::string stListRemoveExp::toPython( stPyConversionData *inData ) {
	std::string result;
	result = "del " + listExp->toPython( inData ) + "[ " + index->toPython( inData ) + " ]";

	return result;
}

std::string stVectorElementExp::toPython( stPyConversionData *inData ) {
	std::string result = exp->toPython( inData );

	switch( element ) {
		case VE_X:
			result += ".x";
			break;
		case VE_Y:
			result += ".y";
			break;
		case VE_Z:
			result += ".z";
			break;
	}

	return result;
}

std::string stListIndexAssignExp::toPython( stPyConversionData *inData ) {
	std::string result;

	result = listExp->toPython( inData ) + "[ " + indexExp->toPython( inData ) + " ] = " + assignment->toPython( inData );;

	return result;
}

std::string stArrayIndexAssignExp::toPython( stPyConversionData *inData ) {
	return std::string( "UNSUPPORTED" );
}

std::string stVectorElementAssignExp::toPython( stPyConversionData *inData ) {
	std::string result = exp->toPython( inData );

	switch( element ) {
		case VE_X:
			result += ".x";
			break;
		case VE_Y:
			result += ".y";
			break;
		case VE_Z:
			result += ".z";
			break;
	}

	result += " = " + assignExp->toPython( inData );

	return result;
}

std::string stIfExp::toPython( stPyConversionData *inData ) {
	std::string result;

	result += "if " + cond->toPython( inData ) + ":\n";

	inData->_indents++;

	if( trueCode->type != ET_CODE_ARRAY )
		ADDTABS( inData, result );
	
	result += trueCode->toPython( inData ) + "\n";
	inData->_indents--;

	if( falseCode ) {
		ADDTABS( inData, result );
		result += "else:\n";

		inData->_indents++;

		if( falseCode->type != ET_CODE_ARRAY )
			ADDTABS( inData, result );

		result += falseCode->toPython( inData ) + "\n";
		inData->_indents--;
	}

	return result;
}

std::string stAllExp::toPython( stPyConversionData *inData ) {
	std::string result;

	ADDTABS( inData, result );

	result += "breve.allInstances( \"breve." + stPyConvertSymbol( name ) + "\" )";

	return result;
}

std::string stForExp::toPython( stPyConversionData *inData ) {
	std::string result;
	ADDTABS( inData, result );

	result = "for __i in range( 10 ):\n";
	result += code->toPython( inData );

	return std::string( "" );
}

std::string stEvalExp::toPython( stPyConversionData *inData ) {
	return std::string( "" );
}

std::string stFreeExp::toPython( stPyConversionData *inData ) {
	std::string result;

	result = "del " + expression->toPython( inData );

	return result;
}

std::string stSortExp::toPython( stPyConversionData *inData ) {
	return std::string( "" );
}

std::string stDoubleExp::toPython( stPyConversionData *inData ) {
	char numstr[ 1024 ];
	sprintf( numstr, "%f", doubleValue );
	std::string result = numstr;
	return result;
}

std::string stDuplicateExp::toPython( stPyConversionData *inData ) {
	return expression->toPython( inData );
}

std::string stIntExp::toPython( stPyConversionData *inData ) {
	char numstr[ 1024 ];
	sprintf( numstr, "%i", intValue );
	std::string result = numstr;
	return result;
}

std::string stLoadExp::toPython( stPyConversionData *inData ) {
	std::string result;

	if( !local )
		result += "self.";

	return result + stPyConvertSymbol( _word );
}

std::string stStringExp::toPython( stPyConversionData *inData ) {
	std::string result;

	if( substrings.size() == 0 ) {
		if( string.find( " " ) == std::string::npos ) {
			// Could it maybe be a method name -- we'll have to guess
			return "'''" + stPyConvertSymbol( string ) + "'''";
		}

		return "'''" + string + "'''";
	}

	result = "'''" + string + "''' % ( ";

	for( unsigned int n = 0; n < substrings.size(); n++ ) {
		result += " " + substrings[ n ]->toPython( inData );

		if( n < substrings.size() - 1 )
			result += ",";
		else
			result += " ";
	}
		
	result += ")";

	return result;

}

std::string stSubstringExp::toPython( stPyConversionData *inData ) {
	return loadExp->toPython( inData );
}

std::string stCommentExp::toPython( stPyConversionData *inData ) {
	std::string result;

	ADDTABS( inData, result );
	result += "# " + _comment;

	return result;
}

