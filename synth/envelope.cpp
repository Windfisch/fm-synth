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


#include "envelope.h"

Envelope::Envelope(env_settings_t s, int frames)
{
	level=0;
	t=0;
	state=ATTACK;
	max=ONE;

	nth_frame=frames;

	set_ratefactor(1.0);

	has_release_phase=(s.release>=0);
	
	if (!has_release_phase)
		s.hold=false;

	set_attack(s.attack);
	set_decay(s.decay);
	set_sustain(s.sustain);
	set_release(s.release);
	set_hold(s.hold);
}

void Envelope::set_ratefactor(double factor)
{
	ratefactor=ONE*factor;
	
	set_attack(attack_orig);
	set_decay(decay_orig);
	set_release(release_orig);
}

void Envelope::set_attack(jack_nframes_t a)
{
	attack_orig=a;
	attack=(a*ratefactor >>SCALE)/nth_frame;
	
	if (state==ATTACK)
		t=attack*level >>SCALE;
}

void Envelope::set_decay(jack_nframes_t d)
{
	decay_orig=d;
	decay=(d*ratefactor >>SCALE)/nth_frame;
	
	if ((state==DECAY) && (sustain!=ONE))
		if (sustain<ONE) //to avoid a div. by zero
			t=decay*(ONE-level)/(ONE-sustain);
}

void Envelope::set_sustain(fixed_t s)
{
	sustain=s;
	sustain_orig=s;
}

void Envelope::set_release(jack_nframes_t r)
{
	release_orig=r;
	release=(r*ratefactor >>SCALE)/nth_frame;
	
	if (state==RELEASE)
		if (sustain>0) //to avoid a div. by zero
			t=release*(sustain-level)/sustain;
}


void Envelope::set_hold(bool h)
{
	hold=h;
	if ((h==false) && (state==HOLD))
	{
		t=0;
		state=RELEASE;
	}
}

void Envelope::reattack()
{
	state=ATTACK;
	t=attack*level >>SCALE;
	sustain=sustain_orig;
}

void Envelope::reset()
{
	state=ATTACK;
	sustain=sustain_orig;
	level=0;
	t=0;
}

void Envelope::release_key()
{
	if (has_release_phase)
		if ((state!=RELEASE) && (state!=DONE))
		{
			t=0;
			state=RELEASE;
			sustain=level;
		}
	//if (!has_release_phase) ignore();
}

bool Envelope::still_active()
{
	return (state!=DONE);
}

fixed_t Envelope::get_level() //must be called each frame
{
	switch (state)
	{
		case ATTACK:
			if (t>=attack)
			{
				level=max;
				state=DECAY;
				t=0;
			}
			else //will only happen, if t < attack. so attack will
			{    //always be greater than zero -> no div. by zero
				level=max * t / attack ;
			}
			break;
		
		case DECAY:
			if (t>=decay)
			{
				level=max*sustain >>SCALE;
				if (has_release_phase)
					if (hold)
						state=HOLD;
					else
						state=RELEASE;
				else
					state=DONE;

				t=0;
			}
			else //will only happen, if t < decay. so decay will
			{    //always be greater than zero -> no div. by zero
				level=(ONE - (ONE-sustain)*t/decay)*max >>SCALE;
			}
			break;
		
		case HOLD:
			//does nothing. level must be set properly before entering HOLD state
			break;
			
		case RELEASE:
			if (t>=release)
			{
				level=0;
				state=DONE;
			}
			else //will only happen, if t < release. so release will
			{    //always be greater than zero -> no div. by zero
				level=(sustain - sustain * t/release)*max >>SCALE;
			}
			break;
			
		case DONE:
			//does nothing. level must be set properly before entering DONE state
			break;
	}
	
	++t;
	return level;
}
