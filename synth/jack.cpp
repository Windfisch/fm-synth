#include <string>
#include <list>
#include <iostream>
#include <cstdlib>
#include <jack/jack.h>
#include <jack/midiport.h>

#include "defines.h"
#include "globals.h"

#include "jack.h"
#include "communication.h"
#include "lfos.h"

using namespace std;

//#define DO_DEBUGGING_EVENTS

jack_port_t	*midi_in;
jack_port_t *out_port[N_CHANNELS];
#ifdef STEREO
jack_port_t *out_port2[N_CHANNELS];
#endif

jack_client_t	*jack_client = NULL;


void manage_program_lock(int prog, bool lock) //TODO woandershinschieben?
{
	program_lock[prog]=lock;
	
	if (lock)
		for (int i=0;i<N_CHANNELS;++i)
			channel[i]->kill_program(prog);
}

void process_request()
{
	if (suspend_request.prog==-1)
		for (int i=0;i<128;++i)
			manage_program_lock(i,suspend_request.suspend);
	else
		manage_program_lock(suspend_request.prog,suspend_request.suspend);
	
	suspend_request.done=true;
}






//connect to jack, init some stuff and get information
void init_jack()
{
	jack_client = jack_client_open("flosoftsynth", JackNullOption, NULL);
	if (jack_client == NULL)
		throw string("Registering client failed");

	if (jack_set_process_callback(jack_client, process_callback, 0)) 
		throw string("Registering callback failed");

	if (jack_set_xrun_callback(jack_client, xrun_callback, 0)) 
		throw string("Registering xrun-callback failed");

	midi_in = jack_port_register(jack_client, MIDI_IN_NAME,
					                JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

	if (midi_in == NULL)
		throw string ("Registering MIDI IN failed");

	for (int i=0;i<N_CHANNELS;++i)
	{
		#ifndef STEREO
			out_port[i]=jack_port_register(jack_client, (OUT_NAME+IntToStr(i)).c_str(),
																		 JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
			if (out_port[i]==NULL)
				throw string ("Registering some output port failed");
		#else
			out_port[i]=jack_port_register(jack_client, (OUT_NAME+IntToStr(i)+"L").c_str(),
																		 JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
			out_port2[i]=jack_port_register(jack_client, (OUT_NAME+IntToStr(i)+"R").c_str(),
																		 JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

			if ((out_port[i]==NULL) || (out_port2[i]==NULL))
				throw string ("Registering some output port failed");
		#endif
	}
	
	samp_rate=jack_get_sample_rate(jack_client);
}


//activate client and connect ports
void start_jack(bool connect_audio_out, bool connect_midi_in)
{
	const char **ports;
	
	if (jack_activate(jack_client)) 
		throw string("Activating client failed");
	
	if (connect_audio_out)
	{
		if ((ports = jack_get_ports (jack_client, NULL, JACK_DEFAULT_AUDIO_TYPE, 
																 JackPortIsPhysical|JackPortIsInput)) == NULL)
		{
			output_warning("WARNING: Could not find any physical playback ports. Leaving my ports\n"
										 "         unconnected. You will not hear anything, but you can connect\n"
										 "         them on your own. proceeding...");
		}
		else
		{
			#ifndef STEREO
				int i=0;
				while(ports[i]!=NULL)
				{
					for (int j=0;j<N_CHANNELS;++j)
						if (jack_connect (jack_client, jack_port_name (out_port[j]), ports[i]))
							output_warning("WARNING: could not connect some output port. this may or may not result\n"
														 "         in being unable to produce sound. you can still connect them\n"
														 "         manually. proceeding...");
					++i;
				}
			#else
				if (ports[1]==NULL)
				{
					output_note("NOTE: could not find two output ports. connecting to one, making everything\n"
											"      mono. this is not fatal, proceeding...");
					for (int j=0;j<N_CHANNELS;++j)
					{
						if (jack_connect (jack_client, jack_port_name (out_port[j]), ports[0]))
							output_warning("WARNING: could not connect some output port. this may or may not result\n"
														 "         in being unable to produce sound. you can still connect them\n"
														 "         manually. this is not fatal, proceeding...");

						if (jack_connect (jack_client, jack_port_name (out_port2[j]), ports[0]))
							output_warning("WARNING: could not connect some output port. this may or may not result\n"
														 "         in being unable to produce sound. you can still connect them\n"
														 "         manually. this is not fatal, proceeding...");
					}
				}
				else
				{
					for (int j=0;j<N_CHANNELS;++j)
					{
						if (jack_connect (jack_client, jack_port_name (out_port[j]), ports[0]))
							output_warning("WARNING: could not connect some output port. this may or may not result\n"
														 "         in being unable to produce sound. you can still connect them\n"
														 "         manually. proceeding...");

						if (jack_connect (jack_client, jack_port_name (out_port2[j]), ports[1]))
							output_warning("WARNING: could not connect some output port. this may or may not result\n"
														 "         in being unable to produce sound. you can still connect them\n"
														 "         manually. proceeding...");
					}
				}
			#endif

			free (ports);
		}
	}
	
	if (connect_midi_in)
	{	
		if ((ports = jack_get_ports (jack_client, NULL, JACK_DEFAULT_MIDI_TYPE, 
																 JackPortIsOutput)) == NULL)
		{
			output_warning("WARNING: Could not find any MIDI OUT ports. Leaving my MIDI IN port\n"
										 "         unconnected. I cannot do anything unless you connect it to\n"
										 "         some MIDI OUT port. proceeding...");
		}
		else
		{
			int i=0;
			while(ports[i]!=NULL)
			{
				if (jack_connect (jack_client, ports[i],jack_port_name(midi_in)))
					output_warning("WARNING: could not connect some MIDI OUT to my MIDI IN. this may or may not\n"
												 "         result in being unable to receive any notes. you can still connect\n"
												 "         the port manually. proceeding...");
				++i;
			}
			free(ports);
		}
	}
}

void exit_jack()
{
	jack_deactivate(jack_client);
	jack_client_close(jack_client);
	
	jack_client=NULL;
}

int xrun_callback(void *notused)
{
	static list<float> history;

	list<float>::iterator it;

	float now=float(jack_get_time())/1000000;

	cout << "got an XRUN! if this happens too often, consider reducing CPU usage, for\n  example by setting a voice limit or by quitting other programs"<<endl<<endl;

	history.push_back(now);
	
	//erase all entries older than xrun_time
	it=history.begin();
	while (it!=history.end())
	{
		if (*it < now-xrun_time)
			it=history.erase(it);
		else
			break;
	}
	
	if (history.size() >= xrun_n)
	{
		cout << "PANIC -- TOO MANY XRUNs! killing all voices" << endl<<endl;
		for (int i=0;i<N_CHANNELS;++i)
			channel[i]->panic();
			
		history.clear();
	}
		
	return 0;
}

#define IGNORE_MIDI_OFFSET
int process_callback(jack_nframes_t nframes, void *notused)
{
	#ifdef DO_DEBUGGING_EVENTS
		static jack_nframes_t tmp=0, tmp2=0;
	#endif
	
	static jack_nframes_t next_cleanup=0;
	
	size_t curr_event=0, n_events, i, chan;
	void *inport;

	jack_default_audio_sample_t *outbuf[N_CHANNELS];
	#ifdef STEREO
	jack_default_audio_sample_t *outbuf2[N_CHANNELS];
	#endif
	
	jack_midi_event_t event;
	jack_nframes_t lastframe;
	lastframe=jack_last_frame_time(jack_client);

	if (nframes <= 0) {
		output_note ("NOTE: Process callback called with nframes = 0; bug in JACK?");
		return 0;
	}



	pthread_mutex_lock(&suspend_request_mutex);
	if (suspend_request.done==false)
		process_request();
	pthread_mutex_unlock(&suspend_request_mutex);





	for (i=0;i<N_CHANNELS;++i)
	{
		outbuf[i]=(jack_default_audio_sample_t*) jack_port_get_buffer(out_port[i], nframes);
		#ifdef STEREO
			outbuf2[i]=(jack_default_audio_sample_t*) jack_port_get_buffer(out_port2[i], nframes);
		#endif
		
		if  ( (outbuf[i]==NULL)
	  #ifdef STEREO
	  				                  || (outbuf2[i]==NULL)
	  #endif
	                                                     )
	  {
	   	output_warning("WARNING: jack_port_get_buffer failed, cannot output anything.");
	   	return 0;
		}
	}

	inport = jack_port_get_buffer(midi_in, nframes);
	if (inport == NULL)
	{
		output_warning("WARNING: jack_port_get_buffer failed, cannot receive anything.");
		return 0;
	}

	n_events = jack_midi_get_event_count(inport /*, nframes */);
	
	//as long as there are some events left and getting one fails, get the next
	while ((n_events) && (jack_midi_event_get(&event, inport, curr_event /*, nframes */)))
	{
		output_note("NOTE1: lost a note :(");
		--n_events;
		++curr_event;
	}
	
	if (lastframe>=next_cleanup)
	{
		next_cleanup=lastframe+cleanup_interval;
		for (i=0;i<N_CHANNELS;++i)
			channel[i]->cleanup();
	}
#ifdef DO_DEBUGGING_EVENTS
	if (tmp==0) //DEBUG !!!
	{
		tmp=lastframe;
		channel[0]->set_controller(5,10);
		channel[0]->set_controller(65,127);

		channel[0]->event(0x90,80,64);

//		channel[0]->event(0x90,84,64);
	}
	else if (tmp2==0)
	{
		if (lastframe>tmp+44100*1)
		{
			tmp2=1;
			cout << "BÄÄM" << endl;
			channel[0]->event(0x90,84,64);
		}
	}
	else if (tmp2==1)
	{
		if (lastframe>tmp+44100*2)
		{
			tmp2=2;
			channel[0]->event(0x90,87,5);
			channel[0]->set_controller(58, 0);
			cout << "BÄÄM2" << endl;
		}
	}
	else if (tmp2==2)
	{
		if (lastframe>tmp+44100*3)
		{
			tmp2=3;
			channel[0]->event(0x90,90,127);
			cout << "BÄÄM2" << endl;
		}
	}
	else if (tmp2==3)
	{
		if (lastframe>tmp+44100*4)
		{
			tmp2=4;
			channel[0]->event(0x90,60,96);
			cout << "BÄÄM2" << endl;
		}
	}
	else if (tmp2==4)
	{
		if (lastframe>tmp+44100*5)
		{
			tmp2=5;
			channel[0]->event(0x90,63,32);
			cout << "BÄÄM2" << endl;
		}
	}
	else if (tmp2==5)
	{
		if (lastframe>tmp+44100*6)
		{
			tmp2=6;
			channel[0]->event(0x90,66,60);
			cout << "BÄÄM2" << endl;
		}
	}
	else
	{
		if (lastframe>tmp+44100*8)
		{
			cout << "finished" << endl;
			exit(0);
		}
	}
#endif

#ifdef FRAMESKIP
	if (outtemp_nframes_left)
	{
		jack_nframes_t real_nframes;
		if (outtemp_nframes_left > nframes)
		{
			real_nframes=nframes;
			outtemp_nframes_left-=nframes;
		}
		else
		{
			real_nframes=outtemp_nframes_left;	
			outtemp_nframes_left=0;	
		}

		for (i=0;i<real_nframes;++i)
		{
			for (int j=0;j<N_CHANNELS;++j)
			{
				outbuf[j][i]=outtemp[j];
				#ifdef STEREO
				outbuf2[j][i]=outtemp2[j];
				#endif
			}
		}
	}
	else
		i=0;
		
	//begin where the above loop has stopped, at 0 if the loop wasn't
	//executed
	int upperbound=nframes-frameskip+1;
	if (upperbound<0) upperbound=0;
	for (i=i;i<upperbound;i+=frameskip)
#else
	for (i=0;i<nframes;++i)
#endif
	{
		while ((n_events) && (i>=event.time))
		{
			output_verbose("processing event #"+IntToStr(curr_event)+" of "+IntToStr(n_events)+" events");
			if (event.size > 3)
			{
				output_verbose("  Ignoring MIDI message longer than three bytes, probably a SysEx.");
			}
			else
			{
				chan=event.buffer[0] & 0x0F;
				output_verbose("  channel="+IntToStr(chan)+", data is "+IntToStrHex(event.buffer[0])+" "+IntToStrHex(event.buffer[1])+" "+IntToStrHex(event.buffer[2]));

				channel[chan]->event(event.buffer[0], event.buffer[1], event.buffer[2]);
			}
			
			--n_events;
			++curr_event;

			//as long as there are some events left and getting one fails, get the next
			while ((n_events) && (jack_midi_event_get(&event, inport, curr_event /*, nframes */)))
			{
				output_note("NOTE2: lost a note :(");
				--n_events;
				++curr_event;
			}
		}
		
		maybe_calc_lfos();

		for (int j=0;j<N_CHANNELS;++j)
		{
			#ifndef STEREO
				outbuf[j][i]=jack_default_audio_sample_t(channel[j]->get_sample())/ONE*VOL_FACTOR;
			#else
				jack_default_audio_sample_t sample=jack_default_audio_sample_t(channel[j]->get_sample())/ONE*VOL_FACTOR;
				outbuf[j][i]=channel[j]->balL*sample;
				outbuf2[j][i]=channel[j]->balR*sample;
			#endif  // if the above changes, (1) must also change
			
			#ifdef FRAMESKIP
				for (size_t k=i+frameskip-1;k>i;k--)
				{
					outbuf[j][k]=outbuf[j][i];
					#ifdef STEREO
						outbuf2[j][k]=outbuf2[j][i];
					#endif
				}
			#endif
		}
	}

#ifdef FRAMESKIP
	if (i!=nframes) // nicht aufgegangen?
	{
		for (int j=0;j<N_CHANNELS;++j)
		{ // (1)
			#ifndef STEREO
				outtemp[j]=jack_default_audio_sample_t(channel[j]->get_sample())/ONE*VOL_FACTOR;
			#else
				jack_default_audio_sample_t sample=jack_default_audio_sample_t(channel[j]->get_sample())/ONE*VOL_FACTOR;
				outtemp[j]=channel[j]->balL*sample;
				outtemp2[j]=channel[j]->balR*sample;
			#endif
		}
		
		outtemp_nframes_left=frameskip-nframes+i;

		for (i=i; i<nframes; ++i)
		{
			for (int j=0;j<N_CHANNELS;++j)
			{
				outbuf[j][i]=outtemp[j];
				#ifdef STEREO
				outbuf2[j][i]=outtemp2[j];
				#endif
			}
		}
	}
#endif	
	return 0;
}

