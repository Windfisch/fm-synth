#include <cstdlib>
#include <fstream>

#include "parser.h"
#include "defines.h"
#include "programs.h"
#include "globals.h"
#include "util.h"
#include "readwave.h"


Parser::Parser()
{
	n_osc=0;
	osc=NULL;
	env=NULL;
	for (int i=0;i<128;i++)
		controller_default[i]=0;
}

Parser::~Parser()
{
	uninit_stuff();
}

list<string> Parser::extract_terms(string s)
{
	list<string> result;
	
	size_t p=-1,p2;
	
	s="+"+s+"+";
	
	p=0;
	p2=s.find_first_of("+-",p+1);
	
	while(p2!=string::npos)
	{
		result.push_back(s.substr(p,p2-p));	
		p=p2;
		p2=s.find_first_of("+-",p+1);
	}
	return result;
}

list<string> Parser::extract_factors(string s)
{
	list<string> result;
	
	size_t p=-1,p2;
	
	s="*"+s+"*";
	
	p=0;
	p2=s.find_first_of("*/",p+1);
	
	while(p2!=string::npos)
	{
		result.push_back(s.substr(p,p2-p));		
		p=p2;
		p2=s.find_first_of("*/",p+1);
	}
	return result;
}

list<term_t> Parser::extract_formula(string s)
{
	list<term_t> result;
	term_t tmp;
	list<string> terms=extract_terms(s);
	
	for (list<string>::iterator term=terms.begin(); term!=terms.end(); term++)
	{
		list<string> factors=extract_factors(term->substr(1));
		double fac=	((*term)[0]=='+') ? 1.0 : -1.0;
		string cont="";
		for (list<string>::iterator factor=factors.begin(); factor!=factors.end(); factor++)
		{
			if (factor->find_first_not_of("0123456789.*/+-")==string::npos)
			{
				if ((*factor)[0]=='*')
					fac*=atof((*factor).substr(1).c_str());
				else
				{
					if (atof((*factor).substr(1).c_str())==0)
						throw string("dividing by zero is not allowed");
			
					fac/=atof((*factor).substr(1).c_str());
				}
			}
			else
			{
				if (cont!="")
					throw string("multiplicating controllers is not allowed");
				
				if ((*factor)[0]!='*')
					throw string("dividing through a controller is not allowed");
				
				cont=(*factor).substr(1);
			}
		}
		if (cont=="")
			tmp.c=NO_CONT;
		else
		{
			if (extract_array_name(cont)!="cont")
				throw string("expected 'cont', found '"+extract_array_name(cont)+"'");

			tmp.c=extract_array_index(cont,1);
			if ((tmp.c<0) || (tmp.c>127))
				throw string("invalid controller specified");
		}		
		tmp.f=fac*ONE;
		result.push_back(tmp);
	}
	return result;
}

param_factor_t Parser::parse_pfactor(string s) //TODO fast dasselbe wie oben. mergen?
{ //TODO cont müsste vel heißen       FINDMICH --->  ^ ^ ^
	param_factor_t result;
	result.offset=0;
	result.vel_amount=0;
	
	list<string> terms=extract_terms(s);
	
	for (list<string>::iterator term=terms.begin(); term!=terms.end(); term++)
	{
		list<string> factors=extract_factors(term->substr(1));
		double fac=	((*term)[0]=='+') ? 1.0 : -1.0;
		string cont="";
		for (list<string>::iterator factor=factors.begin(); factor!=factors.end(); factor++)
		{
			if (factor->find_first_not_of("0123456789.*/+-")==string::npos)
			{
				if ((*factor)[0]=='*')
					fac*=atof((*factor).substr(1).c_str());
				else
				{
					if (atof((*factor).substr(1).c_str())==0)
						throw string("dividing by zero is not allowed");
						
					fac/=atof((*factor).substr(1).c_str());
				}
			}
			else
			{
				if (cont!="")
					throw string("multiplicating velocity is not allowed");
				
				if ((*factor)[0]!='*')
					throw string("dividing through velocity is not allowed");
				
				cont=(*factor).substr(1);
			}
		}
		if (cont=="")
		{
			result.offset+= fac*ONE;
		}
		else
		{
			if (cont!="vel")
				throw string("expected 'vel', found '"+cont+"'");
			
			result.vel_amount+= fac*ONE;
		}		
	}
	return result;
}

void Parser::init_stuff()
{
	env=new env_settings_t[n_osc];
	osc=new oscillator_t[n_osc];
	for (int i=0;i<n_osc;i++)
	{
		osc[i].n_osc=n_osc;
		
		osc[i].fm_strength=new fixed_t[n_osc];
		for (int j=0;j<n_osc;j++)
			osc[i].fm_strength[j]=0;
		
		osc[i].output=0;
		osc[i].waveform=0;
		osc[i].factor=ONE;
		osc[i].tremolo_depth=0;
		osc[i].tremolo_lfo=0;
		osc[i].vibrato_depth=0;
		osc[i].vibrato_lfo=0;
		osc[i].custom_wave=NULL;
		
		
		env[i].attack=0;
		env[i].decay=0;
		env[i].sustain=ONE;
		env[i].release=0;
		env[i].hold=true;
	}

	filter.enabled=false;
	filter.env_amount=0;
	filter.env_settings.attack=filter.env_settings.decay=
	   filter.env_settings.release=0;
	filter.env_settings.sustain=0;
	filter.env_settings.hold=true;
	
	filter.freqfactor_offset=0;
	filter.resonance=0;
	filter.trem_strength=0;
	filter.trem_lfo=0;
	
	
	
	pfactor.out=new param_factor_t [n_osc];
	pfactor.fm=new param_factor_t* [n_osc];
	
	pfactor.filter_env.offset=ONE;
	pfactor.filter_env.vel_amount=0;
	
	pfactor.filter_res.offset=ONE;
	pfactor.filter_res.vel_amount=0;
	
	pfactor.filter_offset.offset=ONE;
	pfactor.filter_offset.vel_amount=0;
	
	for (int i=0;i<n_osc;i++)
	{
		pfactor.out[i].offset=0;
		pfactor.out[i].vel_amount=ONE;
		
		pfactor.fm[i]=new param_factor_t [n_osc];
		for (int j=0;j<n_osc;j++)
		{
			pfactor.fm[i][j].offset=ONE;
			pfactor.fm[i][j].vel_amount=0;			
		}
	}

}
void Parser::uninit_stuff()
{
	if (osc)
	{
		for (int i=0;i<n_osc;i++)
			delete [] osc[i].fm_strength;
		
		delete [] osc;
		osc=NULL;
	}
	if (env)
		delete [] env;
}

string Parser::extract_array_name(string s)
{
	size_t p;
	p=s.find('[');
	if (p!=string::npos)
		return s.substr(0,p);
	else
		return s;	
}

int Parser::extract_array_index(string s, int dim)
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

//if this function fails, this WILL BE fatal if unhandled in the
//caller function. so this function throws errors
//if this function fails the settings are in an undefined, illegal
//state. if these settings are given to some oscillator_t by
//operator=, it will probably die while trying to create an array
//with size 0 or so.
void Parser::parse(string fn)
{
	char buf[2000];
	list<term_t> terms;
	string line;
	string var;
	string array;
	string strval;
	float val;
	
	parameter_enum p;
	
	int ind,ind2=0;
	
	int state;
	
	uninit_stuff();
	
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
						
						init_stuff();
						
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
									size_t pos=strval.find(':');
									if (pos==string::npos)
										throw string("expected 'freq:file.wav', found no ':'");
									
									float given_freq=atof(strval.substr(0,pos).c_str());									
									string wavefile=strval.substr(pos+1);

									if (given_freq<=0)
										throw string("illegal freq specified for custom wave '"+wavefile+"'");
									
									osc[ind].custom_wave=new custom_wave_t;
									read_wave(wavefile.c_str(), osc[ind].custom_wave);
									osc[ind].custom_wave->samp_rate/=given_freq;
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
								env[ind].attack=val*samp_rate;
								break;
							case DECAY:
								env[ind].decay=val*samp_rate;
								break;
							case SUSTAIN:
								env[ind].sustain=val*ONE;
								break;
							case RELEASE:
								env[ind].release=val*samp_rate;
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
								filter.env_settings.attack=val*samp_rate/filter_update_frames;
								break;
							case FILTER_DECAY:
								filter.env_settings.decay=val*samp_rate/filter_update_frames;
								break;
							case FILTER_SUSTAIN:
								filter.env_settings.sustain=val*ONE;
								break;
							case FILTER_RELEASE:
								filter.env_settings.release=val*samp_rate/filter_update_frames;
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

					case 2: //read how controllers influence parameters
						p=param_to_enum(array);
						
						ind=extract_array_index(var,1);
						if ( param_needs_index(p) &&  (!((ind>=0) && (ind<n_osc))) )
							throw string("out of array bounds");
												
						parameter_t par;
						par.par=p;

						if (par.par==UNKNOWN)
							throw string("unknown variable ('"+array+"')");

						if (par.par==MODULATION)
						{
							ind2=extract_array_index(var,2);
							if (!((ind2>=0) && (ind2<n_osc)))
								throw string("out of array bounds");
						}

						par.osc=ind;
						par.index=ind2;

						terms=extract_formula(strval);
						for (list<term_t>::iterator it=terms.begin(); it!=terms.end(); it++)
							if (it->c!=NO_CONT)
								affect[it->c].insert(par);
						
						
						formula[par]=terms;
						break;
					
					case 3: //read controller default values
						if (array=="cont")
						{
							ind=extract_array_index(var,1);
							
							if ((ind<0) || (ind>127))
								throw string("out of array bounds");
							
							if ((val<0) || (val>127))
								throw string("value out of range");
								
							controller_default[ind]=val;
						}
						else
							throw string("expected cont, found '"+array+"'");
						
						break;
						
					case 4: //read velocity-influence over certain params
						p=param_to_enum(array);

						ind=extract_array_index(var,1);
						if ( param_needs_index(p) &&  (!((ind>=0) && (ind<n_osc))) )
							throw string("out of array bounds");
						
						switch(p)
						{
							case MODULATION:
								ind2=extract_array_index(var,2);
								if (!((ind2>=0) && (ind2<n_osc)))
									throw string("out of array bounds");

								pfactor.fm[ind][ind2]=parse_pfactor(strval);
								break;
								
							case OUTPUT:
								pfactor.out[ind]=parse_pfactor(strval);
								break;
								
							case FILTER_ENV_AMOUNT:
								pfactor.filter_env=parse_pfactor(strval);
								break;
								
							case FILTER_RESONANCE:
								pfactor.filter_res=parse_pfactor(strval);
								break;
								
							case FILTER_OFFSET:
								pfactor.filter_offset=parse_pfactor(strval);
								break;
								
							default:
								throw string("velocity cannot influence parameter '"+array+"'");
						

				}
				}
			}
		}
	}
	else
		throw string ("could not open '"+fn+"'");
}


program_t Parser::get_results() const
{
	program_t result;

	result.n_osc=n_osc;

	copy(&affect[0],&affect[128],result.controller_affects);

	result.formula=formula;

	result.osc_settings=new oscillator_t[n_osc];

	copy(&osc[0],&osc[n_osc],result.osc_settings);
	result.env_settings=new env_settings_t[n_osc];
	copy(&env[0],&env[n_osc],result.env_settings);

	for (int i=0;i<128;i++)
		result.controller[i]=controller_default[i];
	
	result.filter_settings=filter;
	
	result.sync_factor=sync_factor;
	
	result.pfactor=pfactor;
	
	return result;
}
