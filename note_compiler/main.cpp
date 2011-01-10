//TODO: auf unbenutzte envelopes achten!

#include <iostream>
#include <fstream>

#include "parser.h"
#include "programs.h"
#include "util.h"
#include "../synth/fixed.h"

using namespace std;

ostream &out=cout;
ostream &comment=cout;
program_t prog;

void write_empty_line()
{
	out << "\n";
}

void include_file(string file)
{
	file="templates/"+file;
	
	ifstream in;
	in.open(file.c_str());
	if (!in.good())
		throw string ("include: could not open '"+file+"'");
	
	char tempbuf[2000];
	while (!in.eof())
	{
		in.getline(tempbuf, sizeof(tempbuf));
		out << tempbuf << "\n";
	}
}

void write_env_decs()
{
	for (int i=0;i<prog.n_osc;i++)
		if (prog.env[i].enabled)
			out << "\t\tEnvelope *env"<<i<<";\n";
		else
		{
			comment << "\t\t//envelope"<<i<<" is unused\n";
			if ( (prog.osc[i].ksl!=0) || (prog.osc[i].ksl_const==false) )
				out << "\t\tfixed_t kslval"<<i<<";\n";
		}
}
void write_oscval_decs()
{
	out << "\t\tfixed_t *oscval;\n"
				 "\t\tfixed_t *old_oscval;\n";
}
void write_osc_decs()
{
	for (int i=0;i<prog.n_osc;i++)
		out << "\t\toscillator_t osc"<<i<<";\n";
}
void write_osc_decs2()
{
	for (int i=0;i<prog.n_osc;i++)
		out << "\t\t\toscillator_t osc"<<i<<";\n";
}

void write_sync_decs()
{
	if ((prog.sync_factor!=0) || (prog.sync_factor_const==false))
		out << "\t\tfixed_t sync_factor;\n"
		       "\t\tfixed_t sync_phase;\n";
	else
		comment << "\t\t//sync is disabled\n";
}

void write_filter_decs()
{
	if ((prog.filter.enabled==true) || (prog.filter.enabled_const==false))
	{
		out << "\t\tLowPassFilter filter;\n"
		       "\t\tfilter_params_t filter_params;\n"
		       "\t\tint filter_update_counter;\n";
		if (prog.filter.env_settings.enabled==true)
			out << "\t\tEnvelope *filter_envelope;\n";
		else
			comment << "\t\t//filter envelope is disabled\n";

	}
	else
		comment << "\t\t//filter is disabled\n";
}

void write_pfactor_decs()
{
	out << "\t\tpfactor_value_t pfactor;\n"
				 "\t\tstruct\n"
				 "\t\t{\n";
	write_osc_decs2();
	if ((prog.filter.enabled==true) || (prog.filter.enabled_const==false))
		out << "\t\t\tfilter_params_t filter_params;\n";
	else
		comment << "\t\t\t//filter is disabled\n";
	out << "\t\t} orig;\n";
}


void write_ctor()
{
	int i;
	string tabtmp="";
	
	out << "Note::Note(int n, float v, program_t &prg, jack_nframes_t pf, fixed_t pb, int prg_no, float vol_fac)\n"
	       "{\n"	
	       "\tcurr_prg=&prg;\n"
	       "\t\n"
	       "\toscval=new fixed_t["<< prog.n_osc <<"];\n"
	       "\told_oscval=new fixed_t["<< prog.n_osc <<"];\n"
	       "\tfor (int i=0;i<"<<prog.n_osc<<";i++)\n"
				 "\t\toscval[i]=old_oscval[i]=0;\n"
				 "\t\n"
	       "\tpfactor.out=new fixed_t ["<<prog.n_osc<<"];\n"
				 "\tpfactor.fm=new fixed_t* ["<<prog.n_osc<<"];\n"
				 "\tfor (int i=0;i<"<<prog.n_osc<<";i++)\n"
				 "\t\tpfactor.fm[i]=new fixed_t ["<<prog.n_osc<<"];\n"
				 "\t\n";
	
	for (i=0;i<prog.n_osc;i++)
		if (prog.env[i].enabled)
			out << "\tenv"<<i<<"=new Envelope (prg.env_settings["<<i<<"]);\n";
		else
			comment << "\t//envelope"<<i<<" is disabled\n";

	out << "\t\n";
	
	for (i=0;i<prog.n_osc;i++)
	{
		out << "\tosc"<<i<<"=prg.osc_settings["<<i<<"];\n"
		       "\torig.osc"<<i<<"=prg.osc_settings["<<i<<"];\n";
	}
	
	out << "\t\n";
	
	out << "\t//initalize oscillator.phase to multiples of their wave resolution\n";
	for (i=0;i<prog.n_osc;i++)
	{
		out << "\tosc"<<i<<".phase=";
		if (prog.osc[i].have_custom_wave)
			out << "init_custom_osc_phase(osc"<<i<<".custom_wave->wave_len, osc"<<i<<".custom_wave->samp_rate);\n";
		else
			out << "ONE * PHASE_INIT;\n";
	}
	
	out << "\t\n"
	       "\tdo_ksl();\n"
	       "\t\n"
	       "\t\n";
	
	if ((prog.filter.enabled==true) || (prog.filter.enabled_const==false))
	{
		//a filter_params and orig.filter_params member exist
		out << "\tfilter_params=prg.filter_settings;\n"
		       "\torig.filter_params=prg.filter_settings;\n";
		
		if (prog.filter.enabled_const==false)
		{
			out << "\tif (filter_params.enabled)\n"
			       "\t{\n";
			tabtmp="\t";
		}
		out << tabtmp << "\tfilter_envelope=new Envelope(filter_params.env_settings);\n" <<
		       tabtmp << "\tfilter_update_counter=filter_update_frames;\n";
		if (prog.filter.enabled_const==false)
			out << "\t}\n";
		tabtmp="";
		
		out << "\t\n"
		       "\t\n";
	}
	
	
	if ((prog.sync_factor!=0) || (prog.sync_factor_const==false))
		out << "\tsync_factor=prg.sync_factor;\n"
		       "\tsync_phase=0;\n"
		       "\t\n"
		       "\t\n";


	include_file("ctor.foot");
}

void write_dtor()
{
	int i;
	
	out << "Note::~Note()\n"
	       "{\n";
	
	for (i=0;i<prog.n_osc;i++)
	{
		out << "\tdelete [] osc"<<i<<".fm_strength;\n";
		
		if (prog.env[i].enabled)
			out << "\tdelete env"<<i<<";\n";
		else
			comment << "\t//envelope"<<i<<" is disabled\n";
			
		out << "\tdelete pfactor.fm["<<i<<"];\n";
			
		out << "\t\n";
	}
	
	out << "\t\n"
	       "\tdelete [] oscval;\n"
	       "\tdelete [] old_oscval;\n"
	       "\t\n"
				 "\tdelete [] pfactor.out;\n"
				 "\tdelete [] pfactor.fm;\n"
	       "}\n";
}


void write_recalc_factors()
{
	out << "void Note::recalc_factors()\n"
	       "{\n";
	
	if ((prog.filter.enabled==true) || (prog.filter.enabled_const==false))
	{
		out << "\tpfactor.filter_env=calc_pfactor(curr_prg->pfactor.filter_env, vel);\n"
					 "\tpfactor.filter_res=calc_pfactor(curr_prg->pfactor.filter_res, vel);\n"
					 "\tpfactor.filter_offset=calc_pfactor(curr_prg->pfactor.filter_offset, vel);\n"
					 "\t\n";
	}
	
	out << "\tfor (int i=0;i<"<<prog.n_osc<<";i++)\n"
				 "\t{\n"
				 "\t\tpfactor.out[i]=calc_pfactor(curr_prg->pfactor.out[i], vel) * volume_factor;\n"
				 "\t\t\n"
				 "\t\tfor (int j=0;j<"<<prog.n_osc<<";j++)\n"
				 "\t\t\tpfactor.fm[i][j]=calc_pfactor(curr_prg->pfactor.fm[i][j], vel);\n"
				 "\t}\n";
	
	out << "}\n";	
}

void write_apply_pfactor()
{
	out << "void Note::apply_pfactor()\n"
	       "{\n";
	
	if ((prog.filter.enabled==true) || (prog.filter.enabled_const==false))
		out << "\tfilter_params.env_amount=orig.filter_params.env_amount*pfactor.filter_env /ONE;\n"
					 "\tfilter_params.freqfactor_offset=orig.filter_params.freqfactor_offset*pfactor.filter_offset /ONE;\n"
					 "\tfilter_params.resonance=orig.filter_params.resonance*pfactor.filter_res /ONE;\n"
					 "\t\n";
		
	for (int i=0;i<prog.n_osc;i++)
	{
		out << "\tosc"<<i<<".output=orig.osc"<<i<<".output*pfactor.out["<<i<<"] >>SCALE;\n"
					 "\tfor (int i=0;i<"<<prog.n_osc<<";i++)\n"
					 "\t\tosc"<<i<<".fm_strength[i]=orig.osc"<<i<<".fm_strength[i]*pfactor.fm["<<i<<"][i] >>SCALE;\n";
	}

	out << "}\n";
}

void write_still_active()
{
	out << "bool Note::still_active()\n"
	       "{\n";
	
	out << "\tif (   ";
	if (prog.env[0].enabled)
		out << " ((osc"<<0<<".output>0) && (env"<<0<<"->still_active()))";
	else
		out << " // envelope"<<0<<" is disabled";
		
	for (int i=1;i<prog.n_osc;i++)
	{
		if (prog.env[i].enabled)
			out <<  "\n\t     || ((osc"<<i<<".output>0) && (env"<<i<<"->still_active()))";
		else
			out <<  "\n\t        /* envelope"<<i<<" is disabled */";
	}
	
	out << "  )\n"
	       "\t\treturn true;\n"
	       "\telse\n"
	       "\t\treturn false;\n";
	
		
	out << "}\n";
}

void write_release()
{
	out << "void Note::release()\n"
	       "{\n";

	for (int i=0;i<prog.n_osc;i++)
	{
		if (prog.env[i].enabled)
			out << "\tenv"<<i<<"->release_key();\n";
		else
			comment << "\t//envelope"<<i<<" is disabled\n";
	}
	
	
	if (prog.filter.enabled && prog.filter.enabled_const)
		out << "\n\tfilter_envelope->release_key();\n";
	else if (prog.filter.enabled_const==false)
		out << "\n\tif (filter_params.enabled)\n"
		       "\t\tfilter_envelope->release_key();\n";
	
	out << "}\n";
}

void write_release_quickly()
{
	out << "void Note::release_quickly(jack_nframes_t maxt)\n"
	       "{\n";

	for (int i=0;i<prog.n_osc;i++)
	{
		if (prog.env[i].enabled)
			out << "\tif (env"<<i<<"->get_release() > maxt)\n"
			       "\t\tenv"<<i<<"->set_release(maxt);\n"
			       "\tenv"<<i<<"->release_key();\n"
			       "\t\n";
		else
			comment << "\t//envelope"<<i<<" is disabled\n"
			           "\t\n";
	}
	
	out << "}\n";
}

void write_reattack()
{
	out << "void Note::reattack()\n"
	       "{\n";

	for (int i=0;i<prog.n_osc;i++)
	{
		if (prog.env[i].enabled)
			out << "\tenv"<<i<<"->reattack();\n";
		else
			comment << "\t//envelope"<<i<<" is disabled\n";
	}
	
	
	if (prog.filter.enabled && prog.filter.enabled_const)
		out << "\n\tfilter_envelope->reattack();\n";
	else if (prog.filter.enabled_const==false)
		out << "\n\tif (filter_params.enabled)\n"
		       "\t\tfilter_envelope->reattack();\n";
	
	out << "}\n";
}

void write_do_ksr()
{
	out << "void Note::do_ksr()\n"
	       "{\n";
	       
	for (int i=0;i<prog.n_osc;i++)
	{
		if (prog.env[i].enabled)
			out << "\tenv"<<i<<"->set_ratefactor(1.0 / pow(freq>>SCALE, osc"<<i<<".ksr));\n";
		else
			comment << "\t//envelope"<<i<<" is disabled\n";
	}	       

	out << "}\n";
}

void write_do_ksl()
{
	bool need_ksl=false;
	
	out << "void Note::do_ksl()\n"
	       "{\n";
	       
	       
	for (int i=0;i<prog.n_osc;i++)
		if ( (prog.osc[i].ksl!=0) || (prog.osc[i].ksl_const==false) )	       
	  {
	  	need_ksl=true;
	  	break;
	  }	
	
	if (need_ksl)
	{
		out << "\tdouble tempfreq=double ( freq >> SCALE );\n"
		       "\t\n";
		
		for (int i=0;i<prog.n_osc;i++)
		{
			if ( (prog.osc[i].ksl==0) && (prog.osc[i].ksl_const) )
				comment << "\t//ksl is disabled for oscillator"<<i<<"\n";
			else
			{
				string kslstring = "(  (osc"+IntToStr(i)+".ksl==0) ? ONE : ( fixed_t(double(ONE) / pow(tempfreq, osc"+IntToStr(i)+".ksl)) )  )";
				
				if (prog.env[i].enabled)
					out << "\tenv"<<i<<"->set_max( "<<kslstring<<" );\n";
				else
					out << "\tkslval"<<i<<"="<<kslstring<<";\n";
			}
		}
	}	
	out << "}\n";
}

void write_get_sample()
{
	string tabtemp="";
	
	include_file("get_sample.1");
	
	{ //SYNC TODO
	if (prog.sync_factor_const==false)
	{
		out << "\tif (sync_factor)\n"
		       "\t{\n";
		       
		tabtemp="\t";
	}
	
	if ( (prog.sync_factor_const==false) || (prog.sync_factor!=0) )
	{
		string temp;
		if (prog.sync_factor_const)
			temp=IntToStr(prog.sync_factor);
		else
			temp="sync_factor";
			
		out << tabtemp << "\tsync_phase+=(actual_freq*"<<temp<<"/samp_rate) >> SCALE;\n"
		    << tabtemp << "\t\n"
		    << tabtemp << "\tif (sync_phase >= ONE)\n"
		    << tabtemp << "\t{\n"
		    << tabtemp << "\t\tsync_phase-=ONE;\n"
		    << tabtemp << "\t\t\n";
		
		for (int i=0;i<prog.n_osc;i++)
		{
			string initstring;
			if (prog.osc[i].have_custom_wave)
				initstring="init_custom_osc_phase(osc"+IntToStr(i)+".custom_wave->wave_len, osc"+IntToStr(i)+".custom_wave->samp_rate)";
			else
				initstring="ONE * PHASE_INIT";
			
			string full_command="osc"+IntToStr(i)+".phase="+initstring+";\n";
			
			if ( prog.osc[i].sync_const && prog.osc[i].sync )
				out << tabtemp << "\t\t" << full_command;
			else if ( prog.osc[i].sync_const && (prog.osc[i].sync==false) )
				comment << tabtemp << "\t\t//sync is disabled for osc"<<i<<"\n";
			else //if (prog.osc[i].sync_const==false)
				out << tabtemp << "\t\tif (osc"<<i<<".sync) " <<full_command;
		}
		    
		out << tabtemp << "\t}\n";
		    
	}
	else //if ( (prog.sync_factor_const==true) && (prog.sync_factor==0) )
		comment << "\t//sync is disabled\n";

	if (prog.sync_factor_const==false)
	{
		out << "\t}\n";
		       
		tabtemp="";
	}
	out << "\t\n\t\n";
	}	//SYNC TODO
	
	{ //OSCS TODO
	string outstring_scaled="", outstring_nonscaled="";
	
	for (int i=0;i<prog.n_osc;i++)
	{
		// increment phase
		string phase_inc_lfo = "(  (curr_lfo[osc"+IntToStr(i)+".vibrato_lfo][osc"+IntToStr(i)+".vibrato_depth]*actual_freq >>SCALE)*osc"+IntToStr(i)+".factor/samp_rate)>>SCALE";
		string phase_inc = "(actual_freq*osc"+IntToStr(i)+".factor/samp_rate)>>SCALE";
		if (prog.osc[i].vibrato_depth_const == false)
			out << "\tosc"<<i<<".phase+= ( (osc"<<i<<".vibrato_depth==0) ? ("<<phase_inc<<") : ("<<phase_inc_lfo<<") );\n";
		else if (prog.osc[i].vibrato_depth == 0)
			out << "\tosc"<<i<<".phase+= "<<phase_inc<<";\n";
		else
			out << "\tosc"<<i<<".phase+= "<<phase_inc_lfo<<";\n";

		// calculate phase modulation
		string fm=""; //TODO FINDMICH: das >>SCALE fehlt noch!
		for (int j=0;j<prog.n_osc;j++)
		{
			if (prog.osc[i].fm_strength_const[j] == false)
				fm+="+ (old_oscval["+IntToStr(j)+"] * osc"+IntToStr(i)+".fm_strength["+IntToStr(j)+"]) ";
			else if (prog.osc[i].fm_strength[j]!=0)
				fm+="+ (old_oscval["+IntToStr(j)+"] * "+IntToStr(prog.osc[i].fm_strength[j])+") ";
		}
		
		// generate string for modulated phase
		string phase;
		if (fm!="")
			phase="(  osc"+IntToStr(i)+".phase + ( "+fm+">>SCALE )  )";
		else
			phase="osc"+IntToStr(i)+".phase";
		
		// generate string for wave 
		string wavetemp;
		if (prog.osc[i].have_custom_wave)
		{
			string cw="osc"+IntToStr(i)+".custom_wave";
			wavetemp=cw+"->wave[ (     "+phase+" * "+cw+"->samp_rate   >>(2*SCALE)     ) % "+cw+"->wave_len ]";
		}
		else
		{
			string waveformtemp;
			if (prog.osc[i].waveform_const)
				waveformtemp=IntToStr(prog.osc[i].waveform);
			else
				waveformtemp="osc"+IntToStr(i)+".waveform";
				
			wavetemp="wave["+waveformtemp+"][ ( "+phase+" * WAVE_RES   >>SCALE ) % WAVE_RES ]";
		}

		// finally write "oscval[n]=..."
		out << "\toscval["<<i<<"] = "<<wavetemp;
		// figure out whether we need to multiply with the env, with ksl or not at all
		if (prog.env[i].enabled)
		  out<<" * env"<<i<<"->get_level()  >>SCALE;\n";
		else if ( (prog.osc[i].ksl!=0) || (prog.osc[i].ksl_const==false) )
		// i.e.: if osc[i] has a kslval variable
			out<<" * kslval"<<i<<"  >>SCALE;\n";
		else //no envelope, no kslval
			out<<";\n";
			

		// maybe do tremolo
		string tremlfo;
		if (prog.osc[i].tremolo_lfo_const==false)
			tremlfo="osc"+IntToStr(i)+".tremolo_lfo";
		else
			tremlfo=IntToStr(prog.osc[i].tremolo_lfo);
		
		if (prog.osc[i].tremolo_depth_const==false)
			out << "\tif (osc"<<i<<".tremolo_depth)\n"
			       "\t\toscval["<<i<<"] = oscval["<<i<<"] * curr_lfo["<<tremlfo<<"][osc"<<i<<".tremolo_depth]  >>SCALE;\n";
		else if (prog.osc[i].tremolo_depth!=0)
			out << "\toscval["<<i<<"] = oscval["<<i<<"] * curr_lfo["<<tremlfo<<"]["<<prog.osc[i].tremolo_depth<<"]  >>SCALE;\n";
		else
			comment << "\t//oscillator"<<i<<" has no tremolo\n";
			
		// maybe add this osc to the output
		if (prog.osc[i].output_const==false)
			outstring_scaled+="+ osc"+IntToStr(i)+".output*oscval["+IntToStr(i)+"] ";
		else if (prog.osc[i].output==ONE)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"] ";
		else if (prog.osc[i].output==ONE/2)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]/2 ";
		else if (prog.osc[i].output==ONE/3)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]/3 ";
		else if (prog.osc[i].output==ONE/4)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]/4 ";
		else if (prog.osc[i].output==ONE/5)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]/5 ";
		else if (prog.osc[i].output==ONE/6)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]/6 ";
		else if (prog.osc[i].output==ONE/7)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]/7 ";
		else if (prog.osc[i].output==ONE/8)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]/8 ";
		else if (prog.osc[i].output==ONE/9)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]/9 ";
		else if (prog.osc[i].output==ONE/10)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]/10 ";
		else if (prog.osc[i].output==ONE*2)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]*2 ";
		else if (prog.osc[i].output==ONE*3)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]*3 ";
		else if (prog.osc[i].output==ONE*4)
			outstring_nonscaled+="+ oscval["+IntToStr(i)+"]*4 ";
		else if (prog.osc[i].output!=0)
			outstring_scaled+="+ "+IntToStr(prog.osc[i].output)+"*oscval["+IntToStr(i)+"] ";
		//else: output is 0, ignore it
		
		out << "\t\n";
	}
	
	// generate, check and write the final outstring
	string outstring="";
	if (outstring_scaled!="")
		outstring+="( "+outstring_scaled+"   >>SCALE )";
	if (outstring_nonscaled!="")
		outstring+=" "+outstring_nonscaled;
		
	if (outstring=="")
		throw string ("this instrument has no output at all!");
	
	out << "\tfixed_t out = "<<outstring<<";\n"
	       "\t\n"
	       "\t\n";
	} //OSCS TODO
	
	{ //FILTER TODO
	tabtemp="";
	
	if (prog.filter.enabled_const==false)
	{
		out << "\tif (filter_params.enabled)\n"
		       "\t{\n";
		tabtemp="\t";
	}
	
	if ((prog.filter.enabled_const==false) || (prog.filter.enabled==true))
		out << tabtemp << "\tfilter_update_counter++;\n"
		    << tabtemp << "\tif (filter_update_counter>=filter_update_frames)\n"
		    << tabtemp << "\t{\n"
		    << tabtemp << "\t\tfilter_update_counter=0;\n"
		    << tabtemp << "\t\t\n"
		    << tabtemp << "\t\tfloat cutoff= float(actual_freq)/ONE * \n"
		    << tabtemp << "\t\t\tfloat(curr_lfo[filter_params.trem_lfo][filter_params.trem_strength])/ONE *\n"
		    << tabtemp << "\t\t\t( filter_params.freqfactor_offset  +  filter_envelope->get_level() * filter_params.env_amount / float(ONE) );\n"
		    << tabtemp << "\t\tfilter.set_params( cutoff, filter_params.resonance  );\n"
		    << tabtemp << "\t}\n"
		    << tabtemp << "\t\n"
		    << tabtemp << "\tfilter.process_sample(&out);\n";
	
	if (prog.filter.enabled_const==false)
	{
		out << "\t}\n";
		tabtemp="";
	}
	out << "\t\n";
	}
	
	out << "\treturn out;\n";
	out << "}\n";
}

void write_set_param()
{
	out << "void Note::set_param(const parameter_t &p, fixed_t v)\n"
	       "{\n"
	       "\toscillator_t* sel_osc=NULL;\n"
	       "\toscillator_t* sel_orig_osc=NULL;\n"
	       "\tEnvelope* sel_env=NULL;\n"
				 "\t\n"
	       "\tswitch (p.osc)\n"
	       "\t{\n";
	
	for (int i=0;i<prog.n_osc;i++)
	{
		out << "\t\tcase "<<i<<": sel_osc=&osc"<<i<<"; sel_orig_osc=&orig.osc"<<i<<"; ";
		
		if (prog.env[i].enabled)
			out << "sel_env=env"<<i<<"; ";
		else
			comment << "/* envelope"<<i<<" is disabled */ ";
			
		out << "break;\n";
	}

	out << "\t\t\n"
	       "\t\tdefault: output_note(\"NOTE: trying to change the nonexistent oscillator\"+IntToStr(p.osc));\n"
	       "\t}\n"
	       "\t\n";
	
	include_file("set_param.1");
	
	
	if ((prog.filter.enabled==true) || (prog.filter.enabled_const==false))
	{
		include_file("set_param.filter");
		if (prog.filter.env_settings.enabled)
			include_file("set_param.filterenv");
		else
			include_file("set_param.nofilterenv");
	}
	else
		include_file("set_param.nofilter");
	
	if ((prog.sync_factor!=0) || (prog.sync_factor_const==false))
		out << "\t\t\n"
		       "\t\tcase SYNC_FACTOR: sync_factor=v; break;\n";
	else
		out << "\t\t\n"
		       "\t\tcase SYNC_FACTOR: output_note(\"NOTE: trying to set sync_factor, but it's disabled\"); break;\n";
	
	out << "\t\t\n"
				 "\t\tdefault: throw string(\"trying to set an unknown parameter\");\n"
	       "\t}\n"
	       "}\n";
}

void write_create_note()
{
	out << "extern \"C\" NoteSkel* create_note(int n, float v,program_t &prg, jack_nframes_t pf, fixed_t pb, int prg_no)\n"
	       "{\n"
	       "\treturn new Note(n,v,prg,pf,pb,prg_no);\n"
	       "}\n";
}

void write_destroy()
{
	out << "void Note::destroy()\n"
	       "{\n"
	       "\tdelete this;\n"
	       "}\n";
}

void generate_source()
{
	//#includes and definition of class Note
	include_file("head.1");
	write_env_decs();
	write_empty_line();
	write_oscval_decs();
	write_empty_line();
	write_osc_decs();
	write_empty_line();
	write_sync_decs();
	write_empty_line();
	write_filter_decs();
	write_empty_line();
	write_pfactor_decs();
	include_file("head.2");
	
	
	//implementation of Note's functions
	write_ctor();
	write_dtor();
	write_destroy();
	write_empty_line();
	
	write_recalc_factors();
	write_apply_pfactor();
	write_empty_line();
	
	write_still_active();
	write_release();
	write_release_quickly();
	write_reattack();
	write_empty_line();
	
	write_do_ksr();
	write_do_ksl();
	write_empty_line();
	
	write_get_sample();
	write_empty_line();
	
	write_set_param();

	write_empty_line();
	write_empty_line();
	write_empty_line();
	
	//implementation of create_note and init_vars
	include_file("interface.1");
}

int main(int argc, char** argv)
{
	try
	{
		cerr << "parsing '"<<argv[1]<<"'..." << endl;
		prog=parse(argv[1]);

		generate_source();
	}
	catch(string err)
	{
		cerr << "FATAL: "<<err<<endl;
	}

	return 0;
}
