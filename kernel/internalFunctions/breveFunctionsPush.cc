/*
	the breve simulation environment

	This file was generated automatically by the script /Users/jk/dev/breve/tools/apiwrap.pl.
*/

#include "util.h"
#include "kernel.h"

void breveInitPushFunctions(brNamespace *n);

/*@{*/
/*! \addtogroup InternalFunctions */
#ifdef HAVE_LIBPUSH

#include "pushC.h" /**** MOVE TO push/pushC.h ??? */
#include "push/Code.h"
#include "push/Env.h"
#include "push/Literal.h"
#include "push/CodeUtils.h"

/*!
	\brief A breve API function wrapper for the C-function \ref pushEnvironmentNew.

	See the documentation for \ref pushEnvironmentNew for more details.
*/

int breveFunctionPushEnvironmentNew(brEval arguments[], brEval *result, brInstance *instance) {
	int seed = BRINT(&arguments[0]);

	result->set( pushEnvironmentNew(seed) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushEnvironmentFree.

	See the documentation for \ref pushEnvironmentFree for more details.
*/

int breveFunctionPushEnvironmentFree(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	pushEnvironmentFree(environment);
	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushEnvironmentReadConfigFile.

	See the documentation for \ref pushEnvironmentReadConfigFile for more details.
*/

int breveFunctionPushEnvironmentReadConfigFile(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	char *file = BRSTRING(&arguments[1]);

	pushEnvironmentReadConfigFile(environment, file);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushRun.

	See the documentation for \ref pushRun for more details.
*/

int breveFunctionPushRun(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	PushCode *code = BRPOINTER(&arguments[1]);
 	int steps = BRINT(&arguments[2]);

	result->set( pushRun(environment, code, steps) );

	return EC_OK;
}

int breveFunctionPushInterpreterDone(brEval arguments[], brEval *result, brInstance *instance) {
	push::Env *env = static_cast< push::Env* >( BRPOINTER(&arguments[0]) );

	result->set( (int)env->done() );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushStep.

	See the documentation for \ref pushStep for more details.
*/

int breveFunctionPushStep(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	int steps = BRINT(&arguments[1]);

	result->set( pushStep(environment, steps) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushGetConfigString.

	See the documentation for \ref pushGetConfigString for more details.
*/

int breveFunctionPushGetConfigString(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	char *str = pushGetConfigString(environment);

	result->set( str );

	free( str );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushGetStacksString.

	See the documentation for \ref pushGetStacksString for more details.
*/

int breveFunctionPushGetStacksString(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	char *str = pushGetStacksString(environment);

	result->set( str );

	free( str );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushGetExecStackString.

	See the documentation for \ref pushGetExecStackString for more details.
*/

int breveFunctionPushGetExecStackString(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushGetExecStackString(environment) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushClearStacks.

	See the documentation for \ref pushClearStacks for more details.
*/

int breveFunctionPushClearStacks(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushClearStacks(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushEnvironmentSetListLimit.

	See the documentation for \ref pushEnvironmentSetListLimit for more details.
*/

int breveFunctionPushEnvironmentSetListLimit(brEval arguments[], brEval *result, brInstance *instance) {
	push::Env &env = static_cast< push::Env* >( BRPOINTER(&arguments[0]) )->next();

	env.parameters.max_points_in_program = BRINT(&arguments[1]);

	return EC_OK;
}

int breveFunctionPushEnvironmentGetListLimit(brEval arguments[], brEval *result, brInstance *instance) {
	push::Env &env = static_cast< push::Env* >(BRPOINTER(&arguments[0]))->next();

	result->set( env.parameters.max_points_in_program );

	return EC_OK;
}

int breveFunctionPushEnvironmentGetEvalLimit(brEval arguments[], brEval *result, brInstance *instance) {
	push::Env &env = static_cast< push::Env* >(BRPOINTER(&arguments[0]))->next();

	result->set( env.parameters.evalpush_limit );

	return EC_OK;
}

int breveFunctionPushEnvironmentSetEvalLimit(brEval arguments[], brEval *result, brInstance *instance) {
	push::Env &env = static_cast< push::Env* >(BRPOINTER(&arguments[0]))->next();

	env.parameters.evalpush_limit = BRINT( &arguments[1] );

	return EC_OK;
}

int breveFunctionPushEnvironmentGetRandomPointLimit(brEval arguments[], brEval *result, brInstance *instance) {
	push::Env &env = static_cast< push::Env* >(BRPOINTER(&arguments[0]))->next();

	result->set( env.parameters.max_points_in_random_expression );

	return EC_OK;
}

int breveFunctionPushEnvironmentSetRandomPointLimit(brEval arguments[], brEval *result, brInstance *instance) {
	push::Env &env = static_cast< push::Env* >(BRPOINTER(&arguments[0]))->next();

	env.parameters.max_points_in_random_expression = BRINT( &arguments[1] );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushParse.

	See the documentation for \ref pushParse for more details.
*/

int breveFunctionPushParse(brEval arguments[], brEval *result, brInstance *instance) {
	char *string = BRSTRING(&arguments[0]);

	result->set( pushParse(string) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeGetString.

	See the documentation for \ref pushCodeGetString for more details.
*/

int breveFunctionPushCodeGetString(brEval arguments[], brEval *result, brInstance *instance) {
	PushCode *code = BRPOINTER(&arguments[0]);
	char *str = pushCodeGetString(code);

	result->set( str );

	return EC_OK;
}

/*!
	\brief A function to convert a push program to a brEvalList, recursively if necessary.
*/

brEvalListHead *brevePushCodeToEvalList(const push::Code *code) {
	brEvalListHead *l;
	brEval e;
	unsigned int n;

	l = brEvalListNew();

	for(n = 0; n < (*code)->get_stack().size(); n++ ) {
		if( ((*code)->get_stack()[n])->get_stack().size() == 0 ) {
			e.set( (char*)(*code)->get_stack()[n]->to_string().c_str() );
		} else {
			e.set( brevePushCodeToEvalList( &(*code)->get_stack()[n]) );
		}

		brEvalListInsert(l, 0, &e);
	}

	return l;
}

float brevePushCodeFirstFloat(const push::Code *code, bool *found ) {
	int n;
	const push::CodeArray &stack = (*code)->get_stack();

	for(n = (int)stack.size() - 1; n >= 0; n-- ) {
		if( stack[ n ]->get_stack().size() == 0 ) {
			push::Code c = stack[ n ];
			
			if( typeid( c ) == typeid( push::Literal< double > ) ) {
				push::Env env;

				*found = 1;
				( *c )( env );
				return push::get_stack<double>( env )[ 0 ];
			}

			//if( push::Literal< double > *d = dynamic_cast< push::Literal< double >* >( c.get() ) ) {
			//	*found = 1;
			//	return d->get();
			//}
		} else {
			float r = brevePushCodeFirstFloat( &stack[ n ], found );

			if( *found ) return r;
		}
	}

	return 0.0;
}

int breveFunctionPushCodeFirstFloat(brEval arguments[], brEval *result, brInstance *instance) {
	const push::Code *code = (push::Code*)BRPOINTER(&arguments[0]);
	bool found = false;

	result->set( brevePushCodeFirstFloat( code, &found ) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for \ref brevePushCodeToEvalList.
*/

int breveFunctionPushCodeGetEvalList( brEval arguments[], brEval *result, brInstance *instance) {
	push::Code *code = (push::Code*)BRPOINTER(&arguments[0]);

	result->set( brevePushCodeToEvalList( code) );

	return EC_OK;
}


/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeFree.

	See the documentation for \ref pushCodeFree for more details.
*/

int breveFunctionPushCodeFree(brEval arguments[], brEval *result, brInstance *instance) {
	PushCode *code = BRPOINTER(&arguments[0]);

	pushCodeFree(code);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeSize.

	See the documentation for \ref pushCodeSize for more details.
*/

int breveFunctionPushCodeSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushCode *code = BRPOINTER(&arguments[0]);

	result->set( pushCodeSize(code) );

	return EC_OK;
}

int breveFunctionPushCodeTopLevelSize(brEval arguments[], brEval *result, brInstance *instance) {
	push::Code *p1 = (push::Code*)BRPOINTER(&arguments[0]);

	result->set( (int)(*p1)->get_stack().size() );

	return EC_OK;
}

int breveFunctionPushCodeSwapSublists(brEval arguments[], brEval *result, brInstance *instance) {
	push::Code *p1 = (push::Code*)BRPOINTER(&arguments[0]);
	int x = BRINT(&arguments[1]);
	int y = BRINT(&arguments[2]);

	push::swap_sublist_position(*p1, x, y);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushIntStackSize.

	See the documentation for \ref pushIntStackSize for more details.
*/

int breveFunctionPushIntStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushIntStackSize(environment) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushIntStackPop.

	See the documentation for \ref pushIntStackPop for more details.
*/

int breveFunctionPushIntStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushIntStackPop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushIntStackTop.

	See the documentation for \ref pushIntStackTop for more details.
*/

int breveFunctionPushIntStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushIntStackTop(environment) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushIntStackPush.

	See the documentation for \ref pushIntStackPush for more details.
*/

int breveFunctionPushIntStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	int value = BRINT(&arguments[1]);

	pushIntStackPush(environment, value);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushBoolStackSize.

	See the documentation for \ref pushBoolStackSize for more details.
*/

int breveFunctionPushBoolStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushBoolStackSize(environment) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushBoolStackPop.

	See the documentation for \ref pushBoolStackPop for more details.
*/

int breveFunctionPushBoolStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushBoolStackPop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushBoolStackTop.

	See the documentation for \ref pushBoolStackTop for more details.
*/

int breveFunctionPushBoolStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushBoolStackTop(environment) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushBoolStackPush.

	See the documentation for \ref pushBoolStackPush for more details.
*/

int breveFunctionPushBoolStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	int value = BRINT(&arguments[1]);

	pushBoolStackPush(environment, value);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushFloatStackSize.

	See the documentation for \ref pushFloatStackSize for more details.
*/

int breveFunctionPushFloatStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushFloatStackSize(environment) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushFloatStackPop.

	See the documentation for \ref pushFloatStackPop for more details.
*/

int breveFunctionPushFloatStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushFloatStackPop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushFloatStackTop.

	See the documentation for \ref pushFloatStackTop for more details.
*/

int breveFunctionPushFloatStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushFloatStackTop(environment) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushFloatStackPush.

	See the documentation for \ref pushFloatStackPush for more details.
*/

int breveFunctionPushFloatStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	double value = BRDOUBLE(&arguments[1]);

	pushFloatStackPush(environment, value);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushNameStackSize.

	See the documentation for \ref pushNameStackSize for more details.
*/

int breveFunctionPushNameStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushNameStackSize(environment) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushNameStackPop.

	See the documentation for \ref pushNameStackPop for more details.
*/

int breveFunctionPushNameStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushNameStackPop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushNameStackTop.

	See the documentation for \ref pushNameStackTop for more details.
*/

int breveFunctionPushNameStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushNameStackTop(environment) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushNameStackPush.

	See the documentation for \ref pushNameStackPush for more details.
*/

int breveFunctionPushNameStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	int value = BRINT(&arguments[1]);

	pushNameStackPush( environment, value );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeStackSize.

	See the documentation for \ref pushCodeStackSize for more details.
*/

int breveFunctionPushCodeStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushCodeStackSize(environment) );

	return EC_OK;
}

int breveFunctionPushExecStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( (int) push::get_stack<push::Exec>( ( (push::Env*)environment )->next() ).size() );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeStackPop.

	See the documentation for \ref pushCodeStackPop for more details.
*/

int breveFunctionPushCodeStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushCodeStackPop(environment);

	return EC_OK;
}

int breveFunctionPushExecStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	if( push::get_stack<push::Exec>( ( (push::Env*)environment )->next() ).size() > 0) 
		push::get_stack<push::Exec>( ( (push::Env*)environment )->next() ).pop_back();

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeStackTop.

	See the documentation for \ref pushCodeStackTop for more details.
*/

int breveFunctionPushCodeStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushCodeStackTop(environment) );

	return EC_OK;
}

int breveFunctionPushExecStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( new push::Code( push::get_stack<push::Exec>( ( ( push::Env* )environment )->next() ).back() ) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeStackPush.

	See the documentation for \ref pushCodeStackPush for more details.
*/

int breveFunctionPushCodeStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	PushCode *value = BRPOINTER(&arguments[1]);

	pushCodeStackPush(environment, value);

	return EC_OK;
}

int breveFunctionPushExecStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	PushCode *value = BRPOINTER(&arguments[1]);
	// push::Exec *evalu = (push::Exec*)value 

	// push::get_stack<push::Exec>( ( ( push::Env* )environment )->next() ).push_guarded( push::Exec( *(push::Code*)value ) );
	( ( push::Env* )environment )->next().push_guarded( *(push::Code*)value );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushVectorStackSize.

	See the documentation for \ref pushVectorStackSize for more details.
*/

int breveFunctionPushVectorStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	result->set( pushVectorStackSize(environment) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushVectorStackPop.

	See the documentation for \ref pushVectorStackPop for more details.
*/

int breveFunctionPushVectorStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushVectorStackPop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushVectorStackTop.

	See the documentation for \ref pushVectorStackTop for more details.
*/

int breveFunctionPushVectorStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	slVector v;

	pushVectorStackTop( BRPOINTER(&arguments[0]), (PushVector*)&v );
	result->set( v );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushVectorStackPush.

	See the documentation for \ref pushVectorStackPush for more details.
*/

int breveFunctionPushVectorStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	PushVector *value = (PushVector*)&BRVECTOR(&arguments[1]);

	pushVectorStackPush(environment, value);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeDeletionMutate.

	See the documentation for \ref pushCodeDeletionMutate for more details.
*/

int breveFunctionPushCodeDeletionMutate(brEval arguments[], brEval *result, brInstance *instance) {
 	PushCode *p1 = BRPOINTER(&arguments[0]);

	result->set( pushCodeDeletionMutate(p1) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeFlattenMutate.

	See the documentation for \ref pushCodeFlattenMutate for more details.
*/

int breveFunctionPushCodeFlattenMutate(brEval arguments[], brEval *result, brInstance *instance) {
 	PushCode *p1 = BRPOINTER(&arguments[0]);

	result->set( pushCodeFlattenMutate(p1) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeSubtreeMutate.

	See the documentation for \ref pushCodeSubtreeMutate for more details.
*/

int breveFunctionPushCodeSubtreeMutate(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	PushCode *p1 = BRPOINTER(&arguments[1]);
 	int size = BRINT(&arguments[2]);

	result->set( pushCodeSubtreeMutate(environment, p1, size) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeCrossover.

	See the documentation for \ref pushCodeCrossover for more details.
*/

int breveFunctionPushCodeCrossover(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	PushCode *p1 = BRPOINTER(&arguments[1]);
 	PushCode *p2 = BRPOINTER(&arguments[2]);

	result->set( pushCodeCrossover(environment, p1, p2) );

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeRandom.

	See the documentation for \ref pushCodeRandom for more details.
*/

int breveFunctionPushCodeRandom(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	int size = BRINT(&arguments[1]);

	result->set( pushCodeRandom(environment, size) );

	return EC_OK;
}

int breveFunctionPushCodeDiscrepancy(brEval arguments[], brEval *result, brInstance *instance) {
	push::Code *p1, *p2;

	p1 = (push::Code*)BRPOINTER(&arguments[0]);
	p2 = (push::Code*)BRPOINTER(&arguments[1]);

	result->set( push::discrepancy(*p1, *p2) );

	return EC_OK;
}

int breveFunctionPushCodeTopLevelDiff(brEval arguments[], brEval *result, brInstance *instance) {
	push::Code *p1, *p2;
	unsigned int length, diff, n;
	unsigned int s1, s2;

	p1 = (push::Code*)BRPOINTER(&arguments[0]);
	p2 = (push::Code*)BRPOINTER(&arguments[1]);

	const push::CodeArray& st1 = (*p1)->get_stack();
	const push::CodeArray& st2 = (*p2)->get_stack();

	// get the shortest length

	s1 = st1.size();
	s2 = st2.size();

	if(s1 < s2) length = s1;
	else length = s2;

	// any extra elements are automatically different

	diff = abs(s1 - s2);

	// the lists are stored backwards.

	for(n=0; n < length; n++) {
		if( !equal_to(st1[s1 - (n+1)], st2[s2 - (n+1)])) {
			// printf("%s == %s\n", pushCodeGetString((PushCode*)&st1[n]), pushCodeGetString((PushCode*)&st2[n]));
			diff++;
		}
	}

	result->set( (int)diff );

	return EC_OK;
}

int breveFunctionPushDiversityPoolNew(brEval args[], brEval *result, brInstance *instance) {
	result->set( new push::DiversityPool(BRINT(&args[0])) );
	return EC_OK;
}

int breveFunctionPushDiversityPoolFree(brEval args[], brEval *result, brInstance *instance) {
	push::DiversityPool *pool = (push::DiversityPool*)BRPOINTER(&args[0]);
	delete pool;
	return EC_OK;
}

int breveFunctionPushDiversityPoolAdd(brEval args[], brEval *result, brInstance *instance) {
	push::DiversityPool *pool = (push::DiversityPool*)BRPOINTER(&args[0]);
	push::Code *code = (push::Code*)BRPOINTER(&args[1]);
	pool->addIndividual(*code);
	return EC_OK;
}

int breveFunctionPushDiversityPoolClear(brEval args[], brEval *result, brInstance *instance) {
	push::DiversityPool *pool = (push::DiversityPool*)BRPOINTER(&args[0]);
	pool->clear();
	return EC_OK;
}

int breveFunctionPushDiversityPoolGetCount(brEval args[], brEval *result, brInstance *instance) {
	push::DiversityPool *pool = (push::DiversityPool*)BRPOINTER(&args[0]);
	result->set( pool->getSize() );

	return EC_OK;
}

int breveFunctionPushDiversityPoolSetTolerance(brEval args[], brEval *result, brInstance *instance) {
	push::DiversityPool *pool = (push::DiversityPool*)BRPOINTER(&args[0]);
	pool->setTolerance(BRINT(&args[1]));

	return EC_OK;
}

#endif /* HAVE_LIBPUSH */
/*@}*/

void breveInitPushFunctions(brNamespace *n) {
#ifdef HAVE_LIBPUSH
	brNewBreveCall(n, "pushEnvironmentNew", breveFunctionPushEnvironmentNew, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushEnvironmentFree", breveFunctionPushEnvironmentFree, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushEnvironmentReadConfigFile", breveFunctionPushEnvironmentReadConfigFile, AT_NULL, AT_POINTER, AT_STRING, 0);
 	brNewBreveCall(n, "pushRun", breveFunctionPushRun, AT_INT, AT_POINTER, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushInterpreterDone", breveFunctionPushInterpreterDone, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushStep", breveFunctionPushStep, AT_INT, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushGetConfigString", breveFunctionPushGetConfigString, AT_STRING, AT_POINTER, 0);
 	brNewBreveCall(n, "pushGetStacksString", breveFunctionPushGetStacksString, AT_STRING, AT_POINTER, 0);
 	brNewBreveCall(n, "pushGetExecStackString", breveFunctionPushGetExecStackString, AT_STRING, AT_POINTER, 0);
 	brNewBreveCall(n, "pushClearStacks", breveFunctionPushClearStacks, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushEnvironmentSetListLimit", breveFunctionPushEnvironmentSetListLimit, AT_NULL, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushEnvironmentSetEvalLimit", breveFunctionPushEnvironmentSetEvalLimit, AT_NULL, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushEnvironmentSetRandomPointLimit", breveFunctionPushEnvironmentSetRandomPointLimit, AT_NULL, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushEnvironmentGetEvalLimit", breveFunctionPushEnvironmentGetEvalLimit, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeFirstFloat", breveFunctionPushCodeFirstFloat, AT_DOUBLE, AT_POINTER, 0);
 	brNewBreveCall(n, "pushEnvironmentGetListLimit", breveFunctionPushEnvironmentGetListLimit, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushEnvironmentGetRandomPointLimit", breveFunctionPushEnvironmentGetRandomPointLimit, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushParse", breveFunctionPushParse, AT_POINTER, AT_STRING, 0);
 	brNewBreveCall(n, "pushCodeGetString", breveFunctionPushCodeGetString, AT_STRING, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeGetEvalList", breveFunctionPushCodeGetEvalList, AT_LIST, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeFree", breveFunctionPushCodeFree, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeSize", breveFunctionPushCodeSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeTopLevelSize", breveFunctionPushCodeTopLevelSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeSwapSublists", breveFunctionPushCodeSwapSublists, AT_INT, AT_POINTER, AT_INT, AT_INT, 0);

 	brNewBreveCall(n, "pushIntStackSize", breveFunctionPushIntStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushIntStackPop", breveFunctionPushIntStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushIntStackTop", breveFunctionPushIntStackTop, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushIntStackPush", breveFunctionPushIntStackPush, AT_NULL, AT_POINTER, AT_INT, 0);

 	brNewBreveCall(n, "pushBoolStackSize", breveFunctionPushBoolStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushBoolStackPop", breveFunctionPushBoolStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushBoolStackTop", breveFunctionPushBoolStackTop, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushBoolStackPush", breveFunctionPushBoolStackPush, AT_NULL, AT_POINTER, AT_INT, 0);

 	brNewBreveCall(n, "pushFloatStackSize", breveFunctionPushFloatStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushFloatStackPop", breveFunctionPushFloatStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushFloatStackTop", breveFunctionPushFloatStackTop, AT_DOUBLE, AT_POINTER, 0);
 	brNewBreveCall(n, "pushFloatStackPush", breveFunctionPushFloatStackPush, AT_NULL, AT_POINTER, AT_DOUBLE, 0);

 	brNewBreveCall(n, "pushNameStackSize", breveFunctionPushNameStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushNameStackPop", breveFunctionPushNameStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushNameStackTop", breveFunctionPushNameStackTop, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushNameStackPush", breveFunctionPushNameStackPush, AT_NULL, AT_POINTER, AT_INT, 0);

 	brNewBreveCall(n, "pushCodeStackSize", breveFunctionPushCodeStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeStackPop", breveFunctionPushCodeStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeStackTop", breveFunctionPushCodeStackTop, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeStackPush", breveFunctionPushCodeStackPush, AT_NULL, AT_POINTER, AT_POINTER, 0);

 	brNewBreveCall(n, "pushExecStackSize", breveFunctionPushExecStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushExecStackPop", breveFunctionPushExecStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushExecStackTop", breveFunctionPushExecStackTop, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushExecStackPush", breveFunctionPushExecStackPush, AT_NULL, AT_POINTER, AT_POINTER, 0);

 	brNewBreveCall(n, "pushCodeDiscrepancy", breveFunctionPushCodeDiscrepancy, AT_INT, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeTopLevelDiff", breveFunctionPushCodeTopLevelDiff, AT_INT, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushVectorStackSize", breveFunctionPushVectorStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushVectorStackPop", breveFunctionPushVectorStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushVectorStackTop", breveFunctionPushVectorStackTop, AT_VECTOR, AT_POINTER, AT_VECTOR, 0);
 	brNewBreveCall(n, "pushVectorStackPush", breveFunctionPushVectorStackPush, AT_NULL, AT_POINTER, AT_VECTOR, 0);
 	brNewBreveCall(n, "pushCodeDeletionMutate", breveFunctionPushCodeDeletionMutate, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeFlattenMutate", breveFunctionPushCodeFlattenMutate, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeSubtreeMutate", breveFunctionPushCodeSubtreeMutate, AT_POINTER, AT_POINTER, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushCodeCrossover", breveFunctionPushCodeCrossover, AT_POINTER, AT_POINTER, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeRandom", breveFunctionPushCodeRandom, AT_POINTER, AT_POINTER, AT_INT, 0);

 	brNewBreveCall(n, "pushDiversityPoolNew", breveFunctionPushDiversityPoolNew, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushDiversityPoolFree", breveFunctionPushDiversityPoolFree, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushDiversityPoolClear", breveFunctionPushDiversityPoolClear, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushDiversityPoolGetCount", breveFunctionPushDiversityPoolGetCount, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushDiversityPoolSetTolerance", breveFunctionPushDiversityPoolSetTolerance, AT_NULL, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushDiversityPoolAdd", breveFunctionPushDiversityPoolAdd, AT_NULL, AT_POINTER, AT_POINTER, 0);
#endif /* HAVE_LIBPUSH */
}
