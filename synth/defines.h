#ifndef __DEFINES_H__
#define __DEFINES_H__


#define XRUN_TIME 2.0
#define XRUN_N 8

#define CLEANUP_INTERVAL_SEC 1.0

#define MAX_PORTAMENTO_TIME 2.0

#define FILTER_UPDATE_FREQ_HZ 250
#define LFO_UPDATE_FREQ_HZ 500
#define ENVELOPE_UPDATE_FREQ_HZ 50

//when changing this, also change code marked with FINDLFO!
#define N_LFOS 3
#define N_LFO_LEVELS 1024
#define LFO_MAX 1
extern float LFO_FREQ_HZ[];
#define __LFO_FREQ_HZ {7.0, 5.0, 1.0}

#if N_LFO_LEVELS <= 1
	#error "N_LFO_LEVELS must be greater than one!"
#endif

#define SNH_FREQ_HZ 10
#define SNH_LFO N_LFOS


//init the oscillator phases to wave_res * PHASE_INIT
//negative values are not allowed, zero will cause the program
//to segfault if phase modulation is done, higher values will make
//the probability for a segfault smaller (i.e., zero)
//only decrease if you know what you're doing!
#define PHASE_INIT 100


#define MIDI_IN_NAME "midi_in"
#define OUT_NAME "output"

#define N_CHANNELS 16


//#define STEREO
#define FRAMESKIP

#define VOL_FACTOR (1/20.0)



#define PI 3.141592654





#define WAVE_RES 44100
#define N_WAVEFORMS 5

#define NO_CONT 128

#endif
