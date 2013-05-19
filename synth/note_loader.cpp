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



#include <iostream>
#include <dlfcn.h>

#include "note_loader.h"
#include "programs.h"
#include "note_funcs.h"
#include "globals.h"
#include "shared_object_manager.h"

using namespace std;

void load_note_from_so(string file, program_t &prog)
{
	void *handle;
	
	handle = my_dlopen(file);
	
	try
	{
		prog.create_func=(create_func_t*) dlsym(handle,"create_note");
		if (prog.create_func==NULL)
			throw string("could not find symbol create_note in shared object (")+dlerror()+")";	

		init_func_t* initfunc=(init_func_t*) dlsym(handle,"init_vars");
		if (initfunc==NULL)
			throw string("could not find symbol init_vars in shared object (")+dlerror()+")";	
		
		initfunc(samp_rate, filter_update_frames, wave, curr_lfo, &output_note, &IntToStr);
		

		
		prog.dl_handle=handle;
	}
	catch (string err)
	{
		prog.create_func=NULL;
		prog.dl_handle=NULL;
		dlref_dec(handle);
		throw err;
	}
}

void maybe_unload_note(program_t &prog)
{
	if (prog.dl_handle)
	{
		dlref_dec(prog.dl_handle);
		prog.dl_handle=NULL;
		prog.create_func=NULL;
	}
}
