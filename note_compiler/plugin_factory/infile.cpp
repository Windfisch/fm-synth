#include <jack/jack.h>

#include <cmath>
#include <string>

#include "defines.h"
#include "programs.h"
#include "envelope.h"
#include "fixed.h"
#include "filter.h"
#include "note_skel.h"

using namespace std;

int filter_update_frames=0;
int samp_rate=0;
fixed_t** wave=NULL;
fixed_t** curr_lfo=NULL;

typedef void output_note_func_t(string s);
typedef string IntToStr_func_t(int i);

output_note_func_t* output_note=NULL;
IntToStr_func_t* IntToStr=NULL;

class Note : public NoteSkel
{
	public:
		Note(int n, float v,program_t &prg, jack_nframes_t pf, fixed_t pb, int prg_no, float vol_fac);
		~Note();
		fixed_t get_sample();

		void release_quickly(jack_nframes_t maxt);
		void release();
		void reattack();
		bool still_active();
		void set_param(const parameter_t &p, fixed_t v);
		
		void destroy();
		
	private:
		void do_ksl();
		void do_ksr();

		void recalc_factors();
		void apply_pfactor();

// member variables begin here

		Envelope *env0;
		Envelope *env1;

		fixed_t *oscval;
		fixed_t *old_oscval;

		oscillator_t osc0;
		oscillator_t osc1;

		//sync is disabled

		//filter is disabled

		pfactor_value_t pfactor;
		struct
		{
			oscillator_t osc0;
			oscillator_t osc1;
			//filter is disabled
		} orig;
// member variables end here
};

//this function returns the smallest phase_init possible for a
//given custom_wave which is greater or equal than PHASE_INIT
#define PHASE_INIT 100
inline fixed_t init_custom_osc_phase(int len, fixed_t sr)
{
	return ( (fixed_t(ceil( float(PHASE_INIT) * sr / len / ONE )) *len << (2*SCALE)) / sr);
}


Note::Note(int n, float v, program_t &prg, jack_nframes_t pf, fixed_t pb, int prg_no, float vol_fac)
{
	curr_prg=&prg;
	
	oscval=new fixed_t[2];
	old_oscval=new fixed_t[2];
	for (int i=0;i<2;i++)
		oscval[i]=old_oscval[i]=0;
	
	pfactor.out=new fixed_t [2];
	pfactor.fm=new fixed_t* [2];
	for (int i=0;i<2;i++)
		pfactor.fm[i]=new fixed_t [2];
	
	env0=new Envelope (prg.env_settings[0]);
	env1=new Envelope (prg.env_settings[1]);
	
	osc0=prg.osc_settings[0];
	orig.osc0=prg.osc_settings[0];
	osc1=prg.osc_settings[1];
	orig.osc1=prg.osc_settings[1];
	
	//initalize oscillator.phase to multiples of their wave resolution
	osc0.phase=ONE * PHASE_INIT;
	osc1.phase=ONE * PHASE_INIT;
	
	do_ksl();
	
	
	portamento_frames=0;
	set_portamento_frames(pf);
		
	set_note(n);
	freq=dest_freq;
	set_vel(v);
	set_vol_factor(vol_fac);
	
	pitchbend=pb;
	
	program=prg_no;
}

Note::~Note()
{
	delete [] osc0.fm_strength;
	delete env0;
	delete pfactor.fm[0];
	
	delete [] osc1.fm_strength;
	delete env1;
	delete pfactor.fm[1];
	
	
	delete [] oscval;
	delete [] old_oscval;
	
	delete [] pfactor.out;
	delete [] pfactor.fm;
}
void Note::destroy()
{
	delete this;
}

void Note::recalc_factors()
{
	for (int i=0;i<2;i++)
	{
		pfactor.out[i]=calc_pfactor(curr_prg->pfactor.out[i], vel) * volume_factor;
		
		for (int j=0;j<2;j++)
			pfactor.fm[i][j]=calc_pfactor(curr_prg->pfactor.fm[i][j], vel);
	}
}
void Note::apply_pfactor()
{
	osc0.output=orig.osc0.output*pfactor.out[0] >>SCALE;
	for (int i=0;i<2;i++)
		osc0.fm_strength[i]=orig.osc0.fm_strength[i]*pfactor.fm[0][i] >>SCALE;
	osc1.output=orig.osc1.output*pfactor.out[1] >>SCALE;
	for (int i=0;i<2;i++)
		osc1.fm_strength[i]=orig.osc1.fm_strength[i]*pfactor.fm[1][i] >>SCALE;
}

bool Note::still_active()
{
	if (    ((osc0.output>0) && (env0->still_active()))
	     || ((osc1.output>0) && (env1->still_active()))  )
		return true;
	else
		return false;
}
void Note::release()
{
	env0->release_key();
	env1->release_key();
}
void Note::release_quickly(jack_nframes_t maxt)
{
	if (env0->get_release() > maxt)
		env0->set_release(maxt);
	env0->release_key();
	
	if (env1->get_release() > maxt)
		env1->set_release(maxt);
	env1->release_key();
	
}
void Note::reattack()
{
	env0->reattack();
	env1->reattack();
}

void Note::do_ksr()
{
	env0->set_ratefactor(1.0 / pow(freq>>SCALE, osc0.ksr));
	env1->set_ratefactor(1.0 / pow(freq>>SCALE, osc1.ksr));
}
void Note::do_ksl()
{
}

fixed_t Note::get_sample()
{
	if (freq!=dest_freq)
	{
		// the div.by.zero if p_frames=0 is avoided because then the 
		// if-condition below is always true
		if (portamento_t>=portamento_frames)
			freq=dest_freq;
		else //will only happen if p_t < p_frames -> p_frames is always > 0 -> div. ok
			freq = old_freq + (dest_freq-old_freq)*portamento_t/portamento_frames;
		
		do_ksl();
		
		portamento_t++;
	}

	fixed_t actual_freq=freq*pitchbend >>SCALE;

	fixed_t *temp;
	temp=old_oscval;   //swap the current and old oscval-pointers
	old_oscval=oscval;
	oscval=temp;


	//sync is disabled
	
	
	osc0.phase+= ( (osc0.vibrato_depth==0) ? ((actual_freq*osc0.factor/samp_rate)>>SCALE) : ((  (curr_lfo[osc0.vibrato_lfo][osc0.vibrato_depth]*actual_freq >>SCALE)*osc0.factor/samp_rate)>>SCALE) );
	oscval[0] = wave[1][ ( (  osc0.phase + ( + (old_oscval[0] * 104857) + (old_oscval[1] * osc0.fm_strength[1]) >>SCALE )  ) * WAVE_RES   >>SCALE ) % WAVE_RES ] * env0->get_level()  >>SCALE;
	if (osc0.tremolo_depth)
		oscval[0] = oscval[0] * curr_lfo[0][osc0.tremolo_depth]  >>SCALE;
	
	osc1.phase+= (  (curr_lfo[osc1.vibrato_lfo][osc1.vibrato_depth]*actual_freq >>SCALE)*osc1.factor/samp_rate)>>SCALE;
	oscval[1] = wave[0][ ( osc1.phase * WAVE_RES   >>SCALE ) % WAVE_RES ] * env1->get_level()  >>SCALE;
	//oscillator1 has no tremolo
	
	fixed_t out = ( + osc0.output*oscval[0]    >>SCALE );
	
	
	
	return out;
}

void Note::set_param(const parameter_t &p, fixed_t v)
{
	oscillator_t* sel_osc=NULL;
	oscillator_t* sel_orig_osc=NULL;
	Envelope* sel_env=NULL;
	
	switch (p.osc)
	{
		case 0: sel_osc=&osc0; sel_orig_osc=&orig.osc0; sel_env=env0; break;
		case 1: sel_osc=&osc1; sel_orig_osc=&orig.osc1; sel_env=env1; break;
		
		default: output_note("NOTE: trying to change the nonexistent oscillator"+IntToStr(p.osc));
	}
	
	if ( ((p.par==ATTACK) || (p.par==DECAY) || (p.par==SUSTAIN) || 
	     (p.par==RELEASE) || (p.par==HOLD)) && sel_env==NULL )
	{
		output_note("NOTE: cannot change parameter for envelope"+IntToStr(p.osc)+" because it's disabled");
		return;
	}
		
	switch(p.par)
	{
		case ATTACK: sel_env->set_attack(v*samp_rate >>SCALE); break;
		case DECAY: sel_env->set_decay(v*samp_rate >>SCALE); break;
		case SUSTAIN: sel_env->set_sustain(v); break;
		case RELEASE: sel_env->set_release(v*samp_rate >>SCALE); break;
		case HOLD: sel_env->set_hold(v!=0); break;

		case KSR: sel_osc->ksr=float(v)/ONE; break;
		case KSL: sel_osc->ksl=float(v)/ONE; break;

		case FACTOR: sel_osc->factor=v; break;
		case TREMOLO: sel_osc->tremolo_depth=v; break;
		case TREM_LFO: sel_osc->tremolo_lfo=v; break;
		case VIBRATO: sel_osc->vibrato_depth=v; break;
		case VIB_LFO: sel_osc->vibrato_lfo=v; break;
		case WAVEFORM: sel_osc->waveform=v; break;
		case SYNC: sel_osc->sync=(v!=0); break;
		case MODULATION: sel_orig_osc->fm_strength[p.index]=v; apply_pfactor(); break;
    case OUTPUT: sel_orig_osc->output=v; apply_pfactor(); break;

		case FILTER_ENABLED:
		case FILTER_ENV_AMOUNT:
		case FILTER_ATTACK:
		case FILTER_DECAY:
		case FILTER_SUSTAIN:
		case FILTER_RELEASE:
		case FILTER_HOLD:
		case FILTER_OFFSET:
		case FILTER_RESONANCE:
		case FILTER_TREMOLO:
		case FILTER_TREM_LFO:
			output_note("NOTE: trying to set some filter-param, but filter is disabled");
			break;

		
		case SYNC_FACTOR: output_note("NOTE: trying to set sync_factor, but it's disabled"); break;
		
		default: throw string("trying to set an unknown parameter");
	}
}



extern "C" NoteSkel* create_note(int n, float v,program_t &prg, jack_nframes_t pf, fixed_t pb, int prg_no, float vol_fac)
{
	if (wave==NULL)
		throw string("FATAL: trying to create a new note from a shared object without initalizing\n"
		             "  the object first! this should /never/ happen, please contact the developer");
	
	return new Note(n,v,prg,pf,pb,prg_no,vol_fac);
}

extern "C" void init_vars(int sr, int fupfr, fixed_t **w, fixed_t **clfo, output_note_func_t* out_n, IntToStr_func_t* its)
{
	samp_rate=sr;
	filter_update_frames=fupfr;
	wave=w;
	curr_lfo=clfo;
	IntToStr=its;
	output_note=out_n;
}


