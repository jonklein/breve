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

#ifndef _SOUND_H
#define _SOUND_H

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)

#include <portaudio.h>
#include <sndfile.h>

#define MIXER_SAMPLE_RATE 44100

class brSoundMixer {
	public:
						brSoundMixer();
						~brSoundMixer();

		bool 				StartStream();

		brSoundPlayer*			NextPlayer();
		brSoundPlayer*			NewSinewave( double frequency );
		brSoundPlayer*			NewPlayer( brSoundData *data, float speed );


		std::vector< brSoundPlayer* > 	_players;
		PortAudioStream 		*_stream;
		bool 				_streamShouldEnd;

	private:
};

struct brSoundData {
	float *data;
	unsigned long length;
};

struct brSoundPlayer {
	brSoundData *sound;
	float offset;
	float speed;
	char finished;
	char isSinewave;
	double frequency;
	double phase;
	float volume;
	float balance;
};

float *brSampleUp( float *in, long frames );

brSoundData *brLoadSound(char *file);
void brFreeSoundData(brSoundData *data);

int brPASoundCallback(void *ibuf, void *obuf, unsigned long fbp, PaTimestamp outTime, void *data);



#endif // defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)
#endif // _SOUND_H
