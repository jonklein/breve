#ifndef _SLVECTOR_H
#define _SLVECTOR_H

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

#define MAX(x, y) ((x)>(y)?(x):(y))

/** 
 * \brief a 3x1 vector 
 */
class slVector {
    public:
        double x;
        double y;
        double z;
};

/**
 * \brief a 6x1 spatial vector, frequently manipulated as 2 3x1 vectors.
 * 
 * Used during integration, and to hold linear/angular acceleration or 
 * velocity information.
 */

struct slsVector {
    slVector a;
    slVector b;
};

/**
 * \brief A plane, specified by a normal and a vertex.
 */
struct slPlane {
	slVector normal;
	slVector vertex;
};

#define slVectorLength( V )		(sqrt((V)->x * (V)->x + (V)->y * (V)->y + (V)->z * (V)->z))
#define slVectorIsZero( V )		((V)->x == 0.0 && (V)->y == 0.0 && (V)->z == 0.0)

#define slVectorAdd(a,b,c) ((c)->x=(a)->x+(b)->x,(c)->y=(a)->y+(b)->y,(c)->z=(a)->z+(b)->z)
#define slVectorSub(a,b,c) ((c)->x=(a)->x-(b)->x,(c)->y=(a)->y-(b)->y,(c)->z=(a)->z-(b)->z)
#define slVectorMul(a,b,c) ((c)->x=(a)->x*(b),(c)->y=(a)->y*(b),(c)->z=(a)->z*(b))
#define slVectorCopy(a, b) (memmove((b), (a), sizeof(slVector)))
#define slsVectorCopy(a, b) (memmove((b), (a), sizeof(slsVector)))
#define slVectorDot(a, b) (((a)->x * (b)->x) + ((a)->y * (b)->y) + ((a)->z * (b)->z))

#define slsVectorZero(a) (memset((a), 0, sizeof(slsVector)))
#define slVectorZero(a) (memset((a), 0, sizeof(slVector)))

void slVectorCross( const slVector *,  const slVector *, slVector *);
double slVectorAngle( const slVector *,  const slVector *);

void slVectorNormalize( slVector* ioVector );
float slVectorDist( const slVector *inA, const slVector *inB );

void slVectorSet( slVector*, double, double, double);
void slVectorPrint(const slVector*);
int slVectorCompare(const slVector *, const slVector *);

/**
 * \brief Gives vectors perpendicular to v.
 *
 * Uses cross-products to find two vectors perpendicular to v.
 * Uses either (0, 1, 0) or (1, 0, 0) as the first cross product
 * vector (depending on whether v is already set to one of these).
 */
void slPerpendicularVectors( const slVector *v, slVector *p1, slVector *p2 );

#endif
