#ifndef _WORLDOBJECT_H
#define _WORLDOBJECT_H

#include "slutil.h"
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
		slObjectLine() { 
			_stipple = 0xffff; 
			slVectorSet( &_color, 0, 0, 0 ); 
			_transparency = 1.0; 
		}

		void draw(slCamera *c);
		void step(double timestep) {};

		void setColor( slVector &inColor ) { slVectorCopy( &inColor, &_color ); }
		void setTransparency( float inTransparency ) { _transparency = inTransparency; }
		void setStipple( int inStipple ) { _stipple = inStipple; }

		slVector _color;
		float _transparency;
		int _stipple;

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
			_drawAsPoint = false;
			_drawShadow = true;

			_lightExposure = 0;

			_shape = NULL;
			_displayShape = NULL;

			_proximityRadius = 0.00001;

			_billboardRotation = 0;
			_alpha = 1.0;

			_userData = NULL;

			_e = 0.4;
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

			if( _shape ) 
				_shape -> release();

			if( _displayShape ) 
				_displayShape -> release();
		}

		virtual void draw( slCamera *camera, bool inUseDrawMode = true );

		virtual void step( slWorld *world, double step ) {};

		void setCallbackData( void *data );
		void *getCallbackData();
		void setCollisionE( double e );
		void setCollisionMU( double mu );
		void setNeighborhoodSize( double size );
		void setColor( slVector *color );
		void setDrawShadows( bool drawShadows );
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
		double irSense(slPosition *sensorPos, std::string sensorType);
		double calculateQuality(slPosition* sensorPos, slVector* targetLoc, std::string sensorType,slWorldObject* target=NULL);
		double calcQualNoRay(slPosition* sensorPos, slVector* targetLoc, std::string sensorType,slWorldObject* target=NULL);
		
		std::vector< void* > &getNeighborData();

		inline const slPosition &getPosition() { return _position; }

		inline slShape *getShape() { return _shape; }
		inline slShape *getDisplayShape() { return _displayShape; }

		virtual void updateBoundingBox();

		virtual void setShape( slShape *inShape );
		virtual void setDisplayShape( slShape *inShape );

		virtual void setRotation( double rotation[3][3] );
		virtual void setLocation( slVector *location );

		inline unsigned char getType() { return _type; }

		inline bool isSimulated() { return _simulate; }

		double _proximityRadius;
		slVector _neighborMax;
		slVector _neighborMin;

		slVector _max;
		slVector _min;

	protected:
		slShape *_shape;
		slShape *_displayShape;

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
		bool _drawShadow;
		int _texture;
		char _textureMode;
		unsigned char _drawMode;
		float _billboardRotation;
		float _alpha;
	
		float _textureScaleX, _textureScaleY;

		// bounding box information here is used for "proximity" data
	
		bool _moved;
	
		double _e;
		double _mu;

		std::vector<slWorldObject*> _neighbors;
		std::vector< void* > _neighborData;

		// the list of lines that this object makes to other objects

		std::vector<slObjectConnection*> _connections;

		void *_userData;
};

#endif /* _WORLDOBJECT_H */
