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

#define SPHERE_RESOLUTIONS	10

enum billboardType {
	BBT_NONE = 0,
	BBT_BITMAP,
	BBT_LIGHTMAP
};

/*!
	\brief Holds location and color information for a light.
*/

struct slLight {
	slVector location;
	slVector diffuse;
	slVector ambient;

	unsigned char changed;
};

/*!
	\brief A string of text printed to the GL view.
*/

#ifdef __cplusplus
class slCameraText {
	public:
		slCameraText() { text = NULL; }

		char *text;
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
#endif

/*!
	\brief The camera for the graphical display.

	Holds camera position/location, as well as a variety of other
	rendering data.
*/

#ifdef __cplusplus
class slCamera {
	public:
		slLight lights[8];
		int nLights;

		void updateFrustum();

		int pointInFrustum(slVector *test);
		int minMaxInFrustum(slVector *min, slVector *max);
		int polygonInFrustum(slVector *test, int n);

		// unsigned char enabled;

		slWorldObject *shadowCatcher;

		int flags;

		double zClip;

		// used during drawing

		slBillboardEntry **billboards;
		unsigned int billboardCount;
		unsigned int maxBillboards;
		int billboardDrawList;
	
		slVector billboardX;
		slVector billboardY;
		slVector billboardZ;
	
		int cubeDrawList;
		int sphereDrawLists;
	
		// recompile can be set to 1 at any time to force recompilation 
		// of draw lists next time the world is drawn. 
	
		unsigned char recompile;
	
		std::vector<slCameraText> text;
	
		double textScale;
	
		unsigned char drawMode;
		unsigned char drawLights;
		unsigned char drawFog;
		unsigned char drawSmooth;
		unsigned char drawShadow;
		unsigned char drawShadowVolumes;
		unsigned char drawOutline;
		unsigned char drawReflection;
		unsigned char drawText;
		unsigned char blur;
		double blurFactor;
	
		slVector fogColor;
	
		double fogIntensity;
		double fogStart;
		double fogEnd;
	
		slPlane shadowPlane;
	
		// offset & target of camera
	
		slVector location;
		slVector target; 
		double rotation[3][3];
	
		// rotation & zoom 

		double rx, ry;
		double zoom;
	
		double backgroundScrollX, backgroundScrollY;
	
		// the window's perspective of x and y axis at the current rotation 
	
		slVector xAxis;
		slVector yAxis;
	
		slPlane frustumPlanes[6];
	
		// camera size 
		
		int x;
		int y;
	
		// camera origin on screen view coords 
	
		int ox;
		int oy;
	
		double fov;
	
		int (*activateContextCallback)();
		void (*renderContextCallback)(slWorld *w, slCamera *c);
};
#endif

void slCameraUpdateFrustum(slCamera *);

#ifdef __cplusplus
extern "C" {
#endif
slCamera *slCameraNew(int, int);
void slUpdateCamera(slCamera *);

void slCameraResize(slCamera *, int, int);

void slCameraFree(slCamera *);

void slSetCameraText(slCamera *, int, char *, float, float, slVector *);
void slSetShadowCatcher(slCamera *, slStationary *, slVector *);

void slAddBillboard(slCamera *, slWorldObject *, float, float);

void slSortBillboards(slCamera *);

int slBillboardSortFunc(const void *, const void *);

void slRotateCameraWithMouseMovement(slCamera *, double, double, double);
void slMoveCameraWithMouseMovement(slCamera *, double, double);
void slZoomCameraWithMouseMovement(slCamera *, double, double);

void slCameraSetBounds(slCamera *, unsigned int, unsigned int);
void slCameraGetBounds(slCamera *, unsigned int *, unsigned int *);

void slCameraGetRotation(slCamera *, double *, double *);

void slCameraSetRecompile(slCamera *);

void slCameraSetActivateContextCallback(slCamera *, int (*)(void));
#ifdef __cplusplus
}
#endif
