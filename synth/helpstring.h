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
"    -i   --cleanup-interval N  try cleaning up notes every N seconds\n"\
"    --filter-update-freq FREQ  how often the filter settings, the lfo-\n"\
"    --lfo-update-freq FREQ     or envelope-current-values get updated\n"\
"    --env(elope)-update-freq FREQ  (low -> less accurate, but faster)\n"\
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
