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

/* 
    = significant speed increases were seen by placing a number 
    = of the most frequently used vector and matrix functions
    = into inline macros.

    = this is now currently the default behavior--if MATRIX_FUNCTIONS
    = is defined, then the function versions will be used instead,
    = but they are outdated and potentially non-functional.
*/

typedef double slMatrix[3][3];

#define slMatrixZero(x)         bzero(x, 9 * sizeof(double))
#define slMatrixIdentity(x)     (bzero(x, 9 * sizeof(double)),(x)[0][0] = (x)[1][1] = (x)[2][2] = 1.0)
#define slMatrixCopy(a, b)      bcopy((a), (b), 9 * sizeof(double))

#define slMatrixMulMatrix(m, n, d) \
    (   (d)[0][0] = (m)[0][0] * (n)[0][0] + (m)[0][1] * (n)[1][0] + (m)[0][2] * (n)[2][0], \
        (d)[0][1] = (m)[0][0] * (n)[0][1] + (m)[0][1] * (n)[1][1] + (m)[0][2] * (n)[2][1], \
        (d)[0][2] = (m)[0][0] * (n)[0][2] + (m)[0][1] * (n)[1][2] + (m)[0][2] * (n)[2][2], \
        (d)[1][0] = (m)[1][0] * (n)[0][0] + (m)[1][1] * (n)[1][0] + (m)[1][2] * (n)[2][0], \
        (d)[1][1] = (m)[1][0] * (n)[0][1] + (m)[1][1] * (n)[1][1] + (m)[1][2] * (n)[2][1], \
        (d)[1][2] = (m)[1][0] * (n)[0][2] + (m)[1][1] * (n)[1][2] + (m)[1][2] * (n)[2][2], \
        (d)[2][0] = (m)[2][0] * (n)[0][0] + (m)[2][1] * (n)[1][0] + (m)[2][2] * (n)[2][0], \
        (d)[2][1] = (m)[2][0] * (n)[0][1] + (m)[2][1] * (n)[1][1] + (m)[2][2] * (n)[2][1], \
        (d)[2][2] = (m)[2][0] * (n)[0][2] + (m)[2][1] * (n)[1][2] + (m)[2][2] * (n)[2][2] )

#define slVectorXform(m, v, d) \
    ((d)->x = (m)[0][0] * (v)->x + (m)[0][1] * (v)->y + (m)[0][2] * (v)->z, \
     (d)->y = (m)[1][0] * (v)->x + (m)[1][1] * (v)->y + (m)[1][2] * (v)->z, \
     (d)->z = (m)[2][0] * (v)->x + (m)[2][1] * (v)->y + (m)[2][2] * (v)->z)

#define slVectorInvXform(m, v, d) \
    ((d)->x = (m)[0][0] * (v)->x + (m)[1][0] * (v)->y + (m)[2][0] * (v)->z, \
     (d)->y = (m)[0][1] * (v)->x + (m)[1][1] * (v)->y + (m)[2][1] * (v)->z, \
     (d)->z = (m)[0][2] * (v)->x + (m)[1][2] * (v)->y + (m)[2][2] * (v)->z)

#define slMatrixAdd(m, n, d) \
    ((d)[0][0] = (m)[0][0] + (n)[0][0], (d)[0][1] = (m)[0][1] + (n)[0][1], (d)[0][2] = (m)[0][2] + (n)[0][2], \
     (d)[1][0] = (m)[1][0] + (n)[1][0], (d)[1][1] = (m)[1][1] + (n)[1][1], (d)[1][2] = (m)[1][2] + (n)[1][2], \
     (d)[2][0] = (m)[2][0] + (n)[2][0], (d)[2][1] = (m)[2][1] + (n)[2][1], (d)[2][2] = (m)[2][2] + (n)[2][2])

#define slMatrixSub(m, n, d) \
    ((d)[0][0] = (m)[0][0] - (n)[0][0], (d)[0][1] = (m)[0][1] - (n)[0][1], (d)[0][2] = (m)[0][2] - (n)[0][2], \
     (d)[1][0] = (m)[1][0] - (n)[1][0], (d)[1][1] = (m)[1][1] - (n)[1][1], (d)[1][2] = (m)[1][2] - (n)[1][2], \
     (d)[2][0] = (m)[2][0] - (n)[2][0], (d)[2][1] = (m)[2][1] - (n)[2][1], (d)[2][2] = (m)[2][2] - (n)[2][2])

void slMatrixMulScalar(double m1[3][3], double n, double d[3][3]);

void slMatrixTranspose(double s[3][3], double d[3][3]);

void slSkewSymmetric(slVector *v, double m[3][3]);

void slDetermanant(double m[3][3]);

int slMatrixInvert(double m[3][3], double d[3][3]);

void slMatrixPrint(double m[3][3]);

void slRotationMatrix(slVector *v, double angle, double m[3][3]);

void slMatrixVectorSet(double m[3][3], slVector *diag, slVector *sym);

int slMatrixCompare(double x[3][3], double y[3][3]);

double slDetermanent(double m[3][3]);

void slMatrixToQuat(double mat[3][3], slQuat *q);
