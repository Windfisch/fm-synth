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


#ifndef __IN_SYNTH_CLI_H__
#define __IN_SYNTH_CLI_H__

#include <string>

using namespace std;

void do_in_synth_cli();


//only use this, if you don't want the file-watches to be updated
//i.e., only when reloading a program!
void lock_and_load_program_no_watch_updates(int prg_no, string file);
#endif
