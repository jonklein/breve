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

enum cameraEnablingModes {
	CM_DISABLED = 0,
	CM_UPDATED,
	CM_NOT_UPDATED
};

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

struct slCamera {
	slLight lights[8];
	int nLights;

	unsigned char enabled;

	slWorldObject *shadowCatcher;

	// this flag specifies whether the data in this camera is 
	// automatically being rendered.  In the normal graphical
	// breve, we can go a glReadPixels whenever we want, and 
	// we know that we're getting an up-to-date image.  In 
	// non GUI versions, we may have to update the camera 
	// ourselves.

	int flags;

	double zClip;

	// used during drawing

	slBillboardEntry **billboards;
	int billboardCount;
	int maxBillboards;
	int billboardDrawList;

	int cubeDrawList;

	// recompile can be set to 1 at any time to force recompilation 
	// of draw lists next time the world is drawn. 

	unsigned char recompile;

	slCameraText *text;
	int textCount;
	int maxText;
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
	int (*renderContextCallback)(slWorld *w, slCamera *c);
};

void slCameraUpdateFrustum(slCamera *c);
int slCameraPointInFrustum(slCamera *c, slVector *test);
int slCameraPolygonInFrustum(slCamera *c, slVector *test, int n);

#ifdef __cplusplus
extern "C" {
#endif
slCamera *slNewCamera(int x, int y, int drawMode);
void slUpdateCamera(slCamera *c);

void slCameraResize(slCamera *c, int x, int y);

void slCameraFree(slCamera *c);

void slSetCameraText(slCamera *c, int n, char *string, float x, float y, slVector *v);
void slSetShadowCatcher(slCamera *c, slStationary *s, slVector *normal);

void slAddBillboard(slCamera *c, slWorldObject *object, float size, float z);

void slSortBillboards(slCamera *c);

int slBillboardSortFunc(const void *a, const void *b);

void slRotateCameraWithMouseMovement(slCamera *c, double dx, double dy, double scamx);
void slMoveCameraWithMouseMovement(slCamera *c, double dx, double dy);
void slZoomCameraWithMouseMovement(slCamera *c, double dx, double dy);
#ifdef __cplusplus
}
#endif
