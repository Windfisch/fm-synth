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

void NoteSkel::set_portamento_frames(jack_nframes_t t)
{
	portamento_frames=t;
	portamento_t=0;
}

int NoteSkel::get_program()
{
	return program;
}
