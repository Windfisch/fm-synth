/*
    Copyright (C) 2010-2012 Florian Jung
     
    This file is part of flo's FM synth.

    flo's FM synth is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    flo's FM synth is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flo's FM synth.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __ENVELOPE_H__
#define __ENVELOPE_H__

#include <jack/jack.h>

#include "programs.h"
#include "fixed.h"

// when frames is given, this tells the envelope that get_level() is
// only called every (frames) frames. 1 means "every time", 2 means
// "every second time" and so on.
// the caller must manage to call get_level() to the appropriate times

class Envelope
{
	public:
		Envelope(env_settings_t s, int frames=1);
		void release_key();
		void reattack();
		void reset();
		fixed_t get_level();
		bool still_active();
		void set_hold(bool h);		
		void set_attack(jack_nframes_t a);
		void set_decay(jack_nframes_t d);
		void set_sustain(fixed_t s);
		void set_release(jack_nframes_t r);
		void set_max(fixed_t m)
		{
			max=m;
			if (max>ONE) max=ONE;
			else if (max<0) max=0;
		}
		void set_ratefactor(double factor);
		
		bool get_hold() { return hold; }
		jack_nframes_t get_attack() { return attack_orig; }
		jack_nframes_t get_decay() { return decay_orig; }
		fixed_t get_sustain() { return sustain_orig; }
		jack_nframes_t get_release() { return release_orig; }
		
		
	private:
		fixed_t max;
		jack_nframes_t attack;
		jack_nframes_t decay;
		jack_nframes_t release;
		jack_nframes_t attack_orig;
		jack_nframes_t decay_orig;
		jack_nframes_t release_orig;
		jack_nframes_t rel_t;
		fixed_t sustain;
		fixed_t sustain_orig;
		fixed_t level;
		bool hold;
		bool has_release_phase;
		jack_nframes_t t;
		fixed_t ratefactor;
		
		int nth_frame;
		
		enum
		{
			ATTACK,
			DECAY,
			HOLD,
			RELEASE,
			DONE
		} state;
};

#endif
