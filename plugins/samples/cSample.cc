#include <math.h>

#include "slBrevePluginAPI.h"

int cSampleTest( brEval [], brEval *, void * );
int cSampleTestII( brEval [], brEval *, void * );
int cSampleListTest( brEval [], brEval *, void * );

/* our entry-point function: one call to brNewBreveCall for each function */

DLLEXPORT void cSampleLoadFunctions( void *data ) {
	brNewBreveCall( data, "cSampleTest", cSampleTest, AT_NULL, AT_INT, AT_STRING, 0 );
	brNewBreveCall( data, "cSampleTestII", cSampleTestII, AT_DOUBLE, 0 );
	brNewBreveCall( data, "cSampleListTest", cSampleListTest, AT_NULL, AT_LIST, 0 );
}

/*
 * The first test function takes an int and string input, then prints
 * them to the log output using the slMessage() function.
 */

int cSampleTest( brEval args[], brEval *result, void *i ) {
	printf( "plugin function called with int %d and string \"%s\"\n", BRINT( &args[0] ), BRSTRING( &args[1] ) );

	return EC_OK;
}

/* The second test function just returns a value and has no inputs. */

int cSampleTestII( brEval args[], brEval *result, void *i ) {
	BRPOINTER( result ) = NULL;

	result->set( 3.14159 );

	return EC_OK;
}

int cSampleListTest( brEval *args, brEval *result, void *i ) {
	brEvalListHead *myListHead = BRLIST( &args[0] );
	int numNodes;
	double input1, input2;

	numNodes = myListHead->_vector.size();
	printf( "Total number of elements in list = %d.\n", numNodes );

	input1 = BRDOUBLE( myListHead->_vector[ 0 ] );
	printf( "First Input = %d.\n", input1 );

	input2 = BRDOUBLE( myListHead->_vector[ 1 ] );
	printf( "Second Input = %d.\n", input2 );

	return EC_OK;
}
