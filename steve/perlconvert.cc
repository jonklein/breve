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

#include "perlconvert.h"

std::string stPerlConvertFile( brEngine *inEngine, stSteveData *inSteveData, std::string &inFilename ) {
	std::string result, controller, aliases;
	unsigned int n;

	result += "\nuse strict;\n";
	result += "use breve;\n\n";

	for( n = 0; n < inSteveData->_paths[ inFilename ].size(); n++ ) {
		
	}

	for( n = 0; n < inSteveData->_includes[ inFilename ].size(); n++ ) {
		
	}

	for( n = 0; n < inSteveData->objects.size(); n++ ) {
		stObject *object = inSteveData->objects[ n ];

		if( object->_file == inFilename ) {
			result += stPerlConvertObject( object );

			if( inSteveData->controllerName && !strcmp( object->name.c_str(), inSteveData->controllerName ) )
				controller = object->name + "->new()";

			std::map< std::string, brObject* >::iterator mi;

			for( mi = inEngine->objectAliases.begin(); mi != inEngine->objectAliases.end(); mi++ ) {
				if( mi->second && mi->second->userData == object ) {
					aliases += "breve->{ " + mi->first + " } = " + object->name + "\n";
				}
			}
		}
	}


	result += aliases;
	result += "\n" + controller + ";\n";

	result += "\n1;\n";

	return result;
}

std::string stPerlConvertSymbol( std::string &inValue ) {
	std::string result;
	unsigned int n = 0, m = 0;
	bool upper = false;

	// Hardcode some troublesome symbols -- they're reserved keywords in Perl, but not steve

	if( inValue == "is" )
		return std::string( "isA" );

	if( inValue == "break" )
		return std::string( "snap" ); // hehe, this is fun

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

std::string stPerlConvertVariable( std::string &inValue, bool inLocal ) {
	if( inLocal ) 
		return "$" + stPerlConvertSymbol( inValue );
	else 
		return "$self->{ " + stPerlConvertSymbol( inValue ) + " }";
}

std::string stPerlConvertObject( stObject *inObject ) {
	stPerlConversionData conversionData( inObject );

	std::string result = "";
	std::map< std::string, std::vector< stMethod* > >::iterator mi;
	std::map< std::string, stVar* >::iterator vi;

	std::string supername = "";

	result = "package " + stPerlConvertSymbol( inObject->name ) + ";\n\n";

	if( inObject->_comment.size() > 0 ) {
		ADDTABS( &conversionData, result );
		result += "###" + inObject->_comment + "\n\n";
	}

	ADDTABS( &conversionData, result );
	result += "sub new {\n";
	conversionData._indents++;

	ADDTABS( &conversionData, result );
	result += "my $class = shift;\n";

	ADDTABS( &conversionData, result );
	result += "my $self = {};\n";
		
	ADDTABS( &conversionData, result );
	result += "bless $self, $class;\n";
		
	if( supername.size() != 0 ) {
		ADDTABS( &conversionData, result );
		result += "$self->SUPER::init( $self );\n";
	}

	for( vi = inObject->variables.begin(); vi != inObject->variables.end(); vi++ ) { 
		stVar *var = vi->second;

		ADDTABS( &conversionData, result );
		result += stPerlConvertVariableDeclaration( &conversionData, var, 1 );
	}

	if( inObject->methods[ "init" ].size() != 0 ) {
		ADDTABS( &conversionData, result );
		result += "init( $self );\n"; 
	}

	conversionData._indents--;

	ADDTABS( &conversionData, result );
	result += "}\n\n";

	for( mi = inObject->methods.begin(); mi != inObject->methods.end(); mi++ ) { 
		std::vector< stMethod* > &methodlist = mi->second;

		for( unsigned int m = 0; m < methodlist.size(); m++ ) { 
			result += stPerlConvertMethod( &conversionData, inObject, methodlist[ m ] );
		}
	}

	return result;
}

std::string stPerlConvertVariableDeclaration( stPerlConversionData *inData, stVar *inVar, bool inInstance ) {
	std::string result = stPerlConvertVariable( inVar->name, !inInstance );

	switch( inVar->type->_type ) {
		case AT_INT:
		case AT_DOUBLE:
			result += " = 0";
			break;
		case AT_POINTER:
		case AT_DATA:
		case AT_INSTANCE:
			result += " = undef";
			break;
		case AT_VECTOR:
			result += " = breve->vector()";
			break;
		case AT_MATRIX:
			result += " = breve->matrix()";
			break;
		case AT_STRING:
			result += " = \"\"";
			break;
		case AT_ARRAY:
		case AT_LIST:
			result += " = ()";
			break;
		case AT_HASH:
			result += " = {}";
			break;
		default:
			return UNIMPLEMENTED;
			break;
	}

	result += ";\n";

	return result;
}

std::string stPerlConvertMethod( stPerlConversionData *inData, stObject *inObject, stMethod *inMethod ) {
	std::string result;

	ADDTABS( inData, result );
	result += "sub " + stPerlConvertSymbol( inMethod->name ) + " {\n";

	inData->_indents++;

	if( inMethod->_comment.size() > 0 ) {
		ADDTABS( inData, result );
		result += "###" + inMethod->_comment + "\n\n";
	}

	//
	// Declare the variables
	// 
	ADDTABS( inData, result);

	if( inMethod->keywords.size() > 0 ) {
		result += "my ($self, ";

		for( unsigned int n = 0; n < inMethod->keywords.size(); n++ ) {
			if( n > 0 ) 
				result += ", ";

			result += "$" + stPerlConvertSymbol( inMethod->keywords[ n ]->var->name );
		}

		result += " );\n";
	} else {
		result += "my $self;\n";
	}
	

	//
	// Read in the arguments to local variables from @_
	// 

	ADDTABS( inData, result );
	result += "( $self";

	for( unsigned int n = 0; n < inMethod->keywords.size(); n++ ) {
		result += ", ";

		result += "$" + stPerlConvertSymbol( inMethod->keywords[ n ]->var->name );

		if( inMethod->keywords[ n ]->defaultKey ) {
			result += " = " + inMethod->keywords[ n ]->defaultKey->value->toPerl( inData );
		}
	}

	result += " ) = @_;\n";

	std::vector< stVar* >::iterator vi;
	for( vi = inMethod->variables.begin(); vi != inMethod->variables.end(); vi++ ) { 
		stVar *var = *vi;

		ADDTABS( inData, result );
		result += stPerlConvertVariableDeclaration( inData, var, 0 );
	}

	result += "\n";

	for( unsigned int c = 0; c < inMethod->code.size(); c++ ) {
		ADDTABS( inData, result );

		result += inMethod->code[ c ]->toPerl( inData );

		if( !inMethod->code[ c ]->isCompound() ) 
			result += ";\n";
		else
			result += "\n";
	}

	inData->_indents--;

	ADDTABS( inData, result );
	result += "}\n\n";


	return result;
}

std::string stSuperExp::toPerl( stPerlConversionData *inData ) {
	//
	// SuperExp is handled manually in the method call code for Perl
	// because it requires a change to the method call syntax.
	// 

	return std::string( UNIMPLEMENTED );
}

std::string stRandomExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result = "breve->randomExpression( " + expression->toPerl( inData ) + " )";

	return result;
}

std::string stInstanceExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result = "breve->createInstances( breve->" + stPerlConvertSymbol( name ) + ", " + count->toPerl( inData ) + " )";

	return result;
}

std::string stCodeArrayExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	for( unsigned int n = 0; n < expressions.size(); n++ ) {
		ADDTABS( inData, result );
		result += expressions[ n ]->toPerl( inData ) + ";\n";
	}

	return result;
}

std::string stDieExp::toPerl( stPerlConversionData *inData ) {
	return "raise Exception( " + expression->toPerl( inData ) + " )\n";
}

std::string stListExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result += "[";

	for( unsigned int n = 0; n < expressions.size(); n++ ) {
		result += " " + expressions[ n ]->toPerl( inData );

		if( n != expressions.size() - 1 )
			result += ",";
		else
			result += " ";
	}

	result += "]";
	
	return result;
}

std::string stSelfExp::toPerl( stPerlConversionData *inData ) {
	return std::string( "$self" );
}

std::string stCCallExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result = "breve->breveInternalFunctionFinder." + stPerlConvertSymbol( _function->_name ) + "( self";

	for( unsigned int n = 0; n < _arguments.size(); n++ ) {
		result += ", " + _arguments[ n ]->toPerl( inData );

		if( n == _arguments.size() - 1 )
			result += " ";
	}

	result += ")";

	return result;
}

std::string stPrintExp::toPerl( stPerlConversionData *inData ) {
	std::string result;
	result = "print";

	for( unsigned int n = 0; n < expressions.size(); n++ ) {
		result += " " + expressions[ n ]->toPerl( inData );

		if( n != expressions.size() - 1 )
			result += ",";
	}

	return result;
}

std::string stUnaryExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	switch( op ) {
		case UT_MINUS:
			result = "-";
			break;
		case UT_NOT:
			result = "not ";
			break;
	}

	return "( " + result + expression->toPerl( inData ) + " )";
}

std::string stWhileExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result = "while " + cond->toPerl( inData ) + " {\n";

	inData->_indents++;

	if( code->type != ET_CODE_ARRAY )
		ADDTABS( inData, result );
	
	result += code->toPerl( inData );

	if( !code->isCompound() && code->type != ET_CODE_ARRAY ) 
		result += ";";

	result += "\n";

	inData->_indents--;

	ADDTABS( inData, result );
	result += "}\n";

	return result;
}

std::string stAssignExp::toPerl( stPerlConversionData *inData ) {
	return stPerlConvertVariable( _word, _local ) + " = " + _rvalue->toPerl( inData );
}

std::string stBinaryExp::toPerl( stPerlConversionData *inData ) {
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

	leftstr = left->toPerl( inData );
	rightstr = right->toPerl( inData );

	return "( " + leftstr +  opstr + rightstr + " )";
}

std::string stLengthExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result = "breve->length( " + expression->toPerl( inData ) + " )";

	return result;
}

std::string stMatrixExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result = "breve->matrix( " ;

	for( unsigned int n = 0; n < 9; n++ ) {
		result += " " + expressions[ n ]->toPerl( inData );

		if( n < 8 )
			result += ",";
		else
			result += " ";
	}

	result += ")";

	return result;
}

std::string stMethodExp::toPerl( stPerlConversionData *inData ) {
	std::string result;
	std::vector< stKeyword > positioned;

	if( objectExp->type == ET_SUPER ) {
		// Stupid Perl special case with different syntax!

		result = "$self->SUPER::" + stPerlConvertSymbol( methodName ) + "(";
	} else {
		result = objectExp->toPerl( inData ) + "->" + stPerlConvertSymbol( methodName ) + "(";
	}

	for( unsigned int n = 0; n < arguments.size(); n++ ) {
		result += " " + arguments[ n ]->value->toPerl( inData );

		if( n < arguments.size() - 1 )
			result += ",";
		else
			result += " ";
	}

	result += ")";

	return result;
}

std::string stReturnExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result = "return";
	
	if( expression )
		result += " " + expression->toPerl( inData );
	
	return result;
}

std::string stVectorExp::toPerl( stPerlConversionData *inData ) {
	return "breve->vector( " + _x->toPerl( inData ) + ", " + _y->toPerl( inData ) + ", " + _z->toPerl( inData ) + " )";
}

std::string stForeachExp::toPerl( stPerlConversionData *inData ) {
	std::string result;
	std::string var;
	
	var = stPerlConvertVariable( assignment->_word, assignment->_local );
	result = "foreach " + var + " " + "(" + list->toPerl( inData ) + ") {\n";

	inData->_indents++;

	if( code->type != ET_CODE_ARRAY ) 
		ADDTABS( inData, result );
	
	result += code->toPerl( inData );

	if( !code->isCompound() && code->type != ET_CODE_ARRAY ) 
		result += ";";

	result += "\n";

	inData->_indents--;
	ADDTABS( inData, result );
	result += "}\n\n";

	return result;
}

std::string stCopyListExp::toPerl( stPerlConversionData *inData ) {
	std::string result = "list( " + expression->toPerl( inData ) + ") ";
	return result;
}

std::string stListIndexExp::toPerl( stPerlConversionData *inData ) {
	std::string result;
	
	result = listExp->toPerl( inData ) + "[ " + indexExp->toPerl( inData ) + " ]";

	return result;
}

std::string stArrayIndexExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	if( !local )
		result += "self.";

	result += stPerlConvertVariable( _variableName, local ) + "[ " + index->toPerl( inData ) + " ]";

	return result;
}

std::string stListInsertExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	if( index ) {
		result = listExp->toPerl( inData ) + "[ " + index->toPerl( inData ) + " ] = " + exp->toPerl( inData );
	} else {
		result = listExp->toPerl( inData ) + ".append( " + exp->toPerl( inData ) + " )";
	}

	return result;
}

std::string stListRemoveExp::toPerl( stPerlConversionData *inData ) {
	std::string result;
	result = "del " + listExp->toPerl( inData ) + "[ " + index->toPerl( inData ) + " ]";

	return result;
}

std::string stVectorElementExp::toPerl( stPerlConversionData *inData ) {
	std::string result = exp->toPerl( inData );

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

std::string stListIndexAssignExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result = listExp->toPerl( inData ) + "[ " + indexExp->toPerl( inData ) + " ] = " + assignment->toPerl( inData );;

	return result;
}

std::string stArrayIndexAssignExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	if( !local )
		result += "self.";

	result += stPerlConvertVariable( _variableName, local ) + "[ " + index->toPerl( inData ) + " ] = " + rvalue->toPerl( inData );

	return result;
}

std::string stVectorElementAssignExp::toPerl( stPerlConversionData *inData ) {
	std::string result = exp->toPerl( inData );

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

	result += " = " + assignExp->toPerl( inData );

	return result;
}

std::string stIfExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result += "if( " + cond->toPerl( inData ) + " ) {\n";

	inData->_indents++;

	if( trueCode->type != ET_CODE_ARRAY )
		ADDTABS( inData, result );
	
	result += trueCode->toPerl( inData ); 

	if( !trueCode->isCompound() && trueCode->type != ET_CODE_ARRAY ) 
		result += ";";

	result += "\n";

	inData->_indents--;
	ADDTABS( inData, result );

	result += "}";

	if( falseCode ) {
		ADDTABS( inData, result );
		result += " else {\n";

		inData->_indents++;

		if( falseCode->type != ET_CODE_ARRAY )
			ADDTABS( inData, result );

		result += falseCode->toPerl( inData );

		if( !falseCode->isCompound() && falseCode->type != ET_CODE_ARRAY ) 
			result += ";";

		result += "\n";

		inData->_indents--;

		ADDTABS( inData, result );
		result += "}";
	}

	result += "\n";

	return result;
}

std::string stAllExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result += "breve->allInstances( \"breve->" + stPerlConvertSymbol( name ) + "\" )";

	return result;
}

std::string stForExp::toPerl( stPerlConversionData *inData ) {
	std::string result;
	ADDTABS( inData, result );
	std::string number = "10";

	result += "foreach $i ( 0..";
	result += number;
	result += " ) {\n";

	ADDTABS( inData, result );
	inData->_indents++;
	result += code->toPerl( inData );
	inData->_indents--;

	ADDTABS( inData, result );
	result += " }\n";

	return std::string( "" );
}

std::string stEvalExp::toPerl( stPerlConversionData *inData ) {
	std::string result;
	char format[ 1024 ];

	switch( eval->type() ) {
		case AT_STRING:
			result += "\"";
			result +=  BRSTRING( eval );
			result += "\"";
			break;

		case AT_VECTOR:
			sprintf( format, "breve->vector( %f, %f, %f )", BRVECTOR( eval ).x, BRVECTOR( eval ).y, BRVECTOR( eval ).z );
			result = format;
			break;

		case AT_MATRIX:
			sprintf( format, "breve->matrix( %f, %f, %f, %f, %f, %f, %f, %f, %f )", 
				BRMATRIX( eval )[ 0 ][ 0 ], BRMATRIX( eval )[ 0 ][ 1 ], BRMATRIX( eval )[ 0 ][ 2 ], 
				BRMATRIX( eval )[ 1 ][ 0 ], BRMATRIX( eval )[ 1 ][ 1 ], BRMATRIX( eval )[ 1 ][ 2 ], 
				BRMATRIX( eval )[ 2 ][ 0 ], BRMATRIX( eval )[ 2 ][ 1 ], BRMATRIX( eval )[ 2 ][ 2 ] );
			result = format;
			break;

		case AT_INT:
			sprintf( format, "%d", BRINT( eval ) );
			result = format;
			break;

		case AT_DOUBLE:
			sprintf( format, "%f", BRFLOAT( eval ) );
			result = format;
			break;

		default:
			result = UNIMPLEMENTED;
			break;
	}

	return result;
}

std::string stFreeExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	result = "breve->deleteInstances( " + expression->toPerl( inData ) + " )";

	return result;
}

std::string stSortExp::toPerl( stPerlConversionData *inData ) {
	return std::string( "" );
}

std::string stDoubleExp::toPerl( stPerlConversionData *inData ) {
	char numstr[ 1024 ];
	sprintf( numstr, "%f", doubleValue );
	std::string result = numstr;
	return result;
}

std::string stDuplicateExp::toPerl( stPerlConversionData *inData ) {
	return expression->toPerl( inData );
}

std::string stIntExp::toPerl( stPerlConversionData *inData ) {
	char numstr[ 1024 ];
	sprintf( numstr, "%i", intValue );
	std::string result = numstr;
	return result;
}

std::string stLoadExp::toPerl( stPerlConversionData *inData ) {
	return stPerlConvertVariable( _word, local );
}

std::string stStringExp::toPerl( stPerlConversionData *inData ) {
	std::string result, processed;

	if( substrings.size() == 0 ) {
		if( string.find( " " ) == std::string::npos ) {
			// Could it maybe be a method name -- we'll have to guess
			return "\"" + stPerlConvertSymbol( string ) + "\"";
		}

		return "\"" + string + "\"";
	}

	processed = string;
	int n;

	// go backwards through the string so that the offsets are maintained

	for( n = substrings.size() - 1; n >= 0; n-- ) {
		stSubstringExp *substring = substrings[ n ];

		processed.replace( substring->offset, 0, "%s" );
	}

	result = "\"" + processed + "\" % ( ";

	for( unsigned n = 0; n < substrings.size(); n++ ) {
		result += " " + substrings[ n ]->toPerl( inData );

		if( n < substrings.size() - 1 )
			result += ",";
		else
			result += " ";
	}
		
	result += ")";

	return result;

}

std::string stSubstringExp::toPerl( stPerlConversionData *inData ) {
	return loadExp->toPerl( inData );
}

std::string stCommentExp::toPerl( stPerlConversionData *inData ) {
	std::string result;

	ADDTABS( inData, result );
	result += "# " + _comment;

	return result;
}

