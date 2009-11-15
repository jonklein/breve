#ifndef _WORLDOBJECT_H
#define _WORLDOBJECT_H

#include "slutil.h"
#include "shape.h"

class slWorld;
class slRenderGL;
class slVertexBufferGL;
class slObjectConnection;

class slTransform {
	public:
		void				updateTransform( const slTransform& inParent );
	
		float				_color[ 4 ];

		double 				_rotation[ 3 ][ 3 ];
		slVector 			_location;
		slVector 			_scale;

		double 				_transform[ 4 ][ 4 ];
		double 				_globalTransform[ 4 ][ 4 ];
};

class slWorldObject {
	public:
		friend class slCamera;
		friend class slJoint;
		friend class slWorld;
		friend class slSpring;
		friend class slVclipData;

		slWorldObject();
		virtual ~slWorldObject();
		
		virtual void draw( const slRenderGL& inRenderer );
		virtual void step( slWorld *world, double step ) {};

		void setCallbackData( void *data );
		void *getCallbackData();
		void setCollisionE( double e );
		void setCollisionMU( double mu );
		void setNeighborhoodSize( double size );
		void setColor( slVector *color );
		void setDrawShadows( bool drawShadows );
		void setAlpha( double alpha );
		void setTexture( slTexture2D *inTexture );
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
		virtual void setScale( slVector *scale ) { _transform._scale = *scale; }

		inline unsigned char getType() { return _type; }

		inline bool isSimulated() { return _simulate; }

		double _proximityRadius;
		slVector _neighborMax;
		slVector _neighborMin;

		slVector _max;
		slVector _min;

	protected:
		slShape*					_shape;
		slShape*					_displayShape;

		std::string 				_label;

		bool 						_simulate;
		bool 						_update;

		slTransform 				_transform;

		slPosition 					_position;

		// type is one of the slWorldObjectTypes -- a stationary or a multibody
		// the data pointer is thus a pointer to the corresponding structure.

		unsigned char _type;


		int _lightExposure;

		bool _drawAsPoint;
		bool 						_drawShadow;
		slTexture2D*				_texture;
		char 						_textureMode;
		unsigned char 				_drawMode;
		float 						_billboardRotation;
		slVector 					_color;
		float 						_alpha;
	
		float 						_textureScaleX, _textureScaleY;

		// bounding box information here is used for "proximity" data
	
		bool 						_moved;
	
		double _e;
		double _mu;

		std::vector<slWorldObject*> _neighbors;
		std::vector< void* > 		_neighborData;

		// the list of lines that this object makes to other objects

		std::vector<slObjectConnection*> _connections;

		void*						_userData;
};

/**
 * \brief A line drawn from one object to another.
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

#endif /* _WORLDOBJECT_H */
