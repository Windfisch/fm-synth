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


#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <pthread.h>
#include <jack/ringbuffer.h>

struct request_t
{
	enum request_type_t { NONE, SUSPEND_PROGRAM, RESUME_PROGRAM, PANIC, RELEASE_ALL };
	request_type_t type;
	int prog_or_chan; //if negative, all programs/channels are affected
	
	request_t()
	{
		type=NONE;
	}
	
	request_t(request_type_t type_, int poc)
	{
		type=type_;
		prog_or_chan=poc;
	}
};

// init/uninit
void init_communication();
void uninit_communication();

// for non-audio-threads. mutex-protected
int do_request(request_t request);

// for the audio-thread. NOT mutex-protected
bool request_available();
request_t get_request();
void request_finished(int);

#endif
