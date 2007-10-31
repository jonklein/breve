#include "skybox.h"
#include "simulation.h"
#include "glIncludes.h"

slSkybox::slSkybox() {
	_loaded = false;
}

slSkybox::~slSkybox() {
}

int slSkybox::loadImage( std::string &inPath, int inN ) {
	int result = _textures[ inN ].loadImage( inPath );

	// check and update the _loaded state

	_loaded = true;

	for( int n = 0; n < 6; n++ ) {
		if( !_textures[ n ].isLoaded() ) {
			_loaded = false;
		}
	}

	return result;
}

void slSkybox::draw( slVector *inCameraPos ) {

	if( ! _loaded ) 
		return;
	

	glDisable( GL_CULL_FACE );
	glDisable( GL_BLEND );

	glDepthMask( GL_FALSE );
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	// glLoadIdentity();
	// glFrustum( 1, -1, -1, 1, .9, 3.0 );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glTranslatef( inCameraPos->x, inCameraPos->y, inCameraPos->z );
	glScalef( 5, 5, 5 );
	if( !_textures[ 0 ].isLoaded() ) 
		loadImage( "BoxFront.png", 0 );

	if( !_textures[ 1 ].isLoaded() ) 
		loadImage( "BoxBack.png", 1 );

	if( !_textures[ 2 ].isLoaded() ) 
		loadImage( "BoxLeft.png", 2 );

	if( !_textures[ 3 ].isLoaded() ) 
		loadImage( "BoxRight.png", 3 );

	if( !_textures[ 4 ].isLoaded() ) 
		loadImage( "BoxTop.png", 4  );

	if( !_textures[ 5 ].isLoaded() ) 
		loadImage( "BoxBottom.png", 5 );
 
	glColor4f( 0.0, 1.0, 0.0, 1.0f);
 
	float r = 1.0001f;

	_textures[ 0 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glBegin(GL_QUADS);		
		glTexCoord2f( 0.0, 0.0 ); glVertex3f(  r, -r, -1.0f );
		glTexCoord2f( 0.0, 1.0 ); glVertex3f(  r,  r, -1.0f ); 
		glTexCoord2f( 1.0, 1.0 ); glVertex3f( -r,  r, -1.0f );
		glTexCoord2f( 1.0, 0.0 ); glVertex3f( -r, -r, -1.0f );
	glEnd();
 
	_textures[ 1 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glBegin(GL_QUADS);		
		glTexCoord2f( 1.0, 0.0 ); glVertex3f(  r, -r, 1.0f );
		glTexCoord2f( 1.0, 1.0 ); glVertex3f(  r,  r, 1.0f ); 
		glTexCoord2f( 0.0, 1.0 ); glVertex3f( -r,  r, 1.0f );
		glTexCoord2f( 0.0, 0.0 ); glVertex3f( -r, -r, 1.0f );
	glEnd();
 
	_textures[ 2 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glBegin(GL_QUADS);		
		glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0f, -r, r );	
		glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0f,  r, r ); 
		glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0f,  r,-r );
		glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0f, -r,-r );		
	glEnd();
 
	_textures[ 3 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glBegin(GL_QUADS);		
		glTexCoord2f( 1.0, 0.0 ); glVertex3f( -1.0f, -r,  r );	
		glTexCoord2f( 1.0, 1.0 ); glVertex3f( -1.0f,  r,  r ); 
		glTexCoord2f( 0.0, 1.0 ); glVertex3f( -1.0f,  r, -r );
		glTexCoord2f( 0.0, 0.0 ); glVertex3f( -1.0f, -r, -r );
	glEnd();
 
	_textures[ 4 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glBegin(GL_QUADS);	
		glTexCoord2f( 1.0, 1.0 ); glVertex3f( -r, 1.0f, -r);
		glTexCoord2f( 0.0, 1.0 ); glVertex3f( -r, 1.0f,  r);
		glTexCoord2f( 0.0, 0.0 ); glVertex3f(  r, 1.0f,  r); 
		glTexCoord2f( 1.0, 0.0 ); glVertex3f(  r, 1.0f, -r);
	glEnd();
 
	_textures[ 5 ].bind();
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glBegin(GL_QUADS);		
		glTexCoord2f( 1.0, 0.0 );  glVertex3f( -r, -1.0f, -r );
		glTexCoord2f( 0.0, 0.0 );  glVertex3f( -r, -1.0f,  r );
		glTexCoord2f( 0.0, 1.0 );  glVertex3f(  r, -1.0f,  r ); 
		glTexCoord2f( 1.0, 1.0 );  glVertex3f(  r, -1.0f, -r );
	glEnd();
 
	glPopMatrix();

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	glColor4f( 1.0, 1.0, 1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );

	glDepthMask( GL_TRUE );
	glEnable( GL_CULL_FACE );
}
