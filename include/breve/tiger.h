// i <heart> public class members

#include <vector>
#include <string>
#include <iostream>
#include <string>
#include <fstream>
#include <map>

class slGISPoint {
	public:
		slGISPoint(float x, float y) { _x = x; _y = y; }
		slGISPoint() { _x = 0; _y = 0; };

		slGISPoint normal(slGISPoint &a, slGISPoint &b) {
			double dx, dy;

			dx = a._x - b._x;
			dy = a._y - b._y;
			return slGISPoint(dx, dy);
		}

		float _x, _y;
};

class slGISChain {
	public:
		slGISChain( slGISPoint &start, slGISPoint &end, short type) {
			_points.push_back(start);
			_points.push_back(end);
			_type = type;
		}

		slGISChain() {};

		void insertPoint( slGISPoint &point) {
			std::vector<slGISPoint>::iterator pi = _points.end();
			pi--;

			_points.insert( pi, point);
		}

		std::vector<slGISPoint> _points;
		std::vector<slGISPoint> _normals;
		std::string _name;
		short _type;
};

class slGISData {
	public:
		slGISData(char *filename);

		int parseRT1File(std::string filename);
		int parseRT2File(std::string filename);

		void draw(slCamera *c);
		void compile();

		std::map<int, slGISChain> _chains;

		slGISPoint _center;
		GLuint _list;
};
