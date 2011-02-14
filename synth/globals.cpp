#include "globals.h"

fixed_t **lfo[N_LFOS];
fixed_t *curr_lfo[N_LFOS+1];

fixed_t* wave[N_WAVEFORMS];

fixed_t sample_and_hold[N_LFO_LEVELS];

int sample_and_hold_frames=0;
int lfo_res[N_LFOS];
int lfo_phase[N_LFOS];
float lfo_freq_hz[N_LFOS];

bool verbose=false;
bool fatal_warnings=false;
bool quiet=false;

bool connect_audio=true, connect_midi=true;

#ifdef WATCHFILES
bool watchfiles=true;
#endif

float cleanup_interval_sec=0;
float snh_freq_hz=0;
float max_port_time_sec=0;

float filter_update_freq_hz=0;
float lfo_update_freq_hz=0;
float envelope_update_freq_hz=0;

int filter_update_frames;
int lfo_update_frames;
int envelope_update_frames;

float xrun_time=0;
int xrun_n=0;

#ifndef FRAMESKIP
	int samp_rate;
#else
	int frameskip=-1;
	int samp_rate;

	jack_default_audio_sample_t outtemp[N_CHANNELS];
	#ifdef STEREO
		jack_default_audio_sample_t outtemp2[N_CHANNELS];
	#endif

	jack_nframes_t outtemp_nframes_left=0;
#endif


string programfile[128];



program_t *program_settings;
bool program_lock[128];

program_t default_program;


Channel *channel[N_CHANNELS];


jack_nframes_t cleanup_interval; //in jack frames
