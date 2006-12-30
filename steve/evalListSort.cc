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

#include "steve.h"
#include "evaluation.h"

//
// So this is progress.  After moving brEvalLists over to std::vectors, I started using
// std::sort() to do the sorting and didn't need to rely on global variables.  And all
// was well.  Except that with gcc on Linux, std::sort requires strict weak ordering
// (which is okay) and will crash if the ordering is not strictly weak (ouch!).  Since
// the sort predicate is a breve callback, I really cannot control that, so we have 
// to resort to qsort with globals.  Sorry everybody.  I know I let you all down.
//

class brEvalVectorSorter {

	public:
		brEvalVectorSorter( stInstance *instance, stMethod *method ) {
			mInstance = instance;
			mMethod = method;
		};

		// std::sort compatible results

		bool operator()( const brEval& a, const brEval& b ) {
			return qsortCompare( a, b ) < 0.0;
		}

		// qsort compatible compare

		int qsortCompare( const brEval& a, const brEval& b ) {
			stRunInstance ri;
			brEval result;
			const brEval *args[ 2 ];
			int rcode;

			std::pair< const brEval*, const brEval* > pair( &a, &b );

			// if ( mSeenMap.find( pair ) != mSeenMap.end() ) 
			// 	return mSeenMap[ pair ];

			args[ 0 ] = &a;
			args[ 1 ] = &b;

			ri.instance = mInstance;

			ri.type = mInstance->type;

			rcode = stCallMethod( &ri, &ri, mMethod, args, 2, &result );

			if ( result.type() != AT_DOUBLE ) {
				rcode = stToDouble( &result, &result, &ri );
			}

			int iResult = 0;

			if( BRDOUBLE( &result ) < 0.0 ) iResult = -1;
			else if( BRDOUBLE( &result ) > 0.0 ) iResult = 1;

			mSeenMap[ pair ] = iResult;

			return mSeenMap[ pair ];
		};

	private:
		stInstance *mInstance;

		stMethod *mMethod;

		std::map< std::pair< const brEval*, const brEval* >, int > mSeenMap;

};

brEvalVectorSorter *gSorter;

int brEvalVectorCompare( const void *inA, const void *inB ) {
	const brEval *evalA = (brEval*)inA;
	const brEval *evalB = (brEval*)inB;

	return gSorter->qsortCompare( *evalA, *evalB );
}

int stSortEvalList( brEvalListHead *head, stInstance *caller, stMethod *method ) {
	brEvalVectorSorter sorter( caller, method );

	// So it turns out that std::sort requires strict weak ordering and will crash[!]
	// if you don't have it!  That is strictly weak!
	// std::sort( head->_vector.begin(), head->_vector.end(), sorter );

	// Screw that -- we have to switch back to the old way with qsort and a global variable,
	// but we'll use the same brEvalVectorSorter for compatibility with either method in
	// case we figure out a way to someday go back.

	gSorter = &sorter;

	qsort( &head->_vector[ 0 ], head->_vector.size(), sizeof( brEval ), brEvalVectorCompare );

	return 1;
}
