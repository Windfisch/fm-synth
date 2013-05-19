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


#include <math.h>
#include <stdlib.h>

#include "lfos.h"
#include "globals.h"
#include "fixed.h"
#include "defines.h"

void uninit_lfo(int i)
{
	if (lfo[i])
	{
		for (int j=0;j<lfo_res[i];++j)
			delete [] lfo[i][j];
		
		delete [] lfo[i];
		lfo[i]=NULL;
	}
}

void init_lfo(int i)
{
	//two possible divisions by zero are avoided, because
	//values <= 0 will make the program use the default
	//(nonzero) values.
	lfo_res[i]=samp_rate/lfo_freq_hz[i]/lfo_update_frames;

	lfo[i]=new fixed_t* [lfo_res[i]];
	for (int j=0;j<lfo_res[i];++j)
	{
		lfo[i][j]=new fixed_t [N_LFO_LEVELS];
		float temp=sin(j*2.0*3.141592654/lfo_res[i]);
		for (int k=0;k<N_LFO_LEVELS;++k)
			lfo[i][j][k]= (1.0 + temp*(float(LFO_MAX)*k/N_LFO_LEVELS)) * ONE;
	}

}

void init_snh()
{
	sample_and_hold_frames=samp_rate/snh_freq_hz;
}

void maybe_calc_lfos()
{
	static int lfocnt=0;
	static int snhcnt=0;
	
	if (lfocnt==0)
	{
		lfocnt=lfo_update_frames;
		
		for (int i=0;i<N_LFOS;++i)
		{
			lfo_phase[i]=(lfo_phase[i]+1)%lfo_res[i];
			curr_lfo[i]=lfo[i][lfo_phase[i]];
		}
	}
	
	if (snhcnt==0)
	{
		snhcnt=sample_and_hold_frames;

		//temp ranges between -ONE and ONE
		fixed_t temp = (float(rand())/(RAND_MAX/2)  - 1.0) * ONE;
		
		for (int i=0;i<N_LFO_LEVELS;++i)
			sample_and_hold[i]= temp*i/(N_LFO_LEVELS-1) + ONE;
		
		curr_lfo[SNH_LFO]=sample_and_hold;
		// could be moved to some init function, but looks clearer and
		// does not eat up the cpu too much ;)
	}
	
	--lfocnt;
	--snhcnt;
}
