#ifndef __NOTE_H__
#define __NOTE_H__

#include <jack/jack.h>

#include "programs.h"
#include "envelope.h"
#include "fixed.h"
#include "filter.h"

class Note
{
	public:
		Note(int n, float v,program_t &prg, jack_nframes_t pf, fixed_t pb, int prg_no);
		~Note();
		fixed_t get_sample();
		int get_note();
		void set_note(int n);
		void set_note(int n, bool do_port);
		void set_freq(float f);
		void set_freq(float f, bool do_port);
		void set_pitchbend(fixed_t pb);
		void set_vel(float v);
		void set_portamento_frames(jack_nframes_t f);
		void release_quickly(jack_nframes_t maxt);
		void release();
		void reattack();
		bool still_active();
		void set_param(const parameter_t &p, fixed_t v);
		int get_program(){return program;}
		
	private:
		void do_ksl();
		void do_ksr();

		void recalc_factors();
		void apply_pfactor();

		Envelope **envelope;
		fixed_t freq, dest_freq, old_freq;
		fixed_t vel;
		jack_nframes_t portamento_t, portamento_frames;
		
		fixed_t *oscval;
		fixed_t *old_oscval;
		int n_oscillators;
		oscillator_t *oscillator;
		
		fixed_t sync_factor;
		fixed_t sync_phase;
		
		pfactor_value_t pfactor;
		
		int note;
		int program;
		program_t *curr_prg;
		
		fixed_t pitchbend;
		
		LowPassFilter filter;
		Envelope *filter_envelope;
		filter_params_t filter_params;
		int filter_update_counter;
		
		struct
		{
			oscillator_t *oscillator;
			filter_params_t filter_params;
		} orig;
		
/* *einstellungen: oszillatoren, deren lautstärke etc.
 * note
 * lautstärke
 * *pitchbend
 * *portamento time
 */
};


#endif
