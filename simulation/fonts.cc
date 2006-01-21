/*
 *  fonts.cpp
 *  deskworld
 *
 *  Created by jon klein on 6/17/05.
 *  Copyright 2005 artificial. All rights reserved.
 *
 */

#ifdef HAVE_LIBFTGL

#include "fonts.h"

slFontManager *slFontManager::sharedFontManager() {
	static slFontManager _sharedFontManager;
	return &_sharedFontManager;
}

FTFont *slFontManager::getFont(std::string file, int size) {
	FTFont *font; 
	
	std::pair< std::string, int > index(file, size);

	if( _fonts[ index]) return _fonts[ index];
	
	font = new FTGLTextureFont( file.c_str() );
	
	// printf("loading font %s [%d]\n", file.c_str(), size);
	
	font->FaceSize( size);
	font->CharMap(ft_encoding_unicode);

	_fonts[ index] = font;
	
	return font;
}

#endif
