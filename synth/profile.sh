#!/bin/bash

if [ x$1 = x ]; then
	cnt=5
else
	cnt=$1
fi

for ((i=1;i<=$cnt;i++)); do
	/usr/bin/time -p ./synth -p 0:../../manyosc2.prog -i 60 -x 1000:1 -a -m 2>&1 | grep 'user ' | sed 's/user //';
	#echo "$i / $cnt done" 1>&2
done | python avg_stddev.py
