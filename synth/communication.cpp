#include "communication.h"

pthread_mutex_t suspend_request_mutex;
suspend_request_t suspend_request;

void init_communication()
{
	pthread_mutex_init(&suspend_request_mutex, NULL);
	
	suspend_request.done=true;
}

void uninit_communication()
{
	pthread_mutex_destroy(&suspend_request_mutex);
}
