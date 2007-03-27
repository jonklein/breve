#include "kernel.h"

#ifdef HAVE_LIBPERL

#include "perlInterface.h"

extern PerlInterpreter *my_perl;

static char arbitraryObjectName[9];
static int namedig = 0;

char* newArbitraryName() {
	char name[8];
	sprintf(name,"%d",namedig++);
	return name;
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

	//slMessage(DEBUG_INFO, "\nFound internal method: %s, brInternalFunction address => %08x\n",name,function);

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

	//slMessage(DEBUG_INFO, "brPerlCallInternalFunction(), inFunc => %08x, caller => %08x, inArgs => %08x\n", (unsigned*)inFunc, (unsigned*)caller, (unsigned*)inArgs);

	if(!inFunc) {
		slMessage(DEBUG_ALL, "Function invalid in call to brPerlCallInternalFunction()\n");
	}

	//slMessage(DEBUG_INFO, "Passing %d arguments.\n", argCount);

	for(int i = 0; i < argCount; i++) {

		//SV* arg = av_shift(inArgs); // too slow?
		SV* arg = * av_fetch(inArgs, i, 0); // perhaps faster

		//slMessage(DEBUG_INFO, "arg[%d] is = %08x ::: ", i, *arg);
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
			//slMessage(DEBUG_INFO, "handling NULL.\n");
			brArgs[i].set(NULL);
			break;

		case SVt_IV: // integer
			//slMessage(DEBUG_INFO, "handling integer. (VAL: %08x) \n", SvIVX(arg));
			brArgs[i].set(SvIVX(arg));
			break;

		case SVt_NV: // double
			//slMessage(DEBUG_INFO, "handling double (VAL: %.2f) \n", SvNV(arg));
			brArgs[i].set(SvNV(arg));
			break;

		case SVt_PV: // string
			//slMessage(DEBUG_INFO, "handling pointer (char*) (VAL: %s).\n", SvPVX(arg));		
			brArgs[i].set(SvPVX(arg));
			break;
			
		case SVt_PVNV: // pointer (double)
			//slMessage(DEBUG_INFO, "handling pointer (w/ double), (VAL: %.2f).\n", SvNV(arg));
			brArgs[i].set(SvNV(arg));
			break;

		case SVt_RV: // a Perl reference
			//slMessage(DEBUG_INFO, "handling reference.\n");

			if (sv_isobject(arg)) {

				if(sv_derived_from(arg, "Breve::Vector")) {
					//slMessage(DEBUG_INFO, "handling vector type.\n");
					slVector v = {0.0, 0.0, 0.0};
					AV* obj_array = (AV*) SvRV(arg);

					v.x = SvNV(  *  av_fetch(obj_array, 0, 0));
					v.y = SvNV(  *  av_fetch(obj_array, 1, 0));
					v.z = SvNV(  *  av_fetch(obj_array, 2, 0));

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
				} else if(sv_derived_from(arg, "Breve::Matrix")) {	
					//slMessage(DEBUG_INFO,"Handling the Matrix.\n");
					slMatrix m;
					// dereferencing gives us the SVt_PVHV (pointer to the $self hash)
					HV* obj_hash = (HV*) SvRV(arg);
					
					m[0][0] = SvNV(  *  hv_fetch(obj_hash, "x1", 2, 0)     );
					m[0][1] = SvNV(  *  hv_fetch(obj_hash, "x2", 2, 0)     );
					m[0][2] = SvNV(  *  hv_fetch(obj_hash, "x3", 2, 0)     );

					m[1][0] = SvNV(  *  hv_fetch(obj_hash, "y1", 2, 0)     );
					m[1][1] = SvNV(  *  hv_fetch(obj_hash, "y2", 2, 0)     );
					m[1][2] = SvNV(  *  hv_fetch(obj_hash, "y3", 2, 0)     );

					m[2][0] = SvNV(  *  hv_fetch(obj_hash, "z1", 2, 0)     );
					m[2][1] = SvNV(  *  hv_fetch(obj_hash, "z2", 2, 0)     );
					m[2][2] = SvNV(  *  hv_fetch(obj_hash, "z3", 2, 0)     );
					
					brArgs[i].set(m);

					/*slMessage(0, "%f %f %f, %f %f %f, %f %f %f\n",
							  m[0][0],m[0][1],m[0][2],
						m[1][0],m[1][1],m[1][2],
						m[2][0],m[2][1],m[2][2]);*/

				} else {
					slMessage(DEBUG_ALL,"Don't know how to handle this class type.\n");
					exit(2);
				}

			} else {
				slMessage(0, "What is this reference?: \n");
				
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
