/*
    Copyright (C) 2010-2012 Florian Jung
     
    This file is part of flo's FM synth.

    flo's FM synth is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    flo's FM synth is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flo's FM synth.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <cstdlib>
#include <fstream>

#include "parser.h"
#include "programs.h"
#include "util.h"
#include "../synth/defines.h"
#include "../synth/fixed.h"


void init_oscs(int n_osc, oscillator_t *osc)
{
	for (int i=0;i<n_osc;i++)
	{
		osc[i].n_osc=n_osc;
		
		osc[i].fm_strength=new fixed_t[n_osc];
		osc[i].fm_strength_const=new bool[n_osc];
		for (int j=0;j<n_osc;j++)
		{
			osc[i].fm_strength[j]=0;
			osc[i].fm_strength_const[j]=true;
		}
		
		osc[i].output=0;
		osc[i].output_const=true;
		osc[i].waveform=0;
		osc[i].waveform_const=true;
		osc[i].factor=ONE;
		osc[i].factor_const=true;
		osc[i].tremolo_depth=0;
		osc[i].tremolo_depth_const=true;
		osc[i].tremolo_lfo=0;
		osc[i].tremolo_lfo_const=true;
		osc[i].vibrato_depth=0;
		osc[i].vibrato_depth_const=true;
		osc[i].vibrato_lfo=0;
		osc[i].vibrato_lfo_const=true;
		osc[i].have_custom_wave=false;
		osc[i].sync=false;
		osc[i].sync_const=true;
		osc[i].ksr_const=true;
		osc[i].ksl_const=true;
	}
}

void init_envs(int n_osc, env_settings_t *env)
{
	for (int i=0;i<n_osc;i++)
	{
		env[i].enabled=true;
		env[i].attack=0;
		env[i].attack_const=true;
		env[i].decay=0;
		env[i].decay_const=true;
		env[i].sustain=1.0;
		env[i].sustain_const=true;
		env[i].release=0;
		env[i].release_const=true;
		env[i].hold=true;
		env[i].hold_const=true;
	}
}

void init_filter(filter_params_t &filter)
{
	filter.enabled=false;
	filter.enabled_const=true;
	filter.env_amount=0;
	filter.env_amount_const=true;
	filter.env_settings.attack=0;
	filter.env_settings.attack_const=true;
	filter.env_settings.decay=0;
	filter.env_settings.decay_const=true;
	filter.env_settings.release=0;
	filter.env_settings.release_const=true;
	filter.env_settings.sustain=0;
	filter.env_settings.sustain_const=true;
	filter.env_settings.hold=true;
	filter.env_settings.hold_const=true;
	
	filter.freqfactor_offset=0;
	filter.freqfactor_offset_const=true;
	filter.resonance=0;
	filter.resonance_const=true;
	filter.trem_strength=0;
	filter.trem_strength_const=true;
	filter.trem_lfo=0;
	filter.trem_lfo_const=true;
}


string extract_array_name(string s)
{
	size_t p;
	p=s.find('[');
	if (p!=string::npos)
		return s.substr(0,p);
	else
		return s;	
}

int extract_array_index(string s, int dim)
{
	size_t p=-1,p2;
	for (int i=0;i<dim;i++)
	{
		p=s.find('[',p+1);
		if (p==string::npos) return -1;
	}
	
	p2=s.find(']',p+1);
	if (p2==string::npos) return -1;
	
	return atoi(s.substr(p+1,p2-p-1).c_str());
}



program_t parse(string fn)
{
	int n_osc=0;
	oscillator_t *osc=NULL;
	env_settings_t *env=NULL;
	filter_params_t filter;
	fixed_t sync_factor=0;
	bool sync_factor_const=true;
	
	char buf[2000];
	string line;
	string var;
	string array;
	string strval;
	float val;
	
	parameter_enum p;
	
	int ind,ind2=0;
	
	int state;

	program_t result;

	
	ifstream f;
	f.open(fn.c_str());
	if (f.good())
	{
		state=0;
		while (!f.eof())
		{
			f.getline(buf,sizeof(buf)/sizeof(*buf)-1);
			line=buf;
			line=remove_all_spaces(buf);
			if ((line!="") && (line[0]!='#')) //ignore comments and empty lines
			{
				if (line=="controllers:")
				{
					state=2;
					continue;
				}
				else if (line=="defaults:")
				{
					state=3;
					continue;
				}
				else if (line=="velocity:")
				{
					state=4;
					continue;
				}
				else if (line=="variable:")
				{
					state=5;
					continue;
				}

				var=extract_var(line);
				array=extract_array_name(var);
				strval=extract_val(line);
				val=atof(strval.c_str());
				
				switch (state)
				{
					case 0: //expect and read number of oscillators
						if (var!="oscillators")
							throw string("need to know number of oscillators");
						else
							n_osc=val;
						
						if (n_osc<=0) throw string("invalid number of oscillators");
						
						//init stuff
						env=new env_settings_t[n_osc];
						osc=new oscillator_t[n_osc];
						init_oscs(n_osc, osc);
						init_envs(n_osc, env);
						init_filter(filter);
						
						state=1;
						break;
					
					case 1: //read and set information about oscillator settings
						p=param_to_enum(array);
						
						ind=extract_array_index(var,1);
						if ( param_needs_index(p) &&  (!((ind>=0) && (ind<n_osc))) )
							throw string("out of array bounds");
						
						
						switch (p)
						{
							case MODULATION:						
								ind2=extract_array_index(var,2);
								if (!((ind2>=0) && (ind2<n_osc)))
									throw string("out of array bounds");
							
								osc[ind].fm_strength[ind2]=val*ONE;
								break;
							case OUTPUT:
								osc[ind].output=val*ONE;
								break;
							case WAVEFORM:
								if (isfloat(strval))
								{
									osc[ind].waveform=int(val);
								}
								else
								{
									osc[ind].have_custom_wave=true;
								}
								break;
							case FACTOR:
								osc[ind].factor=val*ONE;
								break;
							case TREMOLO:
								osc[ind].tremolo_depth=int(val);
								break;
							case TREM_LFO:
								if (strval=="snh")
									osc[ind].tremolo_lfo=SNH_LFO;
								else
								{
									osc[ind].tremolo_lfo= int(val);
									if ((val<0) || (val>=N_LFOS))
										throw string("invalid value for tremolo_lfo");
								}
								break;
							case VIBRATO:
								osc[ind].vibrato_depth=val;
								break;
							case VIB_LFO:
								if (strval=="snh")
									osc[ind].vibrato_lfo= SNH_LFO;
								else
								{
									osc[ind].vibrato_lfo= int(val);
									if ((val<0) || (val>=N_LFOS))
										throw string("invalid value for vibrato_lfo");
								}
								break;
							case ATTACK:
								env[ind].attack=val;
								break;
							case DECAY:
								env[ind].decay=val;
								break;
							case SUSTAIN:
								env[ind].sustain=val;
								break;
							case RELEASE:
								env[ind].release=val;
								break;
							case HOLD:
								env[ind].hold=(val!=0);
								break;
							case KSR:
								osc[ind].ksr=val;
								break;
							case KSL:
								osc[ind].ksl=val;
								break;
							case SYNC:
								osc[ind].sync=(val!=0);
								break;
							case FILTER_ENABLED:
								filter.enabled=(val!=0);
								break;
							case FILTER_ENV_AMOUNT:
								filter.env_amount=val;
								break;
							case FILTER_ATTACK:
								filter.env_settings.attack=val;
								break;
							case FILTER_DECAY:
								filter.env_settings.decay=val;
								break;
							case FILTER_SUSTAIN:
								filter.env_settings.sustain=val;
								break;
							case FILTER_RELEASE:
								filter.env_settings.release=val;
								break;
							case FILTER_HOLD:
								filter.env_settings.hold=(val!=0);
								break;
							case FILTER_OFFSET:
								filter.freqfactor_offset=val;
								break;
							case FILTER_RESONANCE:
								filter.resonance=val;
								break;
							case FILTER_TREMOLO:
								filter.trem_strength=int(val);
								break;
							case FILTER_TREM_LFO:
								if (strval=="snh")
									filter.trem_lfo=SNH_LFO;
								else
								{
									filter.trem_lfo=int(val);
									if ((val<0) || (val>=N_LFOS))
										throw string("invalid value for filter_trem_lfo");
								}
								break;
							case SYNC_FACTOR:
								sync_factor=val*ONE;
								break;
							default:
								throw string("unknown variable ('"+array+"')");
						}
						break;
					
					case 5: //read which params shall be variable, even if
					        //there are currently no controllers which change them
					case 4: //read velocity-influence over certain params
					case 2: //read how controllers influence parameters
						p=param_to_enum(array);
						
						ind=extract_array_index(var,1);
						if ( param_needs_index(p) &&  (!((ind>=0) && (ind<n_osc))) )
							throw string("out of array bounds");
						
						if (state==4) //velocity-influence
						{
							switch (p)
							{
								case MODULATION:
								case OUTPUT:
								case FILTER_ENV_AMOUNT:
								case FILTER_RESONANCE:
								case FILTER_OFFSET:
									// everything ok, do nothing
									break;
								
								default: // other params than the above may not be influenced!
									throw string("velocity cannot influence parameter '"+array+"'");
							}
						}
						

						switch (p)
						{
							case MODULATION:						
								ind2=extract_array_index(var,2);
								if (!((ind2>=0) && (ind2<n_osc)))
									throw string("out of array bounds");
							
								osc[ind].fm_strength_const[ind2]=false;
								break;
							case OUTPUT:
								if (state!=4) // not vel.-influence
									osc[ind].output_const=false;
								break;
							case WAVEFORM:
								osc[ind].waveform_const=false; break;
							case FACTOR:
								osc[ind].factor_const=false; break;
							case TREMOLO:
								osc[ind].tremolo_depth_const=false; break;
							case TREM_LFO:
								osc[ind].tremolo_lfo_const=false; break;
							case VIBRATO:
								osc[ind].vibrato_depth_const=false; break;
							case VIB_LFO:
								osc[ind].vibrato_lfo_const=false; break;
							case ATTACK:
								env[ind].attack_const=false; break;
							case DECAY:
								env[ind].decay_const=false; break;
							case SUSTAIN:
								env[ind].sustain_const=false; break;
							case RELEASE:
								env[ind].release_const=false; break;
							case HOLD:
								env[ind].hold_const=false; break;
							case KSR:
								osc[ind].ksr_const=false; break;
							case KSL:
								osc[ind].ksl_const=false; break;
							case SYNC:
								osc[ind].sync_const=false; break;
							case FILTER_ENABLED:
								filter.enabled_const=false; break;
							case FILTER_ENV_AMOUNT:
								filter.env_amount_const=false; break;
							case FILTER_ATTACK:
								filter.env_settings.attack_const=false; break;
							case FILTER_DECAY:
								filter.env_settings.decay_const=false; break;
							case FILTER_SUSTAIN:
								filter.env_settings.sustain_const=false; break;
							case FILTER_RELEASE:
								filter.env_settings.release_const=false; break;
							case FILTER_HOLD:
								filter.env_settings.hold_const=false; break;
							case FILTER_OFFSET:
								filter.freqfactor_offset_const=false; break;
							case FILTER_RESONANCE:
								filter.resonance_const=false; break;
							case FILTER_TREMOLO:
								filter.trem_strength_const=false; break;
							case FILTER_TREM_LFO:
								filter.trem_lfo_const=false; break;
							case SYNC_FACTOR:
								sync_factor_const=false; break;
							default:
								throw string("unknown variable ('"+array+"')");
						}


						
						break;
					
					case 3: //read controller default values
						//ignored						
						break;
				}
			}
		}
		
		
		//some optimizations and checks
		
		for (int i=0;i<n_osc;i++)
			if ((env[i].attack==0) && (env[i].sustain==1.0)
			      && (env[i].release>100))  //TODO FINDMICH besseres kriterium?
				env[i].enabled=false;
		
		if (  ((filter.env_settings.attack==0) && (filter.env_settings.sustain==1.0)
		        && (filter.env_settings.release>100)) //TODO FINDMICH siehe oben
		   || ((filter.env_amount==0) && (filter.env_amount_const==true))  )
		  filter.env_settings.enabled=false;
		
		bool use_sync=false;
		for (int i=0;i<n_osc;i++)
			if ((osc[i].sync==true) || (osc[i].sync_const==false))
			{
				use_sync=true;
				break;
			}
		
		if (!use_sync)
		{
			sync_factor=0;
			sync_factor_const=true;
		}
		
		if ((sync_factor==0) && (sync_factor_const==true))
			for (int i=0;i<n_osc;i++)
			{
				osc[i].sync=false;
				osc[i].sync_const=true;
			}
		
		
		
		for (int i=0;i<n_osc;i++)
			if ( !((osc[i].output==0) && (osc[i].output_const=true)) )
				osc[i].output_const=false;
		
		//end optimizations and checks
		
		result.n_osc=n_osc;
		result.osc=osc;
		result.env=env;
		result.filter=filter;
		result.sync_factor=sync_factor;
		result.sync_factor_const=sync_factor_const;
	}
	else
		throw string ("could not open '"+fn+"'");

	return result;
	//no uninit / free of osc and env here, as it must be done by the caller
}



