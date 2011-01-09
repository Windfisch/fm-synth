#include <iostream>
#include <sstream>

#include "util.h"
#include "globals.h"

string IntToStr(int i)
{
	ostringstream s;
	s<<i;
	return s.str();
}

string IntToStrHex(int i)
{
	ostringstream s;
	s<<std::hex << i;
	return s.str();
}

bool isnum(string s)
{
	for (size_t i=0;i<s.length();i++)
		if (!isdigit(s[i]))
			return false;
	
	return true;
}

bool isfloat(string s)
{
	return (s.find_first_not_of("0123456789.")==string::npos);
}


string remove_all_spaces(string s)
{
	string result;
	
	for (size_t i=0; i<s.length(); i++)
		if ((s[i]!=' ') && (s[i]!='\t'))
			result+=s[i];
	
	return result;
}

string trim_spaces(string s)
{
	string result;
	int i;
	
	for (i=0;i<s.length();i++)
		if ((s[i]!=' ') && (s[i]!='\t'))
			break;
	
	if (i!=s.length())
	{
		result=s.substr(i);
		for (i=result.length()-1;i>=0;i--)
			if ((result[i]!=' ') && (result[i]!='\t'))
				break;
		
		if (i>=0)
			return result.substr(0,i+1);
		else
			return "";
	}
	else
	{
		return "";
	}
}

void output_warning(string s)
{
	cout << s << endl;
	if (fatal_warnings) throw string(s);
}

void output_note(string s)
{
	if (!quiet) cout << s << endl;
}

void output_verbose(string s)
{
	if (verbose) cout << s << endl;
}

bool param_needs_index(parameter_enum p)
{
	switch (p)
	{
		case FILTER_ENABLED:

		case FILTER_ATTACK:
		case FILTER_DECAY:
		case FILTER_SUSTAIN:
		case FILTER_RELEASE:
		case FILTER_HOLD:
		case FILTER_ENV_AMOUNT:

		case FILTER_OFFSET:
		case FILTER_RESONANCE:

		case FILTER_TREMOLO:
		case FILTER_TREM_LFO:
		
		case SYNC_FACTOR:
			return false;
		
		default: return true;
	}
}

parameter_enum param_to_enum(string param)
{
	if (param=="mod")
		return MODULATION;
	else if (param=="out")
		return OUTPUT;
	else if (param=="waveform")
		return WAVEFORM;
	else if (param=="sync")
		return SYNC;
	else if (param=="factor")
		return FACTOR;
	else if (param=="trem")
		return TREMOLO;
	else if (param=="trem_lfo")
		return TREM_LFO;
	else if (param=="vib")
		return VIBRATO;
	else if (param=="vib_lfo")
		return VIB_LFO;
	else if (param=="attack") 
		return ATTACK;
	else if (param=="decay")
		return DECAY;
	else if (param=="sustain")
		return SUSTAIN;
	else if (param=="release")
		return RELEASE;
	else if (param=="hold")
		return HOLD;
	else if (param=="ksl")
		return KSL;
	else if (param=="ksr")
		return KSR;
	else if (param=="filter.enabled")
		return FILTER_ENABLED;
	else if (param=="filter.env_amount")
		return FILTER_ENV_AMOUNT;
	else if (param=="filter.attack")
		return FILTER_ATTACK;
	else if (param=="filter.decay")
		return FILTER_DECAY;
	else if (param=="filter.sustain")
		return FILTER_SUSTAIN;
	else if (param=="filter.release")
		return FILTER_RELEASE;
	else if (param=="filter.hold")
		return FILTER_HOLD;
	else if (param=="filter.offset")
		return FILTER_OFFSET;
	else if (param=="filter.resonance")
		return FILTER_RESONANCE;
	else if (param=="filter.trem")
		return FILTER_TREMOLO;
	else if (param=="filter.trem_lfo")
		return FILTER_TREM_LFO;
	else if (param=="sync_factor")
		return SYNC_FACTOR;
	else
		return UNKNOWN;
}

string extract_var(string s)
{
	size_t p;
	p=s.find('=');
	if (p!=string::npos)
		return s.substr(0,p);
	else
		return "";
}

string extract_val(string s)
{
	size_t p;
	p=s.find('=');
	if (p!=string::npos)
		return s.substr(p+1);
	else
		return s;
}

string fileext(string f)
{
	size_t pos;
	pos=f.rfind('.');
	if (pos!=string::npos)
		return f.substr(pos+1);
	else
		return "";
}
