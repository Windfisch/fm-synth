#include <iostream>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include "in_synth_cli.h"
#include "util.h"
#include "communication.h"
#include "globals.h"
#include "load.h"

using namespace std;

#define PROMPT "> "

void signal_handler(int sig)
{
	cout << endl << PROMPT << flush;
}

void do_request(int prg_no, bool susp)
{
	pthread_mutex_lock(&suspend_request_mutex);
	
	suspend_request.prog=prg_no;
	suspend_request.suspend=susp;
	suspend_request.done=false;
	
	pthread_mutex_unlock(&suspend_request_mutex);
	
	
	
	while (true)
	{
		usleep(100000);

		pthread_mutex_lock(&suspend_request_mutex);
		if (suspend_request.done)
		{
			pthread_mutex_unlock(&suspend_request_mutex);
			break;
		}
		else
			pthread_mutex_unlock(&suspend_request_mutex);
	}
}

void lock_and_load_program(int prg_no, string file)
{
	do_request(prg_no, true);
	
	if (load_program(file,program_settings[prg_no]))
	{
		cout << "success" << endl;
		programfile[prg_no]=file;
	}
	else
		cout << "failed" << endl;
	
	for (int i=0;i<N_CHANNELS;i++)
		channel[i]->maybe_reload_program(prg_no);
	
	do_request(prg_no, false);
}

void do_in_synth_cli()
{
	string input;
	string command;
	string params;
	int num;
	
	if (signal(2,signal_handler)==SIG_ERR)
		output_warning("WARNING: failed to set signal handler in the in-synth-cli. pressing enter will\n"
		               "         kill the synth, so be careful. this is not fatal");

	fatal_warnings=false;

	while (true)
	{
		cout << PROMPT << flush;
		getline(cin,input);
		input=trim_spaces(input);
		
		command=trim_spaces(str_before(input,' ',input));
		params=trim_spaces(str_after(input,' ',""));
		
		if ((command=="exit") || (command=="quit"))
			break;
		else if (command=="reload")
		{
			if ((!isnum(params)) || (params==""))
				cout << "error: expected program-number, found '"<<params<<"'"<<endl;
			else
			{
				num=atoi(params.c_str());
				lock_and_load_program(num, programfile[num]);
			}
		}
		else if (command=="load")
		{
			string prgstr, file;
			prgstr=trim_spaces(str_before(params,' ',params));
			file=trim_spaces(str_after(params,' ',""));
			
			if ((!isnum(prgstr)) || (prgstr==""))
				cout << "error: expected program-number, found '"<<prgstr<<"'"<<endl;
			else if (file=="")
				cout << "error: expected program-file to load, found nothing"<<endl;
			else
			{
				num=atoi(params.c_str());
				lock_and_load_program(num, file);
				
				programfile[num]=file;
			}
		}
		else if (command!="")
		{
			cout << "error: unrecognized command '"<<command<<"'"<<endl;
		}
	}
}
