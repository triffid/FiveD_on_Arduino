#!/bin/sh
# uses gnuplot to plot the output from the sim program
# read the gnuplot manual to adapt this file to your needs

if [ $# -ne 1 ]; then
	echo "Usage: plot.sh <logfile>" 
	exit
fi
base=${1%.*}


/usr/bin/gnuplot <<EOF
# gnuplot script starts here

# plot position in the z-plane
set terminal png enhanced size 600,600
set output "${base}-xy.png"
set grid
set xlabel "x [mm]"
set ylabel "y [mm]"
set size square
plot "$1" using 2:3 with lines notitle

# plot dx/dt, dy/dt and absolute speed
set terminal png enhanced size 600,400
set output "${base}-speed.png"
set xlabel "t [s]"
set ylabel "v [mm/min]"
set size nosquare
plot \
	"$1" using 1:5 title "dx/dt" with lines, \
	"$1" using 1:6 title "dx/dt" with lines, \
	"$1" using 1:8 title "3-axis speed" with lines
EOF
