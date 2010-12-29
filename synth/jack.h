#ifndef __JACK_H__FLO
#define __JACK_H__FLO

#include <jack/jack.h>

int process_callback(jack_nframes_t nframes, void *notused);
int xrun_callback(void *notused);
void init_jack();
void start_jack(bool connect_audio_out=true, bool connect_midi_in=true);
void exit_jack();

#endif
