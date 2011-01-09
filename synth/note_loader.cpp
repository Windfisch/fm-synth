
#include <iostream>
#include <dlfcn.h>

#include "note_loader.h"
#include "programs.h"
#include "note_funcs.h"
#include "globals.h"

using namespace std;

void load_note_from_so(string file, program_t &prog)
{
	void *handle;
	
	handle = dlopen(file.c_str(), RTLD_LAZY);
	
	if (handle==NULL)
		throw string("could not open shared object (")+string(dlerror())+string(")");
	
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
		dlclose(handle);
		throw err;
	}
}

void maybe_unload_note(program_t &prog)
{
	if (prog.dl_handle)
	{
		dlclose(prog.dl_handle);
		prog.dl_handle=NULL;
		prog.create_func=NULL;
	}
}
