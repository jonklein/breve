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
	The multibody is a logical set of links.  It's a slightly different
	paradigm from the independant body/joint system that ODE uses.  If 
	you move/rotate the multibody, for example, all of the links are
	moved/rotated with it.  
	
	The only real rule is that there is exactly one "root" link chosen
	for each multibody.  
*/

#include "simulation.h"

/*!
	\brief Ignores collisions between adjacent multibody links.
*/

void slMultibodyInitCollisionFlags(slMultibody *m, std::vector<slPairEntry*> &pe) {
	slLink *link1, *link2;
	slPairEntry *e;
	std::vector<slLink*>::iterator i1;
	std::vector<slLink*>::iterator i2;

	for(i1 = m->links.begin(); i1 != m->links.end(); i1++) {
		link1 = *i1;

		// set all check flags on or off, depending on whether 
		// self collisions are handled or not.

		for(i2 = m->links.begin(); i2 != m->links.end(); i2++) {
			link2 = *i2;

			if(link1 != link2) {
				e = slVclipPairEntry(pe, link1->clipNumber, link2->clipNumber);

				if(m->handleSelfCollisions) e->flags |= BT_CHECK;
				else if(e->flags & BT_CHECK) e->flags ^= BT_CHECK;
			}
		}
	}

	for(i1 = m->links.begin(); i1 != m->links.end(); i1++) {
		link1 = *i1;

		// then prune out all adjacent pairs.

		std::vector<slJoint*>::iterator ji;

		for(ji = link1->outJoints.begin(); ji != link1->outJoints.end(); ji++) {
			link2 = (*ji)->child;

			if(link1 != link2) {
				e = slVclipPairEntry(pe, link1->clipNumber, link2->clipNumber);

				if(e->flags & BT_CHECK) e->flags ^= BT_CHECK;
			}
		}
	}
}

/*!
	\brief Sets the location and/or rotation of a multibody.  

	The location and/or rotation may be NULL.
*/

void slMultibodyPosition(slMultibody *m, slVector *location, double rot[3][3]) {
	slVector offset;

	if(!m->root) return;

	slMultibodyUpdatePositions(m);

	if(location) {
		slVectorSub(location, &m->root->position.location, &offset);
		slMultibodyOffsetPosition(m, &offset);
	}

	if(rot) {
		double invR[3][3], transform[3][3];

		/* figure out the relative rotation required to take us from the 
		 * current rotation to the new one */

		slMatrixTranspose(m->root->position.rotation, invR);
		slMatrixMulMatrix(rot, invR, transform);

		slMultibodyRotate(m, transform);
	}

	slMultibodyUpdatePositions(m);
}

/*!
	\brief Gives a rotation matrix from an axis and an angle.
*/

void slMultibodyRotAngleToMatrix(slVector *axis, double r, double rot[3][3]) {
	slQuat q;
	slQuatSetFromAngle(&q, r, axis);
	slQuatToMatrix(&q, rot);
}

/*!
	\brief Preforms a relative rotation.
*/

void slMultibodyRotate(slMultibody *mb, double rotation[3][3]) {
	slLink *link;
	slVector toLink, newToLink;
	double newRot[3][3];
	dQuaternion Q;
	slQuat q;
	std::vector<slLink*>::iterator i1;

	for(i1 = mb->links.begin(); i1 != mb->links.end(); i1++) {	
		link = *i1;

		// first, since the whole body is making this rotation, 
		// our location will be changed.  we have to compute 
		// the new position based on the rotation angle and our
		// location relative to the root link.

		slVectorSub(&link->position.location, &mb->root->position.location, &toLink);
		slVectorXform(rotation, &toLink, &newToLink);
		slVectorAdd(&newToLink, &mb->root->position.location, &newToLink);
		dBodySetPosition(link->odeBodyID, newToLink.x, newToLink.y, newToLink.z);
	
		slMatrixMulMatrix(rotation, link->position.rotation, newRot);

		slMatrixCopy(newRot, link->position.rotation);

		// update the ODE state

		slMatrixToQuat(newRot, &q);

		Q[0] = q.s;
		Q[1] = q.x;
		Q[2] = q.y;
		Q[3] = q.z;

		if(link->simulate) dBodySetQuaternion(link->odeBodyID, Q);
	}
}

/*!
	\brief Offsets the position of a multibody.
*/

void slMultibodyOffsetPosition(slMultibody *mb, slVector *offset) {
	slLink *link;
	const dReal *oldP;
	dReal newP[3];
	std::vector<slLink*>::iterator i1;

	for(i1 = mb->links.begin(); i1 != mb->links.end(); i1++) {	
		link = *i1;

		slVectorAdd(&link->position.location, offset, &link->position.location);

		if(link->simulate) {
			oldP = dBodyGetPosition(link->odeBodyID);

			newP[0] = oldP[0] + offset->x;
			newP[1] = oldP[1] + offset->y;
			newP[2] = oldP[2] + offset->z;
			
			dBodySetPosition(link->odeBodyID, newP[0], newP[1], newP[2]);
		}
	}
}

/*!
	\brief Sets the linear and/or rotational velocities of a link.
*/

void slMultibodySetVelocity(slMultibody *mb, slVector *linear, slVector *rotational) {
	slLink *link;
	std::vector<slLink*>::iterator i;

	for(i=mb->links.begin(); i != mb->links.end(); i++) {
		link = *i;

		slLinkSetVelocity(link, linear, rotational);
	}
}

/*!
	\brief Sets the linear and/or rotational accelerations of a link.
*/

void slMultibodySetAcceleration(slMultibody *mb, slVector *linear, slVector *rotational) {
	slLink *link;
	std::vector<slLink*>::iterator i;

	for(i=mb->links.begin(); i != mb->links.end(); i++) {
		link = *i;

		slLinkSetAcceleration(link, linear, rotational);
	}
}


/*!
	\brief Frees a multibody, but does not free its links.

	The links will continue to exist independent of the multibody
	and must be freed seperately.
*/

void slMultibodyFree(slMultibody *m) {
	slNullOrphanMultibodies(m->root);
	m->links.empty();

	delete m;
}

/*!
	\brief Returns all of the callback data for links and joints in a multibody.
*/

slList *slMultibodyAllCallbackData(slMultibody *mb) {
	slList *list = NULL;
	std::vector<slLink*>::iterator li;
	std::vector<slJoint*>::iterator ji;

	for( li = mb->links.begin(); li != mb->links.end(); li++ ) {
		list = slListPrepend(list, (*li)->userData);

		for(ji = (*li)->inJoints.begin(); ji != (*li)->inJoints.end(); ji++ ) {
			list = slListPrepend(list, (*ji)->userData);
		}
	}

	return list;
}

/*!
	\brief NULL out the multibody fields of orphaned link subtrees.
*/

void slNullOrphanMultibodies(slLink *orphan) {
	std::vector<slLink*> links;
	std::vector<slLink*>::iterator li;

	slLinkList(orphan, &links, 0);
	
	for(li = links.begin(); li != links.end(); li++) (*li)->multibody = NULL;
}

/*!
	\brief Find a multibody that a link is attached to.

	This function is only used when a link must recompute its own 
	multibody, such as after a joint break.
*/

slMultibody *slLinkFindMultibody(slLink *root) {
	slJoint *joint;
	std::vector<slJoint*>::iterator ji;
	std::vector<slLink*> links;
	std::vector<slLink*>::iterator li;

	slLinkList(root, &links, 0);

	for(li = links.begin(); li != links.end(); li++ ) {
		slLink *link = *li;

		if(link != root && root->multibody != link->multibody) {
			/* okay!  link has another mb for us!  we need to find the joint */
			/* connecting this new multibody to our old one! */

			for(ji = root->inJoints.begin(); ji != root->inJoints.end(); ji++ ) {
				joint = *ji;

				if(joint->parent && joint->parent->multibody == root->multibody) {
					joint->isMbJoint = 1;
					return link->multibody;
				}
			}

			for(ji = root->outJoints.begin(); ji != root->outJoints.end(); ji++ ) {
				joint = *ji;

				if(joint->child->multibody == root->multibody) {
					joint->isMbJoint = 1;
					return link->multibody;
				}
			}
		}
	}

	return NULL;
}

/*!
	\brief Returns a list of links connected to a root link.
*/

void slLinkList(slLink *root, std::vector<slLink*> *list, int mbOnly) {
	std::vector<slJoint*>::iterator ji;

	if(!root || std::find(list->begin(), list->end(), root) != list->end()) return;

	list->push_back(root);

	for(ji = root->outJoints.begin(); ji != root->outJoints.end(); ji++ ) {
		if(!mbOnly || (*ji)->isMbJoint) 
			slLinkList((*ji)->child, list, mbOnly);
	}

	for(ji = root->inJoints.begin(); ji != root->inJoints.end(); ji++ ) {
		if(!mbOnly || (*ji)->isMbJoint) 
			slLinkList((*ji)->parent, list, mbOnly);
	}
}

/*!
	\brief Counts multibody links and sets the links' mb fields.
*/

int slMultibodyCountLinks(slMultibody *mb) {
	int number = 0;
	std::vector<slLink*>::iterator i;

	for( i = mb->links.begin(); i != mb->links.end(); i++ ) {	
		slLink *link = *i;

		link->multibody = mb;
		number++;
	}

	return number;
}

/*!
	\brief Computes the total mass of a multibody.
*/

double slMultibodyComputeMass(slMultibody *mb) {
	slLink *link;
	double mass = 0.0;
	std::vector<slLink*>::iterator i;

	for(i = mb->links.begin(); i != mb->links.end(); i++ ) {
		link = *i;

		mass += link->shape->mass;
	}

	return mass;
}

/*!
	\brief Sets the normal vector of a prismatic or revolute joint.
*/

int slJointSetNormal(slJoint *joint, slVector *normal) {
	if(joint->type == JT_REVOLUTE) {
		dJointSetHingeAxis(joint->odeJointID, normal->x, normal->y, normal->z);
	} else if(joint->type == JT_PRISMATIC) {
		dJointSetSliderAxis(joint->odeJointID, normal->x, normal->y, normal->z);
	}	

	return 0;
}

/*!
	\brief Modifies the link points of a joint.
*/

int slJointSetLinkPoints(slJoint *joint, slVector *plinkPoint, slVector *clinkPoint, double rotation[3][3]) {
	const double *childR;
	dReal idealR[16];
	dReal savedChildR[16];
	slVector hingePosition, childPosition;
	double ideal[3][3];

	childR = dBodyGetRotation(joint->child->odeBodyID);
	memcpy(savedChildR, childR, sizeof(savedChildR));

	if (joint->parent)
	   slMatrixMulMatrix(joint->parent->position.rotation, rotation, ideal);
	else
	   slMatrixCopy(rotation, ideal);		

	slSlToODEMatrix(ideal, idealR);

	/* compute the hinge position--the plinkPoint in world coordinates */

	if(joint->parent) {
		slVectorXform(joint->parent->position.rotation, plinkPoint, &hingePosition);
		slVectorAdd(&hingePosition, &joint->parent->position.location, &hingePosition);
	} else {
		slVectorCopy(plinkPoint, &hingePosition);
	}

	/* set the ideal positions, so that the anchor command registers the native position */

	slVectorXform(ideal, clinkPoint, &childPosition);
	slVectorSub(&hingePosition, &childPosition, &childPosition);

	dJointAttach(joint->odeJointID, NULL, NULL);

	dBodySetRotation(joint->child->odeBodyID, idealR);
	dBodySetPosition(joint->child->odeBodyID, childPosition.x, childPosition.y, childPosition.z);

	if(joint->parent) dJointAttach(joint->odeJointID, joint->parent->odeBodyID, joint->child->odeBodyID);
	else dJointAttach(joint->odeJointID, NULL, joint->child->odeBodyID);

	switch(joint->type) {
		case JT_REVOLUTE:
			dJointSetHingeAnchor(joint->odeJointID, hingePosition.x, hingePosition.y, hingePosition.z);
			break;
		case JT_FIX:
			dJointSetFixed(joint->odeJointID);
			break;
		case JT_UNIVERSAL:
			dJointSetUniversalAnchor(joint->odeJointID, hingePosition.x, hingePosition.y, hingePosition.z);
			break;
		case JT_BALL:
			dJointSetBallAnchor(joint->odeJointID, hingePosition.x, hingePosition.y, hingePosition.z);
			break;
		default:
			break;
	}

	/* set the proper positions where the link should actually be at this time */

	slVectorXform(joint->child->position.rotation, clinkPoint, &childPosition);
	slVectorSub(&hingePosition, &childPosition, &childPosition);

	dBodySetRotation(joint->child->odeBodyID, savedChildR);
	dBodySetPosition(joint->child->odeBodyID, childPosition.x, childPosition.y, childPosition.z);
	slVectorCopy(&childPosition, &joint->child->position.location);

	return 0;
}

/*!
	\brief Creates an empty new multibody struct.
*/

slMultibody *slMultibodyNew(slWorld *w) {
	slMultibody *m;

	m = new slMultibody;

	m->world = w;
	m->handleSelfCollisions = 0;

	m->root = NULL;

	return m;
}

/*!
	\brief Sets the root of the multibody.

	Sets the root of the multibody.  Other links in the multibody are automatically
	computed.
*/

void slMultibodySetRoot(slMultibody *m, slLink *root) {
	m->root = root;
	root->multibody = m;

	slMultibodyUpdate(m);
}

/*!
	\brief Updates the multibody by recomputing the connected links.

	Recomputes the links, the link count and the mass.
*/

void slMultibodyUpdate(slMultibody *m) {
	if(!m) return;

	m->links.clear();

	slLinkList(m->root, &m->links, 1);

	m->linkCount = slMultibodyCountLinks(m);
	m->mass = slMultibodyComputeMass(m);

	if(m->world->initialized) slMultibodyInitCollisionFlags(m, m->world->clipData->pairList);
}

/*!
	\brief Updates the positions and bounding boxes for all links in a multibody.
*/

void slMultibodyUpdatePositions(slMultibody *mb) {
	std::vector<slLink*>::iterator i;

	for(i = mb->links.begin(); i != mb->links.end(); i++ ) {
		slLink *link = *i;

		slLinkUpdatePositions(link);
		slLinkUpdateBoundingBox(link);
	}
}

/*!
	\brief Converts a breve matrix to an ODE matrix.
*/

void slSlToODEMatrix(double m[3][3], dReal *r) {
	r[0] = m[0][0];
	r[1] = m[0][1];
	r[2] = m[0][2];

	r[4] = m[1][0];
	r[5] = m[1][1];
	r[6] = m[1][2];

	r[8] = m[2][0];
	r[9] = m[2][1];
	r[10] = m[2][2];
}

/*!
	\brief Converts an ODE matrix to a breve matrix.
*/

void slODEToSlMatrix(dReal *r, double m[3][3]) {
	m[0][0] = r[0];
	m[0][1] = r[1];
	m[0][2] = r[2];

	m[1][0] = r[4];
	m[1][1] = r[5];
	m[1][2] = r[6];

	m[2][0] = r[8];
	m[2][1] = r[9];
	m[2][2] = r[10];
}

/*!
	\brief Checks this multibody's links for collisions with other links in 
	the multibody.
*/

int slMultibodyCheckSelfPenetration(slWorld *world, slMultibody *m) {
	slVclipData *vc;
	int x, y;
	slPairEntry *pe;
	std::vector<slLink*>::iterator i1;
	std::vector<slLink*>::iterator i2;

	if(!world->initialized) slVclipDataInit(world);
	vc = world->clipData;

	for(i1 = m->links.begin(); i1 != m->links.end(); i1++ ) {
		slLink *link1 = *i1;

		// start with the next thing in the list.  this way we avoid 
		// doing repeats like 1st-2nd and 2nd-1st

		for( i2 = i1 ; i2 != m->links.end(); i2++ ) {
			slLink *link2 = *i2;

			if(link1 != link2) {
				x = link1->clipNumber;
				y = link2->clipNumber;

				if(x > y) pe = &vc->pairList[x][y];
				else pe = &vc->pairList[y][x];

				if(slVclipFlagsShouldTest(pe->flags) && slVclipTestPair(vc, pe, NULL)) return 1;
			}
		}
	}

	return 0;
}

void *slMultibodyGetCallbackData(slMultibody *m) {
	return m->userData;
}

void slMultibodySetCallbackData(slMultibody *m, void *c) {
	m->userData = c;
}

void slMultibodySetHandleSelfCollisions(slMultibody *m, int n) {
	m->handleSelfCollisions = 1;
}
