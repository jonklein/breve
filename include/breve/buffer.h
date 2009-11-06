#ifndef _SLBUFFER_H
#define _SLBUFFER_H

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

class slBuffer {
	public:
							slBuffer( const char *inData = NULL, int inLength = 0 ) {
								_data = NULL;

								if( inData ) {
									resize( inLength );
									memcpy( _data, inData, inLength );
								}
							}

							slBuffer( const slBuffer& inOther ) {
								set( inOther );
							}

							~slBuffer() {
									clear();	
							}

		inline void			set( const slBuffer& inOther ) {
								set( inOther.data(), inOther.length() );
							}

		inline void			set( const void *inData, int inLength ) {
								resize( inLength );
								memcpy( _data, inData, inLength );
							}

		void				clear() {
								if( _data )
									delete[] _data;

								_data = NULL;
								_length = 0;
							}

		void				resize( unsigned int inLength ) { 
								clear();
	
								_data = new unsigned char[ inLength ];
								_length = inLength;
							}
			

		unsigned int 		length() const { return _length; };
		unsigned char*		data() const { return _data; }

	private:
		unsigned char*		_data;
		unsigned int		_length;
};

#endif
