#ifndef __NOTE_FUNCS_H
#define __NOTE_FUNCS_H

#include <jack/jack.h>

#include "fixed.h"
#include <string>

using namespace std;

class NoteSkel;
struct program_t;

typedef void output_note_func_t(string s);
typedef string IntToStr_func_t(int i);

typedef NoteSkel* create_func_t (int, float, program_t&, jack_nframes_t, fixed_t, int);
typedef void init_func_t(int sr, int fupfr, fixed_t **w, fixed_t **clfo, output_note_func_t* out_n, IntToStr_func_t* its);

#endif
