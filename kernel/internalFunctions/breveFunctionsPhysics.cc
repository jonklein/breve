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

#include "kernel.h"

/*! \addtogroup InternalFunctions */
/*@{*/

/*!
	\brief Set the acceleration of the world's gravity.

	void setGravity(vector acceleration).
*/

int brIWorldSetGravity(brEval args[], brEval *target, brInstance *i) {
	slVector *v = &BRVECTOR(&args[0]);
 
	slWorldSetGravity(i->engine->world, v);
	
	return EC_OK;
}   

/*!
	\brief Generate a rotation matrix from a vector axis and a rotation angle
	about that axis.

	matrix rotationMatrix(vector axis, double rotation).
*/

int brIRotationMatrix(brEval args[], brEval *target, brInstance *i) {
	slVector *vector = &BRVECTOR(&args[0]);
	double rotation = BRDOUBLE(&args[1]);
	slMatrix m;

	slRotationMatrix( vector, rotation, m );
	
	target->set( m );

	return EC_OK;
}

/*!
	\brief Turn collision resolution on or off.

	void worldSetCollisionResolution(int state).
*/

int brIWorldSetCollisionResolution(brEval args[], brEval *target, brInstance *i) {
	slWorldSetCollisionResolution(i->engine->world, BRINT(&args[0]));
	
	return EC_OK;
}

/*@}*/

void breveInitPhysicsFunctions(brNamespace *n) {
	brNewBreveCall(n, "worldSetCollisionResolution", brIWorldSetCollisionResolution, AT_NULL, AT_INT, 0);
	brNewBreveCall(n, "worldSetGravity", brIWorldSetGravity, AT_NULL, AT_VECTOR, 0);
	brNewBreveCall(n, "rotationMatrix", brIRotationMatrix, AT_MATRIX, AT_VECTOR, AT_DOUBLE, 0);
}
