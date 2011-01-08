#ifndef __CHANNEL_H__
#define __CHANNEL_H__


#include <stdint.h>
#include <list>

#include "fixed.h"
#include "programs.h"
#include "note.h"
#include "defines.h"
#include "util.h"


class Channel
{
	public:
		Channel();
		~Channel();
		fixed_t get_sample();
		void event(uint8_t a, uint8_t b, uint8_t c);
		void set_controller(int con,int val);
		void set_program(int prog);
		void set_pitch_bend(float val);
		void note_on(int note, int vel);
		void note_off(int note);
		void really_do_note_off(int note);
		void cleanup();
		void release_all();
		void panic();
		void set_real_portamento_frames();
		void set_portamento_time(int val);
		void set_portamento(int val);
		void set_volume(int val);
		void set_balance(int val);
		void set_n_voices(int val);
		void set_quick_release(int val);
		void reset_controllers();
		
		void set_hold_pedal(bool newstate);
		void set_sostenuto_pedal(bool newstate);
		void set_legato_pedal(bool newstate);
		
		float balL, balR;
	private:
		void recalc_param(const parameter_t &par, program_t &prg);
		void set_user_controller(int con, int val);
		void apply_voice_limit();
		
    fixed_t volume;
    fixed_t portamento_frames, portamento_frames2;
		int program;
		program_t curr_prg;
		
		fixed_t pitchbend;
		float max_pitchbend;
		
		std::list<Note*> notes;
		
		bool do_portamento;
		
		int n_voices;
		jack_nframes_t quick_release;
		
		
		set<int> pressed_keys;
		
		bool hold_pedal_pressed;
		set<int> held_keys;
		
		set<int> sostenuto_keys;
		
		bool legato_pedal_pressed;
};

#endif
