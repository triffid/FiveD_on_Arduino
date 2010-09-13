#!/usr/bin/perl

my $f								= 800; # mm/min

my $x								= 10;	# mm
my $y								= 15;	# mm

my $x_steps_per_mm	= 4;
my $y_steps_per_mm	= 4;
my $f_cpu						= 16000;

my $distance				= sqrt(($x * $x) + ($y * $y)); # mm

my $time						= $distance / $f * 60;	# seconds

my $x_steps					= int($x * $x_steps_per_mm);
my $y_steps					= int($y * $y_steps_per_mm);

printf "X:%gmm (%i steps)\tY:%gmm (%i steps)\tD:%5.2fmm\tF:%imm/min (%imm/sec)\ttime: %i (%g seconds)\n", $x, $x_steps, $y, $y_steps, $distance, $f, $f / 60, int($time * $f_cpu), $time;

$time = int $time * $f_cpu;

my $ct = 0;
my $xd = ($x_steps > 0)?int($time / $x_steps):$time;
my $yd = ($y_steps > 0)?int($time / $y_steps):$time;
my $xc = 0;
my $yc = 0;

# my $td = $time;
# if ((($xd - $xc) < $td) && $x_steps) {
	$td = $xd - $xc;
# }
if ((($yd - $yc) < $td) && $y_steps) {
	$td = $yd - $yc;
}

my $last_x_step_time = 0;
my $last_y_step_time = 0;

while (($x_steps + $y_steps) > 0) {
	my $st = "";

	if ($xc >= $xd) {
		printf "X Speed: %5.2fmm/sec\n", 1 / (($ct - $last_x_step_time) / $f_cpu) / $x_steps_per_mm;
		$last_x_step_time = $ct;
		$xc -= $xd;
		$x_steps--;
		$st .= "X ";
	}
	if ($yc >= $yd) {
		printf "Y Speed: %5.2fmm/sec\n", 1 / (($ct - $last_y_step_time) / $f_cpu) / $y_steps_per_mm;
		$last_y_step_time = $ct;
		$yc -= $yd;
		$y_steps--;
		$st .= "Y ";
	}

	printf "X:%6i (%2.2fmm)\tY:%6i (%2.2fmm)\txcd:%4i/%4i\tycd:%4i/%4i %s\n", $x_steps, $x_steps / $x_steps_per_mm, $y_steps, $y_steps / $y_steps_per_mm, $xc, $xd, $yc, $yd, $st;

	my $x_actual_pos = $x_steps / $x_steps_per_mm;
	my $x_ideal_pos = $x - ($x * $ct / $time);
	my $x_error = $x_ideal_pos - $x_actual_pos;

	my $y_actual_pos = $y_steps / $y_steps_per_mm;
	my $y_ideal_pos = $y - ($y * $ct / $time);
	my $y_error = $y_ideal_pos - $y_actual_pos;

	printf stderr "%5i %s %s %s %s\n", $ct, (($xc == 0)?sprintf("%5.2f", $x_actual_pos):"no-st"), sprintf("%+5.3f", $x_error), (($yc == 0)?sprintf("%5.2f", $y_actual_pos):"no-st"), sprintf("%+5.3f", $y_error);

	$td = $time;
	if (($xd - $xc) < $td) {
		$td = $xd - $xc;
	}
	if (($yd - $yc) < $td) {
		$td = $yd - $yc;
	}

	$xc += $td
		if $x_steps;
	$yc += $td
		if $y_steps;

	printf "\t%5i (%5.2fs) + %3i = ", $ct, $ct / $f_cpu, $td;
	$ct += $td;
	printf "%5i (%5.2fs)\n", $ct, $ct / $f_cpu;
}
