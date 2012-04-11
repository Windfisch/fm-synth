#include <string>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <pthread.h>

#include "jack.h"
#include "load.h"
#include "cli.h"
#include "channel.h"
#include "fixed.h"
#include "programs.h"
#include "defines.h"
#include "globals.h"
#include "in_synth_cli.h"
#include "communication.h"
#include "note_loader.h"
#include "lfos.h"

#ifdef WATCHFILES
	#include "watch_files.h"
#endif

using namespace std;


void cleanup();
void dump_options();

#ifdef WATCHFILES
	pthread_t watcher_thread=-1;
#endif

int main(int argc, char** argv)
{	
	init_communication();
	
  for (int i=0;i<N_LFOS;++i)
  	lfo_freq_hz[i]=0;
  
  try
  {
		parse_args(argc, argv);
		
		add_dir("~/.flosynth", false);
		add_dir("/etc/flosynth", false);

		if (cleanup_interval_sec<=0) cleanup_interval_sec=CLEANUP_INTERVAL_SEC;
		for (int i=0;i<N_LFOS;++i)
			if (lfo_freq_hz[i]<=0) lfo_freq_hz[i]=LFO_FREQ_HZ[i];

		if (snh_freq_hz<=0) snh_freq_hz=SNH_FREQ_HZ;
		#ifdef FRAMESKIP
			if (frameskip<=-1) frameskip=0;
		#endif
		if (max_port_time_sec<=0) max_port_time_sec=MAX_PORTAMENTO_TIME;
		if (filter_update_freq_hz<=0) filter_update_freq_hz=FILTER_UPDATE_FREQ_HZ;
		if (lfo_update_freq_hz<=0) lfo_update_freq_hz=LFO_UPDATE_FREQ_HZ;
		if (envelope_update_freq_hz<=0) envelope_update_freq_hz=ENVELOPE_UPDATE_FREQ_HZ;
		if (xrun_n<=0) xrun_n=XRUN_N;
		if (xrun_time<=0) xrun_time=XRUN_TIME;
		
		dump_options();
		
		#ifdef FRAMESKIP
			frameskip+=1; //because a value of 0 means using each frame,
										//a value of 1 means using each 2nd frame and so on
		#endif
		
		init_jack();

		//this calculation needs the real sampling rate. others don't
		cleanup_interval=cleanup_interval_sec*samp_rate;
		
		
		#ifdef FRAMESKIP			
			samp_rate/=frameskip;
		#endif


		filter_update_frames=samp_rate/filter_update_freq_hz;
		lfo_update_frames=samp_rate/lfo_update_freq_hz;
		envelope_update_frames=samp_rate/envelope_update_freq_hz;
		if (filter_update_frames<1) filter_update_frames=1;
		if (lfo_update_frames<1) lfo_update_frames=1;
		if (envelope_update_frames<1) envelope_update_frames=1;

		int i;


		init_default_program(default_program);


		init_snh();
		
		for (i=0;i<N_LFOS;++i)
			init_lfo(i);
		
		program_settings=new program_t[128];

		for (i=0;i<128;++i)
		{
			program_lock[i]=false;
			
			load_program(programfile[i],program_settings[i]);
		}
		
		fixed_t* temp=new fixed_t[WAVE_RES];
		for (i=0;i<WAVE_RES;i++)
			temp[i]=0;
		
		for (i=0;i<N_WAVEFORMS;++i)
			wave[i]=temp;
		
		for (i=0;i<N_NORMAL_WAVEFORMS;i++)
			wave[i]=new fixed_t[WAVE_RES];
		
		for (i=WAVE_SAW_START; i<WAVE_SAW_START+WAVE_SAW_N; i++)
			wave[i]=new fixed_t[WAVE_RES];
					
		for (i=WAVE_PULSE_START; i<WAVE_PULSE_START+WAVE_PULSE_N; i++)
			wave[i]=new fixed_t[WAVE_RES];
					
		for (i=0;i<WAVE_RES;++i) //init the "normal" waves
		{
			fixed_t temp1, temp2;
			temp1=ONE*sin(i*2.0*3.141592654/WAVE_RES);
			temp2=ONE*sin(i*3.141592654/WAVE_RES);
			wave[0][i]=temp1;
			wave[1][i]=temp2;
			wave[2][i]= (i<=WAVE_RES/2) ? temp1 : 0;
			wave[3][i]= (i<=WAVE_RES/2) ? temp2 : 0;
			wave[4][i]= (i<=WAVE_RES/2) ? 0 : temp2;
			wave[5][i]=float(rand() - RAND_MAX/2) / (RAND_MAX/2) *ONE;
		}

		for (int j=WAVE_SAW_START; j<WAVE_SAW_START+WAVE_SAW_N; j++) //init the saws
		{
			float rising, falling;
			falling=(float) (j-WAVE_SAW_START)/(WAVE_SAW_N-1);
			rising=1.0-falling;

			int a,b,c;
			a=WAVE_RES*(rising/2);
			b=WAVE_RES*(rising/2 + falling);
			c=WAVE_RES*falling;
			
			for (i=0;i<a;i++)
				wave[j][i]=ONE * i/a;
			
			for (i=a;i<b;i++)
				wave[j][i]=ONE  -  2*ONE * (i-a)/c;
				
			for (i=b;i<WAVE_RES;i++)
				wave[j][i]=ONE * (i-b)/a   -  ONE;
		}
		
		for (int j=WAVE_PULSE_START; j<WAVE_PULSE_START+WAVE_PULSE_N; j++) //init the pulses
		{
			float high, low;
			high=(float) (j-WAVE_PULSE_START)/(WAVE_PULSE_N-1);
			low=1.0-high;
			
			int a=WAVE_RES*high;
			
			for (i=0;i<a;i++)
				wave[j][i]=ONE;
			
			for (i=a;i<WAVE_RES;i++)
				wave[j][i]=-ONE;
		}
		
		for (int i=0;i<N_CHANNELS;++i)
			channel[i]=new Channel;
		
		srand (time(NULL));
		
		start_jack(connect_audio, connect_midi);

#ifdef WATCHFILES
		if (watchfiles)
		{
			if (pthread_create(&watcher_thread, NULL, watch_files, NULL) != 0)
			{
				output_warning("WARNING: could not start file-watcher thread. you must inform me about\n"
				               "         updated files manually.");
				watcher_thread=-1;
			}
		}
		else
		{
			output_note("NOTE: you disabled the watching of files. you must inform me about\n"
									"         updated files manually.");
		}
#else
	output_verbose("NOTE: support for watching of files isn't compiled in. you must\n"
								 "         inform me about updated files manually.");
#endif
			
		do_in_synth_cli();
		
		//cleanup();
	}
	catch(string err)
	{
		cout << endl<<endl<< "FATAL: caught an exception: "<<endl<<err<<endl<<"exiting..." << endl;
	}
/*	catch(...)
	{
		cout << "FATAL: caught an unknown exception... exiting..." << endl;
	}*/
	return 0;
}

void cleanup()
{
#ifdef WATCHFILES
	if (watcher_thread!=-1)
	{
		if (pthread_cancel(watcher_thread) != 0)
		{
			output_warning("WARNING: could not cancel watcher thread!");
		}
		else
		{
			pthread_join(watcher_thread,NULL);
		}
	}
#endif
	
	exit_jack();
	
	uninit_communication();
	
	for (int i=0;i<N_CHANNELS;++i)
	{
		delete channel[i];
		channel[i]=NULL;
	}
	
	for (int i=0;i<128;++i)
		maybe_unload_note(program_settings[i]);
	
	delete [] program_settings;
}

void dump_options()
{
	for (int i=0;i<128;++i)
		if (programfile[i]!="")
			cout << "program #"<<i<<":\t'"<<programfile[i]<<"'"<<endl;
	
	cout << endl;
	
	#ifdef FRAMESKIP
		cout << "frameskip:\t\t"<<frameskip<<endl;
	#endif
	cout << "cleanup-interval:\t"<<cleanup_interval_sec<<endl;
	for (int i=0;i<N_LFOS;++i)
		cout << "lfo"<<i<<" freq:\t\t"<<lfo_freq_hz[i]<<endl;

	cout << "sample and hold freq:\t"<<snh_freq_hz<<endl;
	cout << "max portamento time:\t"<<max_port_time_sec<<endl;
	cout << "xrun n/time:\t\t"<<xrun_n<<"/"<<xrun_time<<"s"<<endl;
	cout << "lfo update freq:\t"<<lfo_update_freq_hz<<endl;
	cout << "filter update freq:\t"<<filter_update_freq_hz<<endl;
	cout << "envelope update freq:\t"<<envelope_update_freq_hz<<endl;
	
}
