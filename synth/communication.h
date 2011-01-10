#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <pthread.h>

struct suspend_request_t
{
	int prog; //if negative, all programs are affected
	bool suspend; //true->suspend, false->use them again
	bool done; //must be set to false by the requester,
	           //must be set to true after processing by the requestee
};


extern pthread_mutex_t suspend_request_mutex;
extern suspend_request_t suspend_request;



void init_communication();
void uninit_communication();
#endif
