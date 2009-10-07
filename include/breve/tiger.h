#ifndef _TIGER_H
#define _TIGER_H

// i <heart> public class members

#include <vector>
#include <string>
#include <iostream>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

#include "glIncludes.h"
#include "terrain.h"

class slGISPoint {
	public:
		slGISPoint(float x, float y) { _x = x; _y = y; }
		slGISPoint() { _x = 0; _y = 0; };

		void setNormal(const slGISPoint &b) {
			_nx = _x - b._x;
			_ny = _y - b._y;

			_nx /= (_nx * _nx) + (_ny * _ny);
			_ny /= (_nx * _nx) + (_ny * _ny);
		}

		std::ostream& operator<<(std::ostream& out) {
			out << "( " << _x << "," << _y << " )";
			return out;
		}

		void getVector(slVector *v) {
			v->x = _x;
			v->y = 0;
			v->z = _y;
		}

		void getNormal(slVector *v) {
			v->x = _nx;
			v->y = 0;
			v->z = _ny;
		}

		std::vector< slGISPoint > _connections;

		float _x, _y;
		float _nx, _ny;
};

class slGISChain {
	public:
		slGISChain( const slGISPoint &start, const slGISPoint &end, short type) {
			_points.push_back( start );
			_points.back().setNormal( end );
			_points.push_back( end );
			_points.back().setNormal( start );
			_type = type;
		}

		slGISChain() {};

		void insertPoint( slGISPoint &point) {
			std::vector<slGISPoint>::iterator pi = _points.end();
			pi--;

			_points.insert( pi, point);
		}

		std::vector< slGISPoint > _points;
		std::vector< slGISPoint > _normals;
		std::string _name;
		short _type;
};

class slGISData {
	public:
		slGISData(char *filename, slTerrain *t);

		int parseRT1File(std::string filename);
		int parseRT2File(std::string filename);

		void setTerrain(slTerrain *t) {
			_terrain = t;
		}

		void draw(slCamera *c);
		void compile();

		std::map<int, slGISChain> _chains;
		std::set< slGISPoint > _points;

		slTerrain *_terrain;

		slGISPoint _center;
		GLuint _list;
};

#endif /* _TIGER_H */
