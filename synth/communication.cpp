#include "communication.h"

#include <unistd.h>
#include <iostream>

using namespace std;

jack_ringbuffer_t* request_ringbuf;
jack_ringbuffer_t* result_ringbuf;
pthread_mutex_t request_mutex;


void init_communication()
{
	pthread_mutex_init(&request_mutex, NULL);
	
	request_ringbuf=jack_ringbuffer_create(sizeof(request_t)+1);
	result_ringbuf=jack_ringbuffer_create(sizeof(int)+1);
}

void uninit_communication()
{
	jack_ringbuffer_free(request_ringbuf);
	jack_ringbuffer_free(result_ringbuf);
	
	pthread_mutex_destroy(&request_mutex);
}

int do_request(request_t request)
{
	pthread_mutex_lock(&request_mutex); // only one request at a time
	
	jack_ringbuffer_write(request_ringbuf, (char*)(&request), sizeof(request_t));
	
	do // wait for the answer
	{
		usleep(10000);
	} while (jack_ringbuffer_read_space(result_ringbuf)<sizeof(int));
	
	int result;
	if (jack_ringbuffer_read(result_ringbuf, (char*)(&result), sizeof(int)) != sizeof(int))
		cout << "FATAL: short read from result ringbuffer, expect breakage!" << endl; // TODO handle properly
	
	if (jack_ringbuffer_read_space(result_ringbuf)!=0)
		cout << "FATAL: result ringbuffer not empty, expect breakage!" << endl; // TODO handle properly
	
	pthread_mutex_unlock(&request_mutex);
	
	return result;
}

bool request_available()
{
	return (jack_ringbuffer_read_space(request_ringbuf)>=sizeof(request_t));
}

request_t get_request()
{
	request_t request;
	
	int len=jack_ringbuffer_read(request_ringbuf, (char*)(&request), sizeof(request_t));
	if (len==0)
		cout << "ERROR: no request on the ringbuffer! nothing read, continuing..." << endl;
	else if (len!=sizeof(request_t))
	{
		cout << "possibly FATAL: short read from the request ringbuffer, expect breakage!" << endl;
		request.type=request_t::NONE;
	}
	
	return request;
}

void request_finished(int result)
{
	jack_ringbuffer_write(result_ringbuf, (char*)(&result), sizeof(int));
}
