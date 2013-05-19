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

typedef NoteSkel* create_func_t (int, float, program_t&, jack_nframes_t, fixed_t, int, float);
typedef void init_func_t(int sr, int fupfr, fixed_t **w, fixed_t **clfo, output_note_func_t* out_n, IntToStr_func_t* its);

#endif
