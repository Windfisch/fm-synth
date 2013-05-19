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


#ifndef __FILTER_H__
#define __FILTER_H__

#include "fixed.h"

/**
 * A 24 dB/octave resonant low-pass filter.
 **/
class LowPassFilter
{
public:
	LowPassFilter();

	/**
	 * Reset the filter - clear anything in the delay units of the filter.
	 */
	void reset();
  void set_params(float fc, float res);
	void	process_sample(fixed_t* smp);
private:
	float rate;
	float nyquist;
	double d1, d2, d3, d4;
	double a0, a1, a2, b1, b2;
};

#endif
