set terminal png size 1024,768
set output "temporal.png"
# set terminal x11 persist
set xrange [*:*]
set xlabel "seconds"
set yrange [0:13]
set ylabel "millimeters from target"
set y2range [0:1300]
set y2label "* 100 micrometers from target"
plot	"temporal_data" using ($1):($2):($3 >= 0?$2:$2 + $3 * 50):($3 >= 0?$2 - $3 * 50:$2) title "X" with yerrorbars pt 2 lw 0 lc rgb "red",\
			"temporal_data" using ($1):($4):($5 >= 0?$4:$4 + $5 * 50):($5 >= 0?$4 - $5 * 50:$4) title "Y" with yerrorbars pt 2 lw 0 lc rgb "green",\
			"temporal_data" using ($1):($6 * 1000):($7 >= 0?$6 * 1000:$6 * 1000 + $7 * 50):($7 >= 0?$6 * 1000 - $7 * 50:$6 * 1000) title "Z" with yerrorbars pt 2 lw 0 lc rgb "cyan" axes x1y2,\
			"temporal_data" using ($1):($8):($9 >= 0?$8:$8 + $9 * 50):($9 >= 0?$8 - $9 * 50:$8) title "E" with yerrorbars pt 2 lw 0 lc rgb "violet"
