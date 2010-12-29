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
