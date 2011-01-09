#ifndef __NOTE_LOADER_H__
#define __NOTE_LOADER_H__

#include <string>
#include <jack/jack.h>

#include "programs.h"

using namespace std;

void load_note_from_so(string file, program_t &prog); //throws string
void maybe_unload_note(program_t &prog);
#endif
