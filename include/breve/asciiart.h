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

#define TEXTURE_WIDTH     16
#define TEXTURE_HEIGHT    16

GLubyte gBrickImage[4*TEXTURE_WIDTH*TEXTURE_HEIGHT + 1] = {
#define x 0xED,0xDD,0xFF,0xFF,
#define o 0xDC,0xCC,0xED,0xFF,
    x x x x x x x x o o o o o o o o
    x x x x x x x x o o o o o o o o
    x x x x x x x x o o o o o o o o
    x x x x x x x x o o o o o o o o
    x x x x x x x x o o o o o o o o
    x x x x x x x x o o o o o o o o
    x x x x x x x x o o o o o o o o
    x x x x x x x x o o o o o o o o
    o o o o o o o o x x x x x x x x
    o o o o o o o o x x x x x x x x
    o o o o o o o o x x x x x x x x
    o o o o o o o o x x x x x x x x
    o o o o o o o o x x x x x x x x
    o o o o o o o o x x x x x x x x
    o o o o o o o o x x x x x x x x
    o o o o o o o o x x x x x x x x
    0
#undef x
#undef o
};

GLubyte gPlaid[4*TEXTURE_WIDTH*TEXTURE_HEIGHT + 1] = {
#define x 0x00,0x00,0x22,0xFF,
#define o 0xDC,0xCC,0xED,0x00,
    x x x x x x x x x x x x x x x x
    x x x x x x x x x x x x x x x x
    x x o o o o o o o o o o o o x x
    x x o o o o o o o o o o o o x x
    x x o o o o o o o o o o o o x x
    x x o o o o o o o o o o o o x x
    x x o o o o o o o o o o o o x x
    x x o o o o o o o o o o o o x x
    x x o o o o o o o o o o o o x x
    x x o o o o o o o o o o o o x x
    x x o o o o o o o o o o o o x x
    x x o o o o o o o o o o o o x x
    x x o o o o o o o o o o o o x x
    x x o o o o o o o o o o o o x x
    x x x x x x x x x x x x x x x x
    x x x x x x x x x x x x x x x x
    0
#undef x
#undef o
};
