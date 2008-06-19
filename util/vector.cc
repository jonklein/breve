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

#include <stdio.h>
#include <math.h>

#include "vector.h"

/*!
	\brief computes the cross product of 3x1 vectors s1 and s2, storing the
	output in 3x1 vector d.
*/

void slVectorCross( slVector *s1, slVector *s2, slVector *d ) {
	d->x = ( s1->y * s2->z ) - ( s1->z * s2->y );
	d->y = ( s1->z * s2->x ) - ( s1->x * s2->z );
	d->z = ( s1->x * s2->y ) - ( s1->y * s2->x );
}

/*!
	\brief Normalizes the vector passed.  If the passed vector has length 0,
	the function has no effect.
*/

void slVectorNormalize( slVector *v ) {
	double d;

	d = slVectorLength( v );

	if ( d == 0.0 || d == 1.0 ) return;

	v->x /= d;

	v->y /= d;

	v->z /= d;
}

/*!
	\brief Returns the angle, in radians, between two vectors.
*/

double slVectorAngle( slVector *a, slVector *b ) {
	double dot = slVectorDot( a, b ), ac;

	dot /= ( slVectorLength( a ) * slVectorLength( b ) );

	if( dot > 1.0 ) 
		dot = 1.0;
	else if( dot < -1.0 ) 
		dot = -1.0;

	ac = acos( dot );

	return ac;
}

/*!
	\brief Sets a vector v with values x, y and z.
*/

void slVectorSet( slVector *v, double x, double y, double z ) {
	v->x = x;
	v->y = y;
	v->z = z;
}

/*!
	\brief printfs the x, y and z components of a vector.  Used only for
	development debugging.
*/

void slVectorPrint( slVector *v ) {
	printf( "(%.20f, %.20f, %.20f)\n", v->x, v->y, v->z );
}

/*!
	\brief returns 1 if vectors a and b differ, otherwise 0.
*/

int slVectorCompare( slVector *a, slVector *b ) {
	if ( a->x != b->x ) return 1;

	if ( a->y != b->y ) return 1;

	if ( a->z != b->z ) return 1;

	return 0;
}

float slVectorDist( slVector *inA, slVector *inB ) {
	slVector dest;
	slVectorSub( inA, inB, &dest );
	return slVectorLength( &dest );
}
