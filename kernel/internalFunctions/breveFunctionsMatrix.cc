#include "kernel.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

#ifdef HAVE_LIBGSL
int brIMatrixNew(brEval args[], brEval *target, brInstance *i) {
	gsl_matrix *m;

	BRPOINTER(target) = m = gsl_matrix_alloc(BRINT(&args[0]), BRINT(&args[1]));

	if(!m) {
		slMessage(DEBUG_ALL, "Could not create matrix gsl_matrix_alloc failed\n");
		return EC_ERROR;
	}

	gsl_matrix_set_zero(m);

	return EC_OK;
}

int brIMatrixFree(brEval args[], brEval *target, brInstance *i) {
	if(BRPOINTER(&args[0])) gsl_matrix_free(BRPOINTER(&args[0]));
	return EC_OK;
}

int brIMatrixGet(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = gsl_matrix_get(BRPOINTER(&args[0]), BRINT(&args[1]), BRINT(&args[2]));
	return EC_OK;
}

int brIMatrixSet(brEval args[], brEval *target, brInstance *i) {
	gsl_matrix_set(BRPOINTER(&args[0]), BRINT(&args[1]), BRINT(&args[2]), BRDOUBLE(&args[3]));
	return EC_OK;
}
#endif

void breveInitMatrixFunctions(brNamespace *n) {
#ifdef HAVE_LIBGSL
	brNewBreveCall(n, "matrixNew", brIMatrixNew, AT_POINTER, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "matrixFree", brIMatrixFree, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "matrixGet", brIMatrixGet, AT_DOUBLE, AT_POINTER, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "matrixSet", brIMatrixSet, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_DOUBLE, 0);
#endif
}
