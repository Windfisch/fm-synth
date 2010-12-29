#include <iostream>
#include <cstdlib>
#include <getopt.h>

#include "util.h"
#include "globals.h"
#include "load.h"

using namespace std;

void show_help()
{
	cout << "TODO: help text" << endl;
}

void show_version()
{
	cout << "TODO: softsynth version foo" << endl;
}

void parse_args(int argc, char** argv)
{
	static const struct option long_options[]={
			{"help",          no_argument,        0, 'h'},
			{"version",       no_argument,        0, 'V'},
			{"verbose",       no_argument,        0, 'v'},
			{"quiet",       no_argument,        0, 'q'},
			{"fatal-warnings",       no_argument,        0, 'F'},
			{"frameskip",     required_argument,  0, 'f'},
			{"xruns",     required_argument,  0, 'x'},
			{"dir",           required_argument,  0, 'd'},
			{"directory",     required_argument,  0, 'd'},
			{"program",       required_argument,  0, 'p'},
			{"cleanup-interval",       required_argument,  0, 'i'},
			{"lfo0-freq",       required_argument,  0, 400}, //FINDLFO
			{"lfo1-freq",       required_argument,  0, 401},
			{"lfo2-freq",       required_argument,  0, 402},
			{"snh-freq",       required_argument,  0, 304},
			{"sample-and-hold-freq",       required_argument,  0, 304},
			{"conf",           required_argument,  0, 'c'},
			{"config",           required_argument,  0, 'c'},
			{"max-port",           required_argument,  0, 303},
			{"max-port-time",           required_argument,  0, 303},
			{"max-portamento-time",           required_argument,  0, 303},
			{"filter-update-freq",           required_argument,  0, 305},
			{"lfo-update-freq",           required_argument,  0, 306},
			{"no-connect-audio-out",          no_argument,        0, 'a'},
			{"no-connect-audio",          no_argument,        0, 'a'},
			{"dont-connect-audio-out",          no_argument,        0, 'a'},
			{"dont-connect-audio",          no_argument,        0, 'a'},
			{"no-connect-midi-in",          no_argument,        0, 'm'},
			{"no-connect-midi",          no_argument,        0, 'm'},
			{"dont-connect-midi-in",          no_argument,        0, 'm'},
			{"dont-connect-midi",          no_argument,        0, 'm'},
			0 };
	
	while (optind<argc)
	{
		int index=-1;
		int result=getopt_long(argc,argv,"hVf:d:p:i:c:x:vqFam", long_options, &index);
		if (result==-1) break;
		
		switch (result)
		{
			case 'h': show_help(); exit(0); break;
			case 'V': show_version(); exit(0); break;
			case 'v': verbose=true; quiet=false; break;
			case 'q': quiet=true; verbose=false; break;
			case 'F': fatal_warnings=true; break;
			case 'c': read_config(optarg); break;
			case 'a': connect_audio=false; break;
			case 'm': connect_midi=false;  break;
			case 'f': if (optarg)
								{
									frameskip=atoi(optarg);
									if (frameskip<=0) frameskip=0;
								}
								break;
			case 'x':	if (optarg)
								{
									string stropt=optarg;
									size_t pos=stropt.find(':');
									if (pos==string::npos)
										output_warning("expected 'n_xruns:time' in --xruns option, found no ':'");
									
									xrun_n=atoi(stropt.substr(0,pos).c_str());
									xrun_time=atof(stropt.substr(pos+1).c_str());
									
									if (xrun_n<=0) xrun_n=0;
									if (xrun_time<=0) xrun_time=0;
								}
								break;
			case 'd': add_dir(optarg); break;
			case 'p': {
									string str=optarg;
									size_t pos=str.find_first_of(":=,");
									if (pos!=string::npos)
									{
										string numstr=str.substr(0,pos);
										if (isnum(numstr))
										{
											int num=atoi(numstr.c_str());
											
											if ((num>=0) && (num<=127))
											{
												if (programfile[num]=="")
													programfile[num]=str.substr(pos+1);
												else
													output_note("NOTE: program #"+IntToStr(num)+" has already been defined. ignoring it...");
											}
											else
											{
												output_warning("WARNING: number out of range (0..127) in --program option.\n  ignoring the option...");
											}
										}
										else
										{
											output_warning("WARNING: not a number in --program option. ignoring the option...");
										}
									}
									else
									{
										output_warning("WARNING: missing number in --program option. ignoring the option...");
									}
								}
								break;
			case 'i':	if (isfloat(optarg))
									cleanup_interval_sec=atof(optarg);
								else
									output_warning("WARNING: not a number in --interval option. ignoring it...");
								break;
			case 304:	if (isfloat(optarg))
									snh_freq_hz=atof(optarg);
								else
									output_warning("WARNING: not a number in --sample-and-hold-freq option. ignoring it...");
								break;
			case 400: //FINDLFO
			case 401:
			case 402:
								if (isfloat(optarg))
									lfo_freq_hz[result-400]=atof(optarg);
								else
									output_warning("WARNING: not a number in --lfoN-freq option. ignoring it...");
								break;
			case 303:	if (isfloat(optarg))
									max_port_time_sec=atof(optarg);
								else
									output_warning("WARNING: not a number in --max-portamento-time option. ignoring it...");
								break;
								
			case 305:	if (isfloat(optarg))
									if (atoi(optarg)<=0)
										output_warning("WARNING: filter-update-freq must be positive. ignoring it...");
									else
										filter_update_freq_hz=atof(optarg);
								else
									output_warning("WARNING: not a number in --filter-update-freq option. ignoring it...");
								break;
			case 306:	if (isfloat(optarg))
									if (atoi(optarg)<=0)
										output_warning("WARNING: lfo-update-freq must be positive. ignoring it...");
									else
										lfo_update_freq_hz=atof(optarg);
								else
									output_warning("WARNING: not a number in --lfo-update-freq option. ignoring it...");
								break;
								
			default: cout << "ERROR: invalid command line options. try the --help switch" << endl;
							 exit(1);
		}
	}
			

}
