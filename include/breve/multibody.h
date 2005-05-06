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

#ifdef __cplusplus
#include <vector>
#include <algorithm>

#include "simulation.h"

/*!
	\brief A logical collection of attached links.
*/

class slMultibody { 
	public:
		slMultibody(slWorld *w);
		~slMultibody();

		void initCollisionFlags(slVclipData *cd);
		void setHandleSelfCollisions(int n);

		slList *allCallbackData();

		void *getCallbackData();
		void setCallbackData(void *c);

		void setRoot(slLink *root);

		void setAcceleration(slVector *linear, slVector *rotational);
		void setVelocity(slVector *linear, slVector *rotational);

		void updatePositions();

		int checkSelfPenetration();
		int countLinks();
		void update();

		slWorld *_world;
		slLink *_root;

		std::vector<slLink*> _links;

		unsigned char _handleSelfCollisions;

		void *_userData;
};

#endif

#ifdef __cplusplus
extern "C"{
#endif

int slInitShapeList(slShape **slist, slPosition **plist, slMultibody *root);

void slMultibodyPosition(slMultibody *m, slVector *location, double rotation[3][3]);
void slMultibodyRotAngleToMatrix(slVector *axis, double r, double rot[3][3]);

void slMultibodyOffsetPosition(slMultibody *m, slVector *offset);

slMultibody *slLinkFindMultibody(slLink *root);

void slMultibodyRotate(slMultibody *m, double rotation[3][3]);

void slNullOrphanMultibodies(slLink *orphan);

#ifdef __cplusplus
}
#endif

