#ifndef _WORLDOBJECT_H
#define _WORLDOBJECT_H

#ifdef __cplusplus

#include "util.h"
#include "shape.h"

class slWorld;
class slWorldObject;

template <class T> class handle_ptr {
		T* ptr;

		T** handle;
	public:
		explicit safe_ptr(T* p = 0) : ptr(p) {}
		~safe_ptr()                 {delete ptr;}
		T& operator*()              {return *ptr;}
		T* operator->()             {return ptr;}
};

/*!
	\brief A line drawn from one object to another.
*/

class slObjectConnection {
	public:
		virtual void draw() = 0;

		slWorldObject *_src;
		slWorldObject *_dst;
};

class slObjectLine: public slObjectConnection {
	public:
		void draw() {};

		slVector _color;
		int _stipple;
};

class slWorldObject {
	public:
		slWorldObject() {
			drawMode = 0;
			texture = 0;
			textureMode = 0;
			textureScale = 16;
			simulate = 0;

			shape = NULL;

			proximityRadius = 0.00001;
			neighbors = slStackNew();

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
			if(shape) slShapeFree(shape);
			slStackFree(neighbors);
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
		slStack *neighbors;

		// the list of lines that this object makes to other objects

		std::vector<slObjectLine*> lines;

		void *userData;
};

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

const slStack *slWorldObjectGetNeighbors(slWorldObject *wo);

int slWorldObjectGetLightExposure(slWorldObject *wo);
#ifdef __cplusplus
}
#endif

#endif /* _WORLDOBJECT_H */
