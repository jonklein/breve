/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

#include <gsl/gsl_randist.h>

double stEmRandomGauss();
double stEmRandomExponential();
double stEmRandomGamma(int ithEvent);

/*! \addtogroup InternalFunctions */
/*@{*/

#include "kernel.h"

int brIKeys(brEval arguments[], brEval *result, brInstance *instance) {
	result->set( brEvalHashKeys( BRHASH( &arguments[ 0 ] ) ) );

	return EC_OK;
}


/*!
	\brief Returns a Gaussian random number.

	double randomGauss().
*/

int brIRandomGauss(brEval arguments[], brEval *result, brInstance *instance) {
	result->set( gsl_ran_gaussian( instance->engine->RNG, 1.0) );
	return EC_OK; 
}

/*!
	\brief Returns a random number from the exponential distribution.

	double randomExponential().
*/

int brIRandomExponential(brEval arguments[], brEval *result, brInstance *instance) {
	result->set( stEmRandomExponential() );
	// BRDOUBLE(result)  = stEmRandomExponentialRNG);
	return EC_OK; 
}

/*!
	\brief Returns a number from the Gamma distribution.

	double randomGamma(int).
*/

int brIRandomGamma(brEval arguments[], brEval *result, brInstance *instance) {
	int ithEvent = BRINT(&arguments[0]);
	result->set( stEmRandomGamma(ithEvent) );
	// BRDOUBLE(result) = gsl_ran_gamma( instance->engine->RNG, A, B );
  
	return EC_OK; 
}

/*!
	\brief Checks whether a double is infinity.

	int isinf(double).
*/

int brIIsinf(brEval args[], brEval *target, brInstance *i) {
	target->set( isinf(BRDOUBLE(&args[0])) );
	return EC_OK;
}

/*!
	\brief Gives the log (base 10) of its input.

	double log(double).
*/

int brILog(brEval arguments[], brEval *result, brInstance *instance) {
	double x = BRDOUBLE(&arguments[0]);
	result->set( log(x) ); 
	return EC_OK; 
}

/*!
	\brief Checks whether a double is NaN.

	int isnan(double).
*/

int brIIsnan(brEval args[], brEval *target, brInstance *i) {
	target->set( isnan(BRDOUBLE(&args[0])) );
	return EC_OK;
}

/*!
	\brief Gives the absolute value of a double.

	double abs(double).
*/

int brIAbs(brEval args[], brEval *target, brInstance *i) {
	target->set( fabs(BRDOUBLE(&args[0])) );
	return EC_OK;
}

/*!
	\brief Gives the sine of its input.

	double sin(double).
*/

int brISin(brEval args[], brEval *target, brInstance *i) {
	target->set( sin(BRDOUBLE(&args[0])) );
	return EC_OK;
}

/*!
	\brief Gives the cosine of its input.

	double cos(double).
*/

int brICos(brEval args[], brEval *target, brInstance *i) {
	target->set( cos(BRDOUBLE(&args[0])) );
	return EC_OK;
}

/*!
	\brief Gives the tangent of its input.

	double tan(double).
*/

int brITan(brEval args[], brEval *target, brInstance *i) {
	target->set( tan(BRDOUBLE(&args[0])) );
	return EC_OK;
}

/*!
	\brief Gives the arc-sine of its input.

	double asin(double).
*/

int brIAsin(brEval args[], brEval *target, brInstance *i) {
	target->set( asin(BRDOUBLE(&args[0])) );
	return EC_OK;
}

/*!
	\brief Gives the arc-cosine of its input.

	double acos(double).
*/

int brIAcos(brEval args[], brEval *target, brInstance *i) {
	target->set( acos(BRDOUBLE(&args[0])) );
	return EC_OK;
}

/*!
	\brief Gives the arc-tangent of its input.

	double atan(double).
*/

int brIAtan(brEval args[], brEval *target, brInstance *i) {
	target->set( atan(BRDOUBLE(&args[0])) );
	return EC_OK;
}

/*!
	\brief Gives the principal value of the arc tangent of d1/d2.

	double atan2(double, double).
*/

int brIAtan2(brEval args[], brEval *target, brInstance *i) {
	target->set( atan2(BRDOUBLE(&args[0]), BRDOUBLE(&args[1])) );
	return EC_OK;
}

/*!
	\brief Gives the square root of its input.

	double sqrt(double).
*/

int brISqrt(brEval args[], brEval *target, brInstance *i) {
	target->set( sqrt(BRDOUBLE(&args[0])) );
	return EC_OK;
}

/*!
	\brief Gives the dot product of its two input vectors.

	double sqrt(vector, vector).
*/

int brIDot(brEval args[], brEval *target, brInstance *i) {
	slVector *a = &BRVECTOR(&args[0]);
	slVector *b = &BRVECTOR(&args[1]);

	target->set( slVectorDot(a, b) );

	return EC_OK;
}

/*!
	\brief Gives the angle between two input vectors.

	double angle(vector, vector).
*/

int brIAngle(brEval args[], brEval *target, brInstance *i) {
	slVector *a = &BRVECTOR(&args[0]);
	slVector *b = &BRVECTOR(&args[1]);

	target->set( slVectorAngle(a, b) );

	return EC_OK;
}

/*!
	\brief Gives the cross-product of two input vectors.

	vector cross(vector, vector).
*/

int brICross(brEval args[], brEval *target, brInstance *i) {
	slVector *a = &BRVECTOR(&args[0]);
	slVector *b = &BRVECTOR(&args[1]);
	slVector v;

	slVectorCross( a, b, &v );

	target->set( v );

	return EC_OK;
}

/*!
	\brief Gives the maximum of two inputs.

	double max(double, double).
*/

int brIMax(brEval args[], brEval *target, brInstance *i) {
	if(BRDOUBLE(&args[0]) > BRDOUBLE(&args[1])) target->set( BRDOUBLE(&args[0]) );
	else target->set( BRDOUBLE(&args[1]) );

	return EC_OK;
}

/*!
	\brief Gives the minimum of two inputs.

	double min(double, double).
*/

int brIMin(brEval args[], brEval *target, brInstance *i) {
	if(BRDOUBLE(&args[0]) < BRDOUBLE(&args[1])) target->set( BRDOUBLE(&args[0]) );
	else target->set( BRDOUBLE(&args[1]) );

	return EC_OK;
}

/**
 * exp() wrapper
 */

int brIExp(brEval args[], brEval *target, brInstance *i) {
	target->set( exp( BRDOUBLE(&args[0]) ) );

	return EC_OK;
}


/*!
	\brief Gives the transpose of an input matrix.

	matrix transpose(matrix).
*/

int brITranspose(brEval args[], brEval *target, brInstance *i) {
	slMatrix m;

	slMatrixTranspose(BRMATRIX(&args[0]), m );

	target->set( m );

	return EC_OK;
}

/*!
	\brief Gives the standard deviation of a list of numbers.
	
	double stddev(list).
*/

int brIStddev(brEval args[], brEval *target, brInstance *i) {
	double sum = 0, sumsq = 0, top;
	brEvalListHead *list = BRLIST(&args[0]);
	int n = 0;
	std::vector< brEval* >::iterator li;

	for( li = list->_vector.begin(); li != list->_vector.end(); li++ ) {
		double value;

		if( ( *li )->type() == AT_INT ) {
			value = BRINT( *li );
		} else if( ( *li )->type() == AT_DOUBLE ) {
			value = BRDOUBLE( *li );
		} else {
			slMessage(DEBUG_ALL, "Internal function stddev expects a list of number values\n");
			return EC_ERROR;
		}

		sum += value;
		sumsq += value * value;

		n++;
	}

	sum /= n;
	top = sumsq - (n * sum * sum);

	target->set( sqrt(top / (double)(n - 1)) );

	return EC_OK;
}

/*@}*/

void breveInitMathFunctions(brNamespace *n) {
	brNewBreveCall(n, "keys", brIKeys, AT_LIST, AT_HASH, 0);

	brNewBreveCall(n, "isinf", brIIsinf, AT_INT, AT_DOUBLE, 0);
	brNewBreveCall(n, "isnan", brIIsnan, AT_INT, AT_DOUBLE, 0);
	brNewBreveCall(n, "abs", brIAbs, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "sin", brISin, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "cos", brICos, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "tan", brITan, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "asin", brIAsin, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "acos", brIAcos, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "exp", brIExp, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "atan", brIAtan, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "atan2", brIAtan2, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "sqrt", brISqrt, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "angle", brIAngle, AT_DOUBLE, AT_VECTOR, AT_VECTOR, 0);
	brNewBreveCall(n, "cross", brICross, AT_VECTOR, AT_VECTOR, AT_VECTOR, 0);
	brNewBreveCall(n, "dot", brIDot, AT_DOUBLE, AT_VECTOR, AT_VECTOR, 0);
	brNewBreveCall(n, "max", brIMax, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "min", brIMin, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "transpose", brITranspose, AT_MATRIX, AT_MATRIX, 0);

	brNewBreveCall(n, "randomGauss", brIRandomGauss, AT_DOUBLE, 0);
	brNewBreveCall(n, "randomExponential", brIRandomExponential, AT_DOUBLE, 0);
	brNewBreveCall(n, "randomGamma", brIRandomGamma, AT_DOUBLE, AT_INT, 0);
	brNewBreveCall(n, "log", brILog, AT_DOUBLE, AT_DOUBLE, 0);

	brNewBreveCall(n, "stddev", brIStddev, AT_DOUBLE, AT_LIST, 0);
}

/*
	\brief Return a random double between 0.0 and 1.0.
*/

#define drand() (((double) random()) / 0x7fffffff)

/*!
	\brief Draw from Gaussian/Normal distribution.  

	One may use the routine for non-standard Gauss distributions by 
	simply multiplying the result by the standard deviation you would 
	like and adding the mean.  This assumes the mean is 0.0 and the 
	standard deviation is 1.0, of course.
*/

double stEmRandomGauss() {
	static int bIsExtra = 0;
	static double extraDev;
	double transFactor, radius, x1, x2;

	if (bIsExtra == 0)   /* If we don't have an extra deviate... */
	{
		do { /* Keep drawing until we have a point in the unit disc */
		x1 = 2.0 * drand() - 1.0;
		x2 = 2.0 * drand() - 1.0;
		radius = x1*x1 + x2*x2;	  
	}  while (radius >= 1.0);	  
		/* Such redrawing is reasonable.  The expected number of loop
		   steps is constant (roughly 5).  The probability of a
		   a non-constant number approaches 0 exponentially fast.
		   We could choose to use the true BM transform, but the
		   sinusoidal functions actually COST more than this.  We
		   do it this way to avoid the sinusoidals in the transform.  */

		 /* Box-Muller Transform*/

		transFactor = sqrt(-2.0*log(radius)/radius);
		extraDev = x1 * transFactor;
		bIsExtra = 1;
		return (x2 * transFactor);
	} else  {
		/* we DO have an extra deviate... */
		bIsExtra = 0;		
		return (extraDev);
	}
}  
  
/*!
	\brief Draw from exponential distribution with an assumed mean of 1.0.  
*/

double stEmRandomExponential()
{
	double x = 0;
  
	do {
		x = drand();
	} while (x == 0.0);  

	/* Zero is a problem for the log 8^) */

	return -log(x);
}

/*!
	\brief Draw from the Gamma distribution.  
	
	ithEvent is the ith Event in the Poisson process on which we are waiting.
*/

double stEmRandomGamma(int ithEvent) {
	int j;
	float am,error,s,v1,v2,x,y;

	if (ithEvent < 1) {
		slMessage(DEBUG_ALL, "error in function stEmRandomGamma(): event < 1");
		return 0.0;
	}

	if (ithEvent < 6) { 
		x=1.0;

		for (j=1;j<=ithEvent;j++) x *= drand();

		x = -log(x);
	} else {	   
		/* use rejection  */

		do {
			do {
				do {/* essentially: y = tan(M_PI*drand()); */
					v1=drand();
					v2=2.0*drand()-1.0;
				} while (v1*v1+v2*v2 > 1.0); 

				y=v2/v1;
				am=ithEvent-1;
				s=sqrt(2.0*am+1.0);
				x=s*y+am;
			} while (x <= 0.0); 

			error=(1.0+y*y)*exp(am*log(x/am)-s*y); 
		} while (drand() > error); 
	}

	return x;
}
