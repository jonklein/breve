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

#include "simulation.h"

/*
	\brief Calculates the acceleration/velocity derivatives for a link.
*/

inline int slCalculateDerivs(slLink *r, double *sv, double *dv, slWorld *w) {
	slLinkIntegrationPosition *df, *sf;

	df = (slLinkIntegrationPosition*)&dv[0];
	sf = (slLinkIntegrationPosition*)&sv[0];

	slVectorCopy(&sf->velocity.b, &df->location);
	slAngularVelocityToDeriv(&sf->velocity.a, &sf->rotQuat, &df->rotQuat);

	slsVectorCopy(&r->acceleration, &df->velocity);

	return 0;
}

/*!
	\brief Sums two configuration vectors together for integration

	outv = inv + (derv * w) 
*/

inline void slSumConfigVectors(double *inv, double *derv, double *outv, double w) {
	unsigned int n;

	for(n=0;n<sizeof(slLinkIntegrationPosition)/sizeof(double);n++) outv[n] = inv[n] + (derv[n] * w);
}

/* 
   all integrators currently take the arguments slMultibody body, 
   vector gravity, double timestep and an int indicating whether 
   this is the first time the integrator is being called this timestep.
   this allows us to avoid recomputing the derivitive vector.
*/

int slEuler(slWorld *w, slLink *r, double *deltaT, int skipFirst) {
	double *osv, *sv;
	int err = 0;

	if(r->mobile < 1) {
		if(r->mobile == -1) {
			bcopy(&r->stateVector[r->currentState], &r->stateVector[!r->currentState], sizeof(slLinkIntegrationPosition));
			r->mobile = 0;
		}

		return 0;
	}

	osv = (double*)&r->stateVector[!r->currentState];
	sv = (double*)&r->stateVector[r->currentState];

	if(!skipFirst) err = slCalculateDerivs(r, sv, w->dv[0], w);

	slSumConfigVectors(sv, w->dv[0], osv, *deltaT);

	return err;
}

/*!
	\brief Runge-Kutta 4th order integrator.

	does an RK4 integration for a slMultibody r over timestep deltaT.
	in the case of a collision, we may be reiterating from the same 
	starting point with a smaller deltaT--if this is the case we could 
	save time by skipping the first slCalculateDerivs() call.  if skipFirst 
	is true then the previous dv[0] slVector is used.
*/

int slRK4(slWorld *w, slLink *r, double *deltaT, int skipFirst) {
	double *osv, *sv;
	unsigned int n;
	int err = 0;

	double hDelta;

	hDelta = (*deltaT)/2;

	/* old state slVector and new */

	osv = (double*)&r->stateVector[!r->currentState];
	sv = (double*)&r->stateVector[r->currentState];

	if(r->mobile < 1) {
		if(r->mobile == -1) {
			bcopy(&r->stateVector[r->currentState], &r->stateVector[!r->currentState], sizeof(slLinkIntegrationPosition));
			r->mobile = 0;
		}

		return 0;
	}

	err += slCalculateDerivs(r, sv, w->dv[0], w);
	slSumConfigVectors(sv, w->dv[0], w->dv[4], hDelta);

	err += slCalculateDerivs(r, w->dv[4], w->dv[1], w);
	slSumConfigVectors(sv, w->dv[1], w->dv[4], hDelta);

	err += slCalculateDerivs(r, w->dv[4], w->dv[2], w);
	slSumConfigVectors(sv, w->dv[2], w->dv[4], *deltaT);

	err += slCalculateDerivs(r, w->dv[4], w->dv[3], w);

	for(n=0;n<sizeof(slLinkIntegrationPosition)/sizeof(double);n++) {
		w->dv[0][n] += w->dv[3][n];
		w->dv[1][n] += w->dv[2][n];
	}

	slSumConfigVectors(sv, w->dv[0], osv, (*deltaT) / 6.0);
	slSumConfigVectors(osv, w->dv[1], osv, (*deltaT) / 3.0);

	return err;
}

/* spiderland-multibody-runge-kutta-cash-karp */

#define FMAX(x, y)	((x)>(y)?(x):(y))
#define FMIN(x, y)	((x)<(y)?(x):(y))
#define SAFETY	0.9
#define PGROW	-0.2
#define PSHRNK	-0.25
#define ERRCON	1.89e-4
#define TINY	1.0e-30

int slRKCKRun(slWorld *w, slLink *m, double *deltaT, int skipFirst) {
	double time = 0.0;
	int r;

	r = slRKCK(w, m, &time, 0.00001, *deltaT, deltaT);

	return r;
}

int slRKCK(slWorld *w, slLink *m, double *x, double acc, double stepSize, double *nextSize) {
	double *osv, *sv;
	double errmax, h, htemp, xnew;
	int err = 0;
	unsigned int i;

	osv = (double*)&m->stateVector[!m->currentState];
	sv = (double*)&m->stateVector[m->currentState];

	h = stepSize;

	while(1) {
		err += slRKCKS(w, m, sv, osv, w->dv[9], h);
		errmax = 0.0;

		for(i=0;i<sizeof(slLinkIntegrationPosition)/sizeof(double);i++) {
			w->dv[8][i] = fabs(sv[i]) + fabs(w->dv[0][i] * h) + TINY;
			// w->dv[8][i] = w->dv[0][i] * h + TINY;
			errmax = FMAX(errmax, fabs(w->dv[9][i] / w->dv[8][i]));
		}

		errmax /= acc;
		if(errmax <= 1.0) break;

		htemp = SAFETY * h * pow(errmax, PSHRNK);

		h = (h >= 0.0 ? FMAX(htemp, 0.1 * h) : FMIN(htemp, 2.0 * h));

		xnew = (*x) + h;

		if(xnew == *x) return -1;
	}

	if(errmax > ERRCON) *nextSize = SAFETY * h * pow(errmax, PGROW);
	else *nextSize = 5.0 * h;

	*x += h;

	return err;
}

/* spiderland-multibody-runge-kutta-cash-karp-step */

int slRKCKS(slWorld *w, slLink *m, double *sv, double *osv, double *error, double h) {
	unsigned int n;
	int err = 0;

	/* multipliers */

	double b[5][5] = {
	{			0.2,		 0.0,		   0.0,			  0.0,		  0.0 },
	{		  0.075,	   0.225,		   0.0,			  0.0,		  0.0 },
	{			0.3,		-0.9,		   1.2,			  0.0,		  0.0 },
	{	 -11.0/54.0,		 2.5,	-70.0/27.0,		35.0/27.0,		  0.0 },
	{ 1631.0/55296.0, 175.0/512.0, 575.0/13824.0, 44275.0/110592.0, 253.0/4096.0 } };

	/* vector weights */

	double c[6] = { 37.0/378.0, 0.0, 250.0/621.0, 125.0/594.0, 0.0, 512.0/1771.0 }; 

	/* error weights */

	double dc[6] = { c[0] - 2825.0/27648.0,
					 c[1] - 0.0,
					 c[2] - 18575.0/48384.0,
					 c[3] - 13525.0/55296.0,
					 c[4] - 277.0/14336.0,
					 c[5] - 0.25 };
					  
	/* dv[0 - 6] will be the outputs of the individual iterations */
	/* dv[7] will be the accumulation after each step. */

	err += slCalculateDerivs(m, sv, w->dv[0], w);
	for(n=0;n<sizeof(slLinkIntegrationPosition)/sizeof(double);n++) 
		w->dv[7][n] = sv[n] + h*(b[0][0]*w->dv[0][n]);

	err += slCalculateDerivs(m, w->dv[7], w->dv[1], w);
	for(n=0;n<sizeof(slLinkIntegrationPosition)/sizeof(double);n++) 
		w->dv[7][n] = sv[n] + h*(b[1][0]*w->dv[0][n] +
								 b[1][1]*w->dv[1][n]);

	err += slCalculateDerivs(m, w->dv[7], w->dv[2], w);
	for(n=0;n<sizeof(slLinkIntegrationPosition)/sizeof(double);n++) 
		w->dv[7][n] = sv[n] + h*(b[2][0]*w->dv[0][n] +
								 b[2][1]*w->dv[1][n] +
								 b[2][2]*w->dv[2][n]);

	err += slCalculateDerivs(m, w->dv[7], w->dv[3], w);
	for(n=0;n<sizeof(slLinkIntegrationPosition)/sizeof(double);n++) 
		w->dv[7][n] = sv[n] + h*(b[3][0]*w->dv[0][n] +
								 b[3][1]*w->dv[1][n] +
								 b[3][2]*w->dv[2][n] +
								 b[3][3]*w->dv[3][n]);

	err += slCalculateDerivs(m, w->dv[7], w->dv[4], w);
	for(n=0;n<sizeof(slLinkIntegrationPosition)/sizeof(double);n++) 
		w->dv[7][n] = sv[n] + h*(b[4][0]*w->dv[0][n] +
								 b[4][1]*w->dv[1][n] +
								 b[4][2]*w->dv[2][n] +
								 b[4][3]*w->dv[3][n] +
								 b[4][4]*w->dv[4][n]);

	err += slCalculateDerivs(m, w->dv[7], w->dv[5], w);
	for(n=0;n<sizeof(slLinkIntegrationPosition)/sizeof(double);n++) {
		osv[n] = sv[n] + h*(c[0]*w->dv[0][n] +
							c[2]*w->dv[2][n] +
							c[3]*w->dv[3][n] +
							c[5]*w->dv[5][n]);

		error[n] = h*(dc[0]*w->dv[0][n] +
					  dc[2]*w->dv[2][n] +
					  dc[3]*w->dv[3][n] +
					  dc[4]*w->dv[4][n] +
					  dc[5]*w->dv[5][n]);
	}

	return err;
}

#undef FMAX
#undef FMIN
#undef SAFETY
#undef PGROW
#undef PSHRNK
#undef ERRCON
#undef TINY

#define RKF_MAX_TIMESTEP	0.01
#define RKF_MIN_TIMESTEP	0.0005
#define RKF_TOLERANCE		0.001
#define RKF_MAX(x,y)		((x)>(y)?(x):(y))

int slIntRKF(slWorld *w, slLink *m, double *deltaT, int skipFirst) {
	unsigned int i = 0;
	int simerr = 0;

	double error;
	double sum;

	double timeStep = *deltaT;

	double delta = 0;
	double *sv, *osv;

	osv = (double*)&m->stateVector[!m->currentState];
	sv = (double*)&m->stateVector[m->currentState];

	//-- get w->dv[8] values from object at current time --//
	// Derive( sizeof(slLinkIntegrationPosition), y, w->dv[8], time );
	simerr += slCalculateDerivs(m, sv, w->dv[8], w);

	for ( i=0; i < sizeof(slLinkIntegrationPosition); i++ ) {
		w->dv[0][i] = timeStep * w->dv[8][i];
		w->dv[7][i] = sv[i] + w->dv[0][i] / 4;
	}

	//-- get w->dv[8] values from object at current time + 1/4--//
	// Derive( sizeof(slLinkIntegrationPosition), w->dv[7], w->dv[8], time+timeStep/4 );
	simerr += slCalculateDerivs(m, w->dv[7], w->dv[8], w);

	for (i=0; i < sizeof(slLinkIntegrationPosition)/sizeof(double); i++) {
		w->dv[1][i] = timeStep * w->dv[8][i];
		w->dv[7][i] = sv[i] + ( w->dv[0][i]*3 + w->dv[1][i]*9 ) / 32;
	}

	// Derive( sizeof(slLinkIntegrationPosition), w->dv[7], w->dv[8], time+timeStep*3/8 );
	simerr += slCalculateDerivs(m, w->dv[7], w->dv[8], w);

	 for (i=0; i < sizeof(slLinkIntegrationPosition)/sizeof(double); i++) {
		 w->dv[2][i] = timeStep * w->dv[8][i];
		 w->dv[7][i] = sv[i] + (w->dv[0][i]*1932 - w->dv[1][i]*7200 + w->dv[2][i]*7296) / 2197;
	 }

	//-- get w->dv[8] values from object at current time + 12/13 -//
	// Derive( sizeof(slLinkIntegrationPosition), w->dv[7], w->dv[8], time+timeStep*12/13 );
	simerr += slCalculateDerivs(m, w->dv[7], w->dv[8], w);

	for (i=0; i < sizeof(slLinkIntegrationPosition)/sizeof(double); i++) {
		w->dv[3][i] = timeStep * w->dv[8][i];
		w->dv[7][i] = sv[i] + w->dv[0][i]*439/216 - 8*w->dv[1][i] + w->dv[2][i]*3680/513 - w->dv[3][i]*845/4104;
	}

	//-- get w->dv[8] values from object at current time + -//
	// Derive( sizeof(slLinkIntegrationPosition), w->dv[7], w->dv[8], time+timeStep );
	simerr += slCalculateDerivs(m, w->dv[7], w->dv[8], w);

	for (i=0; i < sizeof(slLinkIntegrationPosition)/sizeof(double); i++) {
		w->dv[4][i] = timeStep * w->dv[8][i];
		w->dv[7][i] = sv[i] - w->dv[0][i]*8/27 + w->dv[1][i]*2 - w->dv[2][i]*3544/2565 + w->dv[3][i]*1859/4104 - w->dv[4][i]*11/40;
	}

	//-- get w->dv[8] values from object at current time + -//
	// Derive(sizeof(slLinkIntegrationPosition), y, w->dv[8], time+timeStep/2);
	// simerr += slCalculateDerivs(m, w->dv[7], w->dv[8], w);
	simerr += slCalculateDerivs(m, w->dv[7], w->dv[8], w);

	for ( i=0; i< sizeof(slLinkIntegrationPosition)/sizeof(double); i++ ) {
		w->dv[5][i] = timeStep * w->dv[8][i];
	}

	//--------------- done with derivs ------------//

	//-- update state variables --//

	for ( i=0; i< sizeof(slLinkIntegrationPosition); i++ ) {
		osv[i] = sv[i] + w->dv[0][i]*25/216 + w->dv[2][i]*1408/2565 + w->dv[3][i]*2197/4104 - w->dv[4][i]*1/5;
	}

	//-- all derivs values calculated, calculate 4th and 5th degree terms --//

	sum = 0.0;
	error = 0.0;

	for ( i=0; i < sizeof(slLinkIntegrationPosition)/sizeof(double); i++ ) {
		sum = fabs ( w->dv[0][i]/360 - w->dv[2][i]*128/4275 - w->dv[3][i]*2197/75204 + w->dv[4][i]/50 +w->dv[5][i]*2/55) ;
		if ( sv[i] != 0.0 ) error = RKF_MAX( error, fabs( sum / sv[ i ]) );
	}

	//-- increment time  --//

	if ( error == 0 ) delta = 5;
	else delta = pow( RKF_TOLERANCE / ( 2*fabs( error ) ),  0.25 );

	//-- don't change the timeStep dramatically because it makes a
	//-- difference by factor of 16
	//-- never increase delta more than 5 --//

	if ( delta > 5 ) delta = 5;
	else if ( delta < 0.1 ) delta = 0.1;

	//-- calculate new timeStep --//  

	timeStep *= delta;

	//-- check the sign --//

	if ( fabs( timeStep ) >= RKF_MAX_TIMESTEP ) timeStep = timeStep < 0 ? -RKF_MAX_TIMESTEP : RKF_MAX_TIMESTEP;

	//-- check singular differential equation case --//

	if ( fabs( timeStep ) < RKF_MIN_TIMESTEP ) timeStep = RKF_MIN_TIMESTEP;
	*deltaT = timeStep;

	return simerr;
}

void slFreeIntegrationVectors(slWorld *w) {
	int n;

	for(n=0;n<DV_VECTOR_COUNT;n++) {
		if(w->dv[n]) delete[] w->dv[n];
		w->dv[n] = NULL;
	}
}

void slAllocIntegrationVectors(slWorld *w) {
	int n;

	for(n=0;n<DV_VECTOR_COUNT;n++) w->dv[n] = new double[sizeof(slLinkIntegrationPosition)];
}

