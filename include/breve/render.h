#ifndef _RENDER_H
#define _RENDER_H

#include "texture.h"

class slCamera;
class slLight;

enum slMatrixMode {
	slMatrixProjection = 1,
	slMatrixGeometry,
	slMatrixTexture
};

enum slBlendMode {
	slBlendNone = 0,
	slBlendAlpha,
	slBlendLight
};

class slMaterial {
	public:
								slMaterial() {
									_texture = NULL;
								}

								~slMaterial() {
									if( _texture ) 
										delete _texture;
								}
								

		float 					_specular[ 4 ];
		float 					_ambient[ 4 ];
		float					_diffuse[ 4 ];
		float					_shininess;

		bool					_twosided;

		std::string				_texturePath;
		slTexture2D*			_texture;
};


class slRender {
	public:
		
};

#if 1
	#define slgl( cmd )			{ ClearErrors( "unknown", 0  ); ( cmd ); ClearErrors( __FILE__, __LINE__ ); }
#else
	#define slgl( cmd ) 		cmd 
#endif

class slRenderGL {
	public:
							slRenderGL();
	
		void				PushMatrix( slMatrixMode inMode ) const;
		void				PopMatrix( slMatrixMode inMode ) const;
		void 				MulMatrix( slMatrixMode inMode, double inMatrix[ 3 ][ 3 ] ) const;
		void 				MulMatrix4( slMatrixMode inMode, float inMatrix[ 16 ] ) const;


		void 				Scale( slMatrixMode inMode, float inX, float inY, float inZ ) const;
		void 				Translate( slMatrixMode inMode, float inX, float inY, float inZ ) const;
		void 				Rotate( slMatrixMode inMode, float inAngle, float inX, float inY, float inZ ) const;

		void 				Clear( float *inColor = NULL ) const;
		void 				ApplyCamera( slCamera *inCamera ) const;
		void				SetBlendMode( slBlendMode inBlendMode );

		void				SetBlendColor( float *inColor ) const;
		void				SetBlendColor( unsigned char *inColor ) const;
		void				SetColorTransformsEnabled( bool inEnabled ) { _colorTransforms = inEnabled; }

		void				PushLight( const slLight *inLight, bool inAmbientOnly = false );
		void				PopLight();
	
		void 				DrawQuad( const slTexture2D& inTexture, const slVector &inCenterPoint, const slVector &inAxis1, const slVector &inAxis2 );

		void 				BeginFlatShadows( slCamera *inCamera, slVector *inLight, float inAlpha = 0.3f );
		void 				EndFlatShadows();
		
		void				BindMaterial( const slMaterial& inMaterial ) const;
		void				UnbindMaterial( const slMaterial& inMaterial ) const;

	protected:
		
		void 				MulShadowMatrix( slPlane *inPlane, slVector *inLight );

		/** \brief Sets the current GL matrix mode. */
		void				SetMatrixMode( slMatrixMode inMode ) const;

		/** \brief Prints out and clears OpenGL errors. */
		void				ClearErrors( const char *inFile, int inLine ) const;
	
	
		bool				_colorTransforms;
		slVertexBufferGL	_quad;
		int					_lightCount;

};

#endif
