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


#include <dlfcn.h>
#include <map>
#include <string>

#include "util.h"
#include "shared_object_manager.h"

using namespace std;

map<void*, int> dl_ref_count;

void* my_dlopen(string file)
{
	void* handle;
	
	if (file.find('/')==string::npos)
		file="./"+file;
	
	handle=dlopen(file.c_str(),RTLD_NOW);

	if (handle==NULL)
		throw string("could not open shared object (")+string(dlerror())+string(")");
	
	if (dl_ref_count[handle]!=0) //the library is already opened
	{
		output_verbose("the requested shared object '"+file+"' is already opened, reusing the handle #"+IntToStr(int(handle)));
		dlclose(handle); //we don't need it opened twice
	}
	else
	{
		output_verbose("the requested shared object '"+file+"' has been loaded with handle #"+IntToStr(int(handle)));
	}
	
	++dl_ref_count[handle];
	
	return handle;
}

void dlref_inc(void* handle)
{
	if (handle==NULL)
		throw string("dlref_inc: tried to increment the ref-count for NULL");
	
	if (dl_ref_count[handle]==0)
		throw string("dlref_inc: tried to increment the ref-count for a nonexistent handle");
		
	++dl_ref_count[handle];
}

void dlref_dec(void* handle)
{
	if (handle==NULL)
		throw string("dlref_inc: tried to increment the ref-count for NULL");
	
	if (dl_ref_count[handle]==0)
		throw string("dlref_inc: tried to decrement the ref-count for a nonexistent handle");
	
	--dl_ref_count[handle];
	
	if (dl_ref_count[handle]==0)
	{
		output_verbose("noone uses dl-handle "+IntToStr(int(handle))+", unloading the shared object...");
		dlclose(handle);
	}
}
