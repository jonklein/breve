#include "../../plugins/slBrevePluginAPI.h"
#include <math.h>
#include <stdio.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

/* Exported function declarations */

int GSLRNG_new(stEval args[], stEval *result, void *i);
int GSLRNG_free(stEval args[], stEval *result, void *i);
int GSLRNG_set(stEval args[], stEval *result, void *i);
int GSLRNG_uniform(stEval args[], stEval *result, void *i);
int GSLRNG_uniform_pos(stEval args[], stEval *result, void *i);
int GSLRNG_uniform_int(stEval args[], stEval *result, void *i);
int GSLRNG_gaussian(stEval args[], stEval *result, void *i);
int GSLRNG_exponential(stEval args[], stEval *result, void *i);
int GSLRNG_poisson(stEval args[], stEval *result, void *i);
int GSLRNG_binomial(stEval args[], stEval *result, void *i);
int GSLRNG_negative_binomial(stEval args[], stEval *result, void *i);
int GSLRNG_gamma(stEval args[], stEval *result, void *i);
int GSLRNG_weibull(stEval args[], stEval *result, void *i);
int GSLRNG_bernoulli(stEval args[], stEval *result, void *i);
int GSLRNG_geometric(stEval args[], stEval *result, void *i);

/* our entry-point function: one call to stNewSteveCall for
each function */

#ifdef WINDOWS
asm(".section .drectve");
asm(".ascii \"-export:GSLRNGLoadFunctions\"");
#endif /* WINDOWS */

GSLRNGLoadFunctions(void *data) {
	stNewSteveCall(data, "GSLRNG_new", GSLRNG_new, AT_POINTER, 0);
	stNewSteveCall(data, "GSLRNG_free", GSLRNG_free, AT_NULL, AT_POINTER, 0);
	stNewSteveCall(data, "GSLRNG_set", GSLRNG_set, AT_NULL, AT_POINTER,AT_INT, 0);
	stNewSteveCall(data, "GSLRNG_uniform", GSLRNG_uniform, AT_DOUBLE, AT_POINTER, 0);
	stNewSteveCall(data, "GSLRNG_uniform_pos", GSLRNG_uniform_pos, AT_DOUBLE, AT_POINTER, 0);
	stNewSteveCall(data, "GSLRNG_uniform_int", GSLRNG_uniform_int, AT_INT, AT_POINTER, AT_INT,0);
	stNewSteveCall(data, "GSLRNG_gaussian", GSLRNG_gaussian, AT_DOUBLE, AT_POINTER, AT_DOUBLE, AT_DOUBLE,0);
	stNewSteveCall(data, "GSLRNG_exponential", GSLRNG_exponential, AT_DOUBLE, AT_POINTER, AT_DOUBLE,0);
	stNewSteveCall(data, "GSLRNG_poisson", GSLRNG_poisson, AT_INT, AT_POINTER, AT_DOUBLE,0);
	stNewSteveCall(data, "GSLRNG_binomial", GSLRNG_binomial, AT_INT, AT_POINTER, AT_DOUBLE,AT_INT,0);
	stNewSteveCall(data, "GSLRNG_negative_binomial", GSLRNG_negative_binomial, AT_INT, AT_POINTER, AT_DOUBLE,AT_INT,0);
	stNewSteveCall(data, "GSLRNG_gamma", GSLRNG_gamma, AT_DOUBLE, AT_POINTER, AT_DOUBLE,AT_DOUBLE,0);
	stNewSteveCall(data, "GSLRNG_weibull", GSLRNG_weibull, AT_DOUBLE, AT_POINTER, AT_DOUBLE,AT_DOUBLE,0);
	stNewSteveCall(data, "GSLRNG_bernoulli", GSLRNG_binomial, AT_INT, AT_POINTER, AT_DOUBLE,0);
	stNewSteveCall(data, "GSLRNG_geometric", GSLRNG_geometric, AT_INT, AT_POINTER, AT_DOUBLE,0);
}

/* Return a pointer to a new gsl_rng: the seed will
be automatically set to 0 */

int GSLRNG_new(stEval *args, stEval *result, void *i) {
    gsl_rng *r;
    r = gsl_rng_alloc(gsl_rng_mt19937);
    STPOINTER(result) = r;
    return EC_OK;
}

int GSLRNG_free(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    gsl_rng_free(r);
    return EC_OK;
}

int GSLRNG_set(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    int seed = STINT(&args[1]);
    gsl_rng_set(r,seed);
    return EC_OK;
}

int GSLRNG_uniform(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    STDOUBLE(result) = gsl_rng_uniform(r);
    return EC_OK;
}

int GSLRNG_uniform_pos(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    STDOUBLE(result) = gsl_rng_uniform_pos(r);
    return EC_OK;
}

int GSLRNG_uniform_int(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    int n = STINT(&args[1]);
    STINT(result) = gsl_rng_uniform_int(r,n);
    return EC_OK;
}


int GSLRNG_gaussian(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    double mu = STDOUBLE(&args[1]);
    double sigma = STDOUBLE(&args[2]);
    STDOUBLE(result) = mu+gsl_ran_gaussian(r,sigma);
    return EC_OK;
}

int GSLRNG_exponential(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    double mu = STDOUBLE(&args[1]);
    STDOUBLE(result) = gsl_ran_exponential(r,mu);
    return EC_OK;
}

int GSLRNG_binomial(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    double p = STDOUBLE(&args[1]);
	int n = STINT(&args[2]);
    STINT(result) = gsl_ran_binomial(r,p,n);
    return EC_OK;
}

int GSLRNG_negative_binomial(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    double p = STDOUBLE(&args[1]);
	int n = STINT(&args[2]);
    STINT(result) = gsl_ran_negative_binomial(r,p,n);
    return EC_OK;
}

int GSLRNG_poisson(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    double mu = STDOUBLE(&args[1]);
    STINT(result) = gsl_ran_poisson(r,mu);
    return EC_OK;
}

int GSLRNG_gamma(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    double a = STDOUBLE(&args[1]);
	double b = STDOUBLE(&args[2]);
    STDOUBLE(result) = gsl_ran_gamma(r,a,b);
    return EC_OK;
}

int GSLRNG_weibull(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    double a = STDOUBLE(&args[1]);
	double b = STDOUBLE(&args[2]);
    STDOUBLE(result) = gsl_ran_weibull(r,a,b);
    return EC_OK;
}

int GSLRNG_bernoulli(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    double p = STDOUBLE(&args[1]);
    STINT(result) = gsl_ran_bernoulli(r,p);
    return EC_OK;
}

int GSLRNG_geometric(stEval *args, stEval *result, void *i) {
    gsl_rng *r = STPOINTER(&args[0]);
    double p = STDOUBLE(&args[1]);
    STINT(result) = gsl_ran_geometric(r,p);
    return EC_OK;
}

