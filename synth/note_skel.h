#ifndef __NOTE_SKEL_H__
#define __NOTE_SKEL_H__

#include <jack/jack.h>

#include "programs.h"
#include "fixed.h"

class NoteSkel
{
	public:
		NoteSkel();
		virtual ~NoteSkel();
		virtual fixed_t get_sample()=0;

		int get_program();
		int get_note();
		void set_note(int n);
		void set_note(int n, bool do_port);
		void set_freq(float f);
		void set_freq(float f, bool do_port);
		void set_pitchbend(fixed_t pb);
		void set_vel(float v);
		void set_vol_factor(float vol_fac);
		void set_portamento_frames(jack_nframes_t f);

		virtual void release_quickly(jack_nframes_t maxt)=0;
		virtual void release()=0;
		virtual void reattack()=0;
		virtual bool still_active()=0;

		virtual void set_param(const parameter_t &p, fixed_t v)=0;
		
		virtual void destroy()=0;
		
	protected:
		virtual void do_ksl()=0;
		virtual void do_ksr()=0;

		virtual void recalc_factors()=0;
		virtual void apply_pfactor()=0;

		fixed_t freq, dest_freq, old_freq;
		fixed_t vel;
		jack_nframes_t portamento_t, portamento_frames;
		
		pfactor_value_t pfactor;
		float volume_factor;
		
		int note;
		int program;
		program_t *curr_prg;
		
		fixed_t pitchbend;
};


#endif
