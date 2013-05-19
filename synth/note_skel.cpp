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


#include <cmath>

#include "note_skel.h"
#include "globals.h"
#include "defines.h"

using namespace std;

NoteSkel::NoteSkel()
{	
}

NoteSkel::~NoteSkel()
{
	
}

void NoteSkel::set_pitchbend(fixed_t pb)
{
	pitchbend=pb;
	
	recalc_actual_freq();
}

void NoteSkel::set_freq(float f)
{
	old_freq=freq;
	dest_freq=f*ONE;
	portamento_t=0;
	
	do_ksr();
}

void NoteSkel::set_freq(float f, bool do_port)
{
	set_freq(f);
	
	if (!do_port)
		old_freq=dest_freq;
}

void NoteSkel::set_note(int n)
{
	note=n;
	set_freq(440.0*pow(2.0,(float)(n-69)/12.0));
}

void NoteSkel::set_note(int n, bool do_port)
{
	note=n;
	set_freq(440.0*pow(2.0,(float)(n-69)/12.0), do_port);
}

int NoteSkel::get_note()
{
	return note;
}

void NoteSkel::set_vel(float v)
{
	vel=v*ONE;
	
	recalc_factors();
	apply_pfactor();
}

void NoteSkel::set_vol_factor(float vol_fac)
{
	volume_factor=vol_fac;
	
	recalc_factors();
	apply_pfactor();
}

void NoteSkel::set_portamento_frames(jack_nframes_t t)
{
	portamento_frames=t;
	portamento_t=0;
}

int NoteSkel::get_program()
{
	return program;
}

