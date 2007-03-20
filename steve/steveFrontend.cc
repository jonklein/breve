#include "steve.h"
#include "xml.h"
#include "evaluation.h"
#include "errorText.h"
#include "java.h"

#include "python.h"
#include "perlInit.h"
// #include "lisp.h"

#include "breveFunctionsSteveDataObject.h"
#include "breveFunctionsSteveObject.h"
#include "breveFunctionsSteveXML.h"

#include "signal.h"

extern const char *yyfile;
extern int lineno;

stSteveData *currentData;

void *brInitFrontendLanguages( brEngine *engine ) {
#ifdef HAVE_LIBPYTHON
        brPythonInit( engine );
#endif
#ifdef HAVE_LIBPERL
		brPerlInit( engine );
#endif
#ifdef HAVE_LIBECL
        brLispInit( engine );
#endif

        return stSteveInit( engine );
}

void stCrashCatcher( int s ) {
	signal( s, SIG_DFL );

	slSetMessageCallbackFunction( slStderrMessageCallback );

	printf( "Signal %d detected -- attempting steve stack trace:\n", s );
	stStackTrace( currentData );

	raise( s );
}

/**
	\brief The breve callback to determine if one object is a subclass of another.
*/
int stSubclassCallback( brObjectType *inType, void *c1, void *c2 ) {
	return stIsSubclassOf( ( stObject* )c1, ( stObject* )c2 );
}

/**
 * The breve callback to call a method in the steve language.
 */

int stCallMethodBreveCallback( void *instanceData, void *methodData, const brEval **arguments, brEval *result ) {
	int r, count = 0;
	stMethod *method = ( stMethod* )methodData;
	stRunInstance ri;

	ri.instance = ( stInstance* )instanceData;

	count = method->keywords.size();

	ri.type = ri.instance->type;

	r = stCallMethod( &ri, &ri, method, arguments, count, result );

	return r;
}

/**
	\brief The steve callback to create a new instance.
*/

brInstance *stInstanceNewCallback( brEngine *engine, brObject *object, const brEval **constructorArgs, int argCount ) {
	stInstance *i = stInstanceNew( ( stObject* )object->userData );

	i->breveInstance = brEngineAddInstance( engine, object, i );

	if( stInstanceInit( i ) != EC_OK ) 
		return NULL;

	return i->breveInstance;

}

/**
	\brief The breve callback to find a method.
*/

void *stFindMethodBreveCallback( void *object, const char *name, unsigned char *argTypes, int args ) {
	int min = args, max = args;

	// In some cases, we may not have acurate argument count information

	if( args == -1 ) {
		min = 0;
		max = 100;
	}

	stMethod *method;

	method = stFindInstanceMethodWithArgRange(( stObject* )object, name, args, args, NULL );

	return method;
}

/**
	\brief The steve language callback to free an instance.
*/

void stInstanceFreeCallback( void *i ) {
	stInstanceFree( (stInstance*)i );
}

/**
 * The canLoad callback for the steve language frontend.
 */

int stCallbackCanLoad( void *inObjectTypeUserData, const char *inFileExtension ) {
	// if( !strcasecmp( inFileExtension, "steve" ) ||
	// 	!strcasecmp( inFileExtension, "breve" ) ||
	// 	!strcasecmp( inFileExtension, "tz" ) )
	// 		return 1;

	// the steve language is the default language of last resort.  Even if we 
	// don't recognize the file extension, we're going to try to execute the simulation 

	return 1;
}

/**
 * Loads a steve simulation file 
 */

int stCallbackLoad( brEngine *engine, void *inDataPtr, const char *file, const char *code ) {
	brObject *controllerClass;
	stSteveData *inData = (stSteveData*)inDataPtr;
	int r = EC_OK;

	if ( stLoadFiles( inData, engine, code, file ) != EC_OK ) return EC_ERROR;

	if( inData->controllerName ) {
		controllerClass = brObjectFind( engine, inData->controllerName );

		if ( !controllerClass ) {
			stParseError( engine, EE_UNKNOWN_CONTROLLER, "Unknown \"Controller\" object" );
			return EC_ERROR;
		}

		/*
		brInstance *controller = brObjectInstantiate( engine, controllerClass, NULL, NULL );

		if( !controller )
			return EC_ERROR;

		brEngineSetController( engine, controller );
		*/

		stInstance *controller;
		controller = stInstanceNew( ( stObject* )controllerClass->userData );

		if( !controller )
			return EC_ERROR;

		controller->breveInstance = brEngineAddInstance( engine, controllerClass, controller );

		brEngineSetController( engine, controller->breveInstance );

		r = stInstanceInit( controller );

		if ( r != EC_OK ) 
			return EC_ERROR;

		inData->singleStatementMethod = new stMethod( "internal-user-input-method", NULL, "<user-input>", 0 );
		stStoreInstanceMethod( (stObject*)controllerClass->userData, "internal-user-input-method", inData->singleStatementMethod );
	}

	return r;
}

int stCallbackLoadWithArchive( brEngine *engine, void *inDataPtr, const char *file, const char *code, const char *archive ) {
	stSteveData *inData = (stSteveData*)inDataPtr;

	//

	inData = 0;

	// 

	return EC_OK;
}

/**
 * \brief Cleanup after steve.
 * 
 * Free all instances, objects, freed instance lists, controller name,
 * defines and the engine.
 */

void stSteveCleanup( void *inDataPtr ) {
	stSteveData *d = (stSteveData*)inDataPtr;

	std::vector< stObject* >::iterator oi;
	std::vector< stInstance* >::iterator ii;

	// free all the instances.

	for ( oi = d->objects.begin(); oi != d->objects.end(); oi++ )
		stObjectFreeAllInstances( *oi );

	// free all the objects.

	for ( oi = d->objects.begin(); oi != d->objects.end(); oi++ )
		stObjectFree( *oi );

	for ( ii = d->freedInstances.begin(); ii != d->freedInstances.end(); ii++ )
		delete *ii;

	if ( d->controllerName ) slFree( d->controllerName );

	// brNamespaceFreeWithFunction(d->defines, (void(*)(void*))stFreeDefine );

	delete d;
}
/**
	\brief Initializes the steve language and sets up the brObjectType structure.
*/

stSteveData *stSteveInit( brEngine *engine ) {
	brNamespace *internal;
	stSteveData *sd;

	brObjectType *breveSteveType = new brObjectType();

	internal = brEngineGetInternalMethods( engine );

	sd = new stSteveData;
	sd->steveObjectType = breveSteveType;

	breveInitSteveDataObjectFuncs( internal );
	breveInitSteveObjectFuncs( internal );
	breveInitXMLFuncs( internal );

	breveSteveType->callMethod 		= stCallMethodBreveCallback;
	breveSteveType->findMethod 		= stFindMethodBreveCallback;
	breveSteveType->isSubclass 		= stSubclassCallback;
	breveSteveType->instantiate 		= stInstanceNewCallback;
	breveSteveType->destroyInstance 	= stInstanceFreeCallback;
	breveSteveType->destroyObjectType	= stSteveCleanup;
	breveSteveType->canLoad			= stCallbackCanLoad;
	breveSteveType->load			= stCallbackLoad;
	breveSteveType->loadWithArchive		= stCallbackLoadWithArchive;
	breveSteveType->userData		= ( void* )sd;
	breveSteveType->_typeSignature 		= STEVE_TYPE_SIGNATURE;

	currentData = sd;

	brEngineRegisterObjectType( engine, breveSteveType );

	return sd;
}


/**
 * \brief Loads steve simulation code to prepare to run a simulation.
 * This is the top-level file reading function--this is the one that
 * is called externally to load in files for a simulation.
 */

int stLoadFiles( stSteveData *sdata, brEngine *engine, const char *code, const char *file ) {
	int r;
	brObject *controller;
	char *path = slStrdup( file );
	const char *enginePath;
	int n = strlen( path ) - 1;

	enginePath = brEngineGetPath( engine );

	while ( n && path[n] != '/' && path[n] != '\\' ) n--;

	path[n] = 0;

	if ( n != 0 && path[0] == '/' ) {
		/* absolute path */

		brAddSearchPath( engine, path );
	} else if ( n != 0 ) {
		/* relative path */

		char *fullpath = new char[strlen( enginePath ) + strlen( path ) + 2];
		sprintf( fullpath, "%s/%s", enginePath, path );

		brAddSearchPath( engine, fullpath );

		delete[] fullpath;
	} else {
		// no path--just a file in the current directory

		brAddSearchPath( engine, enginePath );
	}

	slFree( path );

	r = stParseBuffer( sdata, engine, code, file );

	if ( r != BPE_OK ) return EC_ERROR;

	lineno = 1;

	yyfile = file;

	if( sdata->controllerName ) {
		if ( !sdata->controllerName ) {
			stParseError( engine, PE_NO_CONTROLLER, "No \"Controller\" object has been defined" );
			return EC_ERROR;
		}

		controller = brObjectFind( engine, sdata->controllerName );

		if ( !controller ) {
			stParseError( engine, EE_UNKNOWN_CONTROLLER, "Unknown \"Controller\" object" );
			return EC_ERROR;
		}

	}

	return EC_OK;
}

/**
	\brief Prepares to run a simulation from an XML archive.

	Parses and loads the specified code, then restores the state of the
	simulation from the data stored in the provided XML file.
*/

int stLoadSavedSimulation( stSteveData *sdata, brEngine *engine, const char *code, const char *file, const char *xmlfile ) {
	char *xmlpath = brFindFile( engine, xmlfile, NULL );

	if ( !xmlpath ) {
		slMessage( DEBUG_ALL, "Cannot locate archived XML simulation file \"%s\"\n", xmlfile );
		return EC_ERROR;
	}

	if ( stLoadFiles( sdata, engine, code, file ) != EC_OK ) {
		slFree( xmlpath );
		return EC_ERROR;
	}

	if ( stXMLInitSimulationFromFile( engine, xmlpath ) ) {
		slFree( xmlpath );
		return EC_ERROR;
	}

	slFree( xmlpath );

	return EC_OK;
}

/**
	\brief Parses a single steve file.

	Given a filename, this function loads and parses the file.  Used by
	stParseBuffer, not typically called manually.
*/

int stParseFile( stSteveData *sdata, brEngine *engine, const char *filename ) {

	struct stat fs;
	char *path = NULL;
	int result;

	char *fileString;

	// try to open the file in the current directory.

	path = brFindFile( engine, filename, &fs );

	if ( !path ) {
		slMessage( DEBUG_ALL, "could not locate file %s\n", filename );
		return EC_ERROR;
	}

	fileString = slUtilReadFile( path );

	if ( !fileString ) {
		slMessage( DEBUG_ALL, "error reading file %s\n", path );
		return EC_ERROR;
	}

	result = stParseBuffer( sdata, engine, fileString, filename );

	slFree( fileString );
	slFree( path );

	return result;
}

/**
	\brief Parses the text of a steve file.

	stParseBuffer will first pick out included files and recursively parse
	them.  the engine keeps track of what files it's seen, however, so there
	should be no real danger of including the same file twice.

	Typically called by stParseFile, which reads in the file first.
*/

int stParseBuffer( stSteveData *s, brEngine *engine, const char *buffer, const char *filename ) {
	const char *thisFile;

	if ( std::find( s->filesSeen.begin(), s->filesSeen.end(), std::string( filename ) ) != s->filesSeen.end() ) {
		slMessage( DEBUG_INFO, "skipping \"%s\", already included\n", filename );
		return 0;
	}

	/* if this file hasn't been seen, add it to the parse track and go */

	if ( filename ) {
		thisFile = filename;
		s->filesSeen.push_back( filename );
	} else thisFile = "<untitled>";

	// set the global variables for the parser 
	// preprocess the buffer--look for other included files 

	yyfile = thisFile;

	lineno = 1;

	if ( stPreprocess( s, engine, filename, buffer ) ) 
		return BPE_LIB_ERROR;

	// preprocess changes the yyfile and lineno globals -- reset them

	yyfile = strdup( thisFile );

	lineno = 1;

	stSetParseData( s, buffer, strlen( buffer ) );

	// the REAL parse--set the parse engine so the parser knows 
	// what to do with the info it parses 

	stParseSetEngine( engine );
	stParseSetSteveData( s );

	brClearError( engine );

	if ( yyparse() ) return BPE_SIM_ERROR;

	// free( yyfile );

	return BPE_OK;
}

/**
 * \brief Preprocess a steve document.
 *
 * Called automatically by stParseBuffer -- not to be called manually.
 * 
 * Parse out all of the "@include", or "@path" lines.  by convention
 * all '@' lines are preprocessor directives, but currently there
 *aren't so many.
 */

int stPreprocess( stSteveData *s, brEngine *engine, const char *srcFile, const char *line ) {
	const char *start, *end;
	char *filename;
	int n;
	int include = 0, path = 0, use = 0;
	char useWord[1024];
	const char *oldYyfile = yyfile;
	int oldLineno = lineno;
	std::string current = srcFile;

	/* i don't wanna comment this */

	if ( !line ) return -1;

	/* damn this is ugly */

	do {
		while ( *line == '\n' ) {
			line++;
			lineno++;
			oldLineno++;
		}

		include = 0;

		path = 0;
		use = 0;

		if ( !strncmp( line, "@include", strlen( "@include" ) ) ) include = 1;
		else if ( !strncmp( line, "@path", strlen( "@path" ) ) ) path = 1;
		else if ( !strncmp( line, "@use", strlen( "@use" ) ) ) use = 1;

		if ( include || path || use ) {
			start = end = NULL;
			n = 0;

			if ( use ) {
				n = 4;

				sscanf( line, "@use %s", useWord );

				if ( useWord[strlen( useWord ) - 1] == '.' )
					useWord[strlen( useWord ) - 1] = 0;

				filename = new char[strlen( useWord ) + 4];

				sprintf( filename, "%s.tz", useWord );
			} else {
				while ( line[n] != 0 && line[n] != '\n' ) {
					n++;

					if ( line[n] == '\"' && !start ) start = &line[n + 1];
					else if ( line[n] == '\"' ) end = &line[n];
				}

				filename = new char[( end - start ) + 1];

				strncpy( filename, start, ( end - start ) );
				filename[end - start] = 0;
			}

			if ( include || use ) {
				s->_includes[ current ].push_back( filename );

				char *found = brFindFile( engine, filename, NULL );
				char *filetext = slUtilReadFile( found );

				if( brLoadFile( engine, filetext, filename ) != EC_OK ) {
					yyfile = oldYyfile;
					lineno = oldLineno;
					stParseError( engine, EE_FILE_NOT_FOUND, "Error including file \"%s\"", filename );
					delete[] filename;
					return -1;
				}

				yyfile = oldYyfile;

				lineno = oldLineno;
			} else {
				s->_paths[ current ].push_back( filename );

				brAddSearchPath( engine, filename );
			}

			delete[] filename;
		}
	} while (( line = strchr( line, '\n' ) ) );

	return 0;
}

/**
	\brief Makes a version requirement.
*/

stVersionRequirement *stMakeVersionRequirement( float version, int operation ) {
	stVersionRequirement *b;

	b = new stVersionRequirement;

	b->version = version;
	b->operation = operation;

	return b;
}

/**
	\brief Checks to see whether a version requirement has been fulfilled.
*/

int stCheckVersionRequirement( float version, stVersionRequirement *r ) {
	if ( !r ) return 1;

	switch ( r->operation ) {

		case VR_GT:
			return version > r->version;
			break;

		case VR_GE:
			return version >= r->version;
			break;

		case VR_LT:
			return version < r->version;
			break;

		case VR_LE:
			return version <= r->version;
			break;

		case VR_EQ:
			return version == r->version;
			break;

		case VR_NE:
			return version != r->version;
			break;

		default:
			slMessage( DEBUG_ALL, "unknown operator %d in stCheckVersionRequirement\n", r->operation );
			break;
	}

	return 0;
}

/**
	\brief Reports on current usage of all steve objects.

	Requires any stObject.
*/

void stObjectAllocationReport( stObject *o ) {
	std::vector< stObject* >::iterator oi;

	for ( oi = o->steveData->objects.begin(); oi != o->steveData->objects.end(); oi++ )
		slMessage( DEBUG_ALL, "class %s: %d instances allocated\n", ( *oi )->name.c_str(), ( *oi )->allInstances.size() );
}

/**
	\brief Trigger a steve-language parse error.

	Called when a parse (or other "parse-time") error occurs.
	Prints out error messages and stops the simulation.
*/

void stParseError( brEngine *e, int type, char *proto, ... ) {
	va_list vp;
	brErrorInfo *error = brEngineGetErrorInfo( e );

	// if there is no "primary" error defined, then we do the whole process

	if ( error->type == 0 ) {
		error->type = type;
		error->file = slStrdup( yyfile );
		error->line = lineno;

		va_start( vp, proto );
		vsnprintf( error->message, BR_ERROR_TEXT_SIZE, proto, vp );
		va_end( vp );

		slMessage( DEBUG_ALL, "%s: %s", gErrorNames[type], error->message );
		slMessage( DEBUG_ALL, " at line %d of file \"%s\"", lineno, yyfile );
	} else {
		// char localMessage[BR_ERROR_TEXT_SIZE];
		//// Ignore subsequent parse errors... [?]
		// va_start( vp, proto );
		// vsnprintf( localMessage, BR_ERROR_TEXT_SIZE, proto, vp );
		// va_end( vp );
		// slMessage( DEBUG_ALL, localMessage );
		// slMessage( DEBUG_ALL, " at line %d of file \"%s\"", lineno, yyfile );
	}

	slMessage( DEBUG_ALL, "\n" );
}

/**
 * Set the steve controller object for the simulation.
 */

int stSetControllerName( stSteveData *data, brEngine *engine, const char *controller ) {
	if ( data->controllerName ) {
		stParseError( engine, PE_REDEFINITION, "Redefinition of \"Controller\" object" );
		return -1;
	}

	data->controllerName = slStrdup( controller );

	return 0;
}
