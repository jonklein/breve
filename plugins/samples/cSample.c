#include "slBrevePluginAPI.h"
#include <math.h>

int cSampleTest(brEval args[], brEval *result, void *i);
int cSampleTestII(brEval args[], brEval *result, void *i);
int cSampleListTest(brEval args[], brEval *result, void *i);

/* our entry-point function: one call to brNewBreveCall for each function */

#ifdef WINDOWS
asm(".section .drectve");
asm(".ascii \"-export:cSampleLoadFunctions\"");
#endif /* WINDOWS */

cSampleLoadFunctions(void *data) {
	brNewBreveCall(data, "cSampleTest", cSampleTest, AT_NULL, AT_INT, AT_STRING, 0);
	brNewBreveCall(data, "cSampleTestII", cSampleTestII, AT_DOUBLE, 0);
	brNewBreveCall(data, "cSampleListTest", cSampleListTest, AT_NULL, AT_LIST, 0);
}

/* the first test function takes an int and string input, then prints *
 * them to the log output using the function slMessage.               */

int cSampleTest(brEval args[], brEval *result, void *i) {
	slMessage(0, "plugin function called with int %d and string \"%s\"\n", BRINT(&args[0]), BRSTRING(&args[1]));
	return EC_OK;
}

/* the second test function just returns a value and has no inputs */

int cSampleTestII(brEval args[], brEval *result, void *i) {
	BRDOUBLE(result) = 3.14159;

	return EC_OK;
}

int cSampleListTest(brEval *args, brEval *result, void *i) { 
	brEvalListHead *myListHead = BRLIST(&args[0]);
	brEvalList *list_ptr;
	int numNodes;
	double input1;
	double input2;

	numNodes = myListHead->count;
	printf("Total number of elements in list = %d.\n", numNodes);
	list_ptr = myListHead->start;
	input1 = BRDOUBLE(&list_ptr->eval);
	printf("First Input = %d.\n", input1);
	list_ptr = list_ptr->next;
	input2 = BRDOUBLE(&list_ptr->eval);
	printf("Second Input = %d.\n", input2);
	return EC_OK;
}
