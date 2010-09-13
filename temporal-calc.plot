set terminal png
set output "temporal.png"
plot	"temporal_data" using ($1):($2):($3 * 1) title "X" with yerrorlines,\
	"temporal_data" using ($1):($4):($5 * 1) title "Y" with yerrorlines