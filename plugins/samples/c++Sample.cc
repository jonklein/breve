#include "slBrevePluginAPI.h"

//
// IMPORTANT: We must declare the entry-point function as DLLEXPORT so
// that breve can find it.
//

DLLEXPORT void cppSampleLoadFunctions(void *);

int cppSampleMakeTestClass(brEval [], brEval *, void *);
int cppSampleSetI(brEval [], brEval *, void *);
int cppSampleSetJ(brEval [], brEval *, void *);
int cppSampleGetSum(brEval [], brEval *, void *);

//
// Define our simple C++ class.
// 

class cppSampleTestClass {
	public:
		void setI(int i) { _i = i; }
		void setJ(int j) { _j = j; }
		int getSum() { return _i + _j; }

	private:
		int _i, _j;
};

//
// Create wrapper functions around class instantiation and the 
// class methods we're interested in.
//

int cppSampleMakeTestClass(brEval args[], brEval *target, void *i) {
	cppSampleTestClass *tc = new cppSampleTestClass();

	// we create an instance, and return it to breve as a pointer type

	target->set( tc );

	return EC_OK;
}

int cppSampleSetI(brEval args[], brEval *target, void *i) {
	// breve passes the instance back to us, so we can call the method

	cppSampleTestClass *tc = (cppSampleTestClass *)BRPOINTER(&args[0]);
	tc->setI(BRINT(&args[1]));

	return EC_OK;
}

int cppSampleSetJ(brEval args[], brEval *target, void *i) {
	cppSampleTestClass *tc = (cppSampleTestClass *)BRPOINTER(&args[0]);
	tc->setJ(BRINT(&args[1]));

	return EC_OK;
}

int cppSampleGetSum(brEval args[], brEval *target, void *i) {
	cppSampleTestClass *tc = (cppSampleTestClass *)BRPOINTER(&args[0]);

	// we return the result as a breve integer

	target->set( tc->getSum() );

	return EC_OK;
}

DLLEXPORT void cppSampleLoadFunctions(void *data) {
	// make a call to brNewBreveCall for each wrapper function
	
	brNewBreveCall(data, "cppSampleMakeTestClass", cppSampleMakeTestClass, AT_POINTER, 0);
	brNewBreveCall(data, "cppSampleSetI", cppSampleSetI, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(data, "cppSampleSetJ", cppSampleSetJ, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(data, "cppSampleGetSum", cppSampleGetSum, AT_INT, AT_POINTER, 0);
}
