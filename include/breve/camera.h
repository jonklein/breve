/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

#include "slutil.h"
#include "glIncludes.h"

#ifndef _CAMERA_H
#define _CAMERA_H

#define SPHERE_RESOLUTIONS	10

class slWorldObject;
class slWorld;
class slStationary;

enum billboardType {
	BBT_NONE = 0,
	BBT_BITMAP,
	BBT_LIGHTMAP
};

/**
 * \brief Holds location and color information for a light.
 */

#define MAX_LIGHTS 8

enum slLightTypes {
	LightDisabled			= 0,
	LightPoint,
	LightSpot,
	LightInfinite
};

struct slLight {
	slLight() {
		_constantAttenuation = 1.0;
		_linearAttenuation = 0.0;

		_type = LightDisabled;
	}

	slVector 		_location;
	slVector 		_diffuse;
	slVector 		_ambient;
	slVector 		_specular;

	float			_constantAttenuation;
	float			_linearAttenuation;

	int			_type;
};

/*!
	\brief A string of text printed to the GL view.
*/

class slCameraText {
	public:
		slCameraText() { slVectorZero( &color ); x = 0; y = 0; size = 0; }

		std::string text;
		float x;
		float y;
		slVector color;
		unsigned char size;
};

/*!
	\brief Data for billboarded bitmaps.

	Billboards, no matter how damn simple they should be, turn out to be 
	a huge pain in the ass.  It is very hard to handle the billboards 
	properly when they contain alpha info, and also on multiple pass 
	algorithms.  Therefore, we make a first pass through to find the 
	billboards and compute their current coordinates, sort them back 
	to front and then finally draw them.
*/

struct slBillboardEntry {
	float size;
	float z;

	slWorldObject *object;
};

/*!
	\brief The camera for the graphical display.

	Holds camera position/location, as well as a variety of other
	rendering data.
*/

#include <vector>

class slCamera {
	public:
		slCamera( int width = 200, int height = 200 );
		~slCamera();

		void renderWorld( slWorld *w, int crosshair, int scissor );
		void renderScene( slWorld *w, int crosshair );


		void detectLightExposure( slWorld *w, int size, GLubyte *buffer );

		void updateFrustum();

		int pointInFrustum(slVector *);
		int minMaxInFrustum(slVector *, slVector *);
		int polygonInFrustum(slVector *, int);

		void setBounds(unsigned int, unsigned int);
		void getBounds(unsigned int *, unsigned int *);
		void getRotation(double *, double *);
		void setRecompile();

		void renderBillboards( int flags );
		void processBillboards( slWorld *w );
		void addBillboard( slWorldObject *object, float size, float z );
		void sortBillboards();	
	
		void update();

		void rotateWithMouseMovement( double, double );
		void moveWithMouseMovement( double, double );
		void zoomWithMouseMovement( double, double );

		void setShadowCatcher( slStationary *, slVector * );
				
		void setActivateContextCallback( int (*f)() );
		void setCameraText( int n, char *string, float x, float y, slVector *v );
	
		int select( slWorld *w, int x, int y );
		int vectorForDrag( slWorld *w, slVector *dragVertex, int x, int y, slVector *dragVector );

		void renderShadowVolume( slWorld *w );
		void renderObjectShadowVolumes( slWorld *w );

		std::vector< std::pair< slVector, slVector > > _points;

		slWorldObject *_shadowCatcher;

		int _flags;

		float _zClip;
		float _frontClip;

		// used during drawing

		slBillboardEntry **_billboards;
		unsigned int _billboardCount;
		unsigned int _maxBillboards;
		GLuint _billboardDrawList;
	
		slVector _billboardX;
		slVector _billboardY;
		slVector _billboardZ;
	
		// recompile can be set to 1 at any time to force recompilation 
		// of draw lists next time the world is drawn. 
	
		unsigned char _recompile;
	
		slVector _textColor;

		std::vector<slCameraText> _text;
	
		double _textScale;
	
		unsigned char _drawMode;
		bool _drawLights;
		bool _drawFog;
		bool _drawSmooth;
		bool _drawShadow;
		bool _drawShadowVolumes;
		bool _drawOutline;
		bool _drawReflection;
		bool _drawText;
		bool _drawBlur;

		double _blurFactor;
	
		slVector _fogColor;
	
		double _fogIntensity;
		double						_fogStart;
		double						_fogEnd;
	
		slPlane						_shadowPlane;
		slLight						_lights[ MAX_LIGHTS ];
	
		// offset & target of camera
	
		slVector					_location;
		slVector					_target; 
		double						_rotation[3][3];
	
		// rotation & zoom 

		double						_rx, _ry;
		double						_zoom;
	
		double						_backgroundScrollX, _backgroundScrollY;
	
		// the window's perspective of x and y axis at the current rotation 
	
		slVector					_xAxis;
		slVector					_yAxis;
	
		slPlane						_frustumPlanes[ 6 ];
	
		// camera size 
		
		int							_width;
		int							_height;
	
		// camera origin on screen view coords 
	
		int							_originx;
		int							_originy;
	
		double						_fov;

		int							(*_activateContextCallback)();
		void						(*_renderContextCallback)(slWorld *w, slCamera *c);

		void						clear( slWorld *w );

		void						setTextColor( slVector *inColor ) { slVectorCopy( inColor, &_textColor ); }

	private:
		void stencilFloor( slWorld *w );
		void reflectionPass( slWorld *w, bool inWillDoVolumeShadow );
		void drawFlatShadows( slWorld *w );
		void renderObjects( slWorld *w, unsigned int flags, float inAlphaScale = 1.0f );
		void renderText( slWorld *w, int crosshair );
		void renderLabels( slWorld *w );
		void renderLines( slWorld *w );
		void setupLights( int inAmbientOnly = 0 );
		void drawBackground( slWorld *w );
		void drawFog();


};

int slBillboardSortFunc(const void *, const void *);

#endif /* _CAMERA_H */
