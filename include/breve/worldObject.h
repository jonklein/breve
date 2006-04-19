#ifndef _WORLDOBJECT_H
#define _WORLDOBJECT_H

#include "util.h"
#include "shape.h"

#include <vector>
#include <string>

class slWorld;
class slWorldObject;

/*!
	\brief A line drawn from one object to another.
*/

class slObjectConnection {
	public:
		virtual ~slObjectConnection() { };

		virtual void draw(slCamera *c) = 0;
		virtual void step(double timestep) = 0;

		slWorldObject *_src;
		slWorldObject *_dst;
};

class slObjectLine: public slObjectConnection {
	public:
		slObjectLine() { _stipple = 0xffff; }

		void draw(slCamera *c);
		void step(double timestep) {};

		slVector _color;
		int _stipple;

		void setStipple( int s) { _stipple = s; }
};

class slWorldObject {
	public:
		friend class slCamera;
		friend class slJoint;
		friend class slWorld;
		friend class slSpring;
		friend class slTerrain;
		friend class slVclipData;

		slWorldObject() {
			_drawMode = 0;
			_texture = 0;
			_textureMode = 0;
			_textureScaleX = 16;
			_textureScaleY = 16;
			_simulate = 0;
			_drawAsPoint = 0;

			_lightExposure = 0;

			_shape = NULL;

			_proximityRadius = 0.00001;

			_billboardRotation = 0;
			_alpha = 1.0;

			_userData = NULL;

			_e = 0.4;
			_eT = 0.2;
			_mu = 0.15;

			slVectorSet(&_color, 1, 1, 1);

			slMatrixIdentity(_position.rotation);
			slVectorSet(&_position.location, 0, 0, 0);
		}

		virtual ~slWorldObject() {
			std::vector<slObjectConnection*>::iterator ci;

			for(ci = _connections.begin(); ci != _connections.end(); ci++ ) {
				(*ci)->_src = NULL;
				(*ci)->_dst = NULL;
			}

			if( _shape ) slShapeFree( _shape );
		}

		virtual void draw( slCamera *camera );

		virtual void step( slWorld *world, double step ) {};

		void setCallbackData( void *data );
		void *getCallbackData();
		void setCollisionE( double e );
		void setCollisionET( double eT );
		void setCollisionMU( double mu );
		void setNeighborhoodSize( double size );
		void setColor( slVector *color );
		void setAlpha( double alpha );
		void setTexture( int texture );
		void setTextureMode( int mode );
		void setTextureScale( double scaleX, double scaleY );
		void setBitmapRotation( double rot );
		void setDrawAsPoint( bool p );
		void addDrawMode( int mode );
		void removeDrawMode( int mode );

		void getBounds( slVector *min, slVector *max );

		int getLightExposure( );
		int raytrace( slVector *location, slVector* direction, slVector *erg_dir );

		std::vector< void* > &getNeighborData();

		inline const slPosition &getPosition() { return _position; }
		inline const slShape *getShape() { return _shape; }

		virtual void updateBoundingBox();

		inline unsigned char getType() { return _type; }

		inline bool isSimulated() { return _simulate; }

		double _proximityRadius;
		slVector _neighborMax;
		slVector _neighborMin;

		slVector _max;
		slVector _min;

	protected:
		slShape *_shape;

		std::string _label;

		bool _simulate;
		bool _update;

		slPosition _position;

		// type is one of the slWorldObjectTypes -- a stationary or a multibody
		// the data pointer is thus a pointer to the corresponding structure.

		unsigned char _type;

		slVector _color;

		int _lightExposure;

		bool _drawAsPoint;
		int _texture;
		char _textureMode;
		unsigned char _drawMode;
		float _billboardRotation;
		float _alpha;
	
		float _textureScaleX, _textureScaleY;

		// bounding box information here is used for "proximity" data
	
		bool _moved;
	
		double _e;
		double _eT;
		double _mu;

		std::vector<slWorldObject*> _neighbors;
		std::vector< void* > _neighborData;

		// the list of lines that this object makes to other objects

		std::vector<slObjectConnection*> _connections;

		void *_userData;
};

#endif /* _WORLDOBJECT_H */
