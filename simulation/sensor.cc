#include <stdio.h>
#include <math.h>
#include "simulation.h"
#include "glIncludes.h"
#include "gldraw.h"
#include "camera.h"
#include "volInt.h"
#include "vclip.h"
#include "vclipData.h"
#include "sensor.h"
#include <map>
using std::map;
using std::vector;
using std::string;

#include <assert.h>

int SensorBuilder::test = 3;
map <string, UserSensor*> SensorBuilder::sensors = map<string, UserSensor*>();

double distance_plane_point(slVector *normal, slVector *planePoint, slVector *point){
	return normal->x * (point->x - planePoint->x) 
		 + normal->y * (point->y - planePoint->y) 
		 + normal->z * (point->z - planePoint->z);
}
  /*
   * Drops the perpendicular normal on vector from in the direction of vector to
   * the result is written in result
   *
   * the result is a vector, normal to from starts somewhere on the vector from and ends in to.
   */
void inv_perpendicular( slVector *from, slVector *to, slVector *result ){
	slVector diff;
	slVector proj;
	slVector nfrom;
	double d;
	slVectorCopy(from, &nfrom);
	slVectorNormalize(&nfrom);
	slVectorSub(to, from, &diff);	
	d = slVectorDot(from, &diff);
	d = d / slVectorLength(from);
	slVectorMul(&nfrom, d, &proj);
	slVectorSub(&diff, &proj, result);
//		slMessage(DEBUG_ALL,"resulting Vector:");
//	slVectorPrint(result);
//	slVectorNormalize(result);
}

   
void testperpendicular(){
	slVector a,b,c;
	slVectorSet(&a, 5, 0, 0);
	slVectorSet(&b, 5, 2.5, 0);
	inv_perpendicular(&a, &b, &c);
	slVectorPrint(&c);
}

/**
  * given an array of input-values and resulting output-values
  * this funktion return the interpolated output value of a
  * input value
  * note: the pairs must be sorted by value
  */

double interpolate(pair *pairs, double value, int array_length ){
	int lower = 0;
	int upper = array_length - 1; //index borders
	int middle;
	double gradient;
	double result;
	while(abs(upper - lower) > 1){ 
		middle = (lower + upper)/2;
//			slMessage(DEBUG_ALL,"lower: %d middle: %d upper: %d\n",lower, middle, upper);
		if (pairs[middle].value <= value) lower = middle;
		else upper = middle;
	}
//		slMessage(DEBUG_ALL,"lower: %d middle: %d upper: %d\n",lower, middle, upper);
	if(pairs[upper].value == pairs[lower].value) return pairs[upper].factor;
	gradient = (pairs[upper].factor - pairs[lower].factor)/(pairs[upper].value - pairs[lower].value);
	result = pairs[lower].factor + gradient * (value - pairs[lower].value);
//		slMessage(DEBUG_ALL,"gradient: %f result:%f\n", gradient, result);
	return result;
}

double Sensor::apply_distance_factor(double input, double dist){
	return input * interpolate(distance, dist, distance_length);
}

double Sensor::apply_azimut_factor(double input, double angle){
	return input * interpolate(azimut, angle, azimut_length);
}

double Sensor::apply_incidence_factor(double input, double angle){
	// angle is the difference between the normal vector of the face and the 
	// vector from the sensor to the face angle=0 =^ good reflection
	return input * interpolate(incidence, angle, incidence_length);

}

Sensor::Sensor(){
	slMessage(DEBUG_ALL,"Sensor Constructor\n");
	slVectorSet(&baseDirection, 0, 0, 1);
	// generate the normal vectors for the border planes therefore choose the "middle" vector of each side
	slVectorSet(&borderNormal[0],1,0,0);
	slVectorSet(&borderNormal[1],-1,0,0);
	slVectorSet(&borderNormal[2],0,1,0);
	slVectorSet(&borderNormal[3],0,-1,0);
}



  /*
   * creates a UserSensor with N = rows*columns rays 
   * max_range is the maximal range of the sensor, max_angel the maximal angle
   * the 3 arrays represent the datasheets of the sensor:
   * signal strenght in relation to the distance
   * signal strenth in relation to the azimut angle
   * signal strenght in relation to the incidence angle
   */
UserSensor* SensorBuilder::createUserSensor(const char* name, const int rows, const int columns, const double max_range, const double max_angle, 
				const int distance_length, const double* distance, const double* distance_factor,
				const int azimut_length, const double* azimut, const double* azimut_factor,
				const int incidence_length, const double* incidence, const double* incidence_factor, int error){
	UserSensor* s = new UserSensor(name, rows, columns, max_range, max_angle, 
				distance_length, distance, distance_factor,
				azimut_length, azimut, azimut_factor,
				incidence_length,  incidence, incidence_factor);

	sensors[name] = s;
	slMessage(DEBUG_ALL,"Building sensor %s finished\n", name);			
	return sensors[name]; 
}

//SensorBuilder::~SensorBuilder(){
//TODO delete all sensors  nee es wird ja nie ein objekt erzeugt
//}
  /*
   * returns the UserSensor with name Name or NULL if no Sensor with that name is found
   */
UserSensor* SensorBuilder::getUserSensor(const char* name){
//	slMessage(DEBUG_ALL, "Searching sensor %s...", name);	

	if (sensors.count(name)>=1) {
		UserSensor* s;
		s = sensors[name];
//	slMessage(DEBUG_ALL, "... sensor %s found.\n", name);	
		return s;
	}
	slMessage(DEBUG_ALL, "sensor %s NOT found!!\n", name);	

	return NULL;
}
/*
UserSensor::UserSensor(){
		slMessage(DEBUG_ALL,"Never call this constructor of UserSensor only to make maps work!\n");
//	assert (1==2);
	// why do maps create items? This will never be called!
}
*/

UserSensor::UserSensor(const char* name, const int rows, const int columns, const double max_range, const double max_angle, 
				const int distance_length, const double* distance, const double* distance_factor,
				const int azimut_length, const double* azimut, const double* azimut_factor,
				const int incidence_length, const double* incidence, const double* incidence_factor){
	slMessage(DEBUG_ALL,"UserSensor Constructor\n");
	this->rows = rows;
	this->columns = columns;
	assert(rows%2==1);
	assert(columns%2==1);
	this->distance_length = distance_length;
	this->azimut_length = azimut_length;
	this->incidence_length = incidence_length;
	this->distance = new pair[distance_length];
	this->azimut = new pair[azimut_length];
	this->incidence = new pair[incidence_length];
	this->max_range = max_range;		
	this->max_angle = max_angle;
	double sensor_width = 2 * sin(max_angle);
	double sensor_height = 2 * sin(max_angle);
	double x_step = sensor_width/(rows-1.0);
	double y_step = sensor_height/(columns-1.0);
	double x_start = -sensor_width/2.0;
	double y_start = -sensor_height/2.0;

	//get memory for rayDirection array
	rayDirections = (slVector**)malloc(rows * sizeof(slVector *));
    assert(rayDirections != NULL);
    /* allocate a: columns*/
    for(int i=0; i < columns; i++) {
		rayDirections[i] = (slVector*)malloc(columns * sizeof(slVector));
		assert(rayDirections[i] != NULL);
    }

	//get memory for rayValues array
	rayValues = (rayData**)malloc(rows * sizeof(rayData *));
    assert(rayValues != NULL);
    /* allocate a: columns*/
    for(int i=0; i < columns; i++) {
		rayValues[i] = (rayData*)malloc(columns * sizeof(rayData));
		assert(rayValues[i] != NULL);
    }

	for(int j=0; j<rows; j++){
		for(int i = 0; i<columns; i++){
			rayDirections[i][j].x = x_start + x_step * i;
			rayDirections[i][j].y = y_start + y_step * j;
			rayDirections[i][j].z = 1;
		//	slVectorNormalize(&rayDirections[i][j]);
		//	slMessage(DEBUG_ALL, " ProximitySensor direction[%d][%d]:(%f, %f, %f)\n", i, j, rayDirections[i][j].x, rayDirections[i][j].y, rayDirections[i][j].z);

			rayValues[i][j].distance = 999999;
		}		
	}
	// generate the normal vectors for the border planes therefore choose the "middle" vector of each side
	inv_perpendicular(&rayDirections[0]             [(rows-1)/2], &baseDirection, &borderNormal[0]);
	inv_perpendicular(&rayDirections[columns-1]     [(rows-1)/2], &baseDirection, &borderNormal[1]);
	inv_perpendicular(&rayDirections[(columns-1)/2] [0],          &baseDirection, &borderNormal[2]);
	inv_perpendicular(&rayDirections[(columns-1)/2] [rows-1],     &baseDirection, &borderNormal[3]);

	// if rows or columns ==1 the rayDirectionsVector is identical with the baseDirection 
	// but we dont want to have the same vector 4 times
	if(columns ==1){
		slVectorSet(&borderNormal[0],1,0,0);
		slVectorSet(&borderNormal[1],-1,0,0);
	}
	if(rows ==1){
		slVectorSet(&borderNormal[2],0,1,0);
		slVectorSet(&borderNormal[3],0,-1,0);
	}


	/**
	 * This array describes the behaviour of the sensor related to the 
	 * distance of the reflecting object. Value is the distance 
	 * factor is the percentage of reflection at this distance
	 */
	for(int i=0; i<distance_length; i++){
		this->distance[i].value  = distance[i];
		this->distance[i].factor = distance_factor[i];
	}
	/**
	 * This array describes the behaviour of the sensor related to the 
	 * distance between the reflecting object and the sensor axis.
	 * (good reflection if the object is straight ahead)
	 * Value is the angle 
	 * factor is the percentage of reflection at this angle
	 *   low %
	 *  /
	 * S--high %
	 *  \
	 *   low %
	 */
	for(int i=0; i<azimut_length; i++){
		this->azimut[i].value  = azimut[i];
		this->azimut[i].factor = azimut_factor[i];
	}
	
	/**
	 * This array describes the behaviour of the sensor related to the 
	 * angle between the face of the reflecting object and the sensor axis.
	 * Value is the angle 
	 * factor is the percentage of reflection at this angle
	 * S--| high %   S--/ low %
	 */
	for(int i=0; i<incidence_length; i++){
		this->incidence[i].value  = incidence[i];
		this->incidence[i].factor = incidence_factor[i];
	}
}

	/*
	* neighbours a vector of slWorldobjects which are tested.
	* position the position of the sensor 
	* 
	*/
double irSense2(vector<slWorldObject*>* neighbors, slPosition *sensorPos, string sensorType){
	Sensor* s;
//	slMessage(DEBUG_ALL, "sensor.cc: irSense2 %s\n",sensorType.c_str());
	s = SensorBuilder::getUserSensor(sensorType.c_str());
	if(s!=NULL){
		return s->sense(neighbors, sensorPos);
	}else{
		slMessage(DEBUG_ALL, "sensor.cc: Sensor: ");
		slMessage(DEBUG_ALL, sensorType.c_str());
		slMessage(DEBUG_ALL, " is not yet created.\n");
	}
	return -1;
}


  /* checks all neighbours if they have a shape completely outside the borderplanes
   * of the sensor given by bordernormal[] it will be skipped.
   *
   */

double Sensor::sense(vector<slWorldObject*>* neighbors, slPosition* sensorPos){
//	slMessage(DEBUG_ALL, " Sensor (wo)*::sense start:\n");
	// for all shapes do sense()
	vector< slWorldObject* >::iterator neigh_iter;
	for(neigh_iter = neighbors->begin(); neigh_iter != neighbors->end(); neigh_iter++) {
		slWorldObject *o = *neigh_iter;
		
		if(o->getShape() != NULL){
			slPosition shapePos = o->getPosition();
			if(!insideSensorBorder(o->getShape(), &shapePos, sensorPos)){
				slMessage(DEBUG_ALL,"outside!\n");
			}
			else{ 
				slMessage(DEBUG_ALL,"inside!\n");
				Sensor::sense(o->getShape(), &shapePos, sensorPos);
			}
		}
	}
	return evaluate();
}



/*
 * this function simulates a sensor located at position pos, facing direction dir, 
 * up is for the orientation
 *
 */
double Sensor::sense( slShape *shape, slPosition *shapePos, slPosition *sensorPos ){
	double distance;
	slVector v1;
	slVector v2;
	slVectorSet(&v1, 0.0, 0.0, 0.0);
	slVectorSet(&v2, 0.0, 0.0, 0.0);
	std::vector< slFace* >::iterator fi;
//	slMessage(DEBUG_ALL, " Sensor::sense start:\n");
	double D, X, Y, Z, k;
	slVector pointOnPlane;
	slVector result_point;
	slVector posToPlane;
	slVector testV;
	slVector dir;
	int update, result, planes = 0;
	double best = -10000;
	double u = 0;
	double dist = 0;


	slVectorSet(&result_point, 0.0, 0.0, 0.0);

	for( fi = ((slShape*)shape)->faces.begin(); fi < ((slShape*)shape)->faces.end(); fi++ ) {
		slFace *f = *fi;
		planes++;
		D = slVectorDot(&f->plane.normal, &f->plane.vertex);
		slPlane wcPlane; //world coordinates
		slVectorInvXform(shapePos->rotation, &f->plane.normal, &wcPlane.normal);
		slVectorAdd(&f->plane.vertex, &shapePos->location, &wcPlane.vertex);

		for(int i=0; i<columns; i++){
			for(int j = 0; j<rows; j++){
				slVectorXform(sensorPos->rotation, &rayDirections[i][j], &dir);
				slVectorNormalize(&dir);
				// P1, P2 points on the line
				// P3 point on the plane
				// N normal of the plane
				// u = (N dot P3-P1) / (N dot P2-P1)
				// u *(P2-P1) = the vector from P1 to the plane

				// plane.vertex relativ to wo position
				slVectorSub(&wcPlane.vertex, &sensorPos->location, &posToPlane);
				u = (slVectorDot(&wcPlane.normal, &posToPlane)/(slVectorDot(&wcPlane.normal, &dir)));
				dist = u ;

				X = wcPlane.normal.x * dir.x;
				Y = wcPlane.normal.y * dir.y;
				Z = wcPlane.normal.z * dir.z;
				k = D/(X+Y+Z);
				if((X+Y+Z) != 0.0 && u > 0.0) {
					// we have the length of the matching vector on the plane of this face. 
					slVectorMul(&dir, u, &pointOnPlane);
					slVectorAdd(&pointOnPlane, &sensorPos->location, &pointOnPlane);
					// now figure out if the point in question is within the face 
					result = slClipPoint(&pointOnPlane, f->voronoi, shapePos, f->edgeCount, &update, &distance);
					if(result == 1) { // point inside the face
						slVectorSub(&pointOnPlane, &sensorPos->location, &testV);
						dist = slVectorLength(&testV);
						slVectorCopy(&pointOnPlane, &result_point);
						if(rayValues [i][j].distance > u){
							rayValues[i][j].distance = u;
							rayValues[i][j].azimut = slVectorAngle(&rayDirections[i][j], &baseDirection);
							rayValues[i][j].incidence = M_PI - slVectorAngle(&dir, &wcPlane.normal);
						}//we have a better hit
					} else if(distance < best) {
						best = distance;
						slVectorCopy(&pointOnPlane, &result_point);
					}
				}
			}
		}
	}
	return 1;
}

/* evaluates the sensed values for all rays according
 * to the ray characteristics. Also resets all values
 * so new sensing is possible
 */
double Sensor::evaluate(){
	double sum, split_factor, result;
	sum = 0;
	split_factor = 1.0/(rows*columns);
//slMessage(DEBUG_ALL, "evaluate rows: %d columns%d \n",rows,columns);
	for(int i=0; i < columns; i++){
		for(int j=0; j < rows; j++){
			result = apply_distance_factor(split_factor, rayValues[i][j].distance);
			result = apply_azimut_factor(result, rayValues[i][j].azimut);
			result = apply_incidence_factor(result, rayValues[i][j].incidence);
			if (rayValues[i][j].distance < 999998)
			slMessage(DEBUG_ALL, "rayValues[%d][%d].dist: %f azimut: %f incid: %f result\n", i, j, rayValues[i][j].distance,rayValues[i][j].azimut / M_PI *180, rayValues[i][j].incidence / M_PI *180);
//			slMessage(DEBUG_ALL, " %3.8f ", result);
	//		slMessage(DEBUG_ALL, " %3.8f ", rayValues[i][j].distance);
			rayValues[i][j].distance = 999999;
			sum += result;
		}
//		slMessage(DEBUG_ALL, "\n", result);
	}
//	slMessage(DEBUG_ALL, "sensor::evaluate sum:%f\n", sum);

	return sum;
}
  /*
   * this function returns false, if all points of the boundingbox are on the "outside of a
   * borderplane defined in borderNormal[i]. This can happen, even if no point is inside all
   * planes, representing a case, in wich the bounding box is larger than the sensor cone
   * so the bounding box points are outside, but parts of the shape are still inside.
   */
bool Sensor::insideSensorBorder( slShape *shape, slPosition *shapePos, slPosition *sensorPos){
	slVector min ,max;
	shape->bounds(shapePos, &min, &max);
//	slVectorPrint(&min);
//	slVectorPrint(&max);

	slVector maximumPoints[8];

	maximumPoints[0].x = min.x;
	maximumPoints[0].y = min.y;
	maximumPoints[0].z = min.z;
	maximumPoints[1].x = max.x;
	maximumPoints[1].y = min.y;
	maximumPoints[1].z = min.z;
	maximumPoints[2].x = min.x;
	maximumPoints[2].y = max.y;
	maximumPoints[2].z = min.z;
	maximumPoints[3].x = max.x;
	maximumPoints[3].y = max.y;
	maximumPoints[3].z = min.z;
	maximumPoints[4].x = min.x;
	maximumPoints[4].y = min.y;
	maximumPoints[4].z = max.z;
	maximumPoints[5].x = max.x;
	maximumPoints[5].y = min.y;
	maximumPoints[5].z = max.z;
	maximumPoints[6].x = min.x;
	maximumPoints[6].y = max.y;
	maximumPoints[6].z = max.z;
	maximumPoints[7].x = max.x;
	maximumPoints[7].y = max.y;
	maximumPoints[7].z = max.z;
	double d;
	slVector n;
	int p;
	for(int i=0; i<5; i++){
		for(p=0; p<8; p++){
			slVectorXform(sensorPos->rotation, &borderNormal[i], &n);
			d = distance_plane_point(&n, &sensorPos->location, &maximumPoints[p]);
//				slMessage(DEBUG_ALL,"i:%d p:%d d: %f\n",i,p,d);
			if (d > 0){ 
				p=0;
				// one Point is inside try next plane
				break;
			}
		}
		// all points are on the "outside" of this plane
		if(p==8) return false;
		
	}
	return true;
}




