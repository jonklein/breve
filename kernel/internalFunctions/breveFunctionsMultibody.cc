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

/*! \addtogroup InternalFunctions */
/*@{*/

#include "kernel.h"

#define BRMULTIBODYPOINTER(p) ((slMultibody*)BRPOINTER(p))

/*!
	\brief Sets the root of the multibody.

	void mutibodySetRoot(slMultiBody pointer, slLink pointer).

	Sets the root of the multibody to the given link.
*/

int brISetMultibodyRoot(brEval args[], brEval *target, brInstance *i) {
	slMultibody *mb = BRMULTIBODYPOINTER(&args[0]);
	slLink *root = BRPOINTER(&args[1]);

	slMultibodySetRoot(mb, root);

	return EC_OK;
}

/*!
	\brief Creates a new multibody.

	void multibodyNew().

	Creates a new, unattached multibody.
*/

int brIMultibodyNew(brEval args[], brEval *target, brInstance *i) {
	slMultibody *mb;

	mb = slMultibodyNew(i->engine->world);
	slMultibodySetCallbackData(mb, i);

	BRMULTIBODYPOINTER(target) = mb;

	return EC_OK;
}

/*!
	\brief Returns a list of all objects in a multibody.

	void multibodyAllObjects(slMultibody pointer).

	Returns a list of all links and joints associated with a multibody.
*/

int brIMultibodyAllObjects(brEval args[], brEval *target, brInstance *i) {
	slList *l, *start;
	brEvalListHead *all;

	all = brEvalListNew();

	start = l = slMultibodyAllCallbackData(BRMULTIBODYPOINTER(&args[0]));

	while(l) {
		brEval e;

		e.type = AT_INSTANCE;
		BRINSTANCE(&e) = l->data;

		if(l->data) brEvalListInsert(all, 0, &e);
		l = l->next;
	}

	slListFree(start);

	BRLIST(target) = all;

	return EC_OK;
}

/*!
	\brief Destroys a multibody.

	void multibodyFree(slMultibody).

	Destroys a multibody, but leaves all connected objects intact.
*/

int brIMultibodyFree(brEval args[], brEval *target, brInstance *i) {
	slMultibody *m = BRMULTIBODYPOINTER(&args[0]);

	slWorldSetUninitialized(i->engine->world);

	if(m) slMultibodyFree(m);

	return EC_OK;
}

/*!
	\brief Sets the location of an entire multibody.

	void multibodySetLocation(slMultibody pointer, vector).
*/

int brIMultibodySetLocation(brEval args[], brEval *target, brInstance *i) {
	slMultibody *mb = BRMULTIBODYPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);

	slMultibodyPosition(mb, v, NULL);

	return EC_OK;
}

/*!
	\brief Sets the rotation of an entire multibody.

	void multibodySetLocation(slMultibody pointer, vector, double).

	Rotates about the given vector by the given double amount.  Rotation occurs 
	about the centerpoint of the root object.
*/

int brIMultibodySetRotation(brEval args[], brEval *target, brInstance *i) {
	slMultibody *mb = BRMULTIBODYPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);
	double len = BRDOUBLE(&args[2]);
	double rot[3][3];

	slRotationMatrix(v, len, rot);

	slMultibodyPosition(mb, NULL, rot);

	return EC_OK;
}

int brIMultibodySetRotationMatrix(brEval args[], brEval *target, brInstance *i) {
	slMultibody *mb = BRMULTIBODYPOINTER(&args[0]);

	slMultibodyPosition(mb, NULL, BRMATRIX(&args[1]));

	return EC_OK;
}
/*!
	\brief Does a relative rotation on a multibody.

	void multibodyRotateRelative(slMultibody pointer, vector, double).

	Rotates a multibody around a given vector by a given double amount.
*/

int brIMultibodyRotateRelative(brEval args[], brEval *target, brInstance *i) {
	slMultibody *mb = BRMULTIBODYPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);
	double len = BRDOUBLE(&args[2]);
	double rotation[3][3];

	if(!mb) {
		slMessage(DEBUG_ALL, "null pointer passed to multibodyRotateRelative\n");
		return EC_ERROR;
	}

	slRotationMatrix(v, len, rotation);
	slMultibodyRotate(mb, rotation);

	return EC_OK;
}

/*!
	\brief Turn self-collision handling on or off for a body.

	void multibodyHandleSelfCollisions(slWorldObject pointer body, int state).
*/

int brIMultibodySetHandleSelfCollisions(brEval args[], brEval *target, brInstance *i) { 
	slMultibody *m = BRMULTIBODYPOINTER(&args[0]);

	if(!m) return EC_OK;

	slMultibodySetHandleSelfCollisions(m, BRINT(&args[1]));
   
	return EC_OK;
}

/*!
	\brief Check for self-penetrations in a body.

	int multibodyCheckSelfPenetration(slWorldObject pointer body).
*/

int brIMultibodyCheckSelfPenetration(brEval args[], brEval *target, brInstance *i) { 
	slMultibody *m = BRMULTIBODYPOINTER(&args[0]);

	if(!m) return EC_OK;

	BRINT(target) = slMultibodyCheckSelfPenetration(i->engine->world, m);
   
	return EC_OK;
}

/*@}*/

void breveInitMultibodyFunctions(brNamespace *n) {
	brNewBreveCall(n, "multibodyRotateRelative", brIMultibodyRotateRelative, AT_NULL, AT_POINTER, AT_VECTOR, AT_DOUBLE, 0);
	brNewBreveCall(n, "multibodySetLocation", brIMultibodySetLocation, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "multibodySetRotation", brIMultibodySetRotation, AT_NULL, AT_POINTER, AT_VECTOR, AT_DOUBLE, 0);
	brNewBreveCall(n, "multibodySetRotationMatrix", brIMultibodySetRotationMatrix, AT_NULL, AT_POINTER, AT_MATRIX, 0);
	brNewBreveCall(n, "multibodyNew", brIMultibodyNew, AT_POINTER, 0);
	brNewBreveCall(n, "multibodySetRoot", brISetMultibodyRoot, AT_NULL, AT_POINTER, AT_POINTER, 0);
	brNewBreveCall(n, "multibodyAllObjects", brIMultibodyAllObjects, AT_LIST, AT_POINTER, 0);
	brNewBreveCall(n, "multibodyFree", brIMultibodyFree, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "multibodySetHandleSelfCollisions", brIMultibodySetHandleSelfCollisions, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "multibodyCheckSelfPenetration", brIMultibodyCheckSelfPenetration, AT_INT, AT_POINTER, 0);
}
