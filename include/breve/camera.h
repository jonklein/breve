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

#ifndef _CAMERA_H
#define _CAMERA_H

#include "texture.h"
#include "vector.h"

class slWorldObject;
class slWorld;
class slStationary;
class slRenderGL;

enum slBillboardType {
	slBitmapNone = 0,
	slBitmap,
	slLightmap
};


/*!
 * brief A string of text printed to the GL view.
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

class slSceneNode {
	public:
		
	private:
		// slTransform 		_transform;
};

class slSceneObject : public slSceneNode {

};

class slSceneImage : public slSceneObject {
	public: 
							slSceneImage( slTexture2D* inTexture ) { _texture = inTexture; }

	private:
		slTexture2D* 		_texture;
};

class slSceneText : public slSceneObject {

};

class slSceneGroup : public slSceneNode {
	public:
	private:
		std::vector< slSceneNode* > _nodes;
};

/*!
 * \brief Data for billboarded bitmaps.
 */

struct slBillboardEntry {
	float size;
	float z;

	slWorldObject *object;
};

/** 
 * \brief The camera for the graphical display.
 * Holds camera position/location, as well as a variety of other
 * rendering data.
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

		void renderBillboards( slRenderGL& inRenderer );
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
	
		slWorldObject *select( slWorld *inWorld, int inScreenX, int inScreenY ) const { slMessage( DEBUG_ALL, "Selection not implemented." ); return NULL; }
		int vectorForDrag( slWorld *w, slVector *dragVertex, int x, int y, slVector *dragVector ) const { slMessage( DEBUG_ALL, "Vector for drag not implemented." ); return 0; }

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
	
		slVector _billboardX;
		slVector _billboardY;
		slVector _billboardZ;
	
		// recompile can be set to 1 at any time to force recompilation 
		// of draw lists next time the world is drawn. 
	
		unsigned char _recompile;
	
		slVector _textColor;
		
		slSceneNode*				_overlay;

		std::vector<slCameraText> 	_text;
	
		float						_textScale;
	
		bool						_drawLights;
		bool						_drawFog;
		bool						_drawShadow;
		bool						_drawShadowVolumes;
		bool						_drawOutline;
		bool						_drawReflection;
		bool						_drawText;
		bool						_drawBlur;

		double						_blurFactor;
	
		slVector					_fogColor;
	
		double						_fogIntensity;
		double						_fogStart;
		double						_fogEnd;
	
		slPlane						_shadowPlane;
	
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

		void						readbackToTexture();

		void						initGL();

	private:

		slTexture2D*				_readbackTexture;
		int							_readbackX;
		int							_readbackY;

		void 						stencilFloor( slWorld *w );
		void 						reflectionPass( slWorld *w, bool inWillDoVolumeShadow );
		void 						drawFlatShadows( slWorld *w );
		void 						renderObjects( slWorld *w, unsigned int flags, float inAlphaScale = 1.0f );
		void 						renderText( slWorld *w, int crosshair );
		void 						renderLabels( slWorld *w );
		void 						renderLines( slWorld *w );
		void 						drawBackground( slWorld *w );
		void 						drawFog();

		slVertexBufferGL			_billboardBuffer;
};

#endif /* _CAMERA_H */
