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
using std::vector;
using std::string;

std::map <string, UserSensor*> SensorBuilder::sensors = std::map<string, UserSensor*>();

double distance_plane_point( const slVector *normal, const slVector *planePoint, const slVector *point ){
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
void inv_perpendicular( const slVector *from, const slVector *to, slVector *result ){
	slVector diff;
	slVector proj;
	slVector nfrom;
	double d;
//	slMessage(DEBUG_ALL,"inv_perpendicular\n");
	slVectorCopy(from, &nfrom);
	slVectorNormalize(&nfrom);
	slVectorSub(to, from, &diff);	
	d = slVectorDot(from, &diff);
	d = d / slVectorLength(from);
	slVectorMul(&nfrom, d, &proj);
	slVectorSub(&diff, &proj, result);
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

double Sensor::apply_distance_factor(const double input, const double dist){
	return input * interpolate(distance, dist, distance_length);
}

double Sensor::apply_azimut_factor(const double input, const double angle){
	return input * interpolate(azimut, angle, azimut_length);
}

double Sensor::apply_incidence_factor(const double input, const double angle){
	// angle is the difference between the normal vector of the face and the 
	// vector from the sensor to the face angle=0 = good reflection
	return input * interpolate(incidence, angle, incidence_length);

}

Sensor::Sensor(){
//	slMessage(DEBUG_ALL,"Sensor Constructor\n");
	slVectorSet(&baseDirection, 0, 0, 1);
	// generate the normal vectors for the border planes therefore choose the "middle" vector of each side
	slVectorSet(&borderNormal[0],1,0,0);
	slVectorSet(&borderNormal[1],-1,0,0);
	slVectorSet(&borderNormal[2],0,1,0);
	slVectorSet(&borderNormal[3],0,-1,0);
	slVectorSet(&borderNormal[4],0,0,1);
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
				const int incidence_length, const double* incidence, const double* incidence_factor){
	// check if a sensor with this name already exists and delete it
//	slMessage(DEBUG_ALL,"UserBuilder createUserSensor\n");

	std::map <string, UserSensor*>::iterator iter = sensors.find(name);
	if(iter != sensors.end()){
	//if(iter->first == name){
		slMessage(DEBUG_ALL,"deleting sensor %s \n", name);			
		UserSensor* d = sensors[iter->first];
		delete d;
		sensors.erase(iter);
	}
	UserSensor* s = new UserSensor(name, rows, columns, max_range, max_angle, 
				distance_length, distance, distance_factor,
				azimut_length, azimut, azimut_factor,
				incidence_length,  incidence, incidence_factor);

	sensors[name] = s;
//	slMessage(DEBUG_ALL,"Building sensor %s finished\n", name);			
	return sensors[name]; 
}


  /*
   * returns the UserSensor with name Name or NULL if no Sensor with that name is found
   */
UserSensor* SensorBuilder::getUserSensor(const char* name){
//		slMessage(DEBUG_ALL,"UserBuilder getUserSensor\n");

//	slMessage(DEBUG_ALL, "Searching sensor %s...", name);	

//	if(!sensors.empty())
//        printf( "MyMap has %d", sensors.size() );
	if (sensors.count(name)>=1) {
		UserSensor* s;
		s = sensors[name];
//	slMessage(DEBUG_ALL, "... sensor %s found.\n", name);	
		return s;
	}
	slMessage(DEBUG_ALL, "sensor %s NOT found!!\n", name);	

	return NULL;
}


UserSensor::UserSensor(const char* name, const int rows, const int columns, const double max_range, const double max_angle, 
				const int distance_length, const double* distance, const double* distance_factor,
				const int azimut_length, const double* azimut, const double* azimut_factor,
				const int incidence_length, const double* incidence, const double* incidence_factor){
//	slMessage(DEBUG_ALL,"UserSensor Constructor\n");
	this->rows = rows;
	this->columns = columns;
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
	//We have only one Ray
	if (rows ==1){ x_start = 0; x_step = 0; }
	if (columns ==1){ y_start = 0; y_step = 0;}

	//get memory for rayDirection array
	rayDirections = (slVector**)malloc(rows * sizeof(slVector *));
//    assert(rayDirections != NULL);
    /* allocate a: columns*/
    for(int i=0; i < columns; i++) {
		rayDirections[i] = (slVector*)malloc(columns * sizeof(slVector));
//		assert(rayDirections[i] != NULL);
    }

	//get memory for rayValues array
	rayValues = (rayData**)malloc(rows * sizeof(rayData *));
//    assert(rayValues != NULL);
    /* allocate a: columns*/
    for(int i=0; i < columns; i++) {
		rayValues[i] = (rayData*)malloc(columns * sizeof(rayData));
//		assert(rayValues[i] != NULL);
    }

	for(int j=0; j<rows; j++){
		for(int i = 0; i<columns; i++){
			rayDirections[i][j].x = x_start + x_step * i;
			rayDirections[i][j].y = y_start + y_step * j;
			rayDirections[i][j].z = 1;
			slVectorNormalize(&rayDirections[i][j]);
//			slMessage(DEBUG_ALL, " UserSensor direction[%d][%d]:(%f, %f, %f)\n", i, j, rayDirections[i][j].x, rayDirections[i][j].y, rayDirections[i][j].z);
			rayValues[i][j].azimut = slVectorAngle(&rayDirections[i][j], &baseDirection);
			rayValues[i][j].distance = 999999;
		}		
	}
	// generate the normal vectors for the border planes therefore choose the "middle" vector of each side
	inv_perpendicular(&rayDirections[0]             [(rows-1)/2], &baseDirection, &borderNormal[0]);
	inv_perpendicular(&rayDirections[columns-1]     [(rows-1)/2], &baseDirection, &borderNormal[1]);
	inv_perpendicular(&rayDirections[(columns-1)/2] [0],          &baseDirection, &borderNormal[2]);
	inv_perpendicular(&rayDirections[(columns-1)/2] [rows-1],     &baseDirection, &borderNormal[3]);
	slVectorSet(&borderNormal[4],0,0,1);
	// if rows or columns ==1 the rayDirectionsVector is identical with the baseDirection 
	// but we dont want to have the same vector 4 times
	// to use the optimal vectors use 			insideSensorBorder(o->getShape(), &shapePos, sensorPos, true)
	// everything not hit by the ray will be discarded
	slVector tempBorder;
	if(columns ==1){
		tempBorder.z = 1;
		tempBorder.x = sensor_width;
		tempBorder.y = sensor_height/2.0;
		inv_perpendicular(&tempBorder, &baseDirection, &borderNormal[0]);
		tempBorder.x = sensor_width/2.0;
		tempBorder.y = sensor_height/2.0;
		inv_perpendicular(&tempBorder, &baseDirection, &borderNormal[1]);
		tempBorder.x = sensor_width;
		tempBorder.y = sensor_height;
		inv_perpendicular(&tempBorder, &baseDirection, &borderNormal[2]);
		tempBorder.x = sensor_width/2.0;
		tempBorder.y = sensor_height;
		inv_perpendicular(&tempBorder, &baseDirection, &borderNormal[3]);
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
	bool single = (rows ==1);
	for(neigh_iter = neighbors->begin(); neigh_iter != neighbors->end(); neigh_iter++) {

		slWorldObject *o = *neigh_iter;
		if(o->getShape() != NULL){
			slPosition shapePos = o->getPosition();
			if(insideSensorBorder(o->getShape(), &shapePos, sensorPos, single)){
			//	slMessage(DEBUG_ALL,"inside!\n");
				Sensor::sense(o->getShape(), &shapePos, sensorPos);
			}
			else{ 
			//		slMessage(DEBUG_ALL,"outside!\n");
			}
		}
	}
	return evaluate();
}






/*
 * this function simulates a sensor located at sensorPos
 * shapePos the Position of the shape that will be sensed
 */
void Sensor::sense(const slShape *shape, slPosition *shapePos, slPosition *sensorPos){
	double distance;
	vector<slFace*>::const_iterator fi;
	slVector pointOnPlane;
	slVector posToPlane;
	slVector dir;
	slVector middleDirection;
	int middle;
	int update, result, planes = 0;
	double dist= 0;

	middle = rows/2;
	slVectorXform(sensorPos->rotation, &rayDirections[middle][middle], &middleDirection);
	slVectorNormalize(&middleDirection);

	for(fi = (shape->faces.begin()); fi != (shape->faces.end()); fi++ ) {
		slFace *f = *fi;
		planes++;
		slPlane wcPlane; //world coordinates
		slVectorXform(shapePos->rotation, &f->plane.normal, &wcPlane.normal);
		slVectorAdd(&f->plane.vertex, &shapePos->location, &wcPlane.vertex);
		for(int i = 0; i<columns; i++){
			for(int j = 0; j<rows; j++){
				slVectorXform(sensorPos->rotation, &rayDirections[i][j], &dir);
				slVectorNormalize(&dir);
				// P1, P2 points on the line
				// P3 point on the plane
				// N normal of the plane
				// dist= (N dot P3-P1) / (N dot P2-P1)
				// dist*(P2-P1) = the vector from P1 to the plane

				// plane.vertex relativ to wo position
				slVectorSub(&wcPlane.vertex, &sensorPos->location, &posToPlane);

				// the face is facing the wrong direction
				if( (M_PI/2) > (slVectorAngle(&dir, &wcPlane.normal)) || (slVectorAngle(&dir, &wcPlane.normal) > M_PI)){continue;}
			
				if(fabs(slVectorDot(&wcPlane.normal, &dir))<0.00001){continue;} // the ray is parallel to the plane so it wont hit
				else{
					dist = (slVectorDot(&wcPlane.normal, &posToPlane)/(slVectorDot(&wcPlane.normal, &dir)));
				}

				if(dist > 0.0) {
					// we have the length of the matching vector on the plane of this face. 
					slVectorMul(&dir, dist, &pointOnPlane);
					slVectorAdd(&pointOnPlane, &sensorPos->location, &pointOnPlane);
					// now figure out if the point in question is within the face 
					result = slClipPoint(&pointOnPlane, f->voronoi, shapePos, f->edgeCount, &update, &distance);
					if(result == 1) { // point inside the face
						if(rayValues [i][j].distance > dist){//we have a better hit
							rayValues[i][j].distance = dist;
							rayValues[i][j].incidence = M_PI- slVectorAngle(&middleDirection, &wcPlane.normal);
						//	slMessage(DEBUG_ALL, "%f ",rayValues[i][j].incidence*180/M_PI);
						//slMessage(DEBUG_ALL, " normal: (%f, %f, %f) ",wcPlane.normal.x, wcPlane.normal.y, wcPlane.normal.z );
						//slMessage(DEBUG_ALL, " middleDir: (%f, %f, %f) ",middleDirection.x, middleDirection.y, middleDirection.z );
						}
					}
				}
			}//rows
		}//collumns
	}//faces
}

/* evaluates the sensed values for all rays according
 * to the ray characteristics. Also resets all values
 * so new sensing is possible
 */
double Sensor::evaluate(){
	double sumValues, sumFactors, maxValue, result, factor;
	sumValues = 0;
	sumFactors = 0;
	int verbose = 0;
	double frontDistance = 0;
	maxValue = 1;
//slMessage(DEBUG_ALL, "evaluate rows: %d columns%d \n",rows,columns);
	for(int i=0; i < columns; i++){
		for(int j=0; j < rows; j++){
			factor = apply_azimut_factor(1, rayValues[i][j].azimut);
			sumFactors += factor;
			if (rayValues[i][j].distance > 999998){continue;}
			result = maxValue;
			result = factor* maxValue;
			
			if((i ==columns/2)&&(j ==rows/2)||rows==1) { 
				frontDistance = rayValues[i][j].distance;
			}else{
				frontDistance = sqrt(pow(rayValues[i][j].distance, 2)-pow(sin(rayValues[i][j].azimut)*rayValues[i][j].distance, 2) );
			}
			if(verbose ==2){
				slMessage(DEBUG_ALL, "rayValues[%d][%d]:\n",i,j);
				slMessage(DEBUG_ALL, "value: %.3f azimut:%.3f ",maxValue, rayValues[i][j].azimut);
				
				slMessage(DEBUG_ALL, "value: %.3f\n",result);
				slMessage(DEBUG_ALL, "value: %.3f dist:%.3f ", result, frontDistance);
				//rayValues[i][j].distance);
			}
			if(verbose==1)
				if((i ==columns/2)&&(j ==rows/2)||rows==1)	slMessage(DEBUG_ALL, "%.3f	",rayValues[i][j].distance);
			//laenge der strahlen auf die laenge des mittleren umrechnen falls es nciht shco nder mittelre ist

			result = apply_distance_factor(result, frontDistance);
			if(verbose ==2){
				slMessage(DEBUG_ALL, "value: %.3f\n",result);
				slMessage(DEBUG_ALL, "value: %.3f inc:%.3f  ",result, rayValues[i][j].incidence*180/M_PI);
			}
			result = apply_incidence_factor(result, rayValues[i][j].incidence);
			if(verbose ==2){
				slMessage(DEBUG_ALL, "value: %.3f\n",result);
			}
			rayValues[i][j].distance = 999999;
			sumValues += result;
			
		}
	}
	if(verbose ==2)
		slMessage(DEBUG_ALL, "sensor::evaluate sumValues:%f sumFactors:%f sumV/F: %f\n", sumValues, sumFactors, sumValues/sumFactors);

	return sumValues/sumFactors ;
}

  /*
   * this function returns false, if all points of the boundingbox are on the "outside of a
   * borderplane defined in borderNormal[i]. This can happen, even if no point is inside all
   * planes, representing a case, in wich the bounding box is larger than the sensor cone
   * so the bounding box points are outside, but parts of the shape are still inside.
   */

bool Sensor::insideSensorBorder( const slShape *shape, const slPosition *shapePos, const slPosition *sensorPos, bool singleRay ) const {
	slVector min ,max;
	shape->bounds( shapePos, &min, &max );

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
		slVector singleBorder[5];
	if(singleRay){
		slVectorSet(&singleBorder[0],1,0,0);
		slVectorSet(&singleBorder[1],-1,0,0);
		slVectorSet(&singleBorder[2],0,1,0);
		slVectorSet(&singleBorder[3],0,-1,0);
		slVectorSet(&singleBorder[4],0,0,1);
	}
	for(int i=0; i<5; i++){
		if(singleRay){
			slVectorXform(sensorPos->rotation, &singleBorder[i], &n);
		}else{
			slVectorXform(sensorPos->rotation, &borderNormal[i], &n);
		}
//		slVectorPrint(&n);
		for(p=0; p<8; p++){
			d = distance_plane_point( &n, &sensorPos->location, &maximumPoints[p] );
//				slMessage(DEBUG_ALL,"i:%d p:%d d: %f\n",i,p,d);
			if (d > 0){ 
				p = 0;//last point
				// one Point is inside try next plane
				break;
			}
		}
		// all points are on the "outside" of this plane
		if(p==8) {
//			printf("outside\n");
			return false;
		}
	}
//	printf("inside\n");
	return true;
}

/*
 * checks, if the path from sensorpos->Loc  to targetLoc is blocked by the Shape or not
 * 
 */

bool Sensor::freePath(vector<slWorldObject*>* neighbors, slPosition* sensorPos, slVector* targetLoc, slWorldObject* target = NULL){
	vector< slWorldObject* >::iterator neigh_iter;
	bool isTarget = false;
	double shortestDist = 999999;
	double targetDist = 999999;
	double distance;
	vector<slFace*>::const_iterator fi;
	slVector pointOnPlane;
	slVector posToPlane;
	slVector dir;
	slShape const *shape;
	int update, result = 0;
	double dist= 0;
	bool exact = false;
	if (target!=NULL){
		exact = true;
	}

	slVectorSub(targetLoc, &sensorPos->location, &dir);

	for(neigh_iter = neighbors->begin(); neigh_iter != neighbors->end(); neigh_iter++) {
  		slWorldObject *o = *neigh_iter;
		shape = o->getShape();
		isTarget = (o==target);
		if(shape == NULL){continue;}
		const slPosition *shapePos = &o->getPosition();

		if( !insideSensorBorder( shape, shapePos, sensorPos ) ) {
			//if singleray was true we could only send to agents hit by the ray
			continue;
		}	

		for( fi = ( shape->faces.begin() ); fi != ( shape->faces.end() ); fi++ ) {
			const slFace *f = *fi;
			slPlane wcPlane; //world coordinates

			slVectorXform(shapePos->rotation, &f->plane.normal, &wcPlane.normal);

			slVectorXform(shapePos->rotation, &f->plane.vertex, &wcPlane.vertex);
			slVectorAdd(&wcPlane.vertex, &shapePos->location, &wcPlane.vertex);
/*
			slPlane wcPlane2; //world coordinates
			slPositionPlane( shapePos, &f->plane, &wcPlane2);
			printf("wcplane.normal:");
			slVectorPrint(&wcPlane.normal);
			printf("wcplane.vertex:");
			slVectorPrint(&wcPlane.vertex);
			printf("wcplane2.normal:");
			slVectorPrint(&wcPlane2.normal);
			printf("wcplane2.vertex:");
			slVectorPrint(&wcPlane2.vertex);
*/

			slVectorNormalize(&dir);

//printf(" slVectorAngle(&middleDirection, &wcPlane.normal %f\n",( slVectorAngle(&dir, &wcPlane.normal))*180/M_PI);
			// the face is facing the wrong direction
//		if( (M_PI/2) > (slVectorAngle(&dir, &wcPlane.normal)) || (slVectorAngle(&dir, &wcPlane.normal) > M_PI)){continue;}

			// P1, P2 points on the line
			// P3 point on the plane
			// N normal of the plane
			// dist= (N dot P3-P1) / (N dot P2-P1)   watch out maybe /~0
			// dist*(P2-P1) = the vector from P1 to the plane

			// plane.vertex relativ to wo position
			slVectorSub(&wcPlane.vertex, &sensorPos->location, &posToPlane);
			if(fabs(slVectorDot(&wcPlane.normal, &dir))<0.00001){continue;} // the ray is parallel to the plane so it wont hit
			//slPlaneDistance
			dist = (slVectorDot(&wcPlane.normal, &posToPlane)/(slVectorDot(&wcPlane.normal, &dir)));
			if(dist < 0.0) {continue;}
			// we have the length of the matching vector on the plane of this face. 
			slVectorMul(&dir, dist, &pointOnPlane);
			slVectorAdd(&pointOnPlane, &sensorPos->location, &pointOnPlane);
			// now figure out if the point in question is within the face 
			result = slClipPoint(&pointOnPlane, f->voronoi, shapePos, f->edgeCount, &update, &distance);
			if(result != 1) { continue;}

			// point inside the face
			if(exact){
				if( dist < shortestDist ){ shortestDist = dist;}
				if(isTarget){
					if(dist < targetDist){targetDist = dist;}
					if(targetDist > shortestDist + 0.1){
//				printf("shortestDist: %f targetDist: %f\n",shortestDist, targetDist);
						return false;
					}
					//if(targetDist + 0.1 > shortestDist){continue;} use this if you want konvex shapes
				}else{
					if((shortestDist != 999999)&&(targetDist !=999999)){
						if(targetDist > shortestDist + 0.1){
//				printf("shortestDist: %f targetDist: %f\n",shortestDist, targetDist);
							return false;
						}
					}
				}
			}else{
				printf("not exact!");
				slVector t;
				slVectorSub(targetLoc, &sensorPos->location, &t);
//					printf ("ray-lenght: %f distance(obj->obj):%f\n", dist, slVectorLength(&t));
				if(dist + 2.5 < (slVectorLength(&t))){//2.5 is the robotsize
					return false;
				}
			}
		}//shapes
	}//neighbours

	if((targetDist ==999999)){
//		printf("missed the target!\n");
//		printf("shortestDist: %f targetDist: %f\n",shortestDist, targetDist);
		// that happens if the shape is at the very border of the sensor
		return false;
	}
	return true;
}



	/*
   * Returns the quality of the signal snet from sensorPos, received at targetPos
   * quality is 0 if the distance is too high, the angle too big 
   */
double Sensor::calculateQuality(vector<slWorldObject*>* neighbors, slPosition* sensorPos, slVector* targetLoc, slWorldObject* target = NULL){

	double quality = 1;
	int verbose = 0	;
	slVector dist;
	slVector middleDirection;
	
	slVectorSub(targetLoc, &sensorPos->location, &dist);
	double distance = slVectorLength(&dist);
	distance = distance/2.0; //the light must only go half the way (if reflecting it must go back)
	quality = apply_distance_factor(quality,  distance);
	//slMessage(DEBUG_ALL,"distance:%f quality:%f \n", distance, quality);
	if (fabs(quality)<0.00001){
			if(verbose>1)slMessage(DEBUG_ALL,"distance too big!\n");
		return 0;
	}
	double angle = 0;
	int middle;
	middle = rows/2;
	slVectorXform(sensorPos->rotation, &rayDirections[middle][middle], &middleDirection);
	angle = slVectorAngle(&middleDirection, &dist);
	quality = apply_azimut_factor(quality, angle);

	//slMessage(DEBUG_ALL,"angle:%f quality:%f \n", angle, quality);
	if (fabs(quality)<0.00001){
			if(verbose>1)slMessage(DEBUG_ALL,"angle does not fit!\n");
			return 0;
	}
	if(freePath(neighbors, sensorPos, targetLoc, target)){
		if(verbose>0)slMessage(DEBUG_ALL,"quality path free\n");
		return quality;
	}else {
		if(verbose>0)slMessage(DEBUG_ALL,"quality path blocked\n");
		return 0;
	}

}

double calculateQuality2(vector<slWorldObject*>* neighbors, slPosition* sensorPos, slVector* targetLoc, std::string sensorType, slWorldObject* target = NULL){
	Sensor* s;
	s = SensorBuilder::getUserSensor(sensorType.c_str());
	if(s!=NULL){
		return s->calculateQuality(neighbors, sensorPos, targetLoc, target);
	}else{
		slMessage(DEBUG_ALL, "sensor.cc: Sensor: ");
		slMessage(DEBUG_ALL, sensorType.c_str());
		slMessage(DEBUG_ALL, " is not yet created.\n");
	}
	return -1;
}

	/*
   * Returns the quality of the signal snet from sensorPos, received at targetPos
   * quality is 0 if the distance is too high, the angle too big 
   */
double Sensor::calcQualNoRay(vector<slWorldObject*>* neighbors, slPosition* sensorPos, slVector* targetLoc, slWorldObject* target = NULL){

	double quality = 1;
	int verbose = 0	;
	slVector dist;
	slVector middleDirection;
	
	slVectorSub(targetLoc, &sensorPos->location, &dist);
	double distance = slVectorLength(&dist);
	distance = distance/2.0; //the light must only go half the way (if reflecting it must go back)
	quality = apply_distance_factor(quality,  distance);
	//slMessage(DEBUG_ALL,"distance:%f quality:%f \n", distance, quality);
	if (fabs(quality)<0.00001){
			if(verbose>1)slMessage(DEBUG_ALL,"distance too big!\n");
		return 0;
	}
	double angle = 0;
	int middle;
	middle = rows/2;
	slVectorXform(sensorPos->rotation, &rayDirections[middle][middle], &middleDirection);
	angle = slVectorAngle(&middleDirection, &dist);
	quality = apply_azimut_factor(quality, angle);

	//slMessage(DEBUG_ALL,"angle:%f quality:%f \n", angle, quality);
	if (fabs(quality)<0.00001){
			if(verbose>1)slMessage(DEBUG_ALL,"angle does not fit!\n");
			return 0;
	}
	return quality;

}

double calcQualNoRay2(vector<slWorldObject*>* neighbors, slPosition* sensorPos, slVector* targetLoc, std::string sensorType, slWorldObject* target = NULL){
	Sensor* s;
	s = SensorBuilder::getUserSensor(sensorType.c_str());
	if(s!=NULL){
		return s->calcQualNoRay(neighbors, sensorPos, targetLoc, target);
	}else{
		slMessage(DEBUG_ALL, "sensor.cc: Sensor: ");
		slMessage(DEBUG_ALL, sensorType.c_str());
		slMessage(DEBUG_ALL, " is not yet created.\n");
	}
	return -1;
}
