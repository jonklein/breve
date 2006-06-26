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

int SensorBuilder::test = 3;
map <int,int> SensorBuilder::testmap = map<int,int>();
map <string, UserSensor*> SensorBuilder::sensors = map<string, UserSensor*>();
//map <const char*, UserSensor*> SensorBuilder::sensors;

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
	
//	slVectorPrint(from);
//	slVectorPrint(to);
//	slVectorPrint(&diff);
	d = slVectorDot(from, &diff);
//		slMessage(DEBUG_ALL,"d:%f\n",d);
	d = d / slVectorLength(from);
//		slMessage(DEBUG_ALL,"d:%f\n",d);
	slVectorMul(&nfrom, d, &proj);
//	slVectorPrint(&proj);

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
	//map<const char*, int> sensors;

  
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
	slVectorSet(&borderNormal[0],0,0,1);
	slVectorSet(&borderNormal[1],1,0,0);
	slVectorSet(&borderNormal[2],-1,0,0);
	slVectorSet(&borderNormal[3],0,1,0);
	slVectorSet(&borderNormal[4],0,-1,0);

}


Singleton* Singleton::pinstance = 0;// initialize pointer
Singleton* Singleton::Instance () {
	if (pinstance == 0){  // is it the first call?
	  pinstance = new Singleton; // create sole instance
	}
	return pinstance; // address of sole instance
}

Singleton::Singleton() 
{ 
	//... perform necessary instance initializations 
}
ProximitySensor* ProximitySensor::pinstance = 0;
ProximitySensor* ProximitySensor::getProximitySensor(){
	if (pinstance == 0){  // is it the first call?
	  pinstance = new ProximitySensor(); // create sole instance
	}
	return pinstance; // address of sole instance
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
	slMessage(DEBUG_ALL,"Building sensor rows:%d\n", sensors[name]->rows);	
		
	return sensors[name]; 
}

//SensorBuilder::~SensorBuilder(){
//TODO delete all sensors  nee es wird ja nie ein objekt erzeugt
//}
  /*
   * returns the UserSensor with name Name or NULL if no Sensor with that name is found
   */
UserSensor* SensorBuilder::getUserSensor(const char* name){
	slMessage(DEBUG_ALL, "Searching sensor %s...", name);	

	if (sensors.count(name)>=1) {
		UserSensor* s;
		s = sensors[name];
	slMessage(DEBUG_ALL, "... sensor %s found.\n", name);	
		return s;
	}
	slMessage(DEBUG_ALL, "... sensor %s NOT found!!\n", name);	

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

void testUserSensor(){
	char* name ="testSensor";
	int distance_length = 5;
	double distance[] = {1.0, 2.0, 3.0, 4.0, 5.0};
	double distance_factor[] = {1.0, 0.9, 0.8, 0.7, 0.6};
	int azimut_length = 5;
	double azimut[] = {1.0, 2.0, 3.0, 4.0, 5.0};
	double azimut_factor[] = {1.0, 0.9, 0.8, 0.7, 0.6};
	int incidence_length = 5;
	double incidence[] = {1.0, 2.0, 3.0, 4.0, 5.0};
	double incidence_factor[] = {1.0, 0.9, 0.8, 0.7, 0.6};
	int x = 3;
	UserSensor* a = SensorBuilder::createUserSensor(name, 5, 5, 300.0, 60*M_PI/180, 
				distance_length, distance, distance_factor,
				azimut_length,  azimut, azimut_factor,
				incidence_length,  incidence, incidence_factor, x);

}


ProximitySensor::ProximitySensor(){
	rows=5;
	columns=5;

	distance_length = 7;
	azimut_length = 9;
	incidence_length = 13;
	distance = new pair[distance_length];
	azimut = new pair[azimut_length];
	incidence = new pair[incidence_length];
	max_range = 300;		
	max_angle = 40*M_PI/180.0;
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
	slMessage(DEBUG_ALL, " ProximitySensor direction[%d][%d]:(%f, %f, %f)\n", i, j, rayDirections[i][j].x, rayDirections[i][j].y, rayDirections[i][j].z);

			rayValues[i][j].distance = 999999;
		}

		
	}

	// generate the normal vectors for the border planes therefore choose the "middle" vector of each side
	inv_perpendicular(&rayDirections[0]             [(rows-1)/2], &baseDirection, &borderNormal[0]);
	inv_perpendicular(&rayDirections[columns-1]     [(rows-1)/2], &baseDirection, &borderNormal[1]);
	inv_perpendicular(&rayDirections[(columns-1)/2] [0],          &baseDirection, &borderNormal[2]);
	inv_perpendicular(&rayDirections[(columns-1)/2] [rows-1],     &baseDirection, &borderNormal[3]);

	/**
	 * This array describes the behaviour of the sensor related to the 
	 * distance of the reflecting object. Value is the distance 
	 * factor is the percentage of reflection at this distance
	 */

	distance[0].value  = 0;
	distance[0].factor = 1;
	distance[1].value  = 50;
	distance[1].factor = 0.9;
	distance[2].value  = 100;
	distance[2].factor = 0.8;
	distance[3].value  = 200;
	distance[3].factor = 0.5;
	distance[4].value  = 300;
	distance[4].factor = 0.3;
	distance[5].value  = 301;
	distance[5].factor = 0.0;
	distance[6].value  = 400;
	distance[6].factor = 0.0;

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
	azimut[0].value  = -60;
	azimut[0].factor =  .2;
	azimut[1].value  = -30;
	azimut[1].factor =  .5;
	azimut[2].value  = -20;
	azimut[2].factor =  .7;
	azimut[3].value  = -10;
	azimut[3].factor =  .8;
	azimut[4].value  =   0;
	azimut[4].factor = 1.0;
	azimut[5].value  =  10;
	azimut[5].factor =  .8;
	azimut[6].value  =  20;
	azimut[6].factor =  .7;
	azimut[7].value  =  30;
	azimut[7].factor =  .5;
	azimut[8].value  =  60;
	azimut[8].factor =  .2;
	
	/**
	 * This array describes the behaviour of the sensor related to the 
	 * angle between the face of the reflecting object and the sensor axis.
	 * Value is the angle 
	 * factor is the percentage of reflection at this angle
	 * S--| high %   S--/ low %
	 */
	incidence[0].value = -180;
	incidence[0].factor =   0;
	incidence[1].value  = -90;
	incidence[1].factor = 0.0;
	incidence[2].value  = -60;
	incidence[2].factor =  .2;
	incidence[3].value  = -30;
	incidence[3].factor =  .4;
	incidence[4].value  = -20;
	incidence[4].factor =  .6;
	incidence[5].value  = -10;
	incidence[5].factor =  .8;
	incidence[6].value  =   0;
	incidence[6].factor = 1.0;
	incidence[7].value  =  10;
	incidence[7].factor =  .8;
	incidence[8].value  =  20;
	incidence[8].factor =  .6;
	incidence[9].value  =  30;
	incidence[9].factor =  .4;
	incidence[10].value =  60;
	incidence[10].factor=  .2;
	incidence[11].value =  90;
	incidence[11].factor=   0;
	incidence[12].value = 180;
	incidence[12].factor=   0;
}

RaySensor::RaySensor(){
//	slMessage(DEBUG_ALL, "sensor: creating RaySensor::RaySensor()\n");

	rows = 1;
	columns = 1;

//	rayDirections = new slVector[rows][columns];
	max_range = 300;		
	max_angle = 60*M_PI/180.0;

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

	rayDirections[0][0].x = 0;
	rayDirections[0][0].y = 0;
	rayDirections[0][0].z = 1;
	slVectorNormalize(&rayDirections[0][0]);
	rayValues[0][0].distance = 99999999;

	/**
	 * This array describes the behaviour of the sensor related to the 
	 * distance of the reflecting object. Value is the distance 
	 * factor is the percentage of reflection at this distance
	 */
	distance_length = 2;
	distance = new pair[distance_length];

	distance[0].value  = 0;
	distance[0].factor = 1;
	distance[1].value  = 100;
	distance[1].factor = 1;


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
	azimut_length = 1;
	azimut = new pair[azimut_length];

	azimut[0].value  =   0;
	azimut[0].factor = 1.0;

	
	/**
	 * This array describes the behaviour of the sensor related to the 
	 * angle between the face of the reflecting object and the sensor axis.
	 * Value is the angle 
	 * factor is the percentage of reflection at this angle
	 * S--| high %   S--/ low %
	 */
	incidence_length = 13;
	incidence = new pair[incidence_length];

	incidence[0].value = -180;
	incidence[0].factor =   0;
	incidence[1].value  = -90;
	incidence[1].factor = 0.0;
	incidence[2].value  = -60;
	incidence[2].factor =  .2;
	incidence[3].value  = -30;
	incidence[3].factor =  .4;
	incidence[4].value  = -20;
	incidence[4].factor =  .6;
	incidence[5].value  = -10;
	incidence[5].factor =  .8;
	incidence[6].value  =   0;
	incidence[6].factor = 1.0;
	incidence[7].value  =  10;
	incidence[7].factor =  .8;
	incidence[8].value  =  20;
	incidence[8].factor =  .6;
	incidence[9].value  =  30;
	incidence[9].factor =  .4;
	incidence[10].value =  60;
	incidence[10].factor=  .2;
	incidence[11].value =  90;
	incidence[11].factor=   0;
	incidence[12].value = 180;
	incidence[12].factor=   0;
}
	/*
	* neighbours a vector of slWorldobjects which are tested.
	* position the position of the sensor 
	* up the upwards direction of the sensor
	* direction the direction the sensor is facing
	* type the type of the sensor RAY PROXIMITY PERCEPTION
	* RAY is a single ray
	* PROXIMITY is a 2-d array of rays
	*/
double irSense2(vector<slWorldObject*>* neighbors, slPosition *sensorPos, string sensorType){
	Sensor* s;
	if(sensorType =="PROXIMITY"){
		s = ProximitySensor::getProximitySensor();
		return s->sense(neighbors, sensorPos);
	}else if(sensorType == "RAY"){
		RaySensor rs;
		s = &rs;
		return s->sense(neighbors, sensorPos);
	}else if(sensorType == "PERCEPTION"){
		return -1;
	}else {
//		slMessage(DEBUG_ALL, "sensor.cc: irSense2 %s\n",sensorType.c_str());
		s = SensorBuilder::getUserSensor(sensorType.c_str());
//		slMessage(DEBUG_ALL, "sensor.cc: irSense2\n");
		if(s!=NULL){
			return s->sense(neighbors, sensorPos);
		}else{
			slMessage(DEBUG_ALL, "sensor.cc: Sensor: ");
			slMessage(DEBUG_ALL, sensorType.c_str());
			slMessage(DEBUG_ALL, " is not yet supported. Choose RAY, PROXIMITY or USERSENSOR\n");
		}
		return -1;
	}
	return -1; // This statement will never be reached, but without i get a warning strange
}

  /*
   * this function returns false, if all points of the boundingbox are on the "outside of a
   * borderplane defined in borderNormal[i]. This can happen, even if no point is inside all
   * planes, representing a case, in wich the bounding box is larger than the sensor cone
   * so the bounding box points are outside, but parts of the shape are still inside.
   */
bool Sensor::insideSensorBorder(const slShape *shape, slPosition *shapePos, slPosition *sensorPos){
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

double Sensor::sense(vector<slWorldObject*>* neighbors, slPosition* sensorPos){
//	slMessage(DEBUG_ALL, " Sensor (wo)*::sense start:\n");
	// for all shapes do sense()
	vector< slWorldObject* >::iterator neigh_iter;
	//	for(fi = faces.begin(); fi != faces.end(); fi++ ) {
	for(neigh_iter = neighbors->begin(); neigh_iter != neighbors->end(); neigh_iter++) {

//	slMessage(DEBUG_ALL, " Sensor (wo)*::sense call:\n");
		slWorldObject *o = *neigh_iter;
		
		if(o->getShape() != NULL){
			slPosition shapePos = o->getPosition();
			if(!insideSensorBorder(o->getShape(), &shapePos, sensorPos))	slMessage(DEBUG_ALL,"outside!\n");
			else{ 
					slMessage(DEBUG_ALL,"inside!\n");

				Sensor::sense(o->getShape(), &shapePos, sensorPos);
			}
		}//	slFace *f = *fi;
	}


	return evaluate();
}



/*
 * this function simulates a sensor located at position pos, facing direction dir, up is fpr the orientation
 * 
 *
 *
 */
//double Sensor::sense(const slShape *shape, slPosition *shapePos, slVector *pos,  slVector *dir, slVector *up){
double Sensor::sense(const slShape *shape, slPosition *shapePos, slPosition *sensorPos){
	double distance;
	slVector v1;
	slVector v2;
	slVectorSet(&v1, 0.0, 0.0, 0.0);
	slVectorSet(&v2, 0.0, 0.0, 0.0);
	vector<slFace*>::iterator fi;
//	slMessage(DEBUG_ALL, " Sensor::sense start:\n");
	//schnitt ebene vector
	
//int slShape::pointOnShape(slVector *dir, slVector *point) {
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

	for(fi = (shape->faces.begin()); fi != (shape->faces.end()); fi++ ) {
		slFace *f = *fi;
		planes++;
		D = slVectorDot(&f->plane.normal, &f->plane.vertex);
		slPlane wcPlane; //world coordinates
		slVectorInvXform(shapePos->rotation, &f->plane.normal, &wcPlane.normal);
		slVectorAdd(&f->plane.vertex, &shapePos->location, &wcPlane.vertex);

		for(int i=0; i<columns; i++){
			for(int j = 0; j<rows; j++){
//				slMessage(DEBUG_ALL, "sensor: accessing rayValues[%d][%d].distance:%f\n",i,j,k);
				//change direction
				// rayDirections have the length 1  and rotMatrix is only a rotation matrix so dir should be of the length 1 too
				slVectorXform(sensorPos->rotation, &rayDirections[i][j], &dir);
				slVectorNormalize(&dir);
//				slMessage(DEBUG_ALL, "sensor:[%d, %d] resulting dir vector[%f, %f, %f]\n", i, j, dir->x, dir->y, dir->z);

				// 	slMessage(DEBUG_ALL,"d = %f\n", D);
//				slMessage(DEBUG_ALL, "sensor: accessing rayValues[%d][%d].distance:%f\n",i,j,k);

				//P1, P2 points on the line
				// P3 point on the plane
				// N normal of the plane
				// u = (N dot P3-P1) / (N dot P2-P1)
				// u *(P2-P1) = the vector from P1 to the plane

				// plane.vertex relativ zu wo position
				slVectorSub(&wcPlane.vertex, &sensorPos->location, &posToPlane);

//				slVectorAdd(&posToPlane, &shapePos->location ,&posToPlane);
//				slVectorXform(shapePos->rotation, pos, pos);

				u = (slVectorDot(&wcPlane.normal, &posToPlane)/(slVectorDot(&wcPlane.normal, &dir)));
				dist = u ;//* slVectorlength(dir); // normalize dir!

				X = wcPlane.normal.x * dir.x;
				Y = wcPlane.normal.y * dir.y;
				Z = wcPlane.normal.z * dir.z;

				k = D/(X+Y+Z);
 
				if((X+Y+Z) != 0.0 && u > 0.0) {
//				slMessage(DEBUG_ALL, "sensor: accessing rayValues[%d][%d].distance:%f\n",i,j,k);

					// we have the length of the matching vector on the plane of this face. 

					slVectorMul(&dir, u, &pointOnPlane);
					slVectorAdd(&pointOnPlane, &sensorPos->location, &pointOnPlane);

					// distance = slPlaneDistance(&f->plane, &pointOnPlane);

					// now figure out if the point in question is within the face 

					result = slClipPoint(&pointOnPlane, f->voronoi, shapePos, f->edgeCount, &update, &distance);
//slMessage(DEBUG_ALL, "sensor sense plane:[%f, %f, %f] vector: [%f, %f, %f] position:[%f, %f, %f] distance: %f result: %d\n", f->plane.vertex.x, f->plane.vertex.y, f->plane.vertex.z, dir->x, dir->y, dir->z,pos->x,pos->y,pos->z, dist, result);
					// if this point is within the voronoi region of the plane, it must be on the face.
					// even if it's not, it might be due to small mathematical error, so we'll keep track
					// of the best of the failures.
//						slMessage(DEBUG_ALL, "sensor: result: %d distance:%f\n",result, k);
					//	line()
//						slMessage(DEBUG_ALL, "sensor: result: %d \n",result);

					//slObjectLine *slWorld::addObjectLine( slWorldObject *src, slWorldObject *dst, int stipple, slVector *color ) {
					if(result == 1) {
//				slMessage(DEBUG_ALL, "sensor plane.vertex [%f, %f, %f]\n", wcPlane.vertex.x,wcPlane.vertex.y,wcPlane.vertex.z);
//				slMessage(DEBUG_ALL, "sensor position [%f, %f, %f]\n", pos->x,pos->y,pos->z);
//				slMessage(DEBUG_ALL, "sensor distance u: %f \n", u);
//				slMessage(DEBUG_ALL, "sensor posToPlane: %f \n", slVectorlength(&posToPlane));
						
					slVectorSub(&pointOnPlane, &sensorPos->location, &testV);
					dist = slVectorLength(&testV);
//				slMessage(DEBUG_ALL, "sensor distance2: %f \n", dist);

						slVectorCopy(&pointOnPlane, &result_point);
						//return 0;
						if(rayValues [i][j].distance > u){
	//						slMessage(DEBUG_ALL, "sensor: setting rayValues[%d][%d].distance:%f\n",i,j,u);
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

	//	slMessage(DEBUG_ALL,"warning: no shape xxx point could be found\n");
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

