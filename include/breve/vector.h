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

#include <string.h>

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
#define slVectorCopy(a, b) (memmove((b), (a), sizeof(slVector)))
#define slsVectorCopy(a, b) (memmove((b), (a), sizeof(slsVector)))
#define slVectorDot(a, b) (((a)->x * (b)->x) + ((a)->y * (b)->y) + ((a)->z * (b)->z))

#define slsVectorZero(a) (memset((a), 0, sizeof(slsVector)))
#define slVectorZero(a) (memset((a), 0, sizeof(slVector)))

void slVectorCross(slVector *, slVector *, slVector *);
double slVectorAngle(slVector *, slVector *);

void slVectorNormalize(slVector *);

void slVectorSet(slVector *, double, double, double);
void slVectorPrint(slVector *);
int slVectorCompare(slVector *, slVector *);
