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
    = matrix.c provides matrix math functions that are used in the 
    = simulation environment.  this includes operations on 3x3 matrices
    = for 3D manipulation, and also 6x6 spatial matrices used in the 
    = featherstone algorithm.

    = 3x3 matrices are represented simply as (double [3][3]).

    = the naming convention used here is to begin function names with
    = slMatrix, depending on what kind of data type
    = is processed.  when appropriate, the "operator" functions in
    = this file take input operands and an output location as arguments.
    = for example:
    = slMatrixAdd(a, b, c);    // adds a and b, stores the output in c

    = in MANY cases the correct results will still be obtained when 
    = passing an input argument which is the same as an output 
    = argument, for example: slMatrixAdd(a, b, a).  however,
    = this is greatly frowned upon since it will not always 
    = yield the correct results.  it is marked by the function 
    = when this is strictly forbidden, though it is always discouraged.

    = many of the 3x3 matrix functions are now inline macros defined
    = in the matrix.h header file.
*/

#include "util.h"

/*!
	\brief Compare two 3x3 matrices.

    slMatrixCompare is used for debugging. It returns 1 if the 
    3x3 matrices are the same.
*/

int slMatrixCompare(double x[3][3], double y[3][3]) {
    int n, m;

    for(n=0;n<3;n++) 
        for(m=0;m<3;m++) if(x[m][n] != y[m][n]) return 0;

    return 1;
}

/*!
	\brief Set sets the values of a matrix using a diagonal
    slVector and a symmetric slVector.
*/

void slMatrixVectorSet(double m[3][3], slVector *diag, slVector *sym) {
    m[0][0] = diag->x;
    m[1][1] = diag->y;
    m[2][2] = diag->z;

    m[0][1] = m[1][0] = sym->z;
    m[0][2] = m[2][0] = sym->y;
    m[1][2] = m[2][1] = sym->x;
}

/*!
    \brief slMatrixMulScalar multiplies 3x3 matrix m by scalar n, storing
    the output in 3x3 matrix d.
*/

inline void slMatrixMulScalar(double m[3][3], double n, double d[3][3]) {
    d[0][0] = m[0][0] * n; d[0][1] = m[0][1] * n; d[0][2] = m[0][2] * n;
    d[1][0] = m[1][0] * n; d[1][1] = m[1][1] * n; d[1][2] = m[1][2] * n;
    d[2][0] = m[2][0] * n; d[2][1] = m[2][1] * n; d[2][2] = m[2][2] * n;
}

/*!
    \brief Transposes a 3x3 matrix.

	Transposes s and stores the output in d.  INPUT MATRIX s MAY NOT EQUAL 
	OUTPUT MATRIX d!
*/

inline void slMatrixTranspose(double s[3][3], double d[3][3]) {
    d[0][0] = s[0][0]; d[0][1] = s[1][0]; d[0][2] = s[2][0];    
    d[1][0] = s[0][1]; d[1][1] = s[1][1]; d[1][2] = s[2][1];    
    d[2][0] = s[0][2]; d[2][1] = s[1][2]; d[2][2] = s[2][2];    
}

/*!
	\brief Create a skew-symmetric matrixc from a vector.

    creates a 3x3 matrix m using the skew symmetric operation on vector v.
*/

inline void slSkewSymmetric(slVector *v, double m[3][3]) {
    m[0][0] = 0;     m[0][1] = -v->z; m[0][2] = v->y;
    m[1][0] = v->z;  m[1][1] = 0;     m[1][2] = -v->x;
    m[2][0] = -v->y; m[2][1] = v->x;  m[2][2] = 0;
}

/*!
    \brief Gives the double determanent of a 3x3 matrix.
*/

inline double slDetermanent(double m[3][3]) {
    return 
        (m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])) +
        (m[0][1] * (m[1][2] * m[2][0] - m[1][0] * m[2][2])) +
        (m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]));
}

/*!
    \brief Inverts a 3x3 matrix m.

	Stores the output in 3x3 matrix d, using the slDetermanent function above.
    INPUT MATRIX m MAY NOT EQUAL OUTPUT MATRIX d!
*/

inline int slMatrixInvert(double m[3][3], double d[3][3]) {
    double det = slDetermanent(m);
 
    if(fabs(det) < 10e-22) return -1;

    d[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) / det;
    d[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) / det;
    d[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) / det;

    d[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) / det;
    d[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) / det;
    d[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) / det;

    d[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) / det;
    d[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) / det;
    d[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) / det;

    return 0;
}

/*!
    \brief Prints out a matrix to stdout.  

	For debugging.
*/

void slMatrixPrint(double m[3][3]) {
    printf("%.15f\t%.15f\t%.15f\n", m[0][0], m[0][1], m[0][2]);
    printf("%.15f\t%.15f\t%.15f\n", m[1][0], m[1][1], m[1][2]);
    printf("%.15f\t%.15f\t%.15f\n", m[2][0], m[2][1], m[2][2]);
    printf("\n");
}

/*!
    produces the 3x3 rotation matrix m for a rotation about (normalized) slVector 
	axis v or angle radians.
*/

void slRotationMatrix(slVector *v, double angle, double m[3][3]) {
    double c, s, mc;

    double xy, xz, yz;
    double xs, ys, zs;

    c = cos(angle);
    s = sin(angle);
    mc = 1 - c;

    xy = v->x * v->y * mc;
    xz = v->x * v->z * mc;
    yz = v->y * v->z * mc;

    xs = v->x * s;
    ys = v->y * s;
    zs = v->z * s;

    m[0][0] = (v->x * v->x)*mc + c;
    m[1][0] = xy + zs;
    m[2][0] = xz - ys;

    m[0][1] = xy - zs;
    m[1][1] = (v->y * v->y)*mc + c;
    m[2][1] = yz + xs;

    m[0][2] = xz + ys;
    m[1][2] = yz - xs;
    m[2][2] = (v->z * v->z)*mc + c;
}

/*!
	\brief Gives a rotation quat for a matrix.
*/

void slMatrixToQuat(double mat[3][3], slQuat *q) {
	int largest;

	double T = mat[0][0] + mat[1][1] + mat[2][2] + 1;
	double S;

	if(T > 0.0) {
		S = 0.5 / sqrt(T);

		q->s = 0.25 / S;

		q->x = ( mat[2][1] - mat[1][2] ) * S;
		q->y = ( mat[0][2] - mat[2][0] ) * S;
		q->z = ( mat[1][0] - mat[0][1] ) * S;

		return;
	}

	if(mat[1][1] > mat[0][0]) {
		if(mat[2][2] > mat[1][1]) {
			/* 2, 2 is the largest */
			largest = 2;
		} else {
			/* 1, 1 is the largest */
			largest = 1;
		}
	} else if(mat[2][2] > mat[0][0]) {
		/* 2, 2 is the largest */
		largest = 2;
	} else {
		/* 0, 0 is the largest */
		largest = 0;
	}

	switch(largest) {
		case 0:
			S = sqrt((mat[0][0] - (mat[1][1] + mat[2][2])) + 1);
			q->x = 0.5 * S;
			S = 0.5 / S;
			q->y = (mat[0][1] + mat[1][0]) * S;
			q->z = (mat[2][0] + mat[0][2]) * S;
			q->s = (mat[2][1] - mat[1][2]) * S;
			return;
		case 1:
			S = sqrt((mat[1][1] - (mat[2][2] + mat[0][0])) + 1);
			q->y = 0.5 * S;
			S = 0.5 / S;
			q->x = (mat[1][2] + mat[2][1]) * S;
			q->z = (mat[0][1] + mat[1][0]) * S;
			q->s = (mat[0][2] - mat[2][0]) * S;
			return;
		case 2:
			S = sqrt((mat[2][2] - (mat[0][0] + mat[1][1])) + 1);
			q->z = 0.5 * S;
			S = 0.5 / S;
			q->x = (mat[2][0] + mat[0][2]) * S;
			q->y = (mat[1][2] + mat[2][1]) * S;
			q->s = (mat[1][0] - mat[0][1]) * S;
			return;
		break;
	}
}
