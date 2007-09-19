/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

class slCamera;
class slWorld;

unsigned char *slReadSGIImage( const char *name, int *width, int *height, int *components, int usealpha);
unsigned char *slReadJPEGImage( const char *name, int *width, int *height, int *components, int usealpha);
unsigned char *slReadPNGImage( const char *name, int *width, int *height, int *components, int usealpha);

unsigned char *slReadImage( const char *name, int *height, int *width, int *components, int alpha );

int slPNGWrite( const char *name, int w, int h, unsigned char *buffer, int channels, int reversed, int bit_depth );

int slPNGSnapshot( slWorld *w, slCamera *c, const char *file );
int slPNGSnapshotDepth ( slWorld *w, slCamera *c, const char *file, int lin, double maxRange);

void slReversePixelBuffer( const unsigned char *source, unsigned char *dest, int width, int height );
