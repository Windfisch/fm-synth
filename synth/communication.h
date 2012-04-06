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
