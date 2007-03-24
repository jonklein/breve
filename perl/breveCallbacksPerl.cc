#include "kernel.h"

#ifdef HAVE_LIBPERL

#include "perlInit.h"

PerlInterpreter *my_perl;

struct methodpack {
	char* name; // ideally i'd like to store the sv
	// that way perl isn't doing strcmp() to lookup methods
	// alas call_sv isn't working for these invocations
	int argCount;
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

SV *brPerlTypeFromEval( const brEval *inEval, SV ***prevStackPtr ) {
// actually returns an SV*, AV*, or HV* (all sizeof(int) types though)
	slMessage(DEBUG_INFO, "brPerlTypeFromEval ==> \n");

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
		slMessage(DEBUG_INFO, "Coercing from perlType AT_LIST.\n");
		list = BRLIST( inEval );
		unsigned int n;
		SV * ret;
		for(n = 1; n < list->_vector.size(); n++ ) {
			ret = brPerlTypeFromEval(&list->_vector[n], &sp);
			sv_2mortal(ret);
			XPUSHs(ret);
		}
		ret = brPerlTypeFromEval(&list->_vector[2],&sp);
		XPUSHs(ret);
		ret = brPerlTypeFromEval(&list->_vector[2],&sp);
		XPUSHs(ret);   // callMethod pushes the first value
		result = brPerlTypeFromEval(&list->_vector[0], &sp);
		*prevStackPtr = sp;
		break;
			
	case AT_INSTANCE:
		slMessage(DEBUG_INFO, "Coercing from PerlType AT_INSTANCE.\n");
		breveInstance = BRINSTANCE( inEval );// Is this a native type, or should we make a bridge of it?
            
		if( breveInstance && breveInstance->object->type->_typeSignature == PERL_TYPE_SIGNATURE ) {
			SV *sv_inst = breveInstance->userData;
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
		result = newSViv(BRPOINTER( inEval ));
		break;

	case AT_VECTOR:

		slMessage(DEBUG_INFO, "Coercing from PerlType AT_VECTOR.\n");
		{
			const slVector &v = BRVECTOR( inEval);
			XPUSHs(sv_2mortal(newSVnv(v.x)));
			XPUSHs(sv_2mortal(newSVnv(v.y)));
			XPUSHs(sv_2mortal(newSVnv(v.z)));
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
	slMessage(DEBUG_INFO, "brPerlCallMethod() ==> refobj = %08x, mp = %08x, args..result..\n", (unsigned)ref, (unsigned)mp);

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

	call_method(((methodpack*)mp)->name, G_DISCARD);
	// call_sv(GvSV(meth), G_NOARGS|G_DISCARD);
	// call_method(meth_sv, G_DISCARD|G_NOARGS);
    
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

	//if(strcmp(inName, "PerlTestObject") == 0)
	//	return 0xdeadbeef;

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
		slMessage(DEBUG_INFO, "Result from perl_run() => %d\n", perl_run_result);

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
	slMessage(DEBUG_INFO, "brPerlDestroyGenericPerlObject() ==> inObject = %08x\n",(unsigned)inObject); 
	SvREFCNT_dec((SV*)inObject);
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
