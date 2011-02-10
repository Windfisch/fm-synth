#define HELPSTRING \
"Usage: ./synth [OPTIONS]\n"\
"    -h                         show this help text\n"\
"    -V                         show the version number\n"\
"\n"\
"    -v                         be verbose\n"\
"    -q                         be quiet\n"\
"    -F   --fatal-warnings      make warnings fatal\n"\
"\n"\
"    -c   --conf(ig) FILE       load the given config file\n"\
"    -d   --dir(ectory) DIR     read that directory\n"\
"    -p   --program N:FILE      load FILE at program number N\n"\
"\n"\
"    -f   --frameskip N         only do every Nth frame\n"\
"    -x   --xruns N:T           kill all voices when N xruns or more \n"\
"                               happen in T seconds\n"\
"\n"\
"    --filter-update-freq FREQ  how often the filter settings, the lfo-\n"\
"    --lfo-update-freq FREQ     or envelope-current-values get updated\n"\
"    --env(elope)-update-freq FREQ  (low -> less accurate, but faster)\n"\
"    -i   --cleanup-interval N  try cleaning up notes every N seconds\n"\
"\n"\
"    --lfoN-freq   --snh-freq   set frequency for lfos or the sample-and-\n"\
"        --sample-and-hold-freq   hold-generator\n"\
"    --max-port TIME            set the maximum settable portamento time\n"\
"       --max-port(amento)-time   (a MIDI value of 127 corresponds to this)\n"\
"\n"\
"    -a   --{no|dont}-connect-  don't automatically connect the output\n"\
"                  audio(-out)    ports to the speakers\n"\
"    -m   --{no|dont}-connect-  don't automatically connect the output\n"\
"                    midi(-in)    ports to midi devices\n"\
"\n"\
"    -w   --{no|dont}-watch-    turn off watching files for changes\n"\
"                        files \n"\
""
