#include "slBrevePluginAPI.h"

//
// IMPORTANT: we must define the entry-point function with 'extern "C"' 
// so that breve can find it.
//

extern "C" void cppSampleLoadFunctions(void *data);

int cppSampleMakeTestClass(stEval args[], stEval *target, void *i);
int cppSampleSetI(stEval args[], stEval *target, void *i);
int cppSampleSetJ(stEval args[], stEval *target, void *i);
int cppSampleGetSum(stEval args[], stEval *target, void *i);

#ifdef WINDOWS
asm(".section .drectve");
asm(".ascii \"-export:cppSampleLoadFunctions\"");
#endif /* WINDOWS */


//
// Define our simple C++ class.
//  

class cppSampleTestClass {
	public:
		int setI(int i) { _i = i; }
		int setJ(int j) { _j = j; }
		int getSum() { return _i + _j; }

	private:
		int _i, _j;
};

//
// Create wrapper functions around class instantiation and the 
// class methods we're interested in.
//

int cppSampleMakeTestClass(stEval args[], stEval *target, void *i) {
	cppSampleTestClass *tc = new cppSampleTestClass();

	// we create an instance, and return it to breve as a pointer type

	STPOINTER(target) = tc;

	return EC_OK;
}

int cppSampleSetI(stEval args[], stEval *target, void *i) {
	// the breve call passes the instance back to us, so we can call the method

	cppSampleTestClass *tc = (cppSampleTestClass*)STPOINTER(&args[0]);
	tc->setI(STINT(&args[1]));

	return EC_OK;
}

int cppSampleSetJ(stEval args[], stEval *target, void *i) {
	// the breve call passes the instance back to us, so we can call the method

	cppSampleTestClass *tc = (cppSampleTestClass*)STPOINTER(&args[0]);
	tc->setJ(STINT(&args[1]));

	return EC_OK;
}

int cppSampleGetSum(stEval args[], stEval *target, void *i) {
	// the breve call passes the instance back to us, so we can call the method
		
	cppSampleTestClass *tc = (cppSampleTestClass*)STPOINTER(&args[0]);

	// we return the result as a breve integer

	STINT(target) = tc->getSum();

	return EC_OK;
}

//
// IMPORTANT: we must define the entry-point function with 'extern "C"' 
// so that breve can find it.
//

extern "C" void cppSampleLoadFunctions(void *data) {
	// make a call to stNewSteveCall for each wrapper function
	
	stNewSteveCall(data, "cppSampleMakeTestClass", cppSampleMakeTestClass, AT_POINTER, 0);
	stNewSteveCall(data, "cppSampleSetI", cppSampleSetI, AT_NULL, AT_POINTER, AT_INT, 0);
	stNewSteveCall(data, "cppSampleSetJ", cppSampleSetJ, AT_NULL, AT_POINTER, AT_INT, 0);
	stNewSteveCall(data, "cppSampleGetSum", cppSampleGetSum, AT_INT, AT_POINTER, 0);
}
