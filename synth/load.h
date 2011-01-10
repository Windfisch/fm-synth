#ifndef __LOAD_H__
#define __LOAD_H__

#include <string>
#include "programs.h"

using namespace std;

void add_dir(string directory, bool complain=true);
void read_config(const char *cfg, bool complain=true);
bool load_program(string file, program_t& prog);

#endif
