#ifndef __PROGRAMS_H__
#define __PROGRAMS_H__


#include <map>
#include <set>
#include <list>

#include "fixed.h"
#include "note_funcs.h"
#include <jack/jack.h>

using namespace std;

struct term_t
{
	int c;
	fixed_t f;
};

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

struct parameter_t
{
	int osc;
	int index;
	enum parameter_enum par;
	
	bool operator< (const struct parameter_t &b) const;
};



struct param_factor_t
{
	fixed_t offset;
	fixed_t vel_amount;
};

struct pfactor_formula_t
{
	param_factor_t **fm;
	param_factor_t *out;
	param_factor_t filter_env;
	param_factor_t filter_res;
	param_factor_t filter_offset;
};

struct pfactor_value_t
{
	fixed_t **fm;
	fixed_t *out;
	fixed_t filter_env;
	fixed_t filter_res;
	fixed_t filter_offset;
};

fixed_t calc_pfactor(param_factor_t &formula, fixed_t vel);



struct custom_wave_t
{
	fixed_t samp_rate;
	int wave_len;
	fixed_t* wave;
	
	custom_wave_t()
	{
		wave=NULL;
		wave_len=0;
		samp_rate=0;
	}
	
	~custom_wave_t()
	{
		if (wave) delete [] wave;
	}
};


struct oscillator_t
{
	fixed_t *fm_strength; //this osc gets modulated by osc #i by fm_strength[i].
	fixed_t output;       //NOT: osc #i gets modulated by this osc!
	int waveform;
	fixed_t factor;
	fixed_t phase;
	
	fixed_t tremolo_depth;
	int tremolo_lfo;
	fixed_t vibrato_depth;
	int vibrato_lfo;
	
	custom_wave_t *custom_wave;
	
	int n_osc;
	
	float ksl;
	float ksr;
	
	bool sync;
	
	oscillator_t();
	oscillator_t& operator=(const oscillator_t &that);
};

struct env_settings_t
{
	jack_nframes_t attack;
	jack_nframes_t decay;
	fixed_t sustain;
	jack_nframes_t release;
	bool hold;
};

struct filter_params_t
{
	bool enabled;
	float resonance;
	float freqfactor_offset;
	float env_amount;	
	int trem_strength;
	int trem_lfo;
	env_settings_t env_settings;
};



struct program_t
{
	unsigned int n_osc;
	oscillator_t *osc_settings;
	env_settings_t *env_settings;
	set<parameter_t> controller_affects[128];
	map< parameter_t, list<term_t> > formula;
	int controller[128+1];
	filter_params_t filter_settings;
	fixed_t sync_factor;
	
	pfactor_formula_t pfactor;
		
	
	void *dl_handle;
	create_func_t *create_func;
	

	program_t();
	~program_t();
	
	void set_param(const parameter_t &p, fixed_t v);
	
	void cleanup();
	
	program_t& operator=(const program_t &that);
};




void init_default_program(program_t &p);

#endif
