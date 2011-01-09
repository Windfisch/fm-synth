#ifndef __PROGRAMS_H__
#define __PROGRAMS_H__


#include "../synth/fixed.h"

enum parameter_enum
{
	MODULATION=0,
	OUTPUT,
	WAVEFORM,
	FACTOR,
	TREMOLO,
	TREM_LFO,
	VIBRATO,
	VIB_LFO,
	ATTACK,
	DECAY,
	SUSTAIN,
	RELEASE,
	HOLD,
	KSR,
	KSL,
	SYNC,
	FILTER_ENABLED,
	FILTER_ENV_AMOUNT,
	FILTER_ATTACK,
	FILTER_DECAY,
	FILTER_SUSTAIN,
	FILTER_RELEASE,
	FILTER_HOLD,
	FILTER_OFFSET,
	FILTER_RESONANCE,
	FILTER_TREMOLO,
	FILTER_TREM_LFO,
	SYNC_FACTOR,


	PARAMETER_N_ENTRIES,
	UNKNOWN=-1
};


struct oscillator_t
{
	fixed_t *fm_strength;
	bool *fm_strength_const;
	fixed_t output;      
	bool output_const;      
	bool output_no_pfactor;      
	int waveform;
	bool waveform_const;
	fixed_t factor;
	bool factor_const;
	
	fixed_t tremolo_depth;
	bool tremolo_depth_const;
	int tremolo_lfo;
	bool tremolo_lfo_const;
	fixed_t vibrato_depth;
	bool vibrato_depth_const;
	int vibrato_lfo;
	bool vibrato_lfo_const;
	
	bool have_custom_wave;
	
	int n_osc;
	
	float ksl;
	bool ksl_const;
	float ksr;
	bool ksr_const;
	
	bool sync;
	bool sync_const;
};

struct env_settings_t
{
	bool enabled;
	float attack;
	bool attack_const;
	float decay;
	bool decay_const;
	float sustain;
	bool sustain_const;
	float release;
	bool release_const;
	bool hold;
	bool hold_const;
};

struct filter_params_t
{
	bool enabled;
	bool enabled_const;
	float resonance;
	bool resonance_const;
	float freqfactor_offset;
	bool freqfactor_offset_const;
	float env_amount;	
	bool env_amount_const;	
	int trem_strength;
	bool trem_strength_const;
	int trem_lfo;
	bool trem_lfo_const;
	env_settings_t env_settings;
};

struct program_t
{
	int n_osc;
	oscillator_t *osc;
	env_settings_t *env;
	filter_params_t filter;
	
	fixed_t sync_factor;
	bool sync_factor_const;
};

#endif
