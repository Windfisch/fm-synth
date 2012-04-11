#include <string>
#include <cstring>
#include <dlfcn.h>
#include <cmath>

#include "programs.h"
#include "globals.h"
#include "shared_object_manager.h"

using namespace std;

oscillator_t::oscillator_t()
{
	phase=0;
	fm_strength=NULL;
	
	ksl=0;
	ksr=0;
	
	sync=false;
	
	custom_wave=NULL;
}

oscillator_t& oscillator_t::operator=(const oscillator_t &that)
{
	if (this!=&that)
	{
		this->output=that.output;
		this->waveform=that.waveform;
		this->factor=that.factor;
		this->freq_env_amount=that.freq_env_amount;
		this->freq_env=that.freq_env;
		this->phase=that.phase;
		this->tremolo_depth=that.tremolo_depth;
		this->tremolo_lfo=that.tremolo_lfo;
		this->vibrato_depth=that.vibrato_depth;
		this->vibrato_lfo=that.vibrato_lfo;
		this->custom_wave=that.custom_wave;
		this->n_osc=that.n_osc;
		this->ksl=that.ksl;
		this->ksr=that.ksr;
		this->sync=that.sync;

		if (this->fm_strength)
			delete [] this->fm_strength;

		this->fm_strength=new fixed_t[n_osc];
		memcpy(this->fm_strength, that.fm_strength, sizeof(*that.fm_strength)*n_osc);

		return *this;
	}
	else
		return *this;
}

program_t::program_t()
{
	osc_settings=NULL;
	env_settings=NULL;
	filter_settings.enabled=false;
	sync_factor=0;
	n_osc=0;
	
	pfactor.fm=NULL;
	pfactor.out=NULL;
	pfactor.freq_env_amount=NULL;
	
	create_func=NULL;
	dl_handle=NULL;
}

program_t::~program_t()
{
	cleanup();
}

void program_t::cleanup()
{
	if (osc_settings)
	{
		for (unsigned int i=0;i<n_osc;++i)
			delete [] osc_settings[i].fm_strength;
		delete [] osc_settings;
	}
	if (env_settings)
		delete [] env_settings;
		
	if (pfactor.out)
		delete [] pfactor.out;
	if (pfactor.freq_env_amount)
		delete [] pfactor.freq_env_amount;
	if (pfactor.fm)
	{
		for (unsigned int i=0;i<n_osc;++i)
			delete [] pfactor.fm[i];
		delete [] pfactor.fm;
	}

	if (dl_handle)
		dlref_dec(dl_handle);
	
	dl_handle=NULL;
	create_func=NULL;
}

program_t& program_t::operator=(const program_t &that)
{
	if (this!=&that)
	{
		unsigned int i;
		
		this->cleanup();
		
		for (i=0;i<(sizeof(controller_affects)/sizeof(*controller_affects));++i)
			this->controller_affects[i]=that.controller_affects[i];
		this->formula=that.formula;
		this->n_osc=that.n_osc;
		
		this->osc_settings=new oscillator_t[n_osc];
		this->env_settings=new env_settings_t[n_osc];
		
		memcpy(this->controller, that.controller, sizeof(that.controller));
		copy(&that.osc_settings[0], &that.osc_settings[n_osc], this->osc_settings);
		copy(&that.env_settings[0], &that.env_settings[n_osc], this->env_settings);
		
		this->filter_settings=that.filter_settings;
		
		this->sync_factor=that.sync_factor;
		
		
		
		this->pfactor=that.pfactor;
		
		this->pfactor.out=new param_factor_t [n_osc];
		memcpy(this->pfactor.out, that.pfactor.out, sizeof(param_factor_t)*n_osc);
		
		this->pfactor.freq_env_amount=new param_factor_t [n_osc];
		memcpy(this->pfactor.freq_env_amount, that.pfactor.freq_env_amount, sizeof(param_factor_t)*n_osc);
		
		this->pfactor.fm=new param_factor_t* [n_osc];
		for (i=0;i<n_osc;++i)
		{
			this->pfactor.fm[i]=new param_factor_t [n_osc];
			memcpy(this->pfactor.fm[i], that.pfactor.fm[i], sizeof(param_factor_t)*n_osc);
		}
		
		
		this->create_func=that.create_func;
		this->dl_handle=that.dl_handle;
		if (dl_handle)
			dlref_inc(dl_handle);
		
		return *this;
	}
	else
		return *this;
}

void program_t::set_param(const parameter_t &p, fixed_t v) //ACHTUNG:
{  //wenn das verändert wird, muss auch Note::set_param verändert werden!
	switch(p.par)
	{
		case ATTACK: env_settings[p.osc].attack=v*samp_rate >>SCALE; break;
		case DECAY: env_settings[p.osc].decay=v*samp_rate >>SCALE; break;
		case SUSTAIN: env_settings[p.osc].sustain=v; break;
		case RELEASE: env_settings[p.osc].release=v*samp_rate >>SCALE; break;
		case HOLD: env_settings[p.osc].hold=(v!=0); break;
		
		case KSR: osc_settings[p.osc].ksr=float(v)/ONE; break;
		case KSL: osc_settings[p.osc].ksl=float(v)/ONE; break;

		case FACTOR: osc_settings[p.osc].factor=pow(2.0, (double)v/12.0/ONE)*ONE; break;
		case MODULATION: osc_settings[p.osc].fm_strength[p.index]=v; break;
		case OUTPUT: osc_settings[p.osc].output=v; break;
		case TREMOLO: osc_settings[p.osc].tremolo_depth=v; break;
		case TREM_LFO: osc_settings[p.osc].tremolo_lfo=v; break;
		case VIBRATO: osc_settings[p.osc].vibrato_depth=v; break;
		case VIB_LFO: osc_settings[p.osc].vibrato_lfo=v; break;
		case WAVEFORM: osc_settings[p.osc].waveform=v; break;
		case SYNC: osc_settings[p.osc].sync=(v!=0); break;
		
		case FILTER_ENABLED: filter_settings.enabled=(v!=0); break;
		case FILTER_ENV_AMOUNT: filter_settings.env_amount=float(v)/ONE; break;
		case FILTER_ATTACK: filter_settings.env_settings.attack=v*samp_rate/filter_update_frames >>SCALE; break;
		case FILTER_DECAY: filter_settings.env_settings.decay=v*samp_rate/filter_update_frames >>SCALE; break;
		case FILTER_SUSTAIN: filter_settings.env_settings.sustain=v; break;
		case FILTER_RELEASE: filter_settings.env_settings.release=v*samp_rate/filter_update_frames >>SCALE; break;
		case FILTER_HOLD: filter_settings.env_settings.hold=(v!=0); break;
		case FILTER_OFFSET: filter_settings.freqfactor_offset=float(v)/ONE; break;
		case FILTER_RESONANCE: filter_settings.resonance=float(v)/ONE; break;
		case FILTER_TREMOLO: filter_settings.trem_strength=v; break;
		case FILTER_TREM_LFO: filter_settings.trem_lfo=v; break;
		
		case SYNC_FACTOR: sync_factor=v; break;

		case FREQ_ATTACK: osc_settings[p.osc].freq_env.attack=v*samp_rate >>SCALE; break;
		case FREQ_DECAY: osc_settings[p.osc].freq_env.decay=v*samp_rate >>SCALE; break;
		case FREQ_SUSTAIN: osc_settings[p.osc].freq_env.sustain=v; break;
		case FREQ_RELEASE: osc_settings[p.osc].freq_env.release=v*samp_rate >>SCALE; break;
		case FREQ_HOLD: osc_settings[p.osc].freq_env.hold=(v!=0); break;
		case FREQ_ENV_AMOUNT: osc_settings[p.osc].freq_env_amount=double(v)/ONE; break;
		
		default: throw string("trying to set an unknown parameter");
	}
}


bool parameter_t::operator< (const struct parameter_t &b) const
{
	if (par!=b.par) return (par < b.par);
	else if (osc!=b.osc) return (osc < b.osc);
	else if (index!=b.index) return (index < b.index);
	else return false; //they're equal
}




fixed_t calc_pfactor(param_factor_t &formula, fixed_t vel)
{
	return formula.offset + (formula.vel_amount * vel  >>SCALE);
}

void init_default_program(program_t &p)
{
  p.n_osc=1;
	p.osc_settings=new oscillator_t[1];
	p.env_settings=new env_settings_t[1];
	p.osc_settings[0].n_osc=1;
	p.osc_settings[0].fm_strength=new fixed_t[1];
	p.osc_settings[0].factor=ONE;
	p.osc_settings[0].fm_strength[0]=0;
	p.osc_settings[0].ksl=0;
	p.osc_settings[0].ksr=0;
	p.osc_settings[0].output=ONE;
	p.osc_settings[0].tremolo_depth=0;
	p.osc_settings[0].vibrato_depth=0;
	p.osc_settings[0].waveform=0;
	p.env_settings[0].attack=0;
	p.env_settings[0].decay=0;
	p.env_settings[0].sustain=ONE;
	p.env_settings[0].release=3*samp_rate;
	p.env_settings[0].hold=true;
	p.filter_settings.enabled=false;
	
	
	p.pfactor.out=new param_factor_t [1];
	p.pfactor.freq_env_amount=new param_factor_t [1];
	p.pfactor.fm=new param_factor_t* [1];
	
	p.pfactor.filter_env.offset=ONE;
	p.pfactor.filter_env.vel_amount=0;
	
	p.pfactor.filter_res.offset=ONE;
	p.pfactor.filter_res.vel_amount=0;	
	p.pfactor.filter_offset.offset=ONE;
	p.pfactor.filter_offset.vel_amount=0;
	
	p.pfactor.out[0].offset=0;
	p.pfactor.out[0].vel_amount=ONE;

	p.pfactor.freq_env_amount[0].offset=ONE;
	p.pfactor.freq_env_amount[0].vel_amount=0;
		
	p.pfactor.fm[0]=new param_factor_t [1];
	p.pfactor.fm[0][0].offset=ONE;
	p.pfactor.fm[0][0].vel_amount=0;			

}

