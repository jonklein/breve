#include "skybox.h"
#include "simulation.h"
#include "glIncludes.h"

slSkybox::slSkybox() {
}

slSkybox::~slSkybox() {
}

int slSkybox::loadFrontImage( std::string &inPath ) {
	return _textures[ 0 ].loadImage( inPath );
}

int slSkybox::loadBackImage( std::string &inPath ) {
	return _textures[ 1 ].loadImage( inPath );
}

int slSkybox::loadRightImage( std::string &inPath ) {
	return _textures[ 2 ].loadImage( inPath );
}

int slSkybox::loadLeftImage( std::string &inPath ) {
	return _textures[ 3 ].loadImage( inPath );
}

int slSkybox::loadTopImage( std::string &inPath ) {
	return _textures[ 4 ].loadImage( inPath );
}

int slSkybox::loadBottomImage( std::string &inPath ) {
	return _textures[ 5 ].loadImage( inPath );
}

int slSkybox::loadNumberedImages( std::string &inPath ) {
	return -1;	
}

void slSkybox::draw( slVector *inCameraPos ) {
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
	// glLoadIdentity();

	if( !_textures[ 0 ].isLoaded() ) 
		_textures[ 0 ].loadImage( "BoxFront.png" );
	if( !_textures[ 1 ].isLoaded() ) 
		_textures[ 1 ].loadImage( "BoxBack.png" );
	if( !_textures[ 2 ].isLoaded() ) 
		_textures[ 2 ].loadImage( "BoxLeft.png" );
	if( !_textures[ 3 ].isLoaded() ) 
		_textures[ 3 ].loadImage( "BoxRight.png" );
	if( !_textures[ 4 ].isLoaded() ) 
		_textures[ 4 ].loadImage( "BoxTop.png" );
	if( !_textures[ 5 ].isLoaded() ) 
		_textures[ 5 ].loadImage( "BoxBottom.png" );
 
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
