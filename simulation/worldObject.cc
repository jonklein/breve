#include "simulation.h"

void slWorldObject::draw(slCamera *camera) {
	if(shape) shape->draw(camera, &position, textureScale, drawMode, 0);
}

void slObjectLine::draw(slCamera *camera) {
	slVector *x, *y;

	if(!_src || !_dst) return;

	x = &_src->position.location;
	y = &_dst->position.location;

	if(_stipple) {
		glLineStipple(2, _stipple);
		glEnable(GL_LINE_STIPPLE);
	}

	glColor4f(_color.x, _color.y, _color.z, 0.8);

	glBegin(GL_LINES);

	glVertex3f(x->x, x->y, x->z);
	glVertex3f(y->x, y->y, y->z);

	glEnd();

	if(_stipple) glDisable(GL_LINE_STIPPLE);
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

std::vector<slWorldObject*> &slWorldObjectGetNeighbors(slWorldObject *wo) {
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

int slWorldObjectRaytrace(slWorldObject *wo, slVector *location, slVector* direction, slVector *erg_dir) {

   if(!(wo->shape)) {
      slMessage(DEBUG_ALL, "slWorldObjectRaytrace: This WorldObject has no shape\n");
      return -2;
   }
   
   //direction and location in wo's coordinates
   slVector dir_wo, dir_wo_help;
   slVector loc_wo_help;
   slVector loc_wo;

   slVectorInvXform(wo->position.rotation, direction, &dir_wo_help);
   slVectorMul(&dir_wo_help, -1, &dir_wo);   
   
   slVectorSub(location, &wo->position.location, &loc_wo_help);
   slVectorInvXform(wo->position.rotation, &loc_wo_help, &loc_wo);   
   
//   slMessage(DEBUG_ALL, " [ %f, %f, %f ] %f %f ", dir_wo.x, dir_wo.y, dir_wo.z, atan2(dir_wo.z, dir_wo.x)*180/M_PI, atan2(direction->z, direction->x)*180/M_PI );
//   slMessage(DEBUG_ALL, " [ %f, %f, %f ] ", loc_wo.x, loc_wo.y, loc_wo.z );
   
   slVector point;
   if ( wo->shape->rayHitsShape(&dir_wo, &loc_wo, &point) < 0) {
      slVectorSet(erg_dir, 0.0, 0.0, 0.0); //no hit
      return -1;
   }

   double d = slVectorLength( &point);
   slVectorMul( direction, d, erg_dir);
   
   return 0;
}
