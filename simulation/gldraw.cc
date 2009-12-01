
//
// FULLY DEPRECATED -- left here for reference and for the good memories.
//



#define REFLECTION_ALPHA	.75

#define LIGHTSIZE 64

#define SELECTION_BUFFER_SIZE 512

int slCamera::select( slWorld *w, int x, int y ) {
	slVector cam;
	GLuint *selections;
	GLuint namesInHit, selection_buffer[ SELECTION_BUFFER_SIZE ];
	GLint hits, viewport[4];
	unsigned int min, nearest = 0xffffffff;
	unsigned int hit = w->_objects.size() + 1;

	viewport[ 0 ] = _originx;
	viewport[ 1 ] = _originy;
	viewport[ 2 ] = _width;
	viewport[ 3 ] = _height;

	glSelectBuffer( SELECTION_BUFFER_SIZE, selection_buffer );
	glRenderMode( GL_SELECT );
	slClearGLErrors( "selected buffer" );

	glInitNames();
	glPushName( 0 );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();

	gluPickMatrix(( GLdouble )x, ( GLdouble )( viewport[3] - y ), 5.0, 5.0, viewport );
	slClearGLErrors( "picked matrix" );
	gluPerspective( 40.0, _fov, _frontClip, _zClip );

	// since the selection buffer uses unsigned ints for names, we can't
	// use -1 to mean no selection -- we'll use the number of objects
	// plus 1 to indicate that no selectable object was selected.

	glLoadName( w->_objects.size() + 1 );

	slClearGLErrors( "about to select" );

	slVectorAdd( &_location, &_target, &cam );
	gluLookAt( cam.x, cam.y, cam.z, _target.x, _target.y, _target.z, 0.0, 1.0, 0.0 );

	// Render the objects in the world, with name loading, and with billboards as spheres

	renderObjects( w, DO_BILLBOARDS_AS_SPHERES | DO_LOAD_NAMES );

	hits = glRenderMode( GL_RENDER );

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	slClearGLErrors( "picked" );

	selections = &selection_buffer[0];

	for ( int n = 0; n < hits; ++n ) {
		namesInHit = *selections++;

		// skip over the z-max value

		selections++;
		min = *selections++;

		if ( min < nearest ) {
			nearest = min;

			while ( namesInHit-- )
				hit = *selections++;
		} else
			while ( namesInHit-- )
				selections++;
	}

	glPopName();

	if ( hit == w->_objects.size() + 1 )
		return -1;

	return hit;
}

/**
 * Computes the vector corresponding to a drag in the display.
 * 
 * Computes the location in the same plane as dragVertex that the mouse
 * is being dragged to when the window mouse coordinates are x and y.
 */

int slCamera::vectorForDrag( slWorld *w, slVector *dragVertex, int x, int y, slVector *dragVector ) {
	slPlane plane;
	slVector cam, end;
	GLdouble model[16];
	GLdouble proj[16];
	GLdouble wx, wy;
	GLdouble oxf, oyf, ozf;
	double sD, eD;
	slVector *t;
	GLint view[4];

	// set up the matrices for a regular draw--gluUnProject needs this

	t = &_target;

	view[0] = _originx;
	view[1] = _originy;
	view[2] = _width;
	view[3] = _height;

	glViewport( _originx, _originy, _width, _height );
	glMatrixMode( GL_PROJECTION );

	glPushMatrix();

	glLoadIdentity();

	gluPerspective( 80.0, _fov, _frontClip, _zClip );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	slVectorAdd( &_location, t, &cam );
	gluLookAt( cam.x, cam.y, cam.z, t->x, t->y, t->z, 0.0, 1.0, 0.0 );

	// get the data for gluUnProject

	glGetDoublev( GL_MODELVIEW_MATRIX, model );
	glGetDoublev( GL_PROJECTION_MATRIX, proj );

	y = view[3] - y;

	wx = x;
	wy = y;

	// use gluUnProject to get the point in object space where we are clicking
	// (and at the far reach of our zClip variable).

	gluUnProject( wx, wy, 1.0, model, proj, view, &oxf, &oyf, &ozf );

	end.x = oxf;
	end.y = oyf;
	end.z = ozf;

	// define the plane where the object in question lies

	slVectorCopy( &_location, &plane.normal );
	slVectorNormalize( &plane.normal );
	slVectorCopy( dragVertex, &plane.vertex );

	//	compute the distance
	//		1) from the camera to the object plane
	//		2) from the zClip plane to the object plane

	sD = slPlaneDistance( &plane, &cam );
	eD = slPlaneDistance( &plane, &end );

	eD = fabs( eD );
	sD = fabs( sD );

	// compute the vector from the camera to the end of the zClip plane.
	// this is the vector containing all of the candidate points that the
	// user is dragging--we need to figure out which one we're interested in

	slVectorSub( &end, &cam, dragVector );

	// compute the point on the object plane of the drag vector

	slVectorMul( dragVector, ( sD / ( eD + sD ) ), dragVector );
	slVectorAdd( dragVector, &cam, dragVector );

	glLoadIdentity();

	return 0;
}

void slCamera::renderScene( slWorld *w, int crosshair ) {
	std::vector< slCamera* >::iterator ci;

	if ( w->detectLightExposure() && !w->drawLightExposure() )
		detectLightExposure( w, 200, NULL );

	renderWorld( w, crosshair, 0 );

	for ( ci = w->_cameras.begin(); ci != w->_cameras.end(); ci++ )
		if ( *ci != this )( *ci )->renderWorld( w, 0, 1 );

	if ( w->detectLightExposure() && w->drawLightExposure() )
		detectLightExposure( w, 200, NULL );
}

void slCamera::renderWorld( slWorld *w, int crosshair, int scissor ) {
	slVector cam;
	int flags = 0;

	if ( !w ) 
		return;

	glViewport( _originx, _originy, _width, _height );

	glEnable( GL_BLEND );

	if ( scissor ) {
		flags |= DO_NO_AXIS | DO_NO_BOUND;
		glEnable( GL_SCISSOR_TEST );
		glScissor( _originx, _originy, _width, _height );
	}

	if ( _drawOutline )
		flags |= DO_OUTLINE | DO_BILLBOARDS_AS_SPHERES;

	if ( _recompile ) {
		_recompile = 0;
		flags |= DO_RECOMPILE;
	}

	clear( w );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	drawFog();

	if ( w->backgroundTexture > 0 && !( flags & DO_OUTLINE ) )
		drawBackground( w );

	glEnable( GL_LIGHTING );

	glMatrixMode( GL_PROJECTION );
	gluPerspective( 40.0, _fov, _frontClip, _zClip );

	glEnable( GL_DEPTH_TEST );

	glMatrixMode( GL_MODELVIEW );

	glPushMatrix();

	glLoadIdentity();

	slVectorAdd( &_location, &_target, &cam );

	gluLookAt( cam.x, cam.y, cam.z, _target.x, _target.y, _target.z, 0.0, 1.0, 0.0 );

	updateFrustum();

	w -> _skybox.draw( &this );

	//
	// Lines and draw-commands are special objects which will be rendered before lighting is setup
	//

	std::vector<slDrawCommandList*>::iterator di;
	for ( di = w->_drawings.begin(); di != w->_drawings.end(); di++ )( *di )->draw( this );

	//
	// Setup lighting and effects for the normal objects
	//

	bool flatShadows = !_drawShadowVolumes && _drawShadow && _shadowCatcher;

	setupLights( _drawShadowVolumes );

	int reflectionFlags = 0;

	if ( _drawLights ) {
		if ( _drawReflection || flatShadows )
			stencilFloor( w );

		if ( _drawReflection && !( flags & DO_OUTLINE ) ) {
			slVector toCam;

			if ( !_drawShadowVolumes ) 
				gReflectionAlpha = REFLECTION_ALPHA;
			else 
				gReflectionAlpha = REFLECTION_ALPHA - 0.1;

			slVectorSub( &cam, &_shadowPlane.vertex, &toCam );

			if ( slVectorDot( &toCam, &_shadowPlane.normal ) > 0.0 ) {
				reflectionPass( w, _drawShadowVolumes );
				reflectionFlags = DO_NO_SHADOWCATCHER;
			}
		}
	} 

	renderObjects( w, flags | reflectionFlags | DO_NO_ALPHA );

	renderLines( w );

	slClearGLErrors( "drew non-alpha bodies" );

	// now we do transparent objects and billboards.  they have to come last
	// because they are blended.

	if ( !( flags & DO_BILLBOARDS_AS_SPHERES ) ) {
		processBillboards( w );
		renderBillboards( flags );
	}

	std::vector< slPatchGrid* >::iterator pi;

	for ( pi = w->_patches.begin(); pi != w->_patches.end(); pi++ )
		( *pi )->draw( this );

	if ( _drawLights ) {
		if ( _drawShadowVolumes ) 
			renderShadowVolume( w );
		else if ( flatShadows ) 
			drawFlatShadows( w );
	}

	glDepthMask( GL_FALSE );

	renderObjects( w, flags | DO_ONLY_ALPHA );

	slClearGLErrors( "drew alpha bodies" );

	glDepthMask( GL_TRUE );

	renderLabels( w );

	slClearGLErrors( "rendered labels" );

#if HAVE_LIBENET
	slDrawNetsimBounds( w );
#endif

	if ( w->gisData ) w->gisData->draw( this );

	glPopMatrix();

	glMatrixMode( GL_PROJECTION );

	glLoadIdentity();

	if ( _drawText ) 
		renderText( w, crosshair );

	if ( _drawText && crosshair && !scissor ) {
		glPushMatrix();
		glColor3f( 0, 0, 0 );
		slText( 0, 0, "+", GLUT_BITMAP_9_BY_15 );
		glPopMatrix();
	}

	if ( scissor ) {
		double minY, maxY, minX, maxX;

		minX = -1;
		maxX =  1;
		minY = -1;
		maxY =  1;

		glLineWidth( 2.0 );

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();

		glColor4f( 0, 0, 0, .5 );
		glBegin( GL_LINES );
		glVertex2f( minX, minY );
		glVertex2f( maxX, minY );

		glVertex2f( maxX, minY );
		glVertex2f( maxX, maxY );

		glVertex2f( maxX, maxY );
		glVertex2f( minX, maxY );

		glVertex2f( minX, maxY );
		glVertex2f( minX, minY );
		glEnd();

		glDisable( GL_SCISSOR_TEST );
	}

	if( _drawBlur ) 
		readbackToTexture();
}

void slCamera::clear( slWorld *w ) {
	if ( _drawOutline )
		glClearColor( 1, 1, 1, 0 );
	else
		glClearColor( w->backgroundColor.x, w->backgroundColor.y, w->backgroundColor.z, 1.0 );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	if( _drawBlur ) { 
		setBlendMode( SR_ALPHABLEND );

		glColor4f( 1.0f, 1.0f, 1.0f, _blurFactor );

		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glLoadIdentity();
		glOrtho( -1, 1, -1, 1, -1, 1 );

		glDepthMask( GL_FALSE );

		glEnable( GL_TEXTURE_2D );
		_readbackTexture -> bind();

		GLenum mode = GL_NEAREST;

		if( _readbackTexture -> _texX != _width || _readbackTexture -> _texY != _height )
			mode = GL_LINEAR;

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode );

		if( _readbackTexture -> _texX != 0 && _readbackTexture -> _texY != 0 ) {
			glBegin( GL_TRIANGLE_STRIP );

			glTexCoord2f( 0, 0 );
			glVertex2f( -1, -1 );
			glTexCoord2f( _readbackTexture -> _unitX, 0 );
			glVertex2f(  1, -1 );
			glTexCoord2f( 0, _readbackTexture -> _unitY );
			glVertex2f( -1,  1 );
			glTexCoord2f( _readbackTexture -> _unitX, _readbackTexture -> _unitY );
			glVertex2f(  1,  1 );

			glEnd();
		}

		_readbackTexture -> unbind();

		glPopMatrix();
		glDepthMask( GL_TRUE );
	}
}

void slCamera::drawFog() {
	if ( _drawFog ) {
		GLfloat color[4] = { _fogColor.x, _fogColor.y, _fogColor.z, 1.0 };

		glEnable( GL_FOG );
		glFogf( GL_FOG_DENSITY, _fogIntensity );
		glFogi( GL_FOG_MODE, GL_LINEAR );
		glFogf( GL_FOG_START, _fogStart ) ;
		glFogf( GL_FOG_END, _fogEnd );
		glFogfv( GL_FOG_COLOR, color );
	} else {
		glDisable( GL_FOG );
	}
}

/*!
	\brief Puts 1 into the stencil buffer where the shadows and reflections should fall.
*/

void slCamera::stencilFloor( slWorld *inWorld ) {
	glEnable( GL_STENCIL_TEST );
	glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );

	// glDepthFunc( GL_ALWAYS );
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glStencilFunc( GL_ALWAYS, 1, 0xffffffff );
	glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );

	renderObjects( inWorld, DO_ONLY_SHADOWCATCHER );

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glEnable( GL_DEPTH_TEST );
	// glDepthFunc( GL_LESS );
	glDepthMask( GL_TRUE );
	glDisable( GL_STENCIL_TEST );
}

/*!
	\brief Draws a reflection of all multibody objects whereever the stencil buffer is equal to 1.
*/

void slCamera::reflectionPass( slWorld *w, bool inWillDoShadowVolumes ) {
	glPushMatrix();

	glScalef( 1.0, -1.0, 1.0 );
	glTranslatef( 0.0, -2 * _shadowPlane.vertex.y, 0.0 );

	glEnable( GL_CULL_FACE );
	glCullFace( GL_FRONT );

	// render whereever the buffer is 1, but don't change the values

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_STENCIL_TEST );

	glStencilFunc( GL_LESS, 0, 0xffffffff );

	GLdouble plane[] = { 0, 1, 0, -2 * _shadowPlane.vertex.y };
	glEnable( GL_CLIP_PLANE0 );						
	glClipPlane( GL_CLIP_PLANE0, plane );	

	glStencilOp( GL_INCR, GL_INCR, GL_INCR );
	renderObjects( w, DO_NO_SHADOWCATCHER | DO_NO_BOUND | DO_NO_AXIS | DO_NO_TERRAIN );
	renderBillboards( 0 );

	glPopMatrix();

	glCullFace( GL_BACK );
	glDisable( GL_CLIP_PLANE0 );

	glStencilFunc( GL_EQUAL, 1, 0xffffffff );
	glStencilOp( GL_DECR, GL_DECR, GL_KEEP );
	renderObjects( w, DO_ONLY_SHADOWCATCHER );

	glDisable( GL_STENCIL_TEST );


	setBlendMode( SR_ALPHABLEND );

	renderObjects( w, DO_ONLY_SHADOWCATCHER, inWillDoShadowVolumes ? REFLECTION_ALPHA / 2.0 : REFLECTION_ALPHA );
}

/**
 * \brief Shadows multibody objects on to the specified shadow plane, expecting
 * that the stencil buffer has already been set to 3.
 *
 * The stencil buffer will be modified to 2 where the shadows are drawn.
 */

void slCamera::drawFlatShadows( slWorld *w ) {
	GLfloat shadowMatrix[4][4];
	slShadowMatrix( shadowMatrix, &_shadowPlane, &_lights[ 0 ]._location );

	glPushAttrib( GL_ENABLE_BIT );

	glEnable( GL_STENCIL_TEST );
	glEnable( GL_DEPTH_TEST );

	glStencilFunc( GL_LESS, 0, 0xffffffff );
	glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );

	glEnable( GL_POLYGON_OFFSET_FILL );

	setBlendMode( SR_ALPHABLEND );

	glColor4f( 0.0, 0.0, 0.0, 0.3 );

	glPushMatrix();
	glMultMatrixf( (GLfloat*)shadowMatrix );
	glDisable( GL_LIGHTING );

	renderObjects( w, DO_NO_COLOR | DO_NO_TEXTURE | DO_NO_SHADOWCATCHER | DO_NO_BOUND | DO_NO_AXIS | DO_NO_TERRAIN );
	renderBillboards( DO_NO_COLOR | DO_NO_BOUND );

	glPopMatrix();

	glPopAttrib();
}

/*!
	\brief Render the text associated with the current display.
*/

void slCamera::renderText( slWorld *w, int crosshair ) {
	double fromLeft;
	unsigned int n;
	char textStr[ 128 ];

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_TEXTURE_2D );
	glColor4f( _textColor.x, _textColor.y, _textColor.z, 1.0 );

	snprintf( textStr, sizeof( textStr ), "%.2f", w->_age );

	fromLeft = -1.0 + ( 5.0 / _width );
	slText( fromLeft, 1.0 - ( 20.0 / _height ), textStr, GLUT_BITMAP_HELVETICA_10 );

	if ( crosshair ) {
		snprintf( textStr, sizeof( textStr ), "camera: (%.1f, %.1f, %.1f)",
		          _location.x, _location.y, _location.z );
		slText( fromLeft, -1.0 + ( 5.0 / _height ), textStr, GLUT_BITMAP_HELVETICA_10 );
		snprintf( textStr, sizeof( textStr ), "target: (%.1f, %.1f, %.1f)",
		          _target.x, _target.y, _target.z );
		slText( fromLeft, -1.0 + ( 30.0 / _height ), textStr, GLUT_BITMAP_HELVETICA_10 );
	} else {
		for ( n = 0; n < _text.size(); n++ ) {
			glColor4f( _text[n].color.x, _text[n].color.y, _text[n].color.z, 0.9 );
			slStrokeText( _text[n].x, _text[n].y, _text[n].text.c_str(), _textScale, GLUT_STROKE_ROMAN );
		}
	}
}

/**
 * \brief Draw a texture as a background.
 */

void slCamera::drawBackground( slWorld *w ) {
	static float transX = 0.0, transY = 0.0;

	glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );

	glColor4f( w->backgroundTextureColor.x, w->backgroundTextureColor.y, w->backgroundTextureColor.z, 1.0 );

	glDisable( GL_LIGHTING );

	transX += _backgroundScrollX;
	transY += _backgroundScrollY;

	glMatrixMode( GL_TEXTURE );
	glPushMatrix();
	glTranslatef( transX, transY, 0 );
	
	w -> backgroundTexture -> bind();
	
	glBegin( GL_QUADS );
	glTexCoord2f( .0, .0 );
	glVertex3f( -1, -1, -.1 );
	glTexCoord2f( 1.0, .0 );
	glVertex3f( 1, -1, -.1 );
	glTexCoord2f( 1.0, 1.0 );
	glVertex3f( 1, 1, -.1 );
	glTexCoord2f( .0, 1.0 );
	glVertex3f( -1, 1, -.1 );
	glEnd();
	
	w -> backgroundTexture -> unbind();

	glPopMatrix();

	glDepthMask( GL_TRUE  );
	glEnable( GL_DEPTH_TEST );
}

void slCamera::renderLabels( slWorld *w ) {
	slWorldObject *wo;
	slVector *l;
	std::vector<slWorldObject*>::iterator wi;

	glDisable( GL_DEPTH_TEST );

	glColor3f( 0, 0, 0 );

	for ( wi = w->_objects.begin(); wi != w->_objects.end(); wi++ ) {
		wo = *wi;

		if ( wo && !wo->_label.empty() ) {
			l = &wo->_position.location;
			glPushMatrix();
			glTranslatef( l->x, l->y, l->z );
			slText( 0, 0, wo->_label.c_str() , GLUT_BITMAP_HELVETICA_10 );
			glPopMatrix();
		}
	}

	glEnable( GL_DEPTH_TEST );
}

void slCamera::setBlendMode( slRenderBlendMode inBlendMode ) {
	switch( inBlendMode ) {
		case SR_ALPHABLEND:
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			break;
		case SR_LIGHTBLEND:
			glBlendFunc( GL_ONE, GL_ONE );
			break;
	}
}

void slStrokeText( double x, double y, const char *string, double scale, void *font ) {
	int c;

	glPushMatrix();

	glLineWidth( 1.0 );
	glTranslatef( x, y, 0 );

	glScalef( scale * 0.0005, scale * 0.0005, scale * 0.0005 );

	while (( c = *( string++ ) ) != 0 )
		glutStrokeCharacter( font, c );

	glPopMatrix();
}

void slText( double x, double y, const char *string, void *font ) {

	int c;

	glRasterPos2f( x, y );

	while (( c = *( string++ ) ) != 0 ) 
		glutBitmapCharacter( font, c );
}
	
/*!
 * \brief Renders a stationary object.
 */

void slCamera::processBillboards( slWorld *inWorld ) {
	GLfloat matrix[ 16 ];
	std::vector<slWorldObject*>::iterator wi;

	glGetFloatv( GL_MODELVIEW_MATRIX, matrix );

	_billboardCount = 0;

	for ( wi = inWorld->_objects.begin(); wi != inWorld->_objects.end(); wi++ ) {
		slWorldObject *wo = *wi;

		if ( wo && wo->_textureMode != BBT_NONE && wo->_displayShape && wo->_displayShape->_type == ST_SPHERE ) {
			double z = 0;

			slSphere *ss = static_cast< slSphere* >( wo->_displayShape );

			z = matrix[2] * wo->_position.location.x + matrix[6] * wo->_position.location.y + matrix[10] * wo->_position.location.z;

			addBillboard( wo, ss->_radius, z );
		}
	}

	if( _billboardCount == 0 ) 
		return;

	sortBillboards();

	_billboardX.x = matrix[0];
	_billboardX.y = matrix[4];
	_billboardX.z = matrix[8];

	_billboardY.x = matrix[1];
	_billboardY.y = matrix[5];
	_billboardY.z = matrix[9];

	_billboardZ.x = matrix[2];
	_billboardZ.y = matrix[6];
	_billboardZ.z = matrix[10];
	
	
	float *v;
	
	v = _billboardBuffer.texcoord( 0 );
	v[ 0 ] = 1.0;
	v[ 1 ] = 1.0;

	v = _billboardBuffer.texcoord( 1 );
	v[ 0 ] = 0.0;	
	v[ 1 ] = 1.0;
	
	v = _billboardBuffer.texcoord( 2 );
	v[ 0 ] = 1.0;
	v[ 1 ] = 0.0;
	
	v = _billboardBuffer.texcoord( 3 );
	v[ 0 ] = 0.0;
	v[ 1 ] = 0.0;

	v = _billboardBuffer.vertex( 0 );
	v[ 0 ] =  _billboardX.x + _billboardY.x;  
	v[ 1 ] =  _billboardX.y + _billboardY.y;
	v[ 2 ] =  _billboardX.z + _billboardY.z;

	v = _billboardBuffer.vertex( 1 );
	v[ 0 ] = -_billboardX.x + _billboardY.x;
	v[ 1 ] = -_billboardX.y + _billboardY.y;
	v[ 2 ] = -_billboardX.z + _billboardY.z;

	v = _billboardBuffer.vertex( 2 );
	v[ 0 ] =  _billboardX.x - _billboardY.x;
	v[ 1 ] =  _billboardX.y - _billboardY.y;
	v[ 2 ] =  _billboardX.z - _billboardY.z;

	v = _billboardBuffer.vertex( 3 );
	v[ 0 ] = -_billboardX.x - _billboardY.x;
	v[ 1 ] = -_billboardX.y - _billboardY.y;
	v[ 2 ] = -_billboardX.z - _billboardY.z;

}

/**
 * \brief Renders the objects, assuming that all necessary transformations
 * have been set up.
 */

void slCamera::renderObjects( slWorld *w, unsigned int flags, float inAlphaScale ) {
	slWorldObject *wo;
	unsigned int n;
	bool color = true;

	const int loadNames = ( flags & DO_LOAD_NAMES );
	const int doNoAlpha = ( flags & DO_NO_ALPHA );
	const int doOnlyAlpha = ( flags & DO_ONLY_ALPHA );
	const int doNoShadowCatcher = ( flags & DO_NO_SHADOWCATCHER );
	const int doOnlyShadowCatcher = ( flags & DO_ONLY_SHADOWCATCHER );
	const int doNoTerrain = ( flags & DO_NO_TERRAIN );
	const int doNoTexture = ( flags & DO_NO_TEXTURE );

	_points.clear();

	if ( doOnlyAlpha ) 
		setBlendMode( SR_ALPHABLEND );

	if ( flags & ( DO_OUTLINE | DO_NO_COLOR ) ) 
		color = 0;

	if ( flags & DO_OUTLINE ) 
		glColor4f( 1, 1, 1, 1 );

	for ( n = 0; n < w->_objects.size(); ++n ) {
		int skip = 0;
		wo = w->_objects[n];

		if ( !wo ) skip = 1;
		else if ( wo->_drawMode == DM_INVISIBLE ) skip = 1;
		else if ( doNoAlpha && wo->_alpha != 1.0 ) skip = 1;
		else if ( doOnlyAlpha && wo->_alpha == 1.0 ) skip = 1;
		else if ( doNoShadowCatcher && wo == _shadowCatcher ) skip = 1;
		else if ( doOnlyShadowCatcher && wo != _shadowCatcher ) skip = 1;
		else if ( doNoTerrain && wo->_type == WO_TERRAIN ) skip = 1;
		else if ( wo->_textureMode != BBT_NONE && !( flags & DO_BILLBOARDS_AS_SPHERES ) ) skip = 1;

		if ( !skip ) {
			if ( loadNames )
				glLoadName( n );

			if ( !wo->_drawAsPoint ) {
				if ( color )
					glColor4f( wo->_color.x, wo->_color.y, wo->_color.z, wo->_alpha * inAlphaScale );

				if ( !doNoTexture && wo->_texture ) {

					if ( wo->_textureMode == BBT_LIGHTMAP )
						setBlendMode( SR_LIGHTBLEND );
					else
						setBlendMode( SR_ALPHABLEND );

					wo -> _texture -> bind();
				}

				wo->draw( this );

				if ( !doNoTexture && wo -> _texture )
					wo -> _texture -> unbind();

			} else
				_points.push_back( std::pair< slVector, slVector>( wo->getPosition().location, wo->_color ) );
		}
	}

	if ( !doOnlyAlpha && _points.size() ) {
		glPointSize( 2.0 );

		glEnable( GL_POINT_SMOOTH );
		glBegin( GL_POINTS );

		for ( n = 0; n < _points.size(); ++n ) {
			slVector &v = _points[n].first;
			slVector &c = _points[n].second;

			glColor4d( c.x, c.y, c.z, 1.0 );
			glVertex3d( v.x, v.y, v.z );
		}

		glEnd();
		glDisable( GL_POINT_SMOOTH );
	}

	if ( loadNames )
		glLoadName( w->_objects.size() + 1 );

	// Restore the default blend func

	setBlendMode( SR_ALPHABLEND );
}

/**
 * \brief Renders object neighbor lines.
 */

void slCamera::renderLines( slWorld *w ) {
	slWorldObject *neighbor;
	slVector *x, *y;
	unsigned int n;

	glLineWidth( 1.2 );
	setBlendMode( SR_ALPHABLEND );
	glDepthFunc( GL_ALWAYS );
	glEnable( GL_LINE_SMOOTH );
	glDepthMask( GL_FALSE );

	for ( n = 0; n < w->_objects.size(); ++n ) {
		if ( w->_objects[n] && !( w->_objects[n]->_drawMode & DM_INVISIBLE ) ) {
			if ( w && w->_objects[n]->_drawMode & DM_NEIGHBOR_LINES ) {
				std::vector<slWorldObject*>::iterator wi;

				glColor4f( 0.0, 0.0, 0.0, 0.5 );

				x = &w->_objects[n]->_position.location;

				glBegin( GL_LINES );

				for ( wi = w->_objects[n]->_neighbors.begin(); wi != w->_objects[n]->_neighbors.end(); wi++ ) {
					neighbor = *wi;

					if ( neighbor ) {
						y = &neighbor->_position.location;

						glVertex3f( x->x, x->y, x->z );
						glVertex3f( y->x, y->y, y->z );
					}
				}

				glEnd();
			}
		}
	}

	glDisable( GL_LINE_SMOOTH );

	glLineWidth( 1.0 );

	std::vector< slObjectConnection* >::iterator li;

	for ( li = w->_connections.begin(); li != w->_connections.end(); li++ ) {
		( *li )->draw( this );
	}

	glDepthFunc( GL_LESS );
	glDepthMask( GL_TRUE );
}

/*!
	\brief Renders an x/y axis of a given length.
*/

void slDrawAxis( double x, double y ) {
	x += .02;
	y += .02;

	glEnable( GL_LINE_SMOOTH );

	glBegin( GL_LINES );
	glVertex3f( 0, 0, 0 );
	glVertex3f( x, 0, 0 );
	glVertex3f( 0, y, 0 );
	glVertex3f( 0, 0, 0 );
	glEnd();

	glPushMatrix();
	glTranslated( x + 0.2, -0.2, 0 );
	glScaled( .005, .005, .005 );
	glutStrokeCharacter( GLUT_STROKE_ROMAN, 'x' );
	glPopMatrix();

	glPushMatrix();
	glTranslated( -0.2, y + 0.2, 0 );
	glScaled( .005, .005, .005 );
	glutStrokeCharacter( GLUT_STROKE_ROMAN, 'y' );
	glPopMatrix();

	glDisable( GL_LINE_SMOOTH );
}

/**
 * \brief Compiles a shape into a draw list.
 * The draw list is generated or updated as needed.
 */

int slCompileShape( slShape *s, int drawMode, int flags ) {
	if ( !s->_drawList ) 
		s->_drawList = glGenLists( 1 );

	s->_recompile = 0;

	glNewList( s->_drawList, GL_COMPILE );

	slRenderShape( s, drawMode, flags );

	glEndList();

	return s->_drawList;
}

/**
 * \brief Render a shape.
 * 
 * Typically only called when compiling a drawlist for a shape, this does
 * the actual rendering.
 */

void slRenderShape( slShape *s, int drawMode, int flags ) {
	GLUquadricObj *quad;
	int divisions;

	if ( s->_type == ST_SPHERE ) {
		double radius = (( slSphere * )s )->_radius;

		if ( radius < 20 )
			divisions = 20;
		else
			divisions = ( int )radius;

		quad = gluNewQuadric();

		if ( drawMode != GL_POLYGON )
			gluQuadricDrawStyle( quad, GLU_LINE );

		gluQuadricTexture( quad, GL_TRUE );
		gluQuadricOrientation( quad, GLU_OUTSIDE );
		gluSphere( quad, radius, divisions, divisions );
		gluDeleteQuadric( quad );
	} else {
		std::vector<slFace*>::iterator fi;

		for ( fi = s->faces.begin(); fi != s->faces.end(); fi++ )
			slDrawFace( *fi, drawMode, flags );
	}
}

/**
 * \brief Draws a face, breaking it down into smaller triangles if necessary.
 */

void slDrawFace( slFace *f, int drawMode, int flags ) {
	slVector xaxis, yaxis;
	slVector *norm, *v;
	int pointCount;

	norm = &f->plane.normal;

	slPerpendicularVectors( norm, &xaxis, &yaxis );

	glNormal3f( norm->x, norm->y, norm->z );

	// if they're drawing lines, or if the face isn't broken down, do a normal polygon

	if ( drawMode == GL_LINE_LOOP || !slBreakdownFace( f ) ) {
		glBegin( drawMode );

		for ( pointCount = 0;pointCount < f-> _pointCount ;pointCount++ ) {
			slPoint *p = ( slPoint * )f->points[pointCount];

			v = &p->vertex;

			glTexCoord2f( slVectorDot( v, &xaxis ), slVectorDot( v, &yaxis ) );

			glVertex3d( v->x, v->y, v->z );
		}

		glEnd();
	}
}

/**
 * \brief Recursively break down and draw a face.
 *
 * Breaks down faces into smaller polygons in order to improve the quality of lighting and other effects.
 */

int slBreakdownFace( slFace *f ) {
	slVector diff, middle, subv[3], total, xaxis, yaxis;
	double length = 0;
	slVector *v1, *v2;
	slPoint *p;
	int n, n2;

	slPerpendicularVectors( &f->plane.normal, &xaxis, &yaxis );

	slVectorZero( &total );

	for ( n = 0; n < f-> _pointCount; ++n ) {
		n2 = n + 1;

		if ( n2 == f-> _pointCount )
			n2 = 0;

		p = f->points[n];

		v1 = &p->vertex;

		p = f->points[n2];

		v2 = &p->vertex;

		slVectorSub( v1, v2, &diff );

		slVectorAdd( v1, &total, &total );

		length += slVectorLength( &diff );
	}

	slVectorMul( &total, 1.0 / f-> _pointCount, &total );

	if ( length < 30 ) return 0;

	glBegin( GL_TRIANGLES );

	for ( n = 0; n < f-> _pointCount; ++n ) {
		n2 = n + 1;

		if ( n2 == f -> _pointCount )
			n2 = 0;

		p = f->points[n];

		v1 = &p->vertex;

		p = f->points[n2];

		v2 = &p->vertex;

		slVectorSub( v2, v1, &diff );
		slVectorMul( &diff, .5, &diff );
		slVectorAdd( v1, &diff, &middle );

		slVectorCopy( v1, &subv[0] );
		slVectorCopy( &middle, &subv[1] );
		slVectorCopy( &total, &subv[2] );

		slBreakdownTriangle( &subv[0], 0, &xaxis, &yaxis );

		slVectorCopy( &middle, &subv[0] );
		slVectorCopy( v2, &subv[1] );
		slVectorCopy( &total, &subv[2] );

		slBreakdownTriangle( &subv[0], 0, &xaxis, &yaxis );
	}

	glEnd();

	return 1;
}

void slBreakdownTriangle( slVector *v, int level, slVector *xaxis, slVector *yaxis ) {

	slVector diff, mids[3], subv[3];
	double length = 0;
	int n, n2;

	for ( n = 0; n < 3; ++n ) {
		n2 = n + 1;

		if ( n2 == 3 )
			n2 = 0;

		slVectorSub( &v[n2], &v[n], &diff );

		length += slVectorLength( &diff );

		slVectorMul( &diff, .5, &diff );

		slVectorAdd( &v[n], &diff, &mids[n] );
	}

	if ( length < 120.0f || level > 4 ) {
		for ( n = 0; n < 3; ++n ) {
			glTexCoord2f( slVectorDot( &v[n], xaxis ), slVectorDot( &v[n], yaxis ) );
			glVertex3f( v[n].x, v[n].y, v[n].z );
		}

		return;
	} else {
		slVectorCopy( &v[0], &subv[0] );
		slVectorCopy( &mids[0], &subv[1] );
		slVectorCopy( &mids[2], &subv[2] );

		slBreakdownTriangle( &subv[0], level + 1, xaxis, yaxis );

		slVectorCopy( &mids[0], &subv[0] );
		slVectorCopy( &v[1], &subv[1] );
		slVectorCopy( &mids[1], &subv[2] );

		slBreakdownTriangle( &subv[0], level + 1, xaxis, yaxis );

		slVectorCopy( &mids[1], &subv[0] );
		slVectorCopy( &v[2], &subv[1] );
		slVectorCopy( &mids[2], &subv[2] );

		slBreakdownTriangle( &subv[0], level + 1, xaxis, yaxis );

		slVectorCopy( &mids[0], &subv[0] );
		slVectorCopy( &mids[1], &subv[1] );
		slVectorCopy( &mids[2], &subv[2] );

		slBreakdownTriangle( &subv[0], level + 1, xaxis, yaxis );
	}
}

/**
 * \brief Prints out and clears OpenGL errors.
 */

inline int slClearGLErrors( const char *inID ) {
	unsigned int n;
	int c = 0;

	while ( ( n = glGetError() ) ) {
		#ifdef OPENGLES
			slMessage( DEBUG_ALL, "%s: OpenGL error %d\n", inID, n );
		#else
			slMessage( DEBUG_ALL, "%s: OpenGL error %s\n", inID, gluErrorString( n ) );		
		#endif
		
		c++;
	}

	return c;
}
