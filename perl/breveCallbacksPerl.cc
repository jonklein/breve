#include "kernel.h"

#ifdef HAVE_LIBPERL

#include "perlInit.h"

PerlInterpreter *my_perl;

struct methodpack {
	char* name; // ideally i'd like to store the sv
	// that way perl isn't doing strcmp() to lookup methods
	// alas call_sv isn't working for these invocations
	int argCount;
	int retType;
};

brEngine *breveEngine;
brObjectType *brevePerlType;

// for xsub use
// static void xs_init (pTHX);

EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);
EXTERN_C void boot_Breve (pTHX_ CV* cv);

EXTERN_C void
xs_init(pTHX)
{
	char *file = __FILE__;
	/* DynaLoader is a special case */
	newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
	newXS("Breve::bootstrap", boot_Breve, file);
}

void brPerlTypeToEval( SV* arg, brEval *outEval) {
	switch(SvTYPE(arg)) {

// see google: "illustrated perlguts" for descriptions
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
			slMessage(DEBUG_INFO, "handling NULL.\n");
			outEval->set(0);
			break;

		case SVt_IV: // integer
			slMessage(DEBUG_INFO, "handling integer. (VAL: %08x) \n", SvIVX(arg));
			outEval->set(SvIVX(arg));
			break;

		case SVt_NV: // double
			slMessage(DEBUG_INFO, "handling double (VAL: %.2f) \n", SvNV(arg));
			outEval->set(SvNV(arg));
			break;

		case SVt_PV: // string
			slMessage(DEBUG_INFO, "handling pointer (char*) (VAL: %s).\n", SvPVX(arg));		
			outEval->set(SvPVX(arg));
			break;
			
		case SVt_PVNV: // pointer (double)
			slMessage(DEBUG_INFO, "handling pointer (w/ double), (VAL: %.2f).\n", SvNV(arg));
			outEval->set(SvNV(arg));
			break;

		case SVt_RV: // a Perl reference
			//slMessage(DEBUG_INFO, "handling reference.\n");

			if (sv_isobject(arg)) {

				if(sv_isa(arg, "Breve::Vector")) {
					//slMessage(DEBUG_INFO, "handling vector type.\n");
					slVector v = {0.0, 0.0, 0.0};
					AV* obj_array = (AV*) SvRV(arg);

					v.x = SvNV(  *  av_fetch(obj_array, 0, 0));
					v.y = SvNV(  *  av_fetch(obj_array, 1, 0));
					v.z = SvNV(  *  av_fetch(obj_array, 2, 0));

					outEval->set(v);

				} else if(sv_derived_from(arg, "Breve::Object")) {
					HV* obj_hash = (HV*) SvRV(arg);
					SV** obj_brInstanceVal = hv_fetch(obj_hash, "brInstance", strlen("brInstance"), 0);
					if(obj_brInstanceVal) {
						outEval->set( (brInstance*) SvIVX(*obj_brInstanceVal) );
					} else {
						slMessage(DEBUG_ALL,"$self->{brInstance} not found.\n");
						exit(2);
					}
				} else if(sv_isa(arg, "Breve::Matrix")) {	
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
					
					outEval->set(m);

					/*slMessage(0, "%f %f %f, %f %f %f, %f %f %f\n",
							  m[0][0],m[0][1],m[0][2],
						m[1][0],m[1][1],m[1][2],
						m[2][0],m[2][1],m[2][2]);*/

				} else {
					slMessage(DEBUG_ALL,"Don't know how to handle class type %s.\n", HvNAME( SvSTASH(SvRV(arg)) ));
					exit(1);
				}

			} else {
				slMessage(0, "What is this perl reference?\n" );
			}
			break;
	
		case SVt_PVAV: // array
			slMessage(DEBUG_ALL,"not handling array.\n"); break;
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

SV *brPerlTypeFromEval( const brEval *inEval, SV ***prevStackPtr ) {
// actually returns an SV*, AV*, or HV* (all sizeof(int) types though)
//	slMessage(DEBUG_INFO, "brPerlTypeFromEval ==> \n");

    brEvalListHead *list;
    brInstance *breveInstance;
    SV *result = NULL;
    register SV **sp = *prevStackPtr;
    
    switch ( inEval->type() ) {
	case AT_NULL:
		slMessage(DEBUG_INFO,"Null return, perlType AT_NULL.\n");
		result = 0;
		break;

	case AT_INT:
		slMessage(DEBUG_INFO,"Coercing from perlType AT_INT.\n");
		result = newSViv(BRINT(inEval));
		break;

	case AT_DOUBLE:
		slMessage(DEBUG_INFO,"Coercing from perlType AT_DOUBLE.\n");
		result = newSVnv(BRDOUBLE( inEval ) );
		break;

	case AT_STRING:   // perl will calculate string length using strlen
		slMessage(DEBUG_INFO,"Coercing from perlType AT_STRING.\n");
		result = newSVpv(BRSTRING(inEval), 0 );
		break;

	case AT_LIST:
	{
		slMessage(DEBUG_INFO, "Coercing from perlType AT_LIST.\n");
		list = BRLIST( inEval );
		unsigned int i;
		const unsigned int num = list->_vector.size();
		
		SV * svs[num];
		AV *arr = newAV();

		for(i = 0; i < num; i++ ) {
			svs[i] = brPerlTypeFromEval(&list->_vector[i], &sp);
			av_push(arr, svs[i]);
		}

		//	result =  av_make(num, svs); // makes an AV* from svs
		result = newRV_inc((SV*)arr);

		*prevStackPtr = sp;
		break;
			
	}
	case AT_INSTANCE:
		slMessage(DEBUG_INFO, "Coercing from PerlType AT_INSTANCE.\n");
		breveInstance = BRINSTANCE( inEval );// Is this a native type, or should we make a bridge of it?
            
		if( breveInstance && breveInstance->object->type->_typeSignature == PERL_TYPE_SIGNATURE ) {
			SV *sv_inst = (SV*)breveInstance->userData;
			if(SvROK(sv_inst)) {
				result = sv_inst; // this is the sv (RV), i don't think it has to be mortalized
				// the xsub bridge takes care of this, probably.
			} else {
				slMessage(DEBUG_INFO, "Could not convert breve internal type AT_INSTANCE.\n");
			}
		} else if( breveInstance ) {
		   
		} else {
			result = NULL;
		}

		break;
			
	case AT_POINTER:
		slMessage(DEBUG_INFO, "Coercing from PerlType AT_POINTER.\n");
		slMessage(DEBUG_INFO, "Value is 0x%08x.\n", BRPOINTER(inEval));
		result = newSViv((int)BRPOINTER( inEval ));
		//slMessage(DEBUG_INFO,"Result = %08x, value = %08x \n",result, SvPV_nolen(result);
		break;

	case AT_VECTOR:

		slMessage(DEBUG_INFO, "Coercing from PerlType AT_VECTOR.\n");
		{
			const slVector &v = BRVECTOR( inEval );
			AV *arr = newAV();
			av_push(arr, newSVnv(v.x));
			av_push(arr, newSVnv(v.y));
			av_push(arr, newSVnv(v.z));

			result = newRV_inc((SV*)arr);

			sv_bless(result, gv_stashpv("Breve::Vector", 0));

			*prevStackPtr = sp;
		}
		break;

	case AT_MATRIX:
		slMessage(DEBUG_INFO, "Coercing from PerlType AT_MATRIX.\n");
		break;
	case AT_ARRAY:
		slMessage(DEBUG_INFO, "Coercing from PerlType AT_ARRAY.\n");
		break;
	case AT_DATA:
		slMessage(DEBUG_INFO, "Coercing from PerlType AT_DATA.\n");
		break;
	case AT_HASH:
		slMessage(DEBUG_INFO, "Coercing from PerlType AT_HASH.\n");
		break;
	default:
		slMessage( DEBUG_ALL, "Could not convert breve internal type \"%d\" to a Perl type\n", inEval->type() );


		break;
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
/**
 * A breveObjectType callback to call a method in Perl
 */

int brPerlCallMethod(void *ref, void *mp, const brEval **inArguments, brEval *outResult ) {
	slMessage(DEBUG_INFO, "brPerlCallMethod() ==> refobj = %08x, mp = %08x, mp->name = %s\n", (unsigned)ref, (unsigned)mp, ((methodpack*)mp)->name);
	
	int count;

	dSP;
    
	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs((SV*)ref);
    
	slMessage(DEBUG_INFO, "Taking %d arguments.\n",((methodpack*)mp)->argCount);
                
	for(int i = 0; i < ((methodpack*)mp)->argCount; i++) {
		SV *ret = brPerlTypeFromEval(inArguments[i], &SP);	

		sv_2mortal(ret);
		XPUSHs(ret);
	}

	PUTBACK;

	count = call_method(((methodpack*)mp)->name, G_SCALAR);
	// call_sv(GvSV(meth), G_NOARGS|G_DISCARD);
	// call_method(meth_sv, G_DISCARD|G_NOARGS);

	SPAGAIN;

	SV* returned_sv = POPs;

	//slMessage(0, "from called perl method: %s\n",((methodpack*)mp)->name);
	brPerlTypeToEval(returned_sv, outResult);

	PUTBACK;
	FREETMPS;
	LEAVE;

	return EC_OK;
}

/**
 * A breveObjectType callback to instantiate a class in Perl
*/
brInstance *brPerlInstantiate( brEngine *inEngine, brObject* inObject, const brEval **inArgs, int inArgCount ) {
    slMessage(DEBUG_INFO, "brPerlInstantiate(...)\n");

    HV* package_stash = (HV*)inObject->userData;
	
    // should convert this to perl api
    // instead of taking the shortcut
    // that involves parsing
	
    char inst_line[128];
    sprintf(inst_line,"$brTempInst = new %s",HvNAME(package_stash));
    eval_pv(inst_line, TRUE);

    SV* created_sv = get_sv("main::brTempInst", FALSE);

    if(!created_sv) { // if(!new_obj)
		slMessage( DEBUG_ALL, "New Perl object not added to breve engine.\n");
		slMessage( DEBUG_ALL, "breve Perl objects must inherit from package Bobject.\n");
		return NULL;
    }

    // increment reference count to this Sv so when it
    // goes out of scope breve still retains it
    SvREFCNT_inc(created_sv);

    // do i need to decrement the stash? a la python.cc
    brInstance *newInst = brEngineAddInstance( inEngine, inObject, created_sv);

	// store a copy of the instance pointer inside the perl object
	// for when perl calls internal breve functions it needs to provide
	// the brInstance* of itself
	HV* objself = (HV*)SvRV(created_sv); // deference into the HV*
	const char* key = "brInstance";
	SV **hv_store_result;
	hv_store_result = hv_store(objself, key, strlen(key), newSViv(newInst), 0);

    slMessage(DEBUG_INFO, "newInst: %08x, created_sv: %08x\n",(unsigned)newInst, (unsigned) created_sv);
	
    return newInst;
}

/**
 * A breveObjectType callback to locate a method in a Perl object.
 */
void *brPerlFindMethod( void *package_stash, const char *inName, unsigned char *inTypes, int inCount ) {
// todo consider effects of autoloading

	slMessage(DEBUG_INFO, "brPerlFindMethod() => package_stash = %08x, inName = %s,, inCount = %d\n", (unsigned)package_stash, inName , inCount);

	// dashes in steve to underscores in perl
    char *name_perlified = (char*)malloc(128);
    strcpy(name_perlified,inName);
    for(unsigned i = 0; i < strlen(name_perlified); i++) {
		if(name_perlified[i] == '-') {
			name_perlified[i] = '_';
		}
    }

    GV* gv;
    gv = gv_fetchmeth((HV*)package_stash, name_perlified, strlen(name_perlified), 0);
    if(gv) {
		methodpack *mp = new methodpack;
		mp->name = name_perlified;
		mp->argCount = inCount;
		//printf("Returning gv = %08x\n",gv);
		// just gonna let it leak
		return mp;
    } else {
		slMessage(DEBUG_INFO, "Method %s not found.\n", inName);
		return NULL;
    }
}	

/**
 * A breveObjectType callback to locate a class in Perl.
 * 
 * @param inData 	The userdata callback pointer to a __main__ module 
 * @param inName	The name of the desired object
 */
void *brPerlFindObject( void *inData, const char *inName ) {
	slMessage(DEBUG_INFO, "brPerlFindObject() ==> inData = %08x, inName = %s\n", (unsigned)inData, inName);
	HV* package_stash =
		gv_stashpv(inName, false); // don't create a new package if not found
     
	if(package_stash) { // found the package
		return package_stash;
	}

	return NULL;
}

/**
 * A brObjectType callback to determine whether one object is a subclass of another.  Used by 
 * collision detection to determine if a handler is installed for an object pair.
 */
int brPerlIsSubclass( brObjectType *inType, void *inClassA, void *inClassB ) {
	// todo
	return 0;
}

/**
 * The Perl canLoad breve object callback
 */
int brPerlCanLoad( void *inObjectData, const char *inExtension ) {
	slMessage(DEBUG_INFO, "brPerlCanLoad() => inObjectdata = %08x, inExtension = %s\n", (unsigned)inObjectData, inExtension);
	if( !strcasecmp( inExtension, "pl" ) ||
		!strcasecmp( inExtension, "pm" ) ||
		!strcasecmp( inExtension, "pmbreve" ) ||
		!strcasecmp( inExtension, "plbreve" ) )
		return 1;

	return 0;
}

/**
 * The Perl load breve object callback
 */
int brPerlLoad( brEngine *inEngine, void *inObjectTypeUserData, const char *inFilename, const char *inFiletext ) {
	slMessage(DEBUG_INFO, "brPerlLoad(...) ==> inFilename = %s, inFiletex = %s\n", inFilename, inFiletext);
	int result = EC_OK;
	int exitstatus = 0;
	FILE *fp = fopen( inFilename, "r" );
     
	if( fp ) {
		//char *embedding[] = {"", "-d:DProf", (char*)inFilename}; // for profiling / debugging
		char *embedding[] = {"", (char*)inFilename};
		exitstatus = perl_parse(my_perl, xs_init, 2, embedding, (char**)NULL);
		if(!exitstatus) {
			slMessage(DEBUG_INFO,"Successfully loaded Breve Perl file.\n");
			breveEngine = inEngine; // saving this for later callbacks
			// TODO do i actually use this anywhere?
		} else {
			result = EC_ERROR;
		}
          
		// execute the main package code
		int perl_run_result = perl_run(my_perl);
		if(perl_run_result) {
			slMessage(0, "Error %d while executing %s.\n", perl_run_result, inFilename);
			return EC_ERROR;
		}
		fclose( fp );

	} else {
		slMessage(DEBUG_INFO,"File not found. Use inFileText [NOT IMPLEMENTED]?\n");
	}
     
	return result;
}

/**
 * A brObjectType callback to clean up a generic Perl object.  This method is used as the 
 * destructor for both objects and methods
 *
 * @param inObject		A void pointer to a Perl object.
 */
void brPerlDestroyGenericPerlObject( void *inObject ) {
	//slMessage(DEBUG_INFO, "X brPerlDestroyGenericPerlObject() ==> inObject = %08x\n",(unsigned)inObject); 
//	SvREFCNT_dec((SV*)inObject);
}

void brPerlShutdown() {
	slMessage( 0, "Destroying Perl runtime.\n");
	perl_destruct(my_perl);
	perl_free(my_perl);
	PERL_SYS_TERM();	
}

void brPerlInit( brEngine *breveEngine ) {
	slMessage(DEBUG_INFO, "Initializing Perl frontend.\n");
     
	PERL_SYS_INIT3(NULL,NULL,NULL); //argc, argv, env
	my_perl = perl_alloc();
	perl_construct(my_perl);
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
    
	brevePerlType = new brObjectType();

	brevePerlType->userData			= NULL;
	
	brevePerlType->findMethod		= brPerlFindMethod;
	brevePerlType->findObject		= brPerlFindObject;
	brevePerlType->instantiate		= brPerlInstantiate;
	brevePerlType->callMethod 		= brPerlCallMethod;
	brevePerlType->isSubclass 		= brPerlIsSubclass;
	brevePerlType->destroyObject 	= brPerlDestroyGenericPerlObject;
	brevePerlType->destroyMethod 	= brPerlDestroyGenericPerlObject;
	brevePerlType->destroyInstance 	= brPerlDestroyGenericPerlObject;
	brevePerlType->canLoad			= brPerlCanLoad;
	brevePerlType->load				= brPerlLoad;
	brevePerlType->_typeSignature	= PERL_TYPE_SIGNATURE;
	 
	brEngineRegisterObjectType( breveEngine, brevePerlType );
	
	breveInitPerlFunctions( breveEngine->internalMethods );
}

#endif /* HAVE_LIBPERL */
