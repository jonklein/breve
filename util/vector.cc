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

#include "util.h"

/*
	= vector.c provides vector math functions for simple 3x1 vectors
	= as well as 6x1 "spatial" vectors used by the physical simulation
	= algorithms.

	= vector related operations which also require the use of vectors
	= are included in the file matrix.c.  this includes vector 
	= transformations.

	= the naming convention used here is to begin function names with
	= slVector or slsVector, depending on what kind of data type
	= is processed.  when appropriate, the "operator" functions in
	= this file take input operands and an output location as arguments.
	= for example:
	= slVectorAdd(a, b, c);	// adds a and b, stores the output in c

	= as with the matrix operations, many of the vector functions have
	= been replaced by inline macros for the sake of preformance.  
*/

/*
	= slVectorCross computes the cross product of 3x1 vectors 
	= s1 and s2, storing the output in 3x1 vector d.
*/

inline void slVectorCross(slVector *s1, slVector *s2, slVector *d) {
	d->x = (s1->y * s2->z) - (s1->z * s2->y);
	d->y = (s1->z * s2->x) - (s1->x * s2->z);
	d->z = (s1->x * s2->y) - (s1->y * s2->x);
}

/*
	= slVectorNormalize normalizes the vector passed.  if the 
	= passed vector has length 0, the function has no effect.
*/

void slVectorNormalize(slVector *v) {
	double d;
	
	d = slVectorLength(v);
	if(d == 0.0 || d == 1.0) return;

	v->x /= d;
	v->y /= d;	
	v->z /= d;
}

/* 
	= slsVectorAngle returns the angle, in radians, between two vectors.
*/

double slVectorAngle(slVector *a, slVector *b) {
	double dot = slVectorDot(a, b), ac;

	dot /= (slVectorLength(a) * slVectorLength(b));
	ac = acos(dot);

	return ac;
}

/*
	= slVectorZero sets all elements of a 3x1 vector to 0.0.
*/

inline void slVectorZero(slVector *v) {
	bzero(v, sizeof(slVector));
}

/*
	= slsVectorZero sets all elements of a 6x1 vector to 0.0.
*/

inline void slsVectorZero(slsVector *s) {
	bzero(s, sizeof(slsVector));
}

/*
	= slsVectorAdd adds 6x1 vectors s1 and s2, storing the output in 
	= 6x1 vector d.
*/

inline void slsVectorAdd(slsVector *s1, slsVector *s2, slsVector *d) {
	d->a.x = s1->a.x + s2->a.x;
	d->a.y = s1->a.y + s2->a.y;
	d->a.z = s1->a.z + s2->a.z;
	d->b.x = s1->b.x + s2->b.x;
	d->b.y = s1->b.y + s2->b.y;
	d->b.z = s1->b.z + s2->b.z;
}

/*
	= slsVectorAdd subtracts 6x1 vectors s2 from s2, storing the output in 
	= 6x1 vector d.
*/

inline void slsVectorSub(slsVector *s1, slsVector *s2, slsVector *d) {
   d->a.x = s1->a.x - s2->a.x;
   d->a.y = s1->a.y - s2->a.y;
   d->a.z = s1->a.z - s2->a.z;
   d->b.x = s1->b.x - s2->b.x;
   d->b.y = s1->b.y - s2->b.y;
   d->b.z = s1->b.z - s2->b.z;
}

/*
	= slsVectorMul multiplies a 6x1 vector s1 by a scalar n,
	= storing the result in 6x1 vector d.
*/

inline void slsVectorMul(slsVector *s1, double n, slsVector *d) {
	d->a.x = s1->a.x * n;
	d->a.y = s1->a.y * n;
	d->a.z = s1->a.z * n;
	d->b.x = s1->b.x * n;
	d->b.y = s1->b.y * n;
	d->b.z = s1->b.z * n;
}

/*
	= slsVectorDot returns the "spatial dot product" for two 6x1 spatial 
	= vectors.
	=
	= NOTE!  the spatial dot product of a 6x1 vector is not what you
	= might expect from 3x1 dot products.  for two 6x1 spatial vectors
	= [a1 b1] . [a2 b2], the spatial dot product is *NOT* 
	= (a1 . a2) + (b1 . b2), but instead (b1 . a2) + (a1 . b2).
	=
	= as you can guess from the urgency of that comment, i got pretty
	= badly burned on that point.
*/

inline double slsVectorDot(slsVector *s1, slsVector *s2) {
	return (s1->b.x * s2->a.x) + (s1->b.y * s2->a.y) + (s1->b.z * s2->a.z) +
		(s1->a.x * s2->b.x) + (s1->a.y * s2->b.y) + (s1->a.z * s2->b.z);
}

/*
	= slVectorSet sets a vector v with values x, y and z.
*/

void slVectorSet(slVector *v, double x, double y, double z) {
	v->x = x;
	v->y = y;
	v->z = z;
}

/*
	= slsVectorSet sets a spatial vector v with the six
	= values given.
*/

void slsVectorSet(slsVector *v, double x1, double y1, double z1, double x2, double y2, double z2) {
	v->a.x = x1;
	v->a.y = y1;
	v->a.z = z1;

	v->b.x = x2;
	v->b.y = y2;
	v->b.z = z2;
}

/*
	= slNewVector allocates (using slMalloc) a new vector and 
	= sets it to (x, y, z).
*/

slVector *slNewVector(double x, double y, double z) {
	slVector *v;
	
	v = slMalloc(sizeof(slVector));
	v->x = x; 
	v->y = y;
	v->z = z;
	
	return v;
}

/*
	= slVectorPrint printfs the x, y and z components of a vector.
	= used only for development debugging.
*/

void slVectorPrint(slVector *v) {
	printf("(%.20f, %.20f, %.20f)\n", v->x, v->y, v->z);
}

/*
	= slsVectorPrint prints a spatial vector by printing 
	= its two slVector components.
*/

void slsVectorPrint(slsVector *v) {
	slVectorPrint(&v->a);
	slVectorPrint(&v->b);
}

/*
	= slVectorCompare returns 1 if vectors a and b differ, otherwise 0.
*/

int slVectorCompare(slVector *a, slVector *b) {
	if(a->x != b->x) return 1;
	if(a->y != b->y) return 1;
	if(a->z != b->z) return 1;

	return 0;
}

/*
	= slVectorInArray returns 1 if vector v appears anywhere in the 
	= array of vectors a (which is n elements long)
*/

int slVectorInArray(slVector *v, slVector *a, int n) {
	int c;

	for(c=0;c<n;c++) if(!slVectorCompare(v, &a[c])) return 1;

	return 0;
}
