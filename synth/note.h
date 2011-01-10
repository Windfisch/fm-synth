#ifndef __NOTE_H__
#define __NOTE_H__

#include <jack/jack.h>

#include "programs.h"
#include "envelope.h"
#include "fixed.h"
#include "filter.h"
#include "note_skel.h"

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

		Envelope **envelope;
		
		int env_frame_counter;
		
		fixed_t *envval;
		fixed_t *oscval;
		fixed_t *old_oscval;
		int n_oscillators;
		oscillator_t *oscillator;
		
		fixed_t sync_factor;
		fixed_t sync_phase;
		
		pfactor_value_t pfactor;
		
		LowPassFilter filter;
		Envelope *filter_envelope;
		filter_params_t filter_params;
		int filter_update_counter;
		
		struct
		{
			oscillator_t *oscillator;
			filter_params_t filter_params;
		} orig;
};


#endif
