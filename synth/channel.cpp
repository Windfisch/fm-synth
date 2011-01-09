#include "channel.h"

#include "math.h"
#include "globals.h"

#include "note.h"

#include "note_funcs.h"


Channel::Channel()
{
	volume=ONE;
	set_program(0);
	curr_prg.controller[NO_CONT]=1;
	quick_release=0;
	portamento_frames2=portamento_frames=0;
	do_portamento=false;
	pitchbend=ONE;
	n_voices=0;
	
	max_pitchbend=1.0;
	
	set_balance(64);
	
	pressed_keys.clear();
	held_keys.clear();
	sostenuto_keys.clear();
	hold_pedal_pressed=false;
	legato_pedal_pressed=false;
}

Channel::~Channel()
{
	panic(); //deletes all notes and empties notes-list
}

void Channel::cleanup()
{
	list<NoteSkel*>::iterator it;
	for (it=notes.begin(); it!=notes.end(); it++)
		if ((*it)->still_active()==false)
		{
			(*it)->destroy();
			it=notes.erase(it);
		}
}

fixed_t Channel::get_sample()
{
	fixed_t sum=0;
	
	for (list<NoteSkel*>::iterator it=notes.begin(); it!=notes.end(); it++)
		sum+=(*it)->get_sample();

	return sum*volume >>SCALE;
}

void Channel::event(uint8_t a, uint8_t b, uint8_t c)
{
	switch(a & 0xF0)
	{
		case 0x80: note_off(b); break;
		case 0x90: note_on(b,c); break;
		case 0xA0: break; //IMPLEMENTME: polyphonic aftertouch (note, dynamic)
		case 0xB0: set_controller(b,c); break;
		case 0xC0: set_program(b); break;
		case 0xD0: break; //IMPLEMENTME: monotonic aftertouch (dynamic)
		case 0xE0: set_pitch_bend( ( (((b&0x7F) + ((c&0x7F)<<7)) - 8192) / 8192.0 ) * max_pitchbend ); break;
		case 0xF0: break; //own controls/sysex (to be implemented) IMPLEMENTME
		default: output_verbose("NOTE: got unknown command "+ IntToStrHex(a&0xF0) +", ignoring it\n");
		;
	}
}

void Channel::note_off(int note)
{
	pressed_keys.erase(note);
	
	if (hold_pedal_pressed)
		held_keys.insert(note);
	else if (sostenuto_keys.find(note)!=sostenuto_keys.end())
		/* do nothing */;
	else
		really_do_note_off(note);
}

void Channel::really_do_note_off(int note)
{
	for (list<NoteSkel*>::iterator it=notes.begin(); it!=notes.end(); it++)
		if ((*it)->get_note()==note)
			(*it)->release();	
}

void Channel::note_on(int note, int vel)
{
	list<NoteSkel*>::iterator it;
	if (vel>0) //note on
	{
		pressed_keys.insert(note);
		
		if ( (n_voices==1) && (!notes.empty()) ) //we're in monomode
		{
			//no need to create a new note; reuse the existing
			NoteSkel *n; //i'm lazy
			n= *(notes.begin());
			
			if (n->get_program() != program)
			{
				//if the program has changed, kill the previous note and
				//create a new one
				n->destroy();
				notes.clear();
				
				NoteSkel *newnote=NULL;
				if (curr_prg.create_func==NULL)
					newnote = new Note(note,(float)vel/128.0,
														 curr_prg,
														 portamento_frames, pitchbend, 
														 program);
				else
					newnote = curr_prg.create_func(note,(float)vel/128.0,
																				 curr_prg,
																				 portamento_frames, pitchbend, 
																				 program);

				notes.push_back(newnote);
			}
			else //program did not change
			{
				//if not still active, don't do portamento
				n->set_note(note,n->still_active());
				n->set_vel((float)vel/128.0);
				if ((legato_pedal_pressed==false) || !n->still_active()) n->reattack();
				//no need to push back. would become #1 instead of #1
			}
		}
		else //we're in polymode
		{
			bool neednewnote=true;
			//if (always_reattack) always_reattack is always true when in polymode
				for (it=notes.begin(); it!=notes.end(); it++)
					if ( ((*it)->get_note()==note) && ((*it)->get_program()==program) )
					{
						neednewnote=false;
						(*it)->reattack();
						(*it)->set_vel((float)vel/128.0);
						notes.push_back(*it); //reorder notes
						notes.erase(it);
						break;
					}

			if (neednewnote)
			{
				NoteSkel *newnote=NULL;
				if (curr_prg.create_func==NULL)
					newnote = new Note(note,(float)vel/128.0,
														 curr_prg,
														 portamento_frames, pitchbend, 
														 program);
				else
					newnote = curr_prg.create_func(note,(float)vel/128.0,
																				 curr_prg,
																				 portamento_frames, pitchbend, 
																				 program);

				notes.push_back(newnote);
			}

			apply_voice_limit();
		}
	}                                                  
	else //note off
	{
		note_off(note);
	}
		
}

void Channel::set_n_voices(int val)
{
	n_voices=val;
	
	if ((n_voices<=0) || (n_voices>=128))
		n_voices=0; //unlimited
	
	apply_voice_limit();
}

void Channel::apply_voice_limit()
{
	if (n_voices) //is a limit defined?
	{
		int diff=notes.size()-n_voices;
		if (diff>0)
		{
			list<NoteSkel*>::iterator it=notes.begin();

			if (quick_release)
				for (int i=0;i<diff;i++)
				{
					(*it)->release_quickly(quick_release);
					it++;
				}
			else
				for (int i=0;i<diff;i++)
				{
					(*it)->destroy();
					it=notes.erase(it);
				}
		}
	}
}


void Channel::set_controller(int con,int val)
{
	switch (con)
	{
		case 5:   set_portamento_time(val); break;
		case 7:   set_volume(val); break;
		case 8:   set_balance(val); break;
		case 65:  set_portamento(val); break;
		case 64:  set_hold_pedal(val>=64); break;
		case 66:  set_sostenuto_pedal(val>=64); break;
		case 68:  set_legato_pedal(val>=64); break;
		case 119: set_quick_release(val);
		case 120: panic(); break;
		case 121: reset_controllers(); break;
		case 123: release_all(); break;
		case 126: set_n_voices(val); break;
		case 127: set_n_voices(999); break;
		default:  set_user_controller(con,val); break;
	}
}

void Channel::set_user_controller(int con, int val)
{
	curr_prg.controller[con]=val;
	for (set<parameter_t>::iterator it=curr_prg.controller_affects[con].begin(); it!=curr_prg.controller_affects[con].end(); it++)
		recalc_param(*it,curr_prg);
}

void Channel::recalc_param(const parameter_t &par, program_t &prg)
{
	fixed_t val=0;
	
	list<term_t> *l;
	l=&(prg.formula[par]);
	
	for (list<term_t>::iterator it=l->begin(); it!=l->end(); it++)
		val+=curr_prg.controller[it->c]*it->f;
	
	if (val<0) val=0;

	// now we have the final value of the formula in units of fixed_t
	// in the range 0..+infinity
	
	switch(par.par)
	{
		case SUSTAIN: 
		case FILTER_SUSTAIN: if (val>ONE) val=ONE;  break;
				
		case TREM_LFO: 
		case VIB_LFO:  
		case FILTER_TREM_LFO: val=val>>SCALE; if (val>=N_LFOS+1) val=N_LFOS+1 -1; break;
		
		case TREMOLO:
		case VIBRATO:  
		case FILTER_TREMOLO: val=val>>SCALE; if (val>=N_LFO_LEVELS) val=N_LFO_LEVELS-1; break;
		
		case WAVEFORM:  val=val>>SCALE; if (val>=N_WAVEFORMS) val=N_WAVEFORMS-1; break;
					
		case FILTER_RESONANCE: if (val>ONE) val=ONE; break;

		default: break;
	}

	// now we have the value clipped to the valid range. for stuff
	// expecting real numbers, it's in units of fixed_t. for booleans
	// it's zero or nonzero. for stuff expecting integers, like lfo,
	// waveform etc it's in int (i.e., val/ONE is very small, while
	// val is what we want)

	for (list<NoteSkel*>::iterator it=notes.begin(); it!=notes.end(); it++)
		(*it)->set_param(par, val);
	
	curr_prg.set_param(par, val);
}

void Channel::reset_controllers()
{
	program_t *orig=&program_settings[program];
	
	for (int i=0;i<128;i++)
		set_user_controller(i,orig->controller[i]);
}

void Channel::set_quick_release(int val)
//ranges from zero to one second.
{
	quick_release=samp_rate*val/128;
}

void Channel::set_volume(int val)
{
	volume=val*ONE/128;
}

void Channel::set_balance(int val)
{
#ifdef STEREO
	balR=val/64.0;
	balL=(128-val)/64.0;
#endif
}

void Channel::set_portamento(int val)
{
	if (val>=64)
	{
		do_portamento=true;
		set_real_portamento_frames();
	}
	else
	{
		do_portamento=false;
		set_real_portamento_frames();
	}
}

void Channel::set_portamento_time(int val)
{
	portamento_frames2=samp_rate*val*max_port_time_sec/128;
	if (do_portamento)
		set_real_portamento_frames();
}

void Channel::set_real_portamento_frames()
{
	if (do_portamento)
		portamento_frames=portamento_frames2;
	else
		portamento_frames=0;
		
	list<NoteSkel*>::iterator it;
	for (it=notes.begin(); it!=notes.end(); it++)
		(*it)->set_portamento_frames(portamento_frames);
}

void Channel::set_hold_pedal(bool newstate)
{
	if (hold_pedal_pressed!=newstate)
	{
		hold_pedal_pressed=newstate;
		
		if (newstate==false)
		{
			//check for all held keys: is the key not pressed any more?
			//                         is the key not in sostenuto_keys?
			//if both conditions are fulfilled, release that note
			for (set<int>::iterator it=held_keys.begin(); it!=held_keys.end(); it++)
				if ( (pressed_keys.find(*it)==pressed_keys.end()) &&
					   (sostenuto_keys.find(*it)==sostenuto_keys.end()) )
						note_off(*it);
			
			held_keys.clear();
		}
	}
}

void Channel::set_sostenuto_pedal(bool newstate)
{
	// !sostenuto_keys.empty() equals pedal_pressed
	if ( newstate != !sostenuto_keys.empty() )
	{
		if (newstate)
		{
			sostenuto_keys=pressed_keys;
		}
		else
		{
			if (hold_pedal_pressed==false)
				for (set<int>::iterator it=sostenuto_keys.begin(); it!=sostenuto_keys.end(); it++)
					if (pressed_keys.find(*it)==pressed_keys.end())
						really_do_note_off(*it);
			
			sostenuto_keys.clear();
		}
	}
}

void Channel::set_legato_pedal(bool newstate)
{
	legato_pedal_pressed=newstate;
}
	
void Channel::panic()
{
	list<NoteSkel*>::iterator it;
	for (it=notes.begin(); it!=notes.end();)
	{
		(*it)->destroy();
		it=notes.erase(it);
	}
}

void Channel::release_all()
{
	list<NoteSkel*>::iterator it;
	for (it=notes.begin(); it!=notes.end(); it++)
		(*it)->release();
}

void Channel::set_program(int prog)
{
	program=prog;
	curr_prg=program_settings[program];
}

void Channel::set_pitch_bend(float val)
{
	pitchbend=pow(2.0,val/12.0)*ONE;
	
	list<NoteSkel*>::iterator it;
	for (it=notes.begin(); it!=notes.end(); it++)
		(*it)->set_pitchbend(pitchbend);
}
