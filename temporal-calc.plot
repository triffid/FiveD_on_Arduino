set terminal png
set output "temporal.png"
set xrange [*:*]
plot	"temporal_data" using ($1):($2):($3 * 1) title "X" with yerrorlines,\
	"temporal_data" using ($1):($4):($5 * 1) title "Y" with yerrorlines,\
	"temporal_data" using ($1):($6):($7 * 1) title "Z" with yerrorlines,\
	"temporal_data" using ($1):($8):($9 * 1) title "E" with yerrorlines
