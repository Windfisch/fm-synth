#include <string>
#include <fstream>
#include <cstdlib>
#include <sys/types.h>
#include <dirent.h>


#include "util.h"
#include "globals.h"
#include "parser.h"
#include "note_loader.h"


using namespace std;

void add_dir(string directory, bool complain=true)
{
	DIR *dir;
	dir=opendir(directory.c_str());
	if (dir)
	{
		string n, snum;
		int num;
		dirent *entry=NULL;
		
		while ( (entry=readdir(dir)) != NULL )
		{
			n=entry->d_name;
			if (fileext(n)=="prog")
			{
				snum=n.substr(0,3);
				if (isnum(snum))
				{
					num=atoi(snum.c_str());
					if ((num>=0) && (num<=127))
					{
						if (programfile[num]=="")
							programfile[num]=n;
						else
							output_verbose ("NOTE: found two or more .prog files with same number. ignoring '"+n+"'");
					}
					else
					{
						output_note ("NOTE: found .prog file with invalid number, ignoring it... ('"+n+"')");
					}
				}
				else
				{
					output_note ("NOTE: found .prog file which does not start with a number, ignoring it... ('"+n+"')");
				}
			}
		}
		closedir(dir);
	}
	else
	{
		if (complain)
			output_warning("WARNING: could not open directory '"+directory+"'!\n"
										 "         this is not fatal, ignoring and proceeding...");
	}
}

void read_config(const char *cfg, bool complain=true)
{
	char buf[2000];
	
	ifstream f;
	f.open(cfg);
	
	if (f.good())
	{
		string line;
		while (!f.eof())
		{
			f.getline(buf,sizeof(buf)/sizeof(*buf)-1);
			line=buf;
			line=trim_spaces(line);

			if ((line!="") && (line[0]!='#')) //ignore comments and empty lines
			{
				if (line.substr(0,string("include ").length())=="include ")
				{
					add_dir(trim_spaces(line.substr(string("include ").length())));
				}
				else
				{
					string var,val;
					var=trim_spaces(extract_var(line));
					val=trim_spaces(extract_val(line));

					if (isnum(var)) //programmzuweisung
					{
						int num=atoi(var.c_str());
						if ((num>=0) && (num<=127))
						{
							if (programfile[num]=="")
								programfile[num]=val;
							else
								output_verbose ("NOTE: program #"+IntToStr(num)+" has already been defined. ignoring it...");
						}
						else
						{
							output_warning("WARNING: number out of range (0..127) in program assignment. ignoring it...");
						}
					}
					else
					{
						float valf=atof(val.c_str());
							
						if (var=="frameskip")
						{
							#ifdef FRAMESKIP
								if (valf<0)
									output_warning("WARNING: invalid value for '"+var+"' ("+val+"). ignoring it...");
									
								if (frameskip==-1)
									frameskip=valf;
								else
									output_verbose("NOTE: ignoring value for frameskip, another setting overrides this.");
							#else
								output_warning("WARNING: while parsing conf: support for frameskipping isn't compiled in!");
							#endif
						}
						else
						{
							if (valf<=0)
								output_warning("WARNING: invalid value for '"+var+"' ("+val+"). ignoring it...");
							
							if ((var=="snh_freq") || (var=="sample_and_hold_freq"))
							{
								if (snh_freq_hz==0)
									snh_freq_hz=valf;
								else
									output_verbose("NOTE: ignoring value for sample_and_hold_freq, another setting overrides this.");
							}
							if (var=="lfo0_freq") //FINDLFO
							{
								if (lfo_freq_hz[0]==0)
									lfo_freq_hz[0]=valf;
								else
									output_verbose("NOTE: ignoring value for "+var+", another setting overrides this.");
							}
							if (var=="lfo1_freq")
							{
								if (lfo_freq_hz[1]==0)
									lfo_freq_hz[1]=valf;
								else
									output_verbose("NOTE: ignoring value for "+var+", another setting overrides this.");
							}
							if (var=="lfo2_freq")
							{
								if (lfo_freq_hz[2]==0)
									lfo_freq_hz[2]=valf;
								else
									output_verbose("NOTE: ignoring value for "+var+", another setting overrides this.");
							}
							else if ((var=="cleanup-interval") || (var=="clean"))
							{
								if (cleanup_interval_sec==0)
									cleanup_interval_sec=valf;
								else
									output_verbose("NOTE: ignoring value for cleanup-interval, another setting overrides this.");
							}
							else if ((var=="max_port") || (var=="max_port_time") || (var=="max_portamento_time"))
							{
								if (max_port_time_sec==0)
									max_port_time_sec=valf;
								else
									output_verbose("NOTE: ignoring value for max-portamento-time, another setting overrides this.");
							}
							else if (var=="lfo_update_freq")
							{
								if (lfo_update_freq_hz==0)
									lfo_update_freq_hz=valf;
								else
									output_verbose("NOTE: ignoring value for lfo_update_freq, another setting overrides this.");
							}
							else if (var=="filter_update_freq")
							{
								if (filter_update_freq_hz==0)
									filter_update_freq_hz=valf;
								else
									output_verbose("NOTE: ignoring value for filter_update_freq, another setting overrides this.");
							}
							else if ((var=="envelope_update_freq") || (var=="env_update_freq"))
							{
								if (envelope_update_freq_hz==0)
									envelope_update_freq_hz=valf;
								else
									output_verbose("NOTE: ignoring value for envelope_update_freq, another setting overrides this.");
							}
							else
								output_warning("WARNING: unknown variable '"+var+"'. ignoring it...");
						}
					}
				}
			}
		}
	}
	else
	{
		output_warning("WARNING: could not open config file '"+string(cfg)+"'.\nignoring this file...");
	}
}

bool load_program(string file, program_t& prog)
{
	if (file!="")
	{
		try
		{
			prog=parse(file);
			
			// try to load the appropriate .so file
			if (access(  (file+".so").c_str(), R_OK ) == 0)
			{
				try
				{
					load_note_from_so(file+".so", prog);
					output_verbose("NOTE: loaded shared object for program '"+file+"'");
				}
				catch (string err)
				{
					output_note("NOTE: could not load shared object '"+file+".so"+"':\n"
											"  "+err+"\n"
											"  this is not fatal, the note has been loaded properly, but generic\n"
											"  unoptimized (slow) code will be used.");
				}        
			}
			
			return true;
		}
		catch (string err)
		{
			output_warning("WARNING: error parsing '"+file+"': "+err+"\n"
										 "  this is not fatal, but the program has NOT been loaded! defaulting to a\n"
										 "  simple program and going on...");
			prog=default_program;
			
			return false;
		}
	}
	else
	{
		prog=default_program;
		
		return true;
	}
}
