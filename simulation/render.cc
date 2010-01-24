#include "render.h"
#include "world.h"
//#include "camera.h"

slRenderGL::slRenderGL() {
	_quad.resize( 4, VB_UV | VB_XYZ );
	_lightCount = 0;
	
	_colorTransforms = true;
}

void slCamera::initGL() {
#ifndef OPENGLES
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_FASTEST );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
	glHint( GL_FOG_HINT, GL_NICEST );
#endif

	glLineWidth( 2 );
	glPolygonOffset( -7.0f, -1.0f );

	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glDepthMask( GL_TRUE );	
	glEnable( GL_COLOR_MATERIAL );

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	
	glEnable( GL_CULL_FACE );
}


void slRenderGL::ClearErrors( const char *inFile, int inLine ) const {
	GLenum e = glGetError();

	if( e )		
		slMessage( DEBUG_ALL, "OpenGL error %d at line %d of file \"%s\"\n", e, inLine, inFile );
}

void slRenderGL::SetBlendMode( slBlendMode inBlendMode ) {
	if( inBlendMode == slBlendNone ) {
		slgl( glDisable( GL_BLEND ) );
		return;
	}

	slgl( glEnable( GL_BLEND ) );
	
	switch( inBlendMode ) {
		case slBlendAlpha:
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			break;
		case slBlendLight:
			glBlendFunc( GL_ONE, GL_ONE );
			break;
			
		default:
			break;
	}
}


void slRenderGL::BindMaterial( const slMaterial& inMaterial ) const {
	glMaterialfv( GL_FRONT, GL_AMBIENT, inMaterial._ambient );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, inMaterial._diffuse );
	glMaterialfv( GL_FRONT, GL_SPECULAR, inMaterial._specular );
	
	float shine = pow( 2.0, 10.0 * inMaterial._shininess );

	if( shine > 128.0 )
		shine = 128.0;

	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, shine ); 

	glColor4f( inMaterial._diffuse[ 0 ], inMaterial._diffuse[ 1 ], inMaterial._diffuse[ 2 ], inMaterial._diffuse[ 3 ] );

	// I CAN HAZ TEXTURZ?

	if( inMaterial._texture )
		inMaterial._texture -> bind();
}

void slRenderGL::UnbindMaterial( const slMaterial& inMaterial ) const {
	if( inMaterial._texture )
		inMaterial._texture -> bind();
}


void slRenderGL::ReadToTexture( slCamera& inCamera ) {
	// During a live-resize drag, the texture is going to be continually resized and 
	// will be degraded as a result.  Therefore, we'll only do the resize if we
	// get the same size request twice in a row indicating that the redraw is 
	// not occurring during a window resize.

	if( inCamera._width != inCamera._readbackX || inCamera._height != inCamera._readbackY ) {
		inCamera._readbackX = inCamera._width;
		inCamera._readbackY = inCamera._height;
		return;
	}

	// slgl( glFlush() );

	inCamera.readbackTexture() -> resize( inCamera._width, inCamera._height, false );
	inCamera.readbackTexture() -> bind();
	slgl( glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, inCamera._width, inCamera._height ) );
	inCamera.readbackTexture() -> unbind();
}



void slRenderGL::DrawQuad( const slTexture2D& inTexture, const slVector &inCenterPoint, const slVector &inAxis1, const slVector &inAxis2 ) {
	slVector v1, v2, v3, v4;
		
	slVectorCopy( &inCenterPoint, &v1 );
	slVectorSub( &v1, &inAxis1, &v1 );
	slVectorSub( &v1, &inAxis2, &v1 );

	slVectorCopy( &inCenterPoint, &v2 );
	slVectorSub( &v2, &inAxis1, &v2 );
	slVectorAdd( &v2, &inAxis2, &v2 );

	slVectorCopy( &inCenterPoint, &v3 );
	slVectorAdd( &v3, &inAxis1, &v3 );
	slVectorSub( &v3, &inAxis2, &v3 );

	slVectorCopy( &inCenterPoint, &v4 );
	slVectorAdd( &v4, &inAxis1, &v4 );
	slVectorAdd( &v4, &inAxis2, &v4 );

	float *v, *t;
	v = _quad.vertex( 0 );
	t = _quad.texcoord( 0 );
	v[ 0 ] = v1.x;
	v[ 1 ] = v1.y;
	v[ 2 ] = v1.z;
	t[ 0 ] = 0;
	t[ 1 ] = 0;

	v = _quad.vertex( 1 );
	t = _quad.texcoord( 1 );
	v[ 0 ] = v2.x;
	v[ 1 ] = v2.y;
	v[ 2 ] = v2.z;
	t[ 0 ] = 0;
	t[ 1 ] = inTexture._unitY;

	v = _quad.vertex( 2 );
	t = _quad.texcoord( 2 );
	v[ 0 ] = v3.x;
	v[ 1 ] = v3.y;
	v[ 2 ] = v3.z;
	t[ 0 ] = inTexture._unitX;
	t[ 1 ] = 0;

	v = _quad.vertex( 3 );
	t = _quad.texcoord( 3 );
	v[ 0 ] = v4.x;
	v[ 1 ] = v4.y;
	v[ 2 ] = v4.z;
	t[ 0 ] = inTexture._unitX;
	t[ 1 ] = inTexture._unitY;
	
	if( inTexture.isLoaded() ) {
		inTexture.bind();
		_quad.bind();
		_quad.draw( VB_TRIANGLE_STRIP );
		_quad.unbind();
		inTexture.unbind();
	}
}

void slRenderGL::ApplyCamera( slCamera *inCamera ) const {
	slgl( glViewport( inCamera -> _originx, inCamera -> _originy, inCamera -> _width, inCamera -> _height ) );

	SetMatrixMode( slMatrixProjection );	
	slgl( glLoadIdentity() );
	
	// slgl( gluPerspective( 40.0, inCamera -> _width / inCamera -> _height inCamera -> _frontClip, inCamera -> _zClip ) );
	// homemade gluPerspective for OpenGL ES

	float m11 = 1.0f / tan( DEGTORAD( 40 / 2.0f ) );
	float m00 = m11 / ( (float) inCamera -> _width / inCamera -> _height );
	float m22 = (     inCamera -> _frontClip + inCamera -> _zClip  ) / ( inCamera -> _frontClip - inCamera -> _zClip );
	float m23 = ( 2 * inCamera -> _frontClip * inCamera -> _zClip  ) / ( inCamera -> _frontClip - inCamera -> _zClip );

	float projection[ 16 ] = {   
		m00,  0.0f, 0.0f, 0.0f,
		0.0f,  m11, 0.0f, 0.0f,
		0.0f, 0.0f,  m22, -1,
		0.0f, 0.0f,  m23, 0.0f
	};

	slgl( glMultMatrixf( projection ) );

	// homemade gluLookAt
	// gluLookAt( position.x, position.y, position.z, inCamera -> _target.x, inCamera -> _target.y, inCamera -> _target.z, 0.0, 1.0, 0.0 );

	SetMatrixMode( slMatrixGeometry );	
	slgl( glLoadIdentity() );
	
	slVector position, s, u, f, up;

	slVectorAdd( &inCamera -> _location, &inCamera -> _target, &position );

	slVectorSet( &up, 0.0, 1.0, 0.0 );

	slVectorMul( &inCamera -> _location, -1, &f );
	slVectorNormalize( &f );
	slVectorCross( &f, &up, &s );
	slVectorNormalize( &s );
	slVectorCross( &s, &f, &u );
	slVectorNormalize( &u );

	float modelview[ 16 ] = {   
		s.x, u.x, -f.x, 0,
		s.y, u.y, -f.y, 0,
		s.z, u.z, -f.z, 0,
		  0,   0,   0, 1 
	};
	
	slgl( glMultMatrixf( modelview ) );
	slgl( glTranslatef( -position.x, -position.y, -position.z ) ); 




	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glDepthMask( GL_TRUE );	
}

void slRenderGL::SetBlendColor( float *inColor ) const {
	if( _colorTransforms )
		glColor4f( inColor[ 0 ], inColor[ 1 ], inColor[ 2 ], inColor[ 3 ] );
}

void slRenderGL::SetBlendColor( unsigned char *inColor ) const {
	if( _colorTransforms )
		glColor4ub( inColor[ 0 ], inColor[ 1 ], inColor[ 2 ], inColor[ 3 ] );
}

void slRenderGL::Clear( float *inColor ) const {
	if( inColor )
		glClearColor( inColor[ 0 ], inColor[ 1 ], inColor[ 2 ], inColor[ 3 ] );
	else 
		glClearColor( 0, 0, 0, 1 );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void slRenderGL::SetMatrixMode( slMatrixMode inMode ) const {
	GLenum mode = 0;
	
	switch( inMode ) {
		case slMatrixProjection:
			mode = GL_PROJECTION;
			break;
		case slMatrixGeometry:
			mode = GL_MODELVIEW;
			break;
		case slMatrixTexture:
			mode = GL_TEXTURE;
			break;
	}
	
	slgl( glMatrixMode( mode ) );
}

void slRenderGL::PushMatrix( slMatrixMode inMode ) const {
	SetMatrixMode( inMode );
	slgl( glPushMatrix() );
}

void slRenderGL::PopMatrix( slMatrixMode inMode ) const {
	SetMatrixMode( inMode );
	slgl( glPopMatrix() );
}

void slRenderGL::SetIdentity( slMatrixMode inMode ) const {
	SetMatrixMode( inMode );
	slgl( glLoadIdentity() );
}


void slRenderGL::MulMatrix( slMatrixMode inMode, double m[3][3] ) const {
	SetMatrixMode( inMode );

	float d[ 16 ];

	d[ 0 ] = m[0][0];
	d[ 1 ] = m[1][0];
	d[ 2 ] = m[2][0];
	d[ 3 ] = 0;

	d[ 4 ] = m[0][1];
	d[ 5 ] = m[1][1];
	d[ 6 ] = m[2][1];
	d[ 7 ] = 0;

	d[ 8 ] = m[0][2];
	d[ 9 ] = m[1][2];
	d[ 10 ] = m[2][2];
	d[ 11 ] = 0;

	d[ 12 ] = 0;
	d[ 13 ] = 0;
	d[ 14 ] = 0;
	d[ 15 ] = 1;

	slgl( glMultMatrixf( d ) );
}

void slRenderGL::MulMatrix4( slMatrixMode inMode, float inMatrix[ 16 ] ) const {
	SetMatrixMode( inMode );

	float d[ 16 ];

	d[ 0 ] = inMatrix[ 0  ];
	d[ 1 ] = inMatrix[ 4  ];
	d[ 2 ] = inMatrix[ 8  ];
	d[ 3 ] = inMatrix[ 12 ];

	d[ 4 ] = inMatrix[ 1  ];
	d[ 5 ] = inMatrix[ 5  ];
	d[ 6 ] = inMatrix[ 9  ];
	d[ 7 ] = inMatrix[ 13 ];

	d[ 8 ] =  inMatrix[ 2  ];
	d[ 9 ] =  inMatrix[ 6  ];
	d[ 10 ] = inMatrix[ 10 ];
	d[ 11 ] = inMatrix[ 14 ];

	d[ 12 ] = inMatrix[ 3  ];
	d[ 13 ] = inMatrix[ 7  ];
	d[ 14 ] = inMatrix[ 11 ];
	d[ 15 ] = inMatrix[ 15 ];

	slgl( glMultMatrixf( (float*)d ) );
}

void slRenderGL::Translate( slMatrixMode inMode, float inX, float inY, float inZ ) const {
	SetMatrixMode( inMode );
	slgl( glTranslatef( inX, inY, inZ ) );
}

void slRenderGL::Scale( slMatrixMode inMode, float inX, float inY, float inZ ) const {
	SetMatrixMode( inMode );
	slgl( glScalef( inX, inY, inZ ) );
}

void slRenderGL::Rotate( slMatrixMode inMode, float inAngle, float inX, float inY, float inZ ) const {
	SetMatrixMode( inMode );
	slgl( glRotatef( inAngle, inX, inY, inZ ) );
}


//void slRenderGL::drawImage() {
//
//}

// void slRenderGL::enableFog( ) {

void slRenderGL::PushLight( const slLight *inLight, bool inAmbientOnly ) {
	if( inLight -> _type ) {
		if( _lightCount == 0 )
			glEnable( GL_LIGHTING );
		
		GLenum lightID = GL_LIGHT0 + _lightCount;

		GLfloat dir[ 4 ] = { inLight -> _location.x, inLight -> _location.y, inLight -> _location.z, 1.0 };
		GLfloat amb[ 4 ] = { inLight -> _ambient.x, inLight -> _ambient.y, inLight -> _ambient.z, 0.0 };
		GLfloat spec[4] = { 0, 0, 0, 0 };
		GLfloat dif[4] = { 0, 0, 0, 0 };

		if ( !inAmbientOnly ) {
			dif[ 0 ] = inLight -> _diffuse.x;
			dif[ 1 ] = inLight -> _diffuse.y;
			dif[ 2 ] = inLight -> _diffuse.z;

			spec[ 0 ] = inLight -> _specular.x;
			spec[ 1 ] = inLight -> _specular.y;
			spec[ 2 ] = inLight -> _specular.z;
		}

		glEnable( lightID );
	
		glLightf( lightID, GL_CONSTANT_ATTENUATION, inLight -> _constantAttenuation );
		glLightf( lightID, GL_LINEAR_ATTENUATION, inLight -> _linearAttenuation );
		glLightfv( lightID, GL_DIFFUSE, dif );
		glLightfv( lightID, GL_AMBIENT, amb );
		glLightfv( lightID, GL_POSITION, dir );
		glLightfv( lightID, GL_SPECULAR, spec );

		_lightCount += 1;
	}
}

void slRenderGL::PopLight() {
	_lightCount -= 1;
	
	if( _lightCount == 0 )
		glDisable( GL_LIGHTING );
}

void slRenderGL::SetDepthWriteEnabled( bool inEnabled ) {
	glDepthMask( inEnabled ? GL_TRUE : GL_FALSE );	
}

void slRenderGL::BeginFlatShadows( slCamera *inCamera, slVector *inLight, float inAlpha ) {
	float color[] = { 0.0, 0.0, 0.0, inAlpha };
	
//	inCamera
	 
	glEnable( GL_POLYGON_OFFSET_FILL );
	 
	PushMatrix( slMatrixGeometry );
	MulShadowMatrix( &inCamera -> _shadowPlane, inLight );
	SetBlendColor( color );
	SetColorTransformsEnabled( false );
}

void slRenderGL::EndFlatShadows() {
	SetColorTransformsEnabled( true );
	PopMatrix( slMatrixGeometry );
	
	glDisable( GL_POLYGON_OFFSET_FILL );
}

/**
 * \brief Set up the rendering matrix for flat shadows on a given plane.
 */

void slRenderGL::MulShadowMatrix( slPlane *inPlane, slVector *inLight ) {
	GLfloat matrix[ 16 ];
	GLfloat lDot =  slVectorDot( &inPlane -> normal, inLight );
	GLfloat vDot = -slVectorDot( &inPlane -> normal, &inPlane -> vertex );

	matrix[ 0  ] = lDot - inLight -> x * inPlane -> normal.x;
	matrix[ 1  ] = 0.f  - inLight -> x * inPlane -> normal.y;
	matrix[ 2  ] = 0.f  - inLight -> x * inPlane -> normal.z;
	matrix[ 3  ] = 0.f  - inLight -> x * vDot;

	matrix[ 4  ] = 0.f  - inLight -> y * inPlane -> normal.x;
	matrix[ 5  ] = lDot - inLight -> y * inPlane -> normal.y;
	matrix[ 6  ] = 0.f  - inLight -> y * inPlane -> normal.z;
	matrix[ 7  ] = 0.f  - inLight -> y * vDot;

	matrix[ 8  ] = 0.f  - inLight -> z * inPlane -> normal.x;
	matrix[ 9  ] = 0.f  - inLight -> z * inPlane -> normal.y;
	matrix[ 10 ] = lDot - inLight -> z * inPlane -> normal.z;
	matrix[ 11 ] = 0.f  - inLight -> z * vDot;

	matrix[ 12 ] = 0.f;
	matrix[ 13 ] = 0.f;
	matrix[ 14 ] = 0.f;
	matrix[ 15 ] = lDot;
	
	MulMatrix4( slMatrixGeometry, matrix );
}

