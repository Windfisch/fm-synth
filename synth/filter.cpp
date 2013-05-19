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


#include "math.h"

#include "filter.h"
#include "defines.h"
#include "globals.h"

LowPassFilter::LowPassFilter()
{
	rate=samp_rate;
	nyquist=rate/2;
	reset();
}

void LowPassFilter::reset()
{
	d1 = d2 = d3 = d4 = 0;
}

void LowPassFilter::set_params(float fc, float res)
{
	// constrain cutoff
#define SAFE 0.99f // filter is unstable _AT_ PI
	if (fc>(nyquist*SAFE))
		fc=nyquist*SAFE;
	if (fc<10)
		{fc = 10;/*d1=d2=d3=d4=0;*/}
	float w = (fc/(float)rate); // cutoff freq [ 0 <= w <= 0.5 ]
	
	// find final coeff values for end of this buffer
	double k, k2, bh; 
	double r = 2*(1-res);
	if(r==0.0) r = 0.001;
	k=tan(w*PI);
	k2 = k*k;
	bh = 1 + (r*k) + k2;
	a0 = a2 = double(k2/bh);
	a1 = a0 * 2;
	b1 = double(2*(k2-1)/-bh);
	b2 = double((1-(r*k)+k2)/-bh);	
}

void LowPassFilter::process_sample	(fixed_t *smp)
{
	fixed_t x,y;
	x = *smp;
	
	// first 2nd-order unit
	y = ( a0*x ) + d1;
	d1 = d2 + ( (a1)*x ) + ( (b1)*y );
	d2 = ( (a2)*x ) + ( (b2)*y );
	x=y;
	// and the second
	
	y = ( a0*x ) + d3;
	d3 = d4 + ( a1*x ) + ( b1*y );
	d4 = ( a2*x ) + ( b2*y );
	
	*smp = y;
}
