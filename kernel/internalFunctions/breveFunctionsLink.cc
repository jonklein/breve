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

#define BRLINKPOINTER(p) ((slLink*)BRPOINTER(p))

/*!
	\brief Creates a new slLink.

	slLink pointer linkNew().
*/

int brILinkNew(brEval args[], brEval *target, brInstance *i) {
	slLink *l;

	l = new slLink(i->engine->world);

	slWorldObjectSetCallbackData(l, i);

	target->set( l );

	return EC_OK;
}

/*!
	\brief Adds a link to the world, and returns a new slWorldObject pointer.

	slWorldObject pointer linkAddToWorld(slLink pointer).
*/

int brILinkAddToWorld(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	slWorldObject *wo;

	wo = slWorldAddObject(i->engine->world, link, WO_LINK);

	target->set( wo );

	return EC_OK;
}

/*!
	\brief Sets the shape of a link.

	void linkSetShape(slLink pointer, slShape pointer).
*/

int brILinkSetShape(brEval args[], brEval *target, brInstance *i) {
	slLink *l = BRLINKPOINTER(&args[0]);
	slShape *s = (slShape*)BRPOINTER(&args[1]);

	l->setShape(s);

	return EC_OK;
}

/*!
	\brief Moves a link to a new location.

	void linkSetLocation(slLink pointer, vector).
*/

int brILinkSetLocation(brEval args[], brEval *target, brInstance *i) {
	slLink *l = BRLINKPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);

	l->setLocation(v);

	return EC_OK;
}

/*!
	\brief Sets the rotation of a link to the given matrix.

	void linkSetRotationMatrix(slLink pointer, matrix).
*/

int brILinkSetRotationMatrix(brEval args[], brEval *target, brInstance *i) {
	slLink *l = BRLINKPOINTER(&args[0]);
	double m[3][3];

	slMatrixCopy(BRMATRIX(&args[1]), m);

	l->setRotation(m);

	return EC_OK;
}

/*!
	\brief Sets the rotation of a link to an angle about a given matrix.

	void linkSetRotation(slLink pointer, vector, double).
*/

int brILinkSetRotation(brEval args[], brEval *target, brInstance *i) {
	slLink *l = BRLINKPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);
	double len = BRDOUBLE(&args[2]);
	double m[3][3];

	slRotationMatrix(v, len, m);
	l->setRotation(m);

	return EC_OK;
}

/*!
	\brief Preforms a relative rotation for a link.

	void linkRotateRelative(slLink pointer, vector, double).
*/

int brILinkRotateRelative(brEval args[], brEval *target, brInstance *i) {
	slLink *l = BRLINKPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);
	double len = BRDOUBLE(&args[2]);
	double m[3][3], nm[3][3];

	slRotationMatrix( v, len, m );
	slMatrixMulMatrix( m, l->getPosition()->rotation, nm );

	l->setRotation( nm );

	return EC_OK;
}

/*!
	\brief Returns the location of a link.

	vector linkGetLocation(slLink pointer).
*/

int brILinkGetLocation(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]); 

	target->set( link->getPosition()->location );

	return EC_OK;
}

/*!
	\brief Returns the rotation matrix of a link.

	matrix linkGetRotation(slLink pointer).
*/

int brILinkGetRotation(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]); 

	target->set( link->getPosition()->rotation );

	return EC_OK;
}

/*!
	\brief Sets the velocity of a link.

	void linkSetVelocity(slLink pointer, vector).
*/

int brILinkSetVelocity(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	slVector *vl = &BRVECTOR(&args[1]);

	link->setVelocity(vl, NULL);

	return EC_OK;
}
/*!
	\brief Turns physical simulation on or off for a body.

	void linkSetPhysics(slLink pointer body, int state).
*/

int brILinkSetPhysics(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	
	if (BRINT(&args[1])) link->enableSimulation();
	else link->disableSimulation();
	
	return EC_OK;
}

/*!
	\brief Sets the angular velocity of a link.

	void linkSetRotationalVelocity(slLink pointer, vector).
*/

int brILinkSetRotationalVelocity(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	slVector *vl = &BRVECTOR(&args[1]);

	link->setVelocity(NULL, vl);

	return EC_OK;
}

/*!
	\brief Sets the linear acceleration of a link.

	void linkSetAcceleration(slLink pointer, vector).
*/

int brILinkSetAcceleration(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);
 
	link->setAcceleration(v, NULL);
 
	return EC_OK;
}
 
/*!
	\brief Gets the linear acceleration of a link.

	vector linkGetAcceleration(slLink pointer).
*/

int brILinkGetAcceleration(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	slVector v;

	link->getAcceleration(&v, NULL);

	target->set( v );

	return EC_OK;
}   

/*!
	\brief Sets the rotational acceleration of a link.

	void linkSetRotationalAcceleration(slLink pointer, vector).
*/

int brILinkSetRotationalAcceleration(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);
 
	link->setAcceleration( NULL, v );
 
	return EC_OK;
}

/*!
	\brief Gets the linear velocity of a link.

	void linkGetVelocity(slLink pointer).
*/

int brILinkGetVelocity(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	slVector v;

	link->getVelocity( &v, NULL );

	target->set( v );

	return EC_OK;
}

/*!
	\brief Gets the angular velocity of a link.

	void linkGetVelocity(slLink pointer).
*/

int brILinkGetRotationalVelocity(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	slVector v;
	
	link->getVelocity( NULL, &v );

	target->set( v );

	return EC_OK;
}

/*!
	\brief Applies a force vector to a link.

	linkSetForce(slLink pointer, vector).
*/

int brILinkSetForce(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);

	link->setForce( &BRVECTOR( &args[1] ) );

	return EC_OK;
}

/*!
	\brief Gets the multibody object associated with the current link.

	object linkGetMultibody(slLink pointer).

	Returns NULL if the link is not connected to a multibody.
*/

int brILinkGetMultibody(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	slMultibody *mb;

	if ((mb = link->getMultibody())) target->set( (brInstance *)mb->getCallbackData() );
	else target->set( (void*)NULL );

	return EC_OK;
}

/*!
	\brief Applies a vector torque to a link.

	void linkSetTorque(slLink pointer, vector).
*/

int brILinkSetTorque(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);

	link->setTorque( &BRVECTOR( &args[1] ) );

	return EC_OK;
}

/*!
	\brief Check for any penetrations in a single link.

	int linkCheckPenetration(slLink pointer body).
*/

int brILinkGetPenetratingObjects(brEval args[], brEval *target, brInstance *i) { 
	slLink *l = BRLINKPOINTER(&args[0]);
	std::vector< void* > penetrations;
	brEvalListHead *head;
	brEval object;
	unsigned int n;

	head = brEvalListNew(); 

	penetrations = l->userDataForPenetratingObjects(i->engine->world);

	for(n = 0; n < penetrations.size(); n++) {
		object.set( (brInstance*)penetrations[ n] );
		brEvalListInsert(head, n, &object);
	}
   
	target->set( head );

	return EC_OK;
}

/*!
	\brief Check for self-penetrations in a single link.

	int linkCheckSelfPenetration(slLink pointer body).
*/

int brILinkCheckSelfPenetration(brEval args[], brEval *target, brInstance *i) { 
	slLink *l = BRLINKPOINTER(&args[0]);

	target->set( l->checkSelfPenetration(i->engine->world) );
   
	return EC_OK;
}

/*!
	\brief Returns the maximum x, y and z coordinates of any points on the link.
	In conjunction with \ref brILinkGetMin, can be used to infer the bounding box
	of a link.

	vector linkGetMax(slLink pointer link).
*/

int brILinkGetMax(brEval args[], brEval *target, brInstance *i) {
	slLink *l = BRLINKPOINTER(&args[0]);

	if(!l) {
		slMessage(DEBUG_ALL, "linkGetMax() called with uninitialized link\n");
		return EC_ERROR;
	}

	l->getBounds(NULL, &BRVECTOR(target));

	return EC_OK;
}

/*!
	\brief Returns the minimum x, y and z coordinates of any points on the link.
	In conjunction with \ref brILinkGetMax, can be used to infer the bounding box
	of a link.

	vector linkGetMin(slLink pointer link).
*/

int brILinkGetMin(brEval args[], brEval *target, brInstance *i) {
	slLink *l = BRLINKPOINTER(&args[0]);

	if(!l) {
		slMessage(DEBUG_ALL, "linkGetMin() called with uninitialized link\n");
		return EC_ERROR;
	}

	l->getBounds(&BRVECTOR(target), NULL);

	return EC_OK;
}

/*!
	\brief Sets the texture of this link.
*/

int brILinkSetTexture(brEval args[], brEval *target, brInstance *i) {
	slLink *m = BRLINKPOINTER(&args[0]);
	int texture = BRINT(&args[1]);

	if(!m) {
		slMessage(DEBUG_ALL, "null pointer passed to setColor\n");
		return EC_ERROR;
	}

	slLinkSetTexture(m, texture);

	return EC_OK;
}

/*!
 	\brief Transforms a vector in world coordinates to a vector in the link's own coordinate system.
 
	vector vectorFromLinkPerspective(slMbLink pointer link, vector worldVector).
*/

int brIVectorFromLinkPerspective(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	slVector *vector = &BRVECTOR(&args[1]);

	if(!link) {
		slMessage(DEBUG_ALL, "vectorFromLinkPerspective called with NULL pointer\n");
		return EC_ERROR;
	}

	slVectorInvXform(link->getPosition()->rotation, vector, &BRVECTOR(target));

	return EC_OK;
} 

/*!
	\brief Sets the text label from a body.

	void setLabel(slWorldObject pointer body, string label).
*/

int brILinkSetLabel(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);
	char *label = BRSTRING(&args[1]);

	if (!link || !label) {
		slMessage(DEBUG_ALL, "NULL pointer passed to linkSetLabel\n");
		return EC_OK;
	}

	link->setLabel(label);

	return EC_OK;
}

/*!
	\brief Removes the text label from a body.

	void removeLabel(slWorldObject pointer body).
*/

int brILinkRemoveLabel(brEval args[], brEval *target, brInstance *i) {
	slLink *link = BRLINKPOINTER(&args[0]);

	if(!link) {
		slMessage(DEBUG_ALL, "NULL pointer passed to linkSetLabel\n");
		return EC_OK;
	}

	link->setLabel("");

	return EC_OK;
}

/*@}*/

void breveInitLinkFunctions(brNamespace *n) {
	brNewBreveCall(n, "linkSetPhysics", brILinkSetPhysics, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "linkCheckSelfPenetration", brILinkCheckSelfPenetration, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "linkGetPenetratingObjects", brILinkGetPenetratingObjects, AT_LIST, AT_POINTER, 0);
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

	brNewBreveCall(n, "linkSetLabel", brILinkSetLabel, AT_NULL, AT_POINTER, AT_STRING, 0);
	brNewBreveCall(n, "linkRemoveLabel", brILinkRemoveLabel, AT_NULL, AT_POINTER, 0);
}
