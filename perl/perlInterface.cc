#include "kernel.h"

#ifdef HAVE_LIBPERL

#include "perlInterface.h"

extern PerlInterpreter *my_perl;

static char arbitraryObjectName[9];
static int namedig = 0;

char* newArbitraryName() {
	sprintf(arbitraryObjectName,"%d",namedig++);
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
		slMessage(DEBUG_ALL, "Could not locate internal breve method: %s.\n", name);
		return NULL;
	}

	slMessage(DEBUG_INFO, "\nFound internal method: %s, brInternalFunction address => %08x\n",name,function);

	return function;
}

/**
 * Perl callback to call a breve internal function. 
 */

void brPerlCallInternalFunction( brInternalFunction *inFunc, brInstance *caller,
								int argCount, AV *inArgs)
{
	brEval resultEval;
	brEval brArgs[128]; // seems like a bad way to pass arguments.

	dSP;

    //    ENTER;
	//    SAVETMPS;

	slMessage(DEBUG_INFO, "brPerlCallInternalFunction(), inFunc => %08x, caller => %08x, inArgs => %08x\n", (unsigned*)inFunc, (unsigned*)caller, (unsigned*)inArgs);

	if(!inFunc) {
		slMessage(DEBUG_ALL, "Function invalid in call to brPerlCallInternalFunction()\n");
	}

	slMessage(DEBUG_INFO, "Passing %d arguments.\n", argCount);

	for(int i = 0; i < argCount; i++) {

		//SV* arg = av_shift(inArgs); // too slow?
		SV* arg = * av_fetch(inArgs, i, 0); // perhaps faster
		brPerlTypeToEval(arg, &brArgs[i]);
		//slMessage(DEBUG_INFO, "arg[%d] is = %08x ::: ", i, *arg);
		//slMessage(DEBUG_INFO, "ARG[%d] == %08x\n", i, SvIV(arg));
	}

	try {
		inFunc->_call(brArgs, &resultEval, caller);
	}
	catch (slException& e) {
		slMessage(DEBUG_ALL, "Exception: %s\n", e._message.c_str());
		exit(1);
	}	 

	SV *returned = brPerlTypeFromEval( &resultEval, &SP );

	SPAGAIN;

	if(returned == 0) {
		slMessage(DEBUG_INFO, "not returning anything.\n");
	} else {
		PUSHs(returned);
	}

	PUTBACK;
	//FREETMPS;
	//LEAVE;
}

void *brPerlSetController( brInstance *controller ) {

	slMessage(DEBUG_ALL,"brPerlSetController() perl invocation.\n");

	brEngineSetController(breveEngine, controller);
	//SvREFCNT_inc(controller);

	return NULL;
}

#endif
