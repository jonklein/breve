#ifndef _WORLDOBJECT_H
#define _WORLDOBJECT_H

#ifdef __cplusplus

#include "util.h"
#include "shape.h"

#include <vector>

class slWorld;
class slWorldObject;

/*!
	\brief A line drawn from one object to another.
*/

class slObjectConnection {
	public:
		virtual void draw(slCamera *c) = 0;
		virtual void step(double timestep) = 0;

		slWorldObject *_src;
		slWorldObject *_dst;
};

class slObjectLine: public slObjectConnection {
	public:
		void draw(slCamera *c);
		void step(double timestep) {};

		slVector _color;
		int _stipple;

		void setStipple( int s) { _stipple = s; }
};

class slWorldObject {
	public:
		slWorldObject() {
			drawMode = 0;
			texture = 0;
			textureMode = 0;
			textureScale = 16;
			simulate = 0;
			_drawAsPoint = 0;

			shape = NULL;

			proximityRadius = 0.00001;

			billboardRotation = 0;
			alpha = 1.0;

			e = 0.4;
			eT = 0.2;
			mu = 0.15;

			slVectorSet(&color, 1, 1, 1);

			slMatrixIdentity(position.rotation);
			slVectorSet(&position.location, 0, 0, 0);
		}

		virtual ~slWorldObject() {
			std::vector<slObjectConnection*>::iterator ci;

			for(ci = connections.begin(); ci != connections.end(); ci++ ) {
				(*ci)->_src = NULL;
				(*ci)->_dst = NULL;
			}

			if(shape) slShapeFree(shape);
		}

		virtual void draw(slCamera *camera);

		virtual void step(slWorld *world, double step) {};

		slShape *shape;

		char *label;

		unsigned char simulate;
		unsigned char update;

		slPosition position;

		// type is one of the slWorldObjectTypes -- a stationary or a multibody
		// the data pointer is thus a pointer to the corresponding structure.

		unsigned char type;

		slVector color;

		int lightExposure;

		bool _drawAsPoint;
		int texture;
		char textureMode;
		unsigned char drawMode;
		float billboardRotation;
		float alpha;
	
		float textureScale;

		// bounding box information here is used for "proximity" data
	
		slVector neighborMax;
		slVector neighborMin;
		slVector max;
		slVector min;
	
		double e;
		double eT;
		double mu;

		double proximityRadius;
		std::vector<slWorldObject*> neighbors;

		// the list of lines that this object makes to other objects

		std::vector<slObjectConnection*> connections;

		void *userData;
};

std::vector<slWorldObject*> &slWorldObjectGetNeighbors(slWorldObject *wo);
#endif

#ifdef __cplusplus
extern "C" {
#endif

void slWorldObjectSetCallbackData(slWorldObject *wo, void *data);
void *slWorldObjectGetCallbackData(slWorldObject *wo);

void slWorldObjectSetCollisionE(slWorldObject *wo, double e);
void slWorldObjectSetCollisionET(slWorldObject *wo, double eT);
void slWorldObjectSetCollisionMU(slWorldObject *wo, double mu);

void slWorldObjectSetNeighborhoodSize(slWorldObject *wo, double size);

void slWorldObjectSetColor(slWorldObject *wo, slVector *color);
void slWorldObjectSetAlpha(slWorldObject *wo, double alpha);

void slWorldObjectSetTexture(slWorldObject *wo, int texture);
void slWorldObjectSetTextureMode(slWorldObject *wo, int mode);
void slWorldObjectSetTextureScale(slWorldObject *wo, double scale);
void slWorldObjectSetBitmapRotation(slWorldObject *wo, double rot);

void slWorldObjectAddDrawMode(slWorldObject *wo, int mode);
void slWorldObjectRemoveDrawMode(slWorldObject *wo, int mode);

int slWorldObjectGetLightExposure(slWorldObject *wo);

int slWorldObjectRaytrace(slWorldObject *wo, slVector *location, slVector* direction, slVector *erg_dir);

#ifdef __cplusplus
}
#endif

#endif /* _WORLDOBJECT_H */
