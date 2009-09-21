#include "simulation.h"
#include "glIncludes.h"
#include "camera.h"
#include "tiger.h"
#include "terrain.h"

#include <iostream>

#define MAX(x,y)    ((x)>(y)?(x):(y))

#define LAT_MUL 111079.81083422346
#define LON_MUL 111079.81083422346

#define LAT_OFF 42.33062596
#define LON_OFF -72.64883895

//
// This is some simple code to read rudimentary data from
// Tiger/Line files
//

/*!
	Loads GISData from a set of Tiger/Line files.

	\param file is the root pathname of a set of Tiger files, without the
	.RTn extensions.  Currently, only files 1 and 2 are used.
*/

#define TOLERANCE 0.001

bool operator==( const slGISPoint &a, const slGISPoint &b ) {
	return ( a._x - b._x < TOLERANCE ) && ( a._y - b._y < TOLERANCE );
}

bool operator<( const slGISPoint &a, const slGISPoint &b ) {

	return (( a._x - b._x ) < -TOLERANCE );
}

slGISData::slGISData( char *file, slTerrain *t ) {

	std::string rt1( file );
	std::string rt2( file );

	rt1 += ".RT1";
	rt2 += ".RT2";

	_terrain = t;

	parseRT1File( rt1 );
	parseRT2File( rt2 );
}

int slGISData::parseRT1File( std::string rt1 ) {
	char line[256];
	std::fstream fs;

	fs.open( rt1.c_str(), std::ios::in );

	if ( !fs.is_open() ) return -1;

	while ( !fs.eof() ) {
		char name[31], tlid[11], type[5], clas[4], slat[11], slon[11], elat[11], elon[11];
		double startLat, endLat, startLon, endLon;

		fs.getline( line, 255 );

		// all of these offset values are specified in the tiger file format

		strncpy( name, &line[19], 30 );
		name[30] = 0;

		strncpy( tlid, &line[5], 10 );
		tlid[10] = 0;

		strncpy( type, &line[49], 4 );
		type[4] = 0;

		strncpy( clas, &line[55], 3 );
		clas[3] = 0;

		strncpy( slon, &line[190], 10 );
		slon[10] = 0;
		strncpy( slat, &line[200], 9 );
		slat[9] = 0;

		strncpy( elon, &line[209], 10 );
		elon[10] = 0;
		strncpy( elat, &line[219], 9 );
		elat[9] = 0;

		int n = 29;

		while ( name[n] == ' ' ) {
			name[n] = '\0';
			n--;
		}

		if ( clas[0] == 'A' ) {

			std::set< slGISPoint >::iterator ei, si;

			double h1, h2;

			short rType = strtol( &clas[1], NULL, 10 );

			int tlidID = strtol( tlid, NULL, 10 );

			startLat = strtod( slat, NULL ) / 1000000.0;

			endLat = strtod( elat, NULL ) /   1000000.0;

			startLon = strtod( slon, NULL ) / 1000000.0;

			endLon = strtod( elon, NULL ) /   1000000.0;

			slGISPoint sp( LON_MUL *( startLon - LON_OFF ), -LAT_MUL *( startLat - LAT_OFF ) );

			slGISPoint ep( LON_MUL *( endLon - LON_OFF ), -LAT_MUL *( endLat - LAT_OFF ) );

			h1 = _terrain->getHeightAtLocation( sp._x, sp._y );

			h2 = _terrain->getHeightAtLocation( ep._x, ep._y );

			if ( h1 != 0.0 || h2 != 0.0 ) {
				if (( ei = _points.find( ep ) ) == _points.end() ) {
					ei = _points.insert( ep ).first;
				} else {
					std::cout << "making existing point " << "\n";
				}

				if (( si = _points.find( sp ) ) == _points.end() ) {
					si = _points.insert( sp ).first;
				} else {
					std::cout << "making existing point " << "\n";
				}

				slGISChain chain( *ei, *si, rType );

				_chains[ tlidID] = chain;
			}
		}
	}

	_center._x = 0;

	_center._y = 0;

	fs.close();

	return 0;
}

int slGISData::parseRT2File( std::string rt2 ) {
	std::fstream fs;
	char line[256];
	char tlidString[20];
	slGISChain *chain;
	int offset;

	fs.open( rt2.c_str(), std::ios::in );

	if ( !fs.is_open() ) return -1;

	while ( !fs.eof() ) {
		double lat, lon;
		std::map<int, slGISChain>::iterator mi;
		int tlid;

		fs.getline( line, 255 );

		offset = 18;

		strncpy( tlidString, &line[5], 10 );
		tlidString[10] = 0;

		tlid = strtol( tlidString, NULL, 10 );

		mi = _chains.find( tlid );

		if ( mi != _chains.end() ) {
			chain = &mi->second;

			do {
				lon = strtod( &line[offset], NULL ) /      1000000.0;
				lat = strtod( &line[offset + 10], NULL ) / 1000000.0;

				slGISPoint point( LON_MUL *( lon - LON_OFF ), -LAT_MUL *( lat - LAT_OFF ) );

				if ( lat != 0.0 && lon != 0.0 ) chain->insertPoint( point );

				offset += 19;
			} while ( lat != 0.0 && lon != 0.0 );
		}
	}

	return 0;
}

void slGISData::draw( slCamera *c ) {
	#ifndef OPENGLES
	int n;

	std::map<int, slGISChain>::iterator ci;
	std::vector<slGISPoint>::iterator pi;

	// glDisable(GL_DEPTH_TEST);
	glColor4f( 0.1, 0.1, 0.1, 1.0 );

	glTranslatef( -_center._x, 0, -_center._y );

	for ( ci = _chains.begin(); ci != _chains.end(); ci++ ) {
		if ( ci->second._type < 60 ) {
			slVector first, last;
			double h1, h2;

			float w = 20.0 - ( double )(( int )ci->second._type / 10 );

			if ( w < 5.0 ) w = 5.0;

			ci->second._points[0].getVector( &first );

			ci->second._points.back().getVector( &last );

			h1 = _terrain->getHeightAtLocation( last.x, last.z );

			h2 = _terrain->getHeightAtLocation( first.x, first.z );

			last.y = h1 + 2;

			first.y = h1 + 2;

			if (( h1 != 0.0 || h2 != 0.0 ) && ( c->pointInFrustum( &first ) || c->pointInFrustum( &last ) ) ) {
				glBegin( GL_TRIANGLE_STRIP );

				for ( n = 0; n < ( int )ci->second._points.size() - 1; n++ ) {
					slVector tmp, yAxis, left, right, offset;
					float lastY = 0;

					yAxis.x = 0;
					yAxis.y = 1;
					yAxis.z = 0;


					ci->second._points[n].getVector( &tmp );

					offset.x = ci->second._points[n + 1]._x - ci->second._points[n]._x;
					offset.y = 0;
					offset.z = ci->second._points[n + 1]._y - ci->second._points[n]._y;

					slVectorNormalize( &offset );

					if ( n == 0 ) {
						ci->second._points[n].getNormal( &offset );

						slVectorAdd( &tmp, &offset, &right );

						lastY = right.y = MAX( _terrain->getHeightAtLocation( tmp.x + offset.x, tmp.z + offset.z ),
						                       _terrain->getHeightAtLocation( tmp.x - offset.x, tmp.z - offset.z ) ) + 2.0;
						glVertex3f( right.x, right.y, right.z );
					}

					slVectorSub( &tmp, &offset, &left );

					ci->second._points[n + 1].getVector( &tmp );
					slVectorAdd( &tmp, &offset, &right );

					if ( _terrain ) {
						left.y = lastY;

						lastY = right.y = MAX( _terrain->getHeightAtLocation( tmp.x + offset.x, tmp.z + offset.z ),
						                       _terrain->getHeightAtLocation( tmp.x - offset.x, tmp.z - offset.z ) ) + 2.0;
					}

					glVertex3f( left.x, left.y, left.z );

					glVertex3f( right.x, right.y, right.z );

					if ( n == ( int )ci->second._points.size() - 2 ) {
						slVectorSub( &tmp, &offset, &left );
						left.y = lastY;
						glVertex3f( left.x, left.y, left.z );
					}
				}

				glEnd();
			}
		}
	}

	#endif
}
