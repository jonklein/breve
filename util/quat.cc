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
    = quat.c gives utility functions to support quaternions for 
    = storing and manipulating rotations.
*/

#include "util.h"

#include <math.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h> //szymansk Jan 24th 2005

/*!
	\brief Takes a rotation velocity vector and turns it into a quaternion derivative.
*/

slQuat *slAngularVelocityToDeriv(slVector *av, slQuat *rot, slQuat *deriv) {
    deriv->s = ((av->x * -rot->x) + (av->y * -rot->y) + (av->z * -rot->z))/2.0;
    deriv->x = ((av->x *  rot->s) + (av->y * -rot->z) + (av->z *  rot->y))/2.0;
    deriv->y = ((av->x *  rot->z) + (av->y *  rot->s) + (av->z * -rot->x))/2.0;
    deriv->z = ((av->x * -rot->y) + (av->y *  rot->x) + (av->z *  rot->s))/2.0;

    return deriv;
}

/*!
    \brief Takes a normalized rotation quaternion and converts it to a standard rotation matrix.
*/

void slQuatToMatrix(slQuat *q, double m[3][3]) {
    double sx = 2 * q->s * q->x, sy = 2 * q->s * q->y, sz = 2 * q->s * q->z;
    double xx = 2 * q->x * q->x, xy = 2 * q->x * q->y, xz = 2 * q->x * q->z;
    double yy = 2 * q->y * q->y, yz = 2 * q->y * q->z;
    double zz = 2 * q->z * q->z;

    m[0][0] = 1.0 - (yy + zz);
    m[0][1] = (xy - sz);
    m[0][2] = (xz + sy);

    m[1][0] = (xy + sz);
    m[1][1] = 1.0 - (xx + zz);
    m[1][2] = (yz - sx);

    m[2][0] = (xz - sy);
    m[2][1] = (yz + sx);
    m[2][2] = 1.0 - (xx + yy);
}

/*!
    \brief Sets the passed quaternion to the identity rotation.
*/

slQuat *slQuatIdentity(slQuat *q) {
    q->s = 1;

    q->x = 0;
    q->y = 0;
    q->z = 0;

    return q;
}

/*! 
    \brief Normalizes the passed quaternion.
*/

slQuat *slQuatNormalize(slQuat *q) {
    double d;

    d = sqrt((q->s * q->s) + (q->x * q->x) + (q->y * q->y) + (q->z * q->z));

    q->s /= d;
    q->x /= d;
    q->y /= d;
    q->z /= d;

    return q;
}

/*!
    \brief Sets the passed quaternion to a rotation of the passed angle about the passed vector.
*/

slQuat *slQuatSetFromAngle(slQuat *q, double angle, slVector *v) {
    double anglesin;
    slVector axis;

    slVectorCopy(v, &axis);
    slVectorNormalize(&axis);

    anglesin = sin(angle/2.0);

    q->s = cos(angle/2.0);

    q->x = axis.x * anglesin;
    q->y = axis.y * anglesin;
    q->z = axis.z * anglesin;

    return q;
}
