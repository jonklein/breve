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

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)
brSoundMixer *brNewSoundMixer() {
	brSoundMixer *mixer;
	int error;
	int n;

	mixer = new brSoundMixer;
	mixer->nPlayers = 0;
	mixer->maxPlayers = 8;
	mixer->players = slMalloc(mixer->maxPlayers * sizeof(brSoundPlayer*));
	for(n=0;n<mixer->maxPlayers;n++) mixer->players[n] = slMalloc(sizeof(brSoundPlayer));
	mixer->streamShouldEnd = 0;

	error = Pa_OpenDefaultStream(&mixer->stream, 0, 2, paInt32, MIXER_SAMPLE_RATE, 256, 0, brPASoundCallback, mixer);

	if(error) {
		slMessage(DEBUG_ALL, "Error (%d) opening new sound stream!\n", error);
		return NULL;
	}

	if(!Pa_StreamActive(mixer->stream)) Pa_StartStream(mixer->stream);

	return mixer;
}

void brFreeSoundMixer(brSoundMixer *mixer) {
	int n;
	mixer->streamShouldEnd = 1;

	if(!mixer) return;

	Pa_StopStream(mixer->stream);
	while(mixer->stream && Pa_StreamActive(mixer->stream));

	if(mixer->stream) Pa_CloseStream(mixer->stream);
	
	for(n=0;n<mixer->maxPlayers;n++) slFree(mixer->players[n]);
	slFree(mixer->players);

	delete mixer;
}

brSoundPlayer *brNextPlayer(brSoundMixer *mixer) {
	brSoundPlayer *player = NULL;
	int n = 0;

	for(n=0;n<mixer->nPlayers;n++) {
		if(!player && mixer->players[n]->finished) player = mixer->players[n];
	}

	if(!player) {
		if(mixer->nPlayers == mixer->maxPlayers) {
			int oldMax = mixer->maxPlayers;
			mixer->maxPlayers *= 2;
			mixer->players = slRealloc(mixer->players, mixer->maxPlayers * sizeof(brSoundPlayer*));

			for(n=oldMax;n<mixer->maxPlayers;n++) mixer->players[n] = slMalloc(sizeof(brSoundPlayer));
		}

		player = mixer->players[mixer->nPlayers];

		mixer->nPlayers++;
	}

	player->finished = 1;

	return player;
}

brSoundPlayer *brNewSinewave(brSoundMixer *mixer, double frequency) {
	brSoundPlayer *player;
	
	if(!mixer) return NULL;

	player = brNextPlayer(mixer);

	player->isSinewave = 1;
	player->frequency = frequency;
	player->volume = 1;
	player->balance = .5;
	player->phase = 0;
	player->finished = 0;

	// mixer->playerV.push_back(player);

	return player;
}

brSoundPlayer *brNewPlayer(brSoundMixer *mixer, brSoundData *data) {
	brSoundPlayer *player = NULL;

	player = brNextPlayer(mixer);

	player->isSinewave = 0;
	player->sound = data;
	player->offset = 0;
	player->finished = 0;

	return player;
}

brSoundData *brLoadSound(char *file) {
	SF_INFO info;
	SNDFILE *fp;
	brSoundData *data;
	int upsample;

	fp = sf_open(file, SFM_READ, &info);
	if(!fp) return NULL;

	data = new brSoundData;
	data->length = (long)info.frames * info.channels;
	data->data = new int[data->length];

	sf_readf_int(fp, data->data, (long)info.frames);

	upsample = MIXER_SAMPLE_RATE / info.samplerate;
	if(info.channels == 1) upsample *= 2;

	while(upsample != 1) {
		int *realData;

		realData = brSampleUp(data->data, data->length);
		slFree(data->data);
		data->data = realData;
		data->length *= 2;

		upsample /= 2;
	}

	sf_close(fp);

	return data;
}

int *brSampleUp(int *in, long frames) {
	int *out = slMalloc(frames * 2 * sizeof(int));
	int n;

	for(n=0;n<frames;n++) {
		out[n*2] = in[n];
		out[(n*2) + 1] = in[n];
	}

	return out;
}

void brFreeSoundData(brSoundData *data) {
	delete[] data->data;
	delete data;
}

int brPASoundCallback(void *ibuf, void *obuf, unsigned long fbp, PaTimestamp outTime, void *data) {
	brSoundMixer *mixer = data;
	brSoundPlayer *player;
	unsigned int n, p;
	int *out = obuf;
	int total;
	int channel;

	fbp *= 2;

	if(mixer->streamShouldEnd) {
		mixer->streamShouldEnd = 0;
		return 1;
	}

	for(n=0;n<fbp;n++) {
		total = 0;

		channel = n & 1;

		if(mixer->nPlayers) {
			for(p=0;p<mixer->nPlayers;p++) {
				player = mixer->players[p];

				if(player->isSinewave) {
					if(!player->finished) {
						total += (int)(0x7fffffff * sin(player->phase) * player->volume / (int)mixer->nPlayers);

						// last channel -- update the phase 
						if(channel) player->phase += player->frequency;
					}
				} else { 
					if(!player->finished) {
						total += (player->sound->data[player->offset++] / (int)mixer->nPlayers);

						if(player->offset >= player->sound->length) player->finished = 1;
					}
				}
			}

			*out++ = total;
		} else {
			*out++ = 0;
		}
	}

	return 0;
}
#endif /* HAVE_LIBPORTAUDIO && HAVE_LIBSNDFILE */
