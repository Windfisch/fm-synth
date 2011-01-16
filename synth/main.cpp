#include <string>
#include <iostream>
#include <cmath>
#include <cstdlib>

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

using namespace std;


void cleanup();
void dump_options();


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
		if (frameskip<=-1) frameskip=0;
		if (max_port_time_sec<=0) max_port_time_sec=MAX_PORTAMENTO_TIME;
		if (filter_update_freq_hz<=0) filter_update_freq_hz=FILTER_UPDATE_FREQ_HZ;
		if (lfo_update_freq_hz<=0) lfo_update_freq_hz=LFO_UPDATE_FREQ_HZ;
		if (envelope_update_freq_hz<=0) envelope_update_freq_hz=ENVELOPE_UPDATE_FREQ_HZ;
		if (xrun_n<=0) xrun_n=XRUN_N;
		if (xrun_time<=0) xrun_time=XRUN_TIME;
		
		dump_options();
		
		frameskip+=1; //because a value of 0 means using each frame,
		              //a value of 1 means using each 2nd frame and so on
		
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
		
		for (i=0;i<N_WAVEFORMS;++i)
			wave[i]=new fixed_t[WAVE_RES];
			
		for (i=0;i<WAVE_RES;++i)
		{
			float temp1, temp2;
			temp1=sin(i*2.0*3.141592654/WAVE_RES);
			temp2=sin(i*3.141592654/WAVE_RES);
			wave[0][i]=temp1*ONE;
			wave[1][i]=temp2*ONE;
			wave[2][i]=(i<=WAVE_RES/2) ? wave[0][i] : 0;
			wave[3][i]= (i<=WAVE_RES/2) ? wave[1][i] : 0;
			wave[4][i]=(i<WAVE_RES/2) ? ONE : -ONE;
			wave[5][i]=(i<=WAVE_RES/2) ? (ONE*2*i/WAVE_RES) : (ONE*2*i/WAVE_RES - 2*ONE);
			wave[6][i]=(i<=WAVE_RES/2) ? (ONE*2*i/WAVE_RES) : (ONE*2*(WAVE_RES-i)/WAVE_RES);
			wave[7][i]=float(rand() - RAND_MAX/2) / (RAND_MAX/2) *ONE;
		}
		
		for (int i=0;i<N_CHANNELS;++i)
			channel[i]=new Channel;
		
		srand (time(NULL));
		
		start_jack(connect_audio, connect_midi);
		
		do_in_synth_cli();
		
		cleanup();
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
	
	cout << "frameskip:\t\t"<<frameskip<<endl;
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
