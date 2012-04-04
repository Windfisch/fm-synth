#ifndef __DEFINES_H__
#define __DEFINES_H__

#define WATCHFILES //undef if you don't want support for it


#define XRUN_TIME 2.0
#define XRUN_N 8

#define CLEANUP_INTERVAL_SEC 1.0

#define MAX_PORTAMENTO_TIME 2.0

#define FILTER_UPDATE_FREQ_HZ 250
#define LFO_UPDATE_FREQ_HZ 500
#define ENVELOPE_UPDATE_FREQ_HZ 500 //>=10000 for high-quality

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


// saw_start must be >= n_normal
// pulse_start must be > saw_end
// holes are allowed

// 0..6 different stuff
// 10..19 saw
// 20..29 square is allowed, for example

//SAW_N and PULSE_N should be odd numbers, otherwise you won't
//get equal ratios for falling/rising or high/low
//in english: you won't get a "normal" triangle wave or a "normal"
//            square wave then
#define N_NORMAL_WAVEFORMS 6
#define WAVE_SAW_START 100
#define WAVE_SAW_N 201
#define WAVE_PULSE_START 400
#define WAVE_PULSE_N 201

#if ( (WAVE_SAW_START < N_NORMAL_WAVEFORMS) || \
      (WAVE_PULSE_START < WAVE_SAW_START+WAVE_SAW_N) )
	#error NORMAL WAVEFORMS, SAW- AND PULSE-WAVES MAY NOT OVERLAP!
#endif

#define N_WAVEFORMS (WAVE_PULSE_START+WAVE_PULSE_N)



#define NO_CONT 128

#endif
