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

class brEvalVectorSorter {

	public:
		brEvalVectorSorter( stInstance *instance, stMethod *method ) {
			mInstance = instance;
			mMethod = method;
		};

		bool operator()( const brEval *a, const brEval *b ) {
			stRunInstance ri;
			brEval result;
			const brEval *args[ 2 ];
			int rcode;

			std::pair< const brEval*, const brEval* > pair( a, b );

			if ( mSeenMap.find( pair ) != mSeenMap.end() ) return mSeenMap[ pair ];

			args[ 0 ] = a;
			args[ 1 ] = b;

			ri.instance = mInstance;

			ri.type = mInstance->type;

			rcode = stCallMethod( &ri, &ri, mMethod, args, 2, &result );

			if ( result.type() != AT_DOUBLE ) {
				rcode = stToDouble( &result, &result, &ri );
			}

			mSeenMap[ pair ] = ( BRDOUBLE( &result ) < 0.0 );

			return mSeenMap[ pair ];
		};

	private:
		stInstance *mInstance;

		stMethod *mMethod;

		std::map< std::pair< const brEval*, const brEval* >, bool > mSeenMap;

};

int stSortEvalList( brEvalListHead *head, stInstance *caller, stMethod *method ) {
	brEvalVectorSorter sorter( caller, method );

	// So it turns out that std::sort requires strict weak ordering and will crash[!]
	// if you don't have it!  That is strictly weak!

	std::sort( head->_vector.begin(), head->_vector.end(), sorter );

	return 1;
}

int brEvalListCompare( const void *a, const void *b ) {
	return 0;
}
