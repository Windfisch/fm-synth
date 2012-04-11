#!/usr/bin/python

import sys
import math

vals=[]
sum=0
cnt=0

for line in sys.stdin:
	num=float(line)
	vals.append(num)
	sum=sum+num
	cnt=cnt+1

avg=sum/cnt

stddev=0
for val in vals:
	stddev=stddev+ (val-avg)**2

stddev=stddev/cnt
stddev=math.sqrt(stddev)


print "avg is    " , avg
print "stddev is " , stddev
