#include "simulation.h"

void slWorldObject::draw(slCamera *camera) {
	if(shape) slDrawShape(camera, shape, &position, textureScale, drawMode, 0);
}

void slWorldObjectSetCallbackData(slWorldObject *wo, void *data) {
	wo->userData = data;
}

void *slWorldObjectGetCallbackData(slWorldObject *wo) {
	return wo->userData;
}

void slWorldObjectSetCollisionE(slWorldObject *wo, double e) {
	wo->e = e;
}

void slWorldObjectSetCollisionET(slWorldObject *wo, double eT) {
	wo->eT = eT;
}

void slWorldObjectSetCollisionMU(slWorldObject *wo, double mu) {
	wo->mu = mu;
}

void slWorldObjectSetNeighborhoodSize(slWorldObject *wo, double size) {
	wo->proximityRadius = size;
}

/*!
	\brief Sets the texture number for this object.

	Previous versions of this library used -1 to indicate no texture,
	while the current version uses 0.  This function recognizes negative
	numbers for backward compatability.
*/

void slWorldObjectSetTexture(slWorldObject *wo, int texture) {
	if(texture < 0) texture = 0;
	wo->texture = texture;
}

void slWorldObjectSetTextureMode(slWorldObject *wo, int mode) {
	wo->textureMode = mode;
}

void slWorldObjectSetTextureScale(slWorldObject *wo, double scale) {
	wo->textureScale = scale;
}

void slWorldObjectSetBitmapRotation(slWorldObject *wo, double rot) {
	wo->billboardRotation = rot;
}

void slWorldObjectAddDrawMode(slWorldObject *wo, int mode) {
	wo->drawMode |= mode;
}

void slWorldObjectRemoveDrawMode(slWorldObject *wo, int mode) {
	if(wo->drawMode & mode) wo->drawMode ^= mode;
}

const slStack *slWorldObjectGetNeighbors(slWorldObject *wo) {
	return wo->neighbors;
}

int slWorldObjectGetLightExposure(slWorldObject *wo) {
	return wo->lightExposure;
}

void slWorldObjectSetAlpha(slWorldObject *wo, double alpha) {
	wo->alpha = alpha;
}

void slWorldObjectSetColor(slWorldObject *wo, slVector *color) {
	slVectorCopy(color, &wo->color);
}
