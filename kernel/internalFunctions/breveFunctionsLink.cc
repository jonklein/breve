/*****************************************************************************
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2004 Jonathan Klein                                    *
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
	\brief Creates a new slLink.

	slLink pointer linkNew().
*/

int brILinkNew(brEval *args, brEval *target, brInstance *i) {
	slLink *l;

	l = slLinkNew(i->engine->world);

	l->simulate = 0;
	l->callbackData = i;

	BRPOINTER(target) = l;

	return EC_OK;
}

/*!
	\brief Adds a link to the world, and returns a new slWorldObject pointer.

	slWorldObject pointer linkAddToWorld(slLink pointer).
*/

int brILinkAddToWorld(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);
	slWorldObject *wo;

	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to linkAddToWorld\n");
		return EC_ERROR;
	}

	wo = slAddObject(i->engine->world, link, WO_LINK, NULL);
	wo->userData = i;

	BRPOINTER(target) = wo;

	return EC_OK;
}

/*!
	\brief Sets the shape of a link.

	void linkSetShape(slLink pointer, slShape pointer).
*/

int brILinkSetShape(brEval *args, brEval *target, brInstance *i) {
	slLink *l = BRPOINTER(&args[0]);
	slShape *s = BRPOINTER(&args[1]);

	slLinkSetShape(l, s);

	return EC_OK;
}

/*!
	\brief Moves a link to a new location.

	void linkSetLocation(slLink pointer, vector).
*/

int brILinkSetLocation(brEval *args, brEval *target, brInstance *i) {
	slLink *l = BRPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);

	slLinkSetLocation(l, v);

	return EC_OK;
}

/*!
	\brief Sets the rotation of a link to the given matrix.

	void linkSetRotationMatrix(slLink pointer, matrix).
*/

int brILinkSetRotationMatrix(brEval *args, brEval *target, brInstance *i) {
	slLink *l = BRPOINTER(&args[0]);
	double m[3][3];

	slMatrixCopy(BRMATRIX(&args[1]), m);

	slLinkSetRotation(l, m);

	return EC_OK;
}

/*!
	\brief Sets the rotation of a link to an angle about a given matrix.

	void linkSetRotation(slLink pointer, vector, double).
*/

int brILinkSetRotation(brEval *args, brEval *target, brInstance *i) {
	slLink *l = BRPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);
	double len = BRDOUBLE(&args[2]);
	double m[3][3];

	slRotationMatrix(v, len, m);
	slLinkSetRotation(l, m);

	return EC_OK;
}

/*!
	\brief Preforms a relative rotation for a link.

	void linkRotateRelative(slLink pointer, vector, double).
*/

int brILinkRotateRelative(brEval *args, brEval *target, brInstance *i) {
	slLink *l = BRPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);
	double len = BRDOUBLE(&args[2]);
	double m[3][3], nm[3][3];

	slRotationMatrix(v, len, m);
	slMatrixMulMatrix(m, l->position.rotation, nm);

	slLinkSetRotation(l, nm);

	return EC_OK;
}

/*!
	\brief Returns the location of a link.

	vector linkGetLocation(slLink pointer).
*/

int brILinkGetLocation(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]); 

	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to getLocation\n");
		return EC_ERROR;
	}

	slVectorCopy(&link->position.location, &BRVECTOR(target));

	return EC_OK;
}

/*!
	\brief Returns the rotation matrix of a link.

	matrix linkGetRotation(slLink pointer).
*/

int brILinkGetRotation(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]); 

	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to getRotation\n");
		return EC_ERROR;
	}

	slMatrixCopy(&link->position.rotation, BRMATRIX(target));

	return EC_OK;
}

/*!
	\brief Sets the velocity of a link.

	void linkSetVelocity(slLink pointer, vector).
*/

int brILinkSetVelocity(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);
	slVector *vl = &BRVECTOR(&args[1]);

	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to linkSetVelocity\n");
		return EC_ERROR;
	}

	slLinkSetVelocity(link, vl, NULL);

	return EC_OK;
}
/*!
	\brief Turns physical simulation on or off for a body.

	void linkSetPhysics(slLink pointer body, int state).
*/

int brILinkSetPhysics(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);
	
	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to setPhysics\n");
		return EC_ERROR;
	}

	if(BRINT(&args[1])) slLinkEnableSimulation(link);
	else slLinkDisableSimulation(link);
	
	return EC_OK;
}

/*!
	\brief Sets the angular velocity of a link.

	void linkSetRotationalVelocity(slLink pointer, vector).
*/

int brILinkSetRotationalVelocity(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);
	slVector *vl = &BRVECTOR(&args[1]);

	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to linkSetRotationalVelocity\n");
		return EC_ERROR;
	}

	slLinkSetVelocity(link, NULL, vl);

	return EC_OK;
}

/*!
	\brief Sets the linear acceleration of a link.

	void linkSetAcceleration(slLink pointer, vector).
*/

int brILinkSetAcceleration(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);
 
	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to linkSetAcceleration\n");
		return EC_ERROR;
	}
 
	slLinkSetAcceleration(link, v, NULL);
 
	return EC_OK;
}
 
/*!
	\brief Gets the linear acceleration of a link.

	vector linkGetAcceleration(slLink pointer).
*/

int brILinkGetAcceleration(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);

	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to getAcceleration\n");
		return EC_ERROR;
	}

	slVectorCopy(&link->acceleration.b, &BRVECTOR(target));

	return EC_OK;
}   

/*!
	\brief Sets the rotational acceleration of a link.

	void linkSetRotationalAcceleration(slLink pointer, vector).
*/

int brILinkSetRotationalAcceleration(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);
 
	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to setRotationalAcceleration\n");
		return EC_ERROR;
	}
 
	slLinkSetAcceleration(link, NULL, v);
 
	return EC_OK;
}

/*!
	\brief Gets the linear velocity of a link.

	void linkGetVelocity(slLink pointer).
*/

int brILinkGetVelocity(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);

	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to linkGetVelocity\n");
		return EC_ERROR;
	}

	slLinkGetVelocity(link, &BRVECTOR(target), NULL);

	return EC_OK;
}

/*!
	\brief Gets the angular velocity of a link.

	void linkGetVelocity(slLink pointer).
*/

int brILinkGetRotationalVelocity(brEval *args, brEval *target, brInstance *i) {
	slWorldObject *wo = BRPOINTER(&args[0]);
	slLink *link = wo->data;
	
	if(!wo) {
		slMessage(DEBUG_ALL, "null pointer passed to getRotationalVelocity\n");
		return EC_ERROR;
	}

	if(wo->type != WO_LINK) {
		slMessage(DEBUG_ALL, "stationary object passed to getRotationalVelocity\n");
		return EC_ERROR;
	}

	slLinkGetVelocity(link, NULL, &BRVECTOR(target));

	return EC_OK;
}

/*!
	\brief Applies a force vector to a link.

	linkSetForce(slLink pointer, vector).
*/

int brILinkSetForce(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);

	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to setLinkForce\n");
		return EC_ERROR;
	}

	slVectorCopy(&BRVECTOR(&args[1]), &link->externalForce);

	return EC_OK;
}

/*!
	\brief Gets the multibody object associated with the current link.

	object linkGetMultibody(slLink pointer).

	Returns NULL if the link is not connected to a multibody.
*/

int brILinkGetMultibody(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);

	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to setLinkTorque\n");
		return EC_ERROR;
	}

	if(link->mb) BRINSTANCE(target) = link->mb->callbackData;
	else BRINSTANCE(target) = NULL;

	return EC_OK;
}

/*!
	\brief Applies a vector torque to a link.

	void linkSetTorque(slLink pointer, vector).
*/

int brILinkSetTorque(brEval *args, brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);

	if(!link) {
		slMessage(DEBUG_ALL, "null pointer passed to setLinkTorque\n");
		return EC_ERROR;
	}

	slLinkApplyForce(link, NULL, &BRVECTOR(&args[1]));

	return EC_OK;
}

/*!
	\brief Check for any penetrations in a single link.

	int linkCheckPenetration(slLink pointer body).
*/

int brILinkCheckPenetration(brEval args[], brEval *target, brInstance *i) { 
	slLink *l = BRPOINTER(&args[0]);

	if(!l) return EC_OK;

	BRINT(target) = slLinkCheckPenetration(i->engine->world, l);
   
	return EC_OK;
}

/*!
	\brief Check for self-penetrations in a single link.

	int linkCheckSelfPenetration(slLink pointer body).
*/

int brILinkCheckSelfPenetration(brEval args[], brEval *target, brInstance *i) { 
	slLink *l = BRPOINTER(&args[0]);

	if(!l) return EC_OK;

	BRINT(target) = slLinkCheckSelfPenetration(i->engine->world, l);
   
	return EC_OK;
}

/*!
	\brief Returns the maximum x, y and z coordinates of any points on the link.
	In conjunction with \ref brILinkGetMin, can be used to infer the bounding box
	of a link.

	vector linkGetMax(slLink pointer link).
*/

int brILinkGetMax(brEval args[], brEval *target, brInstance *i) {
	slLink *l = BRPOINTER(&args[0]);

	if(!l) {
		slMessage(DEBUG_ALL, "linkGetMax() called with uninitialized link\n");
		return EC_ERROR;
	}

	slVectorCopy(&l->max, &BRVECTOR(target));

	return EC_OK;
}

/*!
	\brief Returns the minimum x, y and z coordinates of any points on the link.
	In conjunction with \ref brILinkGetMax, can be used to infer the bounding box
	of a link.

	vector linkGetMin(slLink pointer link).
*/

int brILinkGetMin(brEval args[], brEval *target, brInstance *i) {
	slLink *l = BRPOINTER(&args[0]);

	if(!l) {
		slMessage(DEBUG_ALL, "linkGetMin() called with uninitialized link\n");
		return EC_ERROR;
	}

	slVectorCopy(&l->min, &BRVECTOR(target));

	return EC_OK;
}

/*!
	\brief Sets the texture of this link.
*/

int brILinkSetTexture(brEval *args, brEval *target, brInstance *i) {
	slLink *m = BRPOINTER(&args[0]);
	int texture = BRINT(&args[1]);

	if(!m) {
		slMessage(DEBUG_ALL, "null pointer passed to setColor\n");
		return EC_ERROR;
	}

	m->texture = texture;

	return EC_OK;
}

/*!
 	\brief Transforms a vector in world coordinates to a vector in the link's own coordinate system.
 
	vector vectorFromLinkPerspective(slMbLink pointer link, vector worldVector).
*/

int brIVectorFromLinkPerspective(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRPOINTER(&args[0]);
	slVector *vector = &BRVECTOR(&args[1]);

	if(!link) {
		slMessage(DEBUG_ALL, "vectorFromLinkPerspective called with NULL pointer\n");
		return EC_ERROR;
	}

	target->type = AT_VECTOR;
	slVectorInvXform(link->position.rotation, vector, &BRVECTOR(target));

	return EC_OK;
} 
/*@}*/

void breveInitLinkFunctions(brNamespace *n) {
	brNewBreveCall(n, "linkSetPhysics", brILinkSetPhysics, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "linkCheckSelfPenetration", brILinkCheckSelfPenetration, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "linkCheckPenetration", brILinkCheckPenetration, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "linkGetMax", brILinkGetMax, AT_VECTOR, AT_POINTER, 0);
	brNewBreveCall(n, "linkGetMin", brILinkGetMin, AT_VECTOR, AT_POINTER, 0);

	brNewBreveCall(n, "linkNew", brILinkNew, AT_POINTER, 0);
	brNewBreveCall(n, "linkSetShape", brILinkSetShape, AT_NULL, AT_POINTER, AT_POINTER, 0);
	brNewBreveCall(n, "linkSetLocation", brILinkSetLocation, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "linkSetRotation", brILinkSetRotation, AT_NULL, AT_POINTER, AT_VECTOR, AT_DOUBLE, 0);
	brNewBreveCall(n, "linkRotateRelative", brILinkRotateRelative, AT_NULL, AT_POINTER, AT_VECTOR, AT_DOUBLE, 0);
	brNewBreveCall(n, "linkSetRotationMatrix", brILinkSetRotationMatrix, AT_NULL, AT_POINTER, AT_MATRIX, 0);
	brNewBreveCall(n, "linkAddToWorld", brILinkAddToWorld, AT_POINTER, AT_POINTER, 0);
	brNewBreveCall(n, "linkSetAcceleration", brILinkSetAcceleration, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "linkSetRotationalAcceleration", brILinkSetRotationalAcceleration, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "linkSetVelocity", brILinkSetVelocity, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "linkSetRotationalVelocity", brILinkSetRotationalVelocity, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "linkSetForce", brILinkSetForce, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "linkSetTorque", brILinkSetTorque, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "linkGetAcceleration", brILinkGetAcceleration, AT_VECTOR, AT_POINTER, 0);
	brNewBreveCall(n, "linkGetRotationalVelocity", brILinkGetRotationalVelocity, AT_VECTOR, AT_POINTER, 0);
	brNewBreveCall(n, "linkGetLocation", brILinkGetLocation, AT_VECTOR, AT_POINTER, 0);
	brNewBreveCall(n, "linkGetRotation", brILinkGetRotation, AT_MATRIX, AT_POINTER, 0);
	brNewBreveCall(n, "linkGetVelocity", brILinkGetVelocity, AT_VECTOR, AT_POINTER, 0);
	brNewBreveCall(n, "linkGetMultibody", brILinkGetMultibody, AT_INSTANCE, AT_POINTER, 0);
	brNewBreveCall(n, "linkSetTexture", brILinkSetTexture, AT_NULL, AT_POINTER, AT_INT, 0);

	brNewBreveCall(n, "vectorFromLinkPerspective", brIVectorFromLinkPerspective, AT_VECTOR, AT_POINTER, AT_VECTOR, 0);
}

