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

/*!
    \brief a 3x1 vector
*/
 
struct slVector {
    double x;
    double y;
    double z;
};

/*!
    \brief a 6x1 spatial vector, frequently manipulated as 2 3x1 vectors.

	Used during integration, and to hold linear/angular acceleration or 
	velocity information.
*/

struct slsVector {
    slVector a;
    slVector b;
};

#define slVectorLength(V)		(sqrt((V)->x * (V)->x + (V)->y * (V)->y + (V)->z * (V)->z))
#define slVectorZeroTest(V)		((V)->x == 0.0 && (V)->y == 0.0 && (V)->z == 0.0)

/*
    = VECTOR_FUNCTIONS are no longer supported, so the inline macros below are
    = now the default.
*/

#define slVectorAdd(a,b,c) ((c)->x=(a)->x+(b)->x,(c)->y=(a)->y+(b)->y,(c)->z=(a)->z+(b)->z)
#define slVectorSub(a,b,c) ((c)->x=(a)->x-(b)->x,(c)->y=(a)->y-(b)->y,(c)->z=(a)->z-(b)->z)
#define slVectorMul(a,b,c) ((c)->x=(a)->x*(b),(c)->y=(a)->y*(b),(c)->z=(a)->z*(b))
#define slVectorCopy(a, b) (bcopy((a), (b), sizeof(slVector)))
#define slsVectorCopy(a, b) (bcopy((a), (b), sizeof(slsVector)))
#define slVectorDot(a, b) (((a)->x * (b)->x) + ((a)->y * (b)->y) + ((a)->z * (b)->z))

#define slsVectorZero(a) (bzero((a), sizeof(slsVector)))
#define slVectorZero(a) (bzero((a), sizeof(slVector)))

void slVectorCross(slVector *s1, slVector *s2, slVector *d);
double slVectorAngle(slVector *a, slVector *b);

void slsVectorAdd(slsVector *s1, slsVector *s2, slsVector *d);
void slsVectorSub(slsVector *s1, slsVector *s2, slsVector *d);
void slsVectorMul(slsVector *s1, double n, slsVector *d);
double slsVectorDot(slsVector *s1, slsVector *s2);

void slVectorNormalize(slVector *v);

void slVectorSet(slVector *v, double x, double y, double z);
void slsVectorSet(slsVector *v, double x1, double y1, double z1, double x2, double y2, double z2);

void slVectorPrint(slVector *v);
void slsVectorPrint(slsVector *v);

slVector *slNewVector(double x, double y, double z);

int slVectorCompare(slVector *a, slVector *b);
int slVectorInArray(slVector *v, slVector *a, int n);
