#ifndef __LOAD_H__
#define __LOAD_H__

#include <string>

using namespace std;

void add_dir(string directory, bool complain=true);
void read_config(const char *cfg, bool complain=true);

#endif
