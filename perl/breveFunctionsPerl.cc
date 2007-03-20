#include "kernel.h"

#include "perlInit.h"

#ifdef HAVE_LIBPERL

extern PerlInterpreter *my_perl;

int brPerlLoadFile( brEval arguments[], brEval *result, brInstance *instance ) {
	printf("brPerlLoadFile(...)\n");
	
	char *file = brFindFile( instance->engine, BRSTRING( &arguments[ 0 ] ), NULL );
			
	if( !file ) {
		throw slException( "cannot locate file \"" + std::string( BRSTRING( &arguments[ 0 ] ) ) + "\"" );
	}

	char *embedding[] = {"", (char*)file};
	
	int exitstatus = perl_parse(my_perl, NULL, 2, embedding, (char**)NULL);
	result->set(exitstatus);

	if(!exitstatus) {
		printf("Successfully loaded perl file %s.\n", file);

	} else {
		slMessage( DEBUG_ALL, "Perl load of file \"%s\" failed\n", file );
		return EC_ERROR;
	}

	return EC_OK;
}

int brPerlInternalTest( brEval arguments[], brEval *result, brInstance *instance ) {
	printf( "Internal test function called\n" );

	return EC_OK;
}

#endif

void breveInitPerlFunctions( brNamespace *n ) {
#ifdef HAVE_LIBPERL
	brNewBreveCall( n, "perlLoadFile", brPerlLoadFile, AT_INT, AT_STRING, 0 );
	brNewBreveCall( n, "perlInternalTest", brPerlInternalTest, AT_INT, AT_STRING, 0 );
#endif
}


