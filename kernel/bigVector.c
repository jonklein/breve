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

#include <stdio.h>

slBigVector *slNewBigVector(int n) {
	slBigVector *b;

	b = slMalloc(sizeof(slBigVector));
	b->size = n;
	b->elements = slMalloc(n * sizeof(double));

	return b;
}

slBigVector *slNewBigMatrix(int n) {
	slBigVector *b;

	b = slMalloc(sizeof(slBigVector));
	b->size = n;
	b->elements = slMalloc(n * sizeof(double));


slBigVector *slBigVectorAdd(slBigVector *a, slBigVector *b, slBigVector *dst) {
	if(a->size != b->size) return NULL;

	for(n=0;n<a->size;n++) dst->elements[n] = a->elements[n] + b->elements[n];

	return dst;
}

slBigVector *slBigVectorSub(slBigVector *a, slBigVector *b, slBigVector *dst) {
	if(a->size != b->size) return NULL;

	for(n=0;n<a->size;n++) dst->elements[n] = a->elements[n] - b->elements[n];

	return dst;
}
