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

		SV* arg = av_shift(inArgs);
		slMessage(DEBUG_INFO, "arg[%d] is = %08x ::: ", i, *arg);
		//slMessage(DEBUG_INFO, "ARG[%d] == %08x\n", i, SvIV(arg));

		switch(SvTYPE(arg)) {

// see http://gisle.aas.no/perl/illguts/ for internals and descriptions
/*
			SVt_NULL,   // 0 
	   SVt_IV,     // 1 
	   SVt_NV,     // 2 
	   SVt_RV,     // 3 
	   SVt_PV,     // 4 
	   SVt_PVIV,   // 5 
	   SVt_PVNV,   // 6 
	   SVt_PVMG,   // 7 
	   SVt_PVBM,   // 8 
	   SVt_PVLV,   // 9 
	   SVt_PVAV,   // 10
	   SVt_PVHV,   // 11 
	   SVt_PVCV,   // 12
	   SVt_PVGV,   //13 
	   SVt_PVFM,   // 14
	   SVt_PVIO // 15 	  */

		case SVt_NULL: // null
			slMessage(DEBUG_ALL, "handling NULL.\n");
			brArgs[i].set(NULL);
			break;

		case SVt_IV: // integer
			slMessage(DEBUG_ALL, "handling integer. (VAL: %08x) \n", SvIVX(arg));
			brArgs[i].set(SvIVX(arg));
			break;

		case SVt_NV: // double
			slMessage(DEBUG_ALL, "handling double (VAL: %.2f) \n", SvNV(arg));
			brArgs[i].set(SvNV(arg));
			break;

		case SVt_PV: // pointer (string?)
			slMessage(DEBUG_ALL, "handling pointer (char*).\n");
			brArgs[i].set(SvPV_nolen(arg));
			break;
			
		case SVt_PVNV: // pointer (double)
			slMessage(DEBUG_ALL, "handling pointer (w/ double), (VAL: %.2f).\n", SvNV(arg));
			brArgs[i].set(SvNV(arg));
			break;

		case SVt_RV: // a Perl reference
			slMessage(DEBUG_ALL, "handling reference.\n");
			// is it a vector?
			if (sv_isobject(arg)) {
				if(sv_derived_from(arg, "Breve::Vector")) {
					slMessage(DEBUG_ALL, "handling vector type.\n");
					slVector v = {0.0, 0.0, 0.0};
					// dereferencing gives us the SVt_PVHV (pointer to the $self hash)
					HV* obj_hash = (HV*) SvRV(arg);
					SV** obj_xval = hv_fetch(obj_hash, "x", 1, 0);
					SV** obj_yval = hv_fetch(obj_hash, "y", 1, 0);
					SV** obj_zval = hv_fetch(obj_hash, "z", 1, 0);

					v.x = SvIVX(*obj_xval);
					v.y = SvIVX(*obj_yval);
					v.z = SvIVX(*obj_zval);

					brArgs[i].set(v);

				} else if(sv_derived_from(arg, "Breve::Object")) {
					HV* obj_hash = (HV*) SvRV(arg);
					SV** obj_brInstanceVal = hv_fetch(obj_hash, "brInstance", strlen("brInstance"), 0);
					if(obj_brInstanceVal) {
						brArgs[i].set( (brInstance*) SvIVX(*obj_brInstanceVal) );
					} else {
						slMessage(DEBUG_ALL,"$self->{brInstance} not found.\n");
						exit(2);
					}
				} else {
					slMessage(DEBUG_ALL,"Don't know how to handle this class type.\n");
					exit(2);
				}

			} else {
				slMessage(DEBUG_ALL, "Not handling unknown reference type.\n");
				exit(2);
			}
			break;
	
		case SVt_PVAV: // array
			slMessage(DEBUG_ALL,"not handling array.\n");
			break;

		case SVt_PVCV: // code ref
			slMessage(DEBUG_ALL,"not handling code ref.\n"); break;
		case SVt_PVHV: // hash
			slMessage(DEBUG_ALL,"not handling hash.\n"); break;
		case SVt_PVMG: // blessed scalar
			slMessage(DEBUG_ALL,"not handling blessed-scalar pointer thing.\n"); break;
		default:
			slMessage(DEBUG_ALL, "Can't handle SvTYPE (%d) for passed SV*.\n", SvTYPE(arg));
		}
	}

	inFunc->_call(brArgs, &resultEval, caller);

	SV *returned = brPerlTypeFromEval( &resultEval, &SP );

	SPAGAIN;
	

	if(returned == 0) {
		slMessage(DEBUG_ALL, "not returning anything.\n");
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
