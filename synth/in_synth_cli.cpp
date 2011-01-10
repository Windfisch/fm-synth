#include <iostream>
#include <string>
#include <signal.h>

#include "in_synth_cli.h"
#include "util.h"

using namespace std;

#define PROMPT "> "

void signal_handler(int sig)
{
	cout << endl << PROMPT << flush;
}

void do_in_synth_cli()
{
	string input;
	string command;
	string params;
	
	if (signal(2,signal_handler)==SIG_ERR)
	{
		cout << "WARNING: failed to set signal handler!" << endl;
	}


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
				//TODO: load program
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
				//TODO: load program
			}
		}
		else if (command!="")
		{
			cout << "error: unrecognized command '"<<command<<"'"<<endl;
		}
	}
}
