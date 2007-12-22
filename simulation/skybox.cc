#include "skybox.h"
#include "simulation.h"
#include "glIncludes.h"

slSkybox::slSkybox() {
	_loaded = false;
}

slSkybox::~slSkybox() {
}

int slSkybox::loadImage( std::string &inPath, int inN ) {
	int result = _textures[ inN ].loadImage( inPath, false );

	// check and update the _loaded state

	_loaded = true;

	for( int n = 0; n < 6; n++ ) {
		if( !_textures[ n ].isLoaded() ) {
			_loaded = false;
		}
	}

	return result;
}

void slSkybox::draw( slVector *inCameraPos, float inZFar ) {

	if( ! _loaded ) 
		return;

	glDisable( GL_CULL_FACE );
	glDisable( GL_BLEND );

	glDepthMask( GL_FALSE );
//	glDisable( GL_LIGHTING );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glTranslatef( inCameraPos->x, inCameraPos->y, inCameraPos->z );
	glScalef( 0.3f * inZFar, 0.3f * inZFar, 0.3f * inZFar );
 
	glMatrixMode( GL_TEXTURE );
	glPushMatrix();
	glLoadIdentity();


	glColor4f( 1.0, 1.0, 1.0, 1.0f);
 
	float r = 1.0001f;
	float ux = 0;
	float uy = 0;

	_textures[ 0 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	ux = _textures[ 0 ]._unitX;
	uy = _textures[ 0 ]._unitY;
	glBegin(GL_QUADS);		
		glTexCoord2f( 0.0, 0.0 ); glVertex3f(  r, -r, -1.0f );
		glTexCoord2f( 0.0, uy  ); glVertex3f(  r,  r, -1.0f ); 
		glTexCoord2f( ux,  uy  ); glVertex3f( -r,  r, -1.0f );
		glTexCoord2f( ux,  0.0 ); glVertex3f( -r, -r, -1.0f );
	glEnd();
 
	_textures[ 1 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	ux = _textures[ 1 ]._unitX;
	uy = _textures[ 1 ]._unitY;
	glBegin(GL_QUADS);		
		glTexCoord2f( ux,  0.0 ); glVertex3f(  r, -r, 1.0f );
		glTexCoord2f( ux,  uy  ); glVertex3f(  r,  r, 1.0f ); 
		glTexCoord2f( 0.0, uy  ); glVertex3f( -r,  r, 1.0f );
		glTexCoord2f( 0.0, 0.0 ); glVertex3f( -r, -r, 1.0f );
	glEnd();
 
	_textures[ 2 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	ux = _textures[ 2 ]._unitX;
	uy = _textures[ 2 ]._unitY;
	glBegin(GL_QUADS);		
		glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0f, -r, r );	
		glTexCoord2f( 0.0, uy  ); glVertex3f( 1.0f,  r, r ); 
		glTexCoord2f( ux,  uy  ); glVertex3f( 1.0f,  r,-r );
		glTexCoord2f( ux,  0.0 ); glVertex3f( 1.0f, -r,-r );		
	glEnd();
 
	_textures[ 3 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	ux = _textures[ 3 ]._unitX;
	uy = _textures[ 3 ]._unitY;
	glBegin(GL_QUADS);		
		glTexCoord2f( ux,  0.0 ); glVertex3f( -1.0f, -r,  r );	
		glTexCoord2f( ux,  uy  ); glVertex3f( -1.0f,  r,  r ); 
		glTexCoord2f( 0.0, uy  ); glVertex3f( -1.0f,  r, -r );
		glTexCoord2f( 0.0, 0.0 ); glVertex3f( -1.0f, -r, -r );
	glEnd();
 
	_textures[ 4 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	ux = _textures[ 4 ]._unitX;
	uy = _textures[ 4 ]._unitY;
	glBegin(GL_QUADS);	
		glTexCoord2f( ux,  uy  ); glVertex3f( -r, 1.0f, -r);
		glTexCoord2f( 0.0, uy  ); glVertex3f( -r, 1.0f,  r);
		glTexCoord2f( 0.0, 0.0 ); glVertex3f(  r, 1.0f,  r); 
		glTexCoord2f( ux,  0.0 ); glVertex3f(  r, 1.0f, -r);
	glEnd();
 
	_textures[ 5 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	ux = _textures[ 5 ]._unitX;
	uy = _textures[ 5 ]._unitY;
	glBegin(GL_QUADS);		
		glTexCoord2f( ux,  0.0 );  glVertex3f( -r, -1.0f, -r );
		glTexCoord2f( 0.0, 0.0 );  glVertex3f( -r, -1.0f,  r );
		glTexCoord2f( 0.0, uy  );  glVertex3f(  r, -1.0f,  r ); 
		glTexCoord2f( ux,  uy  );  glVertex3f(  r, -1.0f, -r );
	glEnd();

	glPopMatrix();

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	// restore states as best we can

	glDepthMask( GL_TRUE );
	glEnable( GL_CULL_FACE );
	glEnable( GL_BLEND );
//	glEnable( GL_LIGHTING );
	_textures[ 5 ].unbind();
}
