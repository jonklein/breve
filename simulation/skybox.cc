#include "skybox.h"
#include "simulation.h"
#include "glIncludes.h"
#include "camera.h"
#include "render.h"

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
		if( !_textures[ n ].isLoaded() )
			_loaded = false;
	}

	return result;
}

void slSkybox::draw( slRenderGL& inRenderer, slCamera *inCamera ) {
	if( ! _loaded ) 
		return;
		
	slVector position;
		
	slVectorAdd( &inCamera -> _location, &inCamera -> _target, &position );

	glDepthMask( GL_FALSE );

	inRenderer.PushMatrix( slMatrixGeometry );
	inRenderer.Translate( slMatrixGeometry, position.x, position.y, position.z );
	inRenderer.Scale( slMatrixGeometry, 0.3f * inCamera -> _zClip, 0.3f * inCamera -> _zClip, 0.3f * inCamera -> _zClip );
 
 	// we go a little past the edge of the cube to ensure no gaps...
	float r = 1.005f;

	float f[] = { 1, 1, 1, 1 };
	inRenderer.SetBlendColor( f );

	slVector c, x, y;
	
	slVectorSet( &c, 0, 0, -1 );
	slVectorSet( &x, -r, 0, 0 );
	slVectorSet( &y, 0, r, 0 );
	inRenderer.DrawQuad( _textures[ 0 ], c, x, y );

	slVectorSet( &c, 0, 0, 1 );
	slVectorSet( &x, r, 0, 0 );
	inRenderer.DrawQuad( _textures[ 1 ], c, x, y );

	slVectorSet( &c, 1, 0, 0 );
	slVectorSet( &x, 0, 0, -r );
	slVectorSet( &y, 0, r, 0 );
	inRenderer.DrawQuad( _textures[ 2 ], c, x, y );

	slVectorSet( &c, -1, 0, 0 );
	slVectorSet( &x, 0, 0, r );
	inRenderer.DrawQuad( _textures[ 3 ], c, x, y );

	slVectorSet( &c, 0, 1, 0 );
	slVectorSet( &x, 0, 0, -r );
	slVectorSet( &y, -r, 0, 0 );
	inRenderer.DrawQuad( _textures[ 4 ], c, x, y );

	slVectorSet( &c, 0, -1, 0 );
	slVectorSet( &x, 0, 0, r );
	inRenderer.DrawQuad( _textures[ 5 ], c, x, y );

	inRenderer.PopMatrix( slMatrixGeometry );

	glDepthMask( GL_TRUE );
}
