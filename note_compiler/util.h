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


#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>
#include "programs.h"

using namespace std;

string IntToStr(int i);
string IntToStrHex(int i);

bool isnum(string s);
bool isfloat(string s);

string remove_all_spaces(string s);
string trim_spaces(string s);

void output_warning(string s);
void output_note(string s);
void output_verbose(string s);

bool param_needs_index(parameter_enum p);
parameter_enum param_to_enum(string param);

string extract_var(string s);
string extract_val(string s);

string fileext(string f);


#endif
