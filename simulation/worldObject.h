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
