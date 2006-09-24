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

#include "kernel.h"

#ifdef WINDOWS
#include "Winbase.h"
#include "Windows.h"
#endif

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)

brSoundMixer::brSoundMixer() {
	int error;

	_streamShouldEnd = false;
	_stream = NULL;

	error = Pa_OpenDefaultStream( &_stream, 0, 2, paFloat32, MIXER_SAMPLE_RATE, 256, 0, brPASoundCallback, this );

	if ( error )
		slMessage( DEBUG_ALL, "Error (%d) opening new sound stream!\n", error );
}

brSoundMixer::~brSoundMixer() {
	_streamShouldEnd = true;

	Pa_StopStream( _stream );

	while ( _stream && Pa_StreamActive( _stream ) );

	if ( _stream ) Pa_CloseStream( _stream );

	_players.clear();
}

brSoundPlayer *brSoundMixer::NextPlayer() {
	brSoundPlayer *player = NULL;
	unsigned int n = 0;

	StartStream();

	for ( n = 0; n < _players.size(); n++ ) {
		if ( _players[n]->finished ) {
			player = _players[ n ];
			break;
		}
	}

	if ( !player ) {
		brSoundPlayer *p = new brSoundPlayer;

		p->finished = 1;
		p->sound = NULL;

		_players.push_back( p );

		player = p;
	}

	player->finished = 1;

	return player;
}

bool brSoundMixer::StartStream() {
	if ( !Pa_StreamActive( _stream ) ) {
		Pa_StartStream( _stream );
#ifdef WINDOWS
		SetPriorityClass( GetCurrentProcess(), NORMAL_PRIORITY_CLASS );
#endif
		return true;
	}

	return false;
}

brSoundPlayer *brSoundMixer::NewSinewave( double frequency ) {
	brSoundPlayer *player;

	StartStream();

	player = NextPlayer();

	player->sound = NULL;

	player->isSinewave = 1;
	player->frequency = frequency;
	player->volume = 1;
	player->balance = .5;
	player->phase = 0;
	player->finished = 0;

	return player;
}

brSoundPlayer *brSoundMixer::NewPlayer( brSoundData *data, float speed ) {
	brSoundPlayer *player = NULL;

	player = NextPlayer();

	if ( speed <= 0.0 ) speed = 1.0;

	player->speed = speed;

	player->isSinewave = 0;

	player->sound = data;

	player->offset = 0;

	player->finished = 0;

	return player;
}

brSoundData *brLoadSound( char *file ) {
	SF_INFO info;
	SNDFILE *fp;
	brSoundData *data;
	int upsample;

	info.format = 0;

	fp = sf_open( file, SFM_READ, &info );

	if ( !fp ) return NULL;

	data = new brSoundData;

	data->length = ( long )info.frames * info.channels;

	data->data = new float[data->length];

	sf_readf_float( fp, data->data, ( long )info.frames );

	upsample = MIXER_SAMPLE_RATE / info.samplerate;

	if ( info.channels == 1 ) upsample *= 2;

	while ( upsample != 1 ) {
		float *newData;

		newData = brSampleUp( data->data, data->length );
		delete[] data->data;
		data->data = newData;
		data->length *= 2;

		upsample /= 2;
	}

	sf_close( fp );

	return data;
}

float *brSampleUp( float *in, long frames ) {
	float *out = new float[frames * 2 * sizeof( int )];
	int n;

	for ( n = 0;n < frames;n++ ) {
		out[n*2] = in[n];
		out[( n*2 ) + 1] = in[n];
	}

	return out;
}

void brFreeSoundData( brSoundData *data ) {
	delete[] data->data;
	delete data;
}

int brPASoundCallback( void *ibuf, void *obuf, unsigned long fbp, PaTimestamp outTime, void *data ) {
	brSoundMixer *mixer = ( brSoundMixer* )data;
	brSoundPlayer *player;
	unsigned int n, p;
	float *out = ( float* )obuf;
	float total;
	int channel;
	unsigned int size = mixer->_players.size();

	fbp *= 2;

	if ( mixer->_streamShouldEnd ) {
		mixer->_streamShouldEnd = false;
		return 1;
	}

	for ( n = 0;n < fbp;n++ ) {
		total = 0;

		channel = ( n & 1 );

		for ( p = 0;p < mixer->_players.size();p++ ) {
			player = mixer->_players[p];

			if ( player->isSinewave ) {
				if ( !player->finished ) {
					total += sin( player->phase ) * player->volume / ( float )size;

					// last channel -- update the phase

					if ( channel ) player->phase += player->frequency;
				}
			} else {
				if ( !player->finished ) {
					total += ( player->sound->data[( int )( player->offset += player->speed )] / ( float )size );

					if ( player->offset >= player->sound->length ) player->finished = 1;
				}
			}

		}

		*out++ = total;
	}

	return 0;
}

#endif /* HAVE_LIBPORTAUDIO && HAVE_LIBSNDFILE */
