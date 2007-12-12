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

#include "simulation.h"
#include "vclip.h"
#include "vclipData.h"

/*!
	\brief Initialize the slVclipData bound lists.

	This must be called to sort the lists for the first time and properly
	initialize the lists.
*/

void slInitBoundSort( slVclipData *d ) {
	unsigned int listSize = d->_count * 2;
	unsigned int x, y;

	d->candidates.clear();

	for ( x = 0;x < 3;x++ ) {
		d->boundListPointers[x].clear();

		if ( listSize != d->boundLists[x].size() )
			slMessage( DEBUG_ALL, "error in init bound sort: list size mismatch %d != %d\n", listSize, d->boundLists[x].size() );

		for ( y = 0;y < listSize;y++ ) d->boundListPointers[x].push_back( &d->boundLists[x][y] );
	}

	slInitBoundSortList( d->boundListPointers[0], d, BT_XAXIS );
	slInitBoundSortList( d->boundListPointers[1], d, BT_YAXIS );
	slInitBoundSortList( d->boundListPointers[2], d, BT_ZAXIS );
}

/*!
	\brief Adds an object-pair as a candidate for collision.

	If the first stage pruning indicates a potential collision, a
	collision candidate is added so that the second stage detection
	is run.
*/

void slAddCollisionCandidate( slVclipData *vc, slPairFlags flags, int x, int y ) {
	slCollisionCandidate c( vc, x, y );

	// the UNKNOWN flag indicates that we have not yet preformed a callback to
	// determine whether further collision detection is necessary.

	if ( flags & BT_UNKNOWN ) flags = vc->initPairFlags( x, y );

	if ( !( flags & BT_CHECK ) ) return;

	vc->candidates[ slVclipPairFlags( vc, x, y )] = c;
}

/*!
	\brief Removes the collision candidate from the candidate list.
*/

void slRemoveCollisionCandidate( slVclipData *vc, int x, int y ) {
	vc->candidates.erase( slVclipPairFlags( vc, x, y ) );
}

bool slBoundSortCompare( const slBoundSort *a, const slBoundSort *b ) {
	if ( *a->value == *b->value ) return( a->number < b->number );

	return ( *a->value < *b->value );
}

int slVclipData::pruneAndSweep() {

	slIsort( this, boundListPointers[ 0 ], BT_XAXIS );
	slIsort( this, boundListPointers[ 1 ], BT_YAXIS );
	slIsort( this, boundListPointers[ 2 ], BT_ZAXIS );

	return 0;
}

/*!
	\brief Run the collision-detection algorithm.

	Tolerance is currently unused.  If pruneOnly is true, then
	detection is stopped after the bounding box stage and no
	collisions are registered, so that only "proximity"
	information is given.  If boundingBoxOnly is true, then
	detection is stopped after the bounding box stage and
	collisions ARE registered.
*/

int slVclipData::clip( double tolerance, int pruneOnly, int boundingBoxOnly ) {
	int result;
	slCollision *ce;
	std::map< slPairFlags* , slCollisionCandidate >::iterator ci;

	collisionCount = 0;

	ce = slNextCollision( this );

	for ( ci = candidates.begin(); ci != candidates.end(); ci++ ) {
		slCollisionCandidate c = ci->second;

		if ( boundingBoxOnly ) {
			if ( world->_objects[c._x] && world->_objects[c._y] && c._x != c._y ) {
				ce->n1 = c._x;
				ce->n2 = c._y;

				ce = slNextCollision( this );
			}
		} else {
			result = testPair( &c, ce );

			if ( result == CT_ERROR ) return -1;

			if ( result == CT_PENETRATE ) ce = slNextCollision( this );
		}
	}

	// collisionCount is pointing to the next empty collision
	// in other words, 1 + the actual number of collisions

	collisionCount--;

	return collisionCount;
}

slCollision *slNextCollision( slVclipData *v, int x, int y ) {
	slCollision *e = slNextCollision( v );

	e->n1 = x;
	e->n2 = y;

	return e;
}

slCollision *slNextCollision( slVclipData *v ) {
	slCollision *e;

	if ( v->collisionCount >= v->collisions.size() ) {
		v->collisions.resize( v->collisions.size() + 10 );
	}

	e = &v->collisions[ v->collisionCount++ ];

	return e;
}

/*!
 * \brief Insertion-sort one of the dimensions of the sort list.
 *
 * As we swap the elements, we set flags indicating whether they are
 * overlapping other objects or not.  When we're done, potential
 * collisions will be overlapping in all 3 dimensions.
 *
 * What happens if there's a tie while we're sorting?
 *
 * If we stop, then we might miss a case: A and B are equal, C comes along
 * and matches.  If we stop now, then we haven't noticed the potential
 * collision between A + C.  So we'll have to continue through equal
 * scores.
 *
 * This doesn't seem like such a big deal, but is a MAJOR slowdown in
 * 2-dimensional simulations--when the objects don't leave a certain
 * plane, we do *n^2 iterations*.  So, to break ties, we rely on the
 * number field of the bound entry.
 *
 * This function is a major bottleneck for large simulations, and has been optimized accordingly.
*/

void slIsort( slVclipData *d, std::vector<slBoundSort*> &list, char boundTypeFlag ) {
	int x, y;

	std::vector<slBoundSort*>::iterator currentI, leftI;

	if ( list.size() == 0 ) return;

	for ( currentI = list.begin() + 1; currentI != list.end(); currentI++ ) {
		const slBoundSort *currentSort = *currentI;

		slBoundSort *leftSort;

		const int currentSortType = currentSort->type;

		x = currentSort->number;

		if ( 1 ) {

			// NaNs mess up the logic of the sort since they are not =, < or > than
			// any other value, meaning they have no proper place in the list.
			// -Infinity does have a proper (yet "meaningless") place in the list

			if ( isnan( *currentSort->value ) ) *currentSort->value = -HUGE_VAL;

			// Keep moving to the left; until:
			// 1) there are no more entries to the left
			// 2) this entry is greater than the entry to the left
			// 3) this entry is equal to the entry on the left AND has a larger number

			leftI = currentI;

			leftSort = *( leftI - 1 );

			while ( leftI > list.begin() && (( *currentSort->value < *leftSort->value ) ||
			                                 ( *currentSort->value == *leftSort->value && currentSort->number < leftSort->number ) ) ) {

				y = leftSort->number;

				if ( currentSortType != leftSort->type && x != y ) {
					slPairFlags flags = slVclipPairFlagValue( d, x, y );

					if ( currentSortType == BT_MIN && ( flags & BT_CHECK ) ) {
						// the min moving to the left a max -- overlap, turn on the flag.

#ifdef DEBUG

						if (( flags & boundTypeFlag ) && *currentSort->value != *leftSort->value ) slMessage( DEBUG_WARN, "vclip inconsistancy [flag already on]!\n" );

#endif
						flags |= boundTypeFlag;

						slVclipPairFlagValue( d, x, y ) = flags;

						if ( slVclipFlagsShouldTest( flags ) ) slAddCollisionCandidate( d, flags, x, y );

					} else if ( currentSortType == BT_MAX && flags & BT_CHECK && *currentSort->value != *leftSort->value ) {
						// the max moving to the left a min -- no overlap, turn off the flag.

#ifdef DEBUG

						if ( !( flags & boundTypeFlag ) ) slMessage( DEBUG_WARN, "vclip inconsistancy [flag already off]!\n" );

#endif

						if ( slVclipFlagsShouldTest( flags ) ) slRemoveCollisionCandidate( d, x, y );

						slVclipPairFlagValue( d, x, y ) ^= boundTypeFlag;
					}
				}

				*( leftI - 1 ) = ( slBoundSort* )currentSort;

				*leftI = leftSort;

				leftI--;

				if ( leftI != list.begin() ) {
					leftSort = *( leftI - 1 );
				}
			}
		}
	}
}

/**
 * Initializes a slBoundSort list by sorting it.  unlike the normal bound sort, the list has not 
 * been previously sorted and the flags are thus not consistant.
 */

void slInitBoundSortList( std::vector<slBoundSort*> &list, slVclipData *v, char boundTypeFlag ) {
	unsigned int n, extend;
	int x, y;
	int otherSide;
	slPairFlags *flags;

	std::sort( list.begin(), list.end(), slBoundSortCompare );

	// zero out this entry for all pairs

	for ( n = 0;n < list.size();n++ ) {

		if ( list[ n ]->type == BT_MIN ) {
			extend = n + 1;

			x = list[n]->number;

			if ( x < 0 || x >= ( int )list.size() ) {
				slMessage( DEBUG_ALL, "vclip init error!  x = %d\n", x );
			}

			// we move to the right until the end of the list, or until
			// the corresponding BT_MAX has been found AND the extend
			// value is greater than the BT_MAX value

			otherSide = -1;

			while ( extend < list.size() && ( otherSide == -1 || *list[otherSide]->value == *list[extend]->value ) ) {
				if ( list[extend]->number == list[n]->number ) otherSide = extend;
				else {
					y = list[extend]->number;

					flags = slVclipPairFlags( v, x, y );

					if ( x < 0 || x >= ( int )list.size() || y < 0 || y >= ( int )list.size() ) {
						slMessage( DEBUG_ALL, "isort init error!  looked up pair ( %d, %d )\n", x, y );
					}

					if ( list[extend]->type == BT_MIN ) {
						if ( *flags & boundTypeFlag ) {
							slMessage( DEBUG_ALL, "vclip init inconsistancy [flag is already on]\n" );
						}

						*flags |= boundTypeFlag;

						if ( slVclipFlagsShouldTest( *flags ) ) slAddCollisionCandidate( v, *flags, x, y );
					}
				}

				extend++;
			}
		}
	}
}

/**
 * Transforms p1 with the position p, placing the transformed plane in pt.
 */

slPlane *slPositionPlane( const slPosition *p, const slPlane *p1, slPlane *pt ) {
	slVectorXform( p->rotation, &p1->normal, &pt->normal );
	slPositionVertex( p, &p1->vertex, &pt->vertex );

	return pt;
}

/**
 * Preforms a second-stage collision check on the specified object pair.
 */

int slVclipData::testPair( slCollisionCandidate *candidate, slCollision *ce ) {
	const slShape *s1 = candidate->_shape1;
	const slShape *s2 = candidate->_shape2;

	const slPosition *p1 = candidate->_position1;
	const slPosition *p2 = candidate->_position2;

	dMatrix3 m1, m2;

	slShape::slMatrixToODEMatrix( p1 -> rotation, m1 );
	slShape::slMatrixToODEMatrix( p2 -> rotation, m2 );

	dGeomSetPosition( s1 -> _odeGeomID[ 0 ], p1 -> location.x, p1 -> location.y, p1 -> location.z );
	dGeomSetPosition( s2 -> _odeGeomID[ 1 ], p2 -> location.x, p2 -> location.y, p2 -> location.z );

	dGeomSetRotation( s1 -> _odeGeomID[ 0 ], m1 );
	dGeomSetRotation( s2 -> _odeGeomID[ 1 ], m2 );

	dGeomMoved( s1 -> _odeGeomID[ 0 ] );
	dGeomMoved( s2 -> _odeGeomID[ 1 ] );

	if( !ce ) {
		dContactGeom geom;

		if( dCollide( s1 -> _odeGeomID[ 0 ], s2 -> _odeGeomID[ 1 ], 1, &geom, sizeof( dContactGeom ) ) ) 
			return CT_PENETRATE;

		return CT_DISJOINT;
	}

	ce -> _contactPoints = dCollide( s1 -> _odeGeomID[ 0 ], s2 -> _odeGeomID[ 1 ], MAX_ODE_CONTACTS, ce -> _contactGeoms, sizeof( dContactGeom ) );

	if( ce -> _contactPoints > 0 ) {
		// printf( "got %d contact points\n", ce -> _contactPoints );
		ce->n1 = candidate -> _x;
		ce->n2 = candidate -> _y;
		return CT_PENETRATE;
	}

	return CT_DISJOINT;
}

/*
 * \brief Find whether a point lies within a voronoi region.
 * 
 * Returns 1 if a point is included within the specified region--
 * otherwise returns 1, update will be the violated feature plane,
 * dist will be the violation distance.
 */

int slClipPoint( const slVector *p, const slPlane *v, const slPosition *vp, const int vcount, int *update, double *dist ) {
	slPlane tVoronoi;
	int n;
	double d;

	*update = -1;

	for ( n = 0;n < vcount;n++ ) {
		slPositionPlane( vp, &v[n], &tVoronoi );

		d = slPlaneDistance( &tVoronoi, p );

		if ( d < 0 ) {

			if ( dist ) 
				*dist = d;

			*update = n;

			return 0;
		}
	}

	return 1;
}

/*
 * Find the maximally violated voronoi plane.
 */

int slClipPointMax( const slVector *p, const slPlane *v, const slPosition *vp, int vcount, int *update, double *dist ) {
	slPlane tVoronoi; // transformed voronoi plane
	int n, minFeature = 0;
	double minScore = 0.0, m;

	*update = -1;

	for ( n = 0;n < vcount;n++ ) {
		slPositionPlane( vp, &v[n], &tVoronoi );

		m = slPlaneDistance( &tVoronoi, p );

		if ( m < minScore ) {
			minScore = m;
			minFeature = n;
		}
	}

	if ( minScore < 0.0 ) {
		*update = minFeature;

		if ( dist ) *dist = minScore;

		return 0;
	}

	return 1;
}
