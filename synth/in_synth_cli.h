#ifndef __IN_SYNTH_CLI_H__
#define __IN_SYNTH_CLI_H__

#include <string>

using namespace std;

void do_in_synth_cli();


//only use this, if you don't want the file-watches to be updated
//i.e., only when reloading a program!
void lock_and_load_program_no_watch_updates(int prg_no, string file);
#endif
