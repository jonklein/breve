#include <map>
using std::map;
using std::vector;
using std::string;

struct rayData{
	double distance;
	double azimut;
	double incidence;
};

struct pair{
    double value;
    double factor;
};

class Sensor{
	public:
		double sense(std::vector<slWorldObject*>* neighbors, slPosition *sensorPos);
		double sense(slVector *pos, slVector *dir);
		double Sensor::calculateQuality(vector<slWorldObject*>* neighbors, slPosition* sensorPos, slVector* targetLoc, slWorldObject* target );
		bool Sensor::freePath(const slShape *shape, slPosition *shapePos, slPosition *sensorPos, slVector *targetLoc, bool exact);
		bool Sensor::freePath(vector<slWorldObject*>* neighbors, slPosition* sensorPos, slVector* to, slWorldObject* target );
		double max_range;
		double max_angle;

	~Sensor(){
		printf("Sensor destructor!\n");
		free (rayDirections);
		free (rayValues);
		delete[] distance;
		delete[] azimut;
		delete[] incidence;
	}
		
	protected:
		Sensor();
		double apply_distance_factor(double input, double dist);	
		double apply_azimut_factor(double input, double angle);
		double apply_incidence_factor(double input, double angle);
		double sense(const slShape *shape, slPosition *shapePos, slPosition *sensorPos );
		bool insideSensorBorder(const slShape *shape, slPosition *shapePos, slPosition *sensorPos);
		slVector baseDirection;
		slVector baseUpDirection;
		slVector borderNormal[5];
  
		int distance_length;
		int azimut_length;
		int incidence_length;
		int rows;
		int columns;
		
		pair* distance;
		pair* azimut;
		pair* incidence;//angle of incidence = Einfallswinkel
		slVector** rayDirections;
		rayData** rayValues;


	private:
		double evaluate();
		static Sensor* pinstance;
		

};


class UserSensor : public Sensor{
	friend class SensorBuilder;
	public:
		UserSensor();
	protected :
		UserSensor(const char* name, const int rows, const int columns, const double max_range, const double max_angle, 
				const int distance_length, const double* distance, const double* distance_factor,
				const int azimut_length, const double* azimut, const double* azimut_factor,
				const int incidence_length, const double* incidence, const double* incidence_factor);
};

class SensorBuilder{
	public:
		static UserSensor* getUserSensor(const char* name);
		static UserSensor* createUserSensor(const char* name, const int rows, const int columns, const double max_range, const double max_angle, 
				const int distance_length, const double* distance, const double* distance_factor,
				const int azimut_length, const double* azimut, const double* azimut_factor,
				const int incidence_length, const double* incidence, const double* incidence_factor);
		static int test;
//	SensorBuilder::~SensorBuilder();

	protected:
		static map<string, UserSensor*> sensors;
		static map<int, int> testmap;

};


double irSense2(std::vector<slWorldObject*>* neighbors,
		slPosition *sensorPos, std::string sensorType);

double calculateQuality2(vector<slWorldObject*>* neighbors, slPosition* sensorPos, slVector* targetLoc, std::string sensorType, slWorldObject* target);


