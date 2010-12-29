#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <jack/jack.h>

#include <string>

#include "programs.h"
#include "channel.h"

using namespace std;


extern fixed_t **lfo[N_LFOS];
extern fixed_t *curr_lfo[N_LFOS+1];

extern fixed_t wave[N_WAVEFORMS][WAVE_RES];

extern fixed_t sample_and_hold[N_LFO_LEVELS];

extern int sample_and_hold_frames;
extern int lfo_res[N_LFOS];
extern int lfo_phase[N_LFOS];
extern float lfo_freq_hz[N_LFOS];

extern bool verbose;
extern bool fatal_warnings;
extern bool quiet;

extern bool connect_audio, connect_midi;


extern float cleanup_interval_sec;
extern float snh_freq_hz;
extern float max_port_time_sec;

extern float filter_update_freq_hz;
extern float lfo_update_freq_hz;

extern int filter_update_frames;
extern int lfo_update_frames;

extern float xrun_time;
extern int xrun_n;

#ifndef FRAMESKIP
	extern int samp_rate;
#else
	extern int frameskip;
	extern int samp_rate;

	extern jack_default_audio_sample_t outtemp[N_CHANNELS];
	#ifdef STEREO
		extern jack_default_audio_sample_t outtemp2[N_CHANNELS];
	#endif

	extern jack_nframes_t outtemp_nframes_left;
#endif


extern string programfile[128];



extern program_t *program_settings;

extern Channel *channel[N_CHANNELS];


extern jack_nframes_t cleanup_interval; //in jack frames


#endif
