#include "kernel.h"

#ifdef HAVE_LIBPERL

#include "perlInterface.h"

static char arbitraryObjectName[8];

char* newArbitraryName() {
	for(int i = 0; i < 8; i++) {
		if(arbitraryObjectName[i] < 97) {
			arbitraryObjectName[i] = 'a';
		}
		if(arbitraryObjectName[i] != 'z') {
			arbitraryObjectName[i]++;
			break;
		}
	}
	return arbitraryObjectName;
}

/**
 * Perl callback to add a Perl instance to the breve engine.
 */
brInstance *brPerlAddInstance( SV* instance ) {

	// Adding the instance requires a brObject type.
	SvREFCNT_inc(instance);
	slMessage(DEBUG_INFO, "Adding a perl object to Breve engine.\n");
	
	if(SvOK(instance)) {
		slMessage(DEBUG_INFO, "Instance %08x (SV*) appears to be valid.\n", instance);
	}

	HV* package_stash =
		SvSTASH(SvRV(instance)); // finding the package stash from the blessed ref SV*s

	char *aname = newArbitraryName();

	brObject *newPerlObj      = brEngineAddObject(breveEngine, brevePerlType, aname, package_stash);
	brInstance *newPerlInst   = brEngineAddInstance(breveEngine, newPerlObj, instance ); // dereferencing the RV

	// store a copy of the instance pointer inside the perl object
	// for when perl calls internal breve functions it needs to provide
	// the brInstance* of itself
	HV* objself = (HV*)SvRV(instance); // deference into the HV*
	const char* key = "brInstance";
	SV **hv_store_result;
	hv_store_result = hv_store(objself, key, strlen(key), newSViv((IV)newPerlInst), 0);

	slMessage(DEBUG_INFO, "Set $self->{brInstance} = <brInstance>.\n");
	
	return newPerlInst;
}

/**
 * Perl callback to find a breve internal function. 
 */
brInternalFunction *brPerlFindInternalFunction( char *name ) {

	brInternalFunction *function = brEngineInternalFunctionLookup( breveEngine, name );

	if ( !function ) {
		slMessage(DEBUG_ALL, "Could not locate internal breve method.\n");
		return NULL;
	}

	slMessage(DEBUG_INFO, "Found internal method: %s, brInternalFunction address => %08x\n\n",name,function);

	return function;
}

/**
 * Perl callback to call a breve internal function. 
 */

SV* brPerlCallInternalFunction( brInternalFunction *inFunc, brInstance *caller,
								int argCount, void *inArgs)
{
	brEval resultEval;
	brEval brArgs[128]; // seems like a bad way to pass arguments.

	dSP;

	int *inArgsArray =(int*) inArgs;

	slMessage(DEBUG_INFO, "brPerlCallInternalFunction(), inFunc => %08x, caller => %08x, inArgs => %08x\n", (unsigned*)inFunc, (unsigned*)caller, (unsigned*)inArgs);

	if(!inFunc) {
		slMessage(DEBUG_ALL, "Function invalid in call to brPerlCallInternalFunction()\n");
	}

// why am i required to pass the caller?
  
	for(int i = 0; i < argCount; i++) {
		brArgs[i].set( inArgsArray[i] );
	}

	inFunc->_call(brArgs, &resultEval, caller);
	
	return brPerlTypeFromEval( &resultEval, &SP );
}

void *brPerlSetController( brInstance *controller ) {

	slMessage(DEBUG_ALL,"brPerlSetController() perl invocation.\n");

	brEngineSetController(breveEngine, controller);
	SvREFCNT_inc(controller);

	return NULL;
}

#endif
