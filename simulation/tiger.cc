#include "simulation.h"
#include "tiger.h"

//
// This is some simple code to read rudimentary data from
// Tiger/Line files
//

slGISData::slGISData(char *file) {
	std::string rt1(file);
	std::string rt2(file);

	rt1 += ".RT1";
	rt2 += ".RT2";

	parseRT1File(rt1);
	parseRT2File(rt2);
}

int slGISData::parseRT1File(std::string rt1) {
	char line[256];
	std::fstream fs;
	double totalLat = 0, totalLon = 0;

	fs.open(rt1.c_str(), std::ios::in);

	if(!fs.is_open()) return -1;

	while(!fs.eof()) {
		char name[31], tlid[11], type[5], clas[4], slat[11], slon[11], elat[11], elon[11];
		double startLat, endLat, startLon, endLon;

		fs.getline(line, 255);
	
		// all of these offset values are specified in the tiger file format

		strncpy(name, &line[19], 30);
		name[30] = 0;

		strncpy(tlid, &line[5], 10);
		tlid[10] = 0;
	
		strncpy(type, &line[49], 4);
		type[4] = 0;

		strncpy(clas, &line[55], 3);
		clas[3] = 0;

		strncpy(slat, &line[190], 10);
		slat[10] = 0;
		strncpy(slon, &line[200], 9);
		slon[9] = 0;

		strncpy(elat, &line[209], 10);
		elat[10] = 0;
		strncpy(elon, &line[219], 9);
		elon[9] = 0;

		int n = 29;

		while(name[n] == ' ') {
			name[n] = '\0';
			n--;
		}

		if(clas[0] == 'A') {
			short rType = strtol(&clas[1], NULL, 10);
			int tlidID = strtol( tlid, NULL, 10);

			startLat = strtod(slat, NULL) / 1000.0;
			endLat = strtod(elat, NULL) / 1000.0;
			startLon = strtod(slon, NULL) / 1000.0;
			endLon = strtod(elon, NULL) / 1000.0;

			slGISPoint sp( startLon, startLat);
			slGISPoint ep( endLon, endLat);

			slGISChain chain( sp, ep, rType);

			_chains[ tlidID] = chain;

			totalLat += startLat + endLat;
			totalLon += startLon + endLon;
		}
	}

	totalLon /= _chains.size() * 2;
	totalLat /= _chains.size() * 2;

	_center._x = totalLon;
	_center._y = totalLat;

	fs.close();

	return 0;
}

int slGISData::parseRT2File(std::string rt2) {
	std::fstream fs;
	char line[256];
	char tlidString[20];
	slGISChain *chain;
	int offset;

	fs.open(rt2.c_str(), std::ios::in);

	if(!fs.is_open()) return -1;

	while(!fs.eof()) {
		double lat, lon;
		std::map<int,slGISChain>::iterator mi;
		int tlid;

		fs.getline(line, 255);

		offset = 18;

		strncpy(tlidString, &line[5], 10);
		tlidString[10] = 0;

		tlid = strtol(tlidString, NULL, 10);

		mi = _chains.find( tlid);

		if(mi != _chains.end()) {
			chain = &mi->second;

			do {
				lat = strtod(&line[offset], NULL) / 1000.0;
				lon = strtod(&line[offset + 10], NULL) / 1000.0;

				slGISPoint point( lon, lat);
	
				if(lat != 0.0 && lon != 0.0) chain->insertPoint( point);
	
				offset += 19;
			} while(lat != 0.0 && lon != 0.0);
		}
	}

	return 0;
}

void slGISData::draw(slCamera *c) {
	std::map<int, slGISChain>::iterator ci;
	std::vector<slGISPoint>::iterator pi;

	glEnable(GL_BLEND);
	glColor4f(0.0, 0.0, 0.0, 0.8);

	for(ci = _chains.begin(); ci != _chains.end(); ci++ ) {
		if( ci->second._type < 60 ) {
			slVector start, end;

			float w = 4.0 - (double)((int)ci->second._type / 10);

			if (w < 0.1) w = 0.1;

			glLineWidth( w);

			pi = ci->second._points.begin();
			start.x = pi->_x - _center._x;
			start.y = 0;
			start.z = pi->_y - _center._y;

			if(slCameraPointInFrustum(c, &start)) {
				glBegin(GL_LINE_STRIP);

				glVertex3f(start.x, start.y, start.z);

				for(pi = ci->second._points.begin() + 1; pi != ci->second._points.end(); pi++ ) {
					glVertex3f(pi->_x - _center._x, 0, pi->_y - _center._y);
				}
	
				glEnd();
			}
		}
	}
}
