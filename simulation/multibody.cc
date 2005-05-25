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
#include "joint.h"
#include "vclip.h"
#include "vclipData.h"

/*!
	\brief Ignores collisions between adjacent multibody links.
*/

void slMultibody::initCollisionFlags(slVclipData *cd) {
	slLink *link1, *link2;
	slPairFlags *flags;
	std::vector<slLink*>::iterator i1;
	std::vector<slLink*>::iterator i2;

	for(i1 = _links.begin(); i1 != _links.end(); i1++) {
		link1 = *i1;

		// set all check flags on or off, depending on whether 
		// self collisions are handled or not.

		for(i2 = _links.begin(); i2 != _links.end(); i2++) {
			link2 = *i2;

			if(link1 != link2) {
				flags = slVclipPairFlags(cd, link1->clipNumber, link2->clipNumber);

				if(_handleSelfCollisions) *flags |= BT_CHECK;
				else if(*flags & BT_CHECK) *flags ^= BT_CHECK;
			}
		}
	}

	for(i1 = _links.begin(); i1 != _links.end(); i1++) {
		link1 = *i1;

		// then prune out all adjacent pairs.

		std::vector<slJoint*>::iterator ji;

		for(ji = link1->outJoints.begin(); ji != link1->outJoints.end(); ji++) {
			link2 = (*ji)->child;

			if(link1 != link2) {
				flags = slVclipPairFlags(cd, link1->clipNumber, link2->clipNumber);

				if(*flags & BT_CHECK) *flags ^= BT_CHECK;
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

	if(!m->_root) return;

	m->updatePositions();

	if(location) {
		slVectorSub(location, &m->_root->position.location, &offset);
		slMultibodyOffsetPosition(m, &offset);
	}

	if(rot) {
		double invR[3][3], transform[3][3];

		/* figure out the relative rotation required to take us from the 
		 * current rotation to the new one */

		slMatrixTranspose(m->_root->position.rotation, invR);
		slMatrixMulMatrix(rot, invR, transform);

		slMultibodyRotate(m, transform);
	}

	m->updatePositions();
}

/*!
	\brief Gives a rotation matrix from an axis and an angle.
*/

// void slMultibodyRotAngleToMatrix(slVector *axis, double r, double rot[3][3]) {
// 	slQuat q;
// 	slQuatSetFromAngle(&q, r, axis);
// 	slQuatToMatrix(&q, rot);
// }

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

	for(i1 = mb->_links.begin(); i1 != mb->_links.end(); i1++) {	
		link = *i1;

		// first, since the whole body is making this rotation, 
		// our location will be changed.  we have to compute 
		// the new position based on the rotation angle and our
		// location relative to the root link.

		slVectorSub(&link->position.location, &mb->_root->position.location, &toLink);
		slVectorXform(rotation, &toLink, &newToLink);
		slVectorAdd(&newToLink, &mb->_root->position.location, &newToLink);
		dBodySetPosition(link->_odeBodyID, newToLink.x, newToLink.y, newToLink.z);
	
		slMatrixMulMatrix(rotation, link->position.rotation, newRot);

		slMatrixCopy(newRot, link->position.rotation);

		// update the ODE state

		slMatrixToQuat(newRot, &q);

		Q[0] = q.s;
		Q[1] = q.x;
		Q[2] = q.y;
		Q[3] = q.z;

		if(link->simulate) dBodySetQuaternion(link->_odeBodyID, Q);
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

	for(i1 = mb->_links.begin(); i1 != mb->_links.end(); i1++) {	
		link = *i1;

		slVectorAdd(&link->position.location, offset, &link->position.location);

		if(link->simulate) {
			oldP = dBodyGetPosition(link->_odeBodyID);

			newP[0] = oldP[0] + offset->x;
			newP[1] = oldP[1] + offset->y;
			newP[2] = oldP[2] + offset->z;
			
			dBodySetPosition(link->_odeBodyID, newP[0], newP[1], newP[2]);
		}
	}
}

/*!
	\brief Sets the linear and/or rotational velocities of all links in 
	the multibody.
*/

void slMultibody::setVelocity(slVector *linear, slVector *rotational) {
	slLink *link;
	std::vector<slLink*>::iterator i;

	for(i=_links.begin(); i != _links.end(); i++) {
		link = *i;

		link->setVelocity(linear, rotational);
	}
}

/*!
	\brief Sets the linear and/or rotational accelerations of all links
	in the multibody.
*/

void slMultibody::setAcceleration(slVector *linear, slVector *rotational) {
	slLink *link;
	std::vector<slLink*>::iterator i;

	for(i=_links.begin(); i != _links.end(); i++) {
		link = *i;

		link->setAcceleration(linear, rotational);
	}
}

/*!
	\brief Returns all of the callback data for links and joints in a multibody.
*/

slList *slMultibody::allCallbackData() {
	slList *list = NULL;
	std::vector<slLink*>::iterator li;
	std::vector<slJoint*>::iterator ji;

	for( li = _links.begin(); li != _links.end(); li++ ) {
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

int slMultibody::countLinks() {
	int number = 0;
	std::vector<slLink*>::iterator i;

	for( i = _links.begin(); i != _links.end(); i++ ) {	
		slLink *link = *i;

		link->multibody = this;
		number++;
	}

	return number;
}

/*!
	\brief Creates an empty new multibody struct associated with a world.

	Calling this method does not automatically add the multibody to the
	specified world.
*/

slMultibody::slMultibody(slWorld *w) {
	_world = w;
	_handleSelfCollisions = 0;
	_root = NULL;
}

/*!
	\brief Sets the root of the multibody.

	Sets the root of the multibody.  Other links in the multibody are automatically
	computed.
*/

void slMultibody::setRoot(slLink *root) {
	_root = root;
	if(_root) _root->multibody = this;

	update();
}

/*!
	\brief Updates the multibody by recomputing the connected links.
*/

void slMultibody::update() {
	_links.clear();

	slLinkList(_root, &_links, 1);

	if(_world->initialized) initCollisionFlags(_world->clipData);
}

/*!
	\brief Updates the positions and bounding boxes for all links in a multibody.
*/

void slMultibody::updatePositions() {
	std::vector<slLink*>::iterator i;

	for(i = _links.begin(); i != _links.end(); i++ ) {
		slLink *link = *i;

		link->updatePositions();
		link->updateBoundingBox();
	}
}

/*!
	\brief Checks this multibody's links for collisions with other links in 
	the multibody.
*/

int slMultibody::checkSelfPenetration() {
	slVclipData *vc;
	slPairFlags *flags;
	std::vector<slLink*>::iterator i1;
	std::vector<slLink*>::iterator i2;

	if(!_world->initialized) slVclipDataInit(_world);
	vc = _world->clipData;

	for(i1 = _links.begin(); i1 != _links.end(); i1++ ) {
		slLink *link1 = *i1;

		// start with the next thing in the list.  this way we avoid 
		// doing repeats like 1st-2nd and 2nd-1st

		for( i2 = i1 ; i2 != _links.end(); i2++ ) {
			slLink *link2 = *i2;

			if(link1 != link2) {
				slCollisionCandidate c;

				c.x = link1->clipNumber;
				c.y = link2->clipNumber;

				flags = slVclipPairFlags(vc, link1->clipNumber, link2->clipNumber);

				if(slVclipFlagsShouldTest(*flags) && vc->testPair(&c, NULL)) return 1;
			}
		}
	}

	return 0;
}

/*!
	\brief Returns the callback user-data for this multibody object.
	
	Returns the user-defined data field for this object.  In the context
	of breve simulations, this data corresponds to the breve instance
	to which this multibody belongs.
*/

void *slMultibody::getCallbackData() {
	return _userData;
}

/*!
	\brief Sets the callback user-data for this multibody object.
	
	Sets a user-defined data field for this object.  In the context
	of breve simulations, this data corresponds to the breve instance
	to which this multibody belongs.
*/

void slMultibody::setCallbackData(void *c) {
	_userData = c;
}

/*!
	\brief Set self-collision handling for this multibody.

	Specifies whether links in this multibody should be allowed to 
	collide with themselves.  Calling this function has no effect
	until the \ref slWorld collision data is reinitialized.
*/

void slMultibody::setHandleSelfCollisions(int n) {
	_handleSelfCollisions = 1;
}

/*!
	\brief Frees a multibody, but does not free its links.

	The links will continue to exist independent of the multibody
	and must be freed seperately.
*/

slMultibody::~slMultibody() {
	slNullOrphanMultibodies(_root);
	_links.empty();
}

