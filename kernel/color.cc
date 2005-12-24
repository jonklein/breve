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

#include <math.h>

#include "util.h"

#define MIN3(x, y, z)	(((x)<(y) && (x)<(z))?(x):((y)<(z)?(y):(z)))
#define MAX3(x, y, z)	(((x)>(y) && (x)>(z))?(x):((y)>(z)?(y):(z)))

void brHSVtoRGB(slVector *hsv, slVector *rgb) {
	int i;
	double f, p, q, t;
	double h, s, v;

	h = hsv->x;
	s = hsv->y;
	v = hsv->z;

	if( s == 0 ) {
		rgb->x = rgb->x = rgb->z = v;
		return;
	}

	h /= 60;			// sector 0 to 5
	i = (int)floor( h );
	f = h - i;		      // factorial part of h
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );

	switch( i ) {
		case 0:
			rgb->x = v;
			rgb->y = t;
			rgb->z = p;
			break;
		case 1:
			rgb->x = q;
			rgb->y = v;
			rgb->z = p;
			break;
		case 2:
			rgb->x = p;
			rgb->y = v;
			rgb->z = t;
			break;
		case 3:
			rgb->x = p;
			rgb->y = q;
			rgb->z = v;
			break;
		case 4:
			rgb->x = t;
			rgb->y = p;
			rgb->z = v;
			break;
		default:		// case 5:
			rgb->x = v;
			rgb->y = p;
			rgb->z = q;
			break;
	}
}

void brRGBtoHSV(slVector *rgb, slVector *hsv) {
	double min, max, delta;

	min = MIN3( rgb->x, rgb->y, rgb->z );
	max = MAX3( rgb->x, rgb->y, rgb->z );
	hsv->z = max;

	delta = max - min;

	if( max != 0.0 )
		hsv->y = delta / max;
	else {
		hsv->y = 0;
		hsv->x = -1;
		return;
	}

	if(delta == 0.0) {
		hsv->x = -1;
		return;
	}

	if( rgb->x == max )
		hsv->x = ( rgb->x - rgb->z ) / delta;		// between yellow & magenta
	else if( rgb->y == max )
		hsv->x = 2 + ( rgb->y - rgb->x ) / delta;	// between cyan & yellow
	else
		hsv->x = 4 + ( rgb->x - rgb->y ) / delta;	// between magenta & cyan

	hsv->x *= 60;				// degrees
	if( hsv->x < 0 )
		hsv->x += 360;
}

#undef MAX3
#undef MIN3

void brUniqueColor(slVector *color, int n) {
	slVector hsv;

	if(n == 0) {
		slVectorSet(color, 0, 0, 0);
		return;
	}

	if(n == 1) {
		slVectorSet(color, 1, 1, 1);
		return;
	}

	n -= 2;

	hsv.x = (n % 8) * 40 + (n / 96) * 20;

	n %= 96;

	if(n < 32) hsv.y = 1.0;
	else if(n < 64) hsv.y = .4;
	else hsv.y = .2;

	hsv.z = 1.0 - ((n % 32) / 8) * .20;

	brHSVtoRGB(&hsv, color);
}
