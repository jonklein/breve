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
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

/* 
	= nr.c, mathematical functions

	= a few functions in this file based loosly Numerical Recipes in C but 
	= i fixed all the nutty stuff they do.  like arrays starting at 1.  
	= what's that all about?  what's up wit dat, sister?

	= other functions are pure klein, baby.
*/

/*!
	\brief Returns the closest power of two which is greater than or equal to 
	the input.                                                               
 
	Used to round up for non-power-of-two textures.
*/ 
 
int slNextPowerOfTwo(int n) {
	int power = 1;

	while(power < n) power <<= 1;

	return power;
}

int slIsinf(double d) {
	return (d == 1.0/0.0);
}
