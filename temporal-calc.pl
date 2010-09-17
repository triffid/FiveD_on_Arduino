#!/usr/bin/perl

use strict;
use Data::Dumper;

my $f								= 800; # mm/min

my $x								= 12;	# mm
my $y								= 8;	# mm
my $z								= 0.4;	#mm
my $e								= 6;		#mm

my $x_steps_per_mm	= 2;
my $y_steps_per_mm	= 2;
my $z_steps_per_mm	= 160;
my $e_steps_per_mm	= 3;

my $f_cpu						= 16000;

my $distance				= sqrt(($x * $x) + ($y * $y) + ($z * $z)); # mm

my $time						= int($distance / $f * 60 * $f_cpu);	# cpu ticks

my $x_steps					= int($x * $x_steps_per_mm);
my $y_steps					= int($y * $y_steps_per_mm);
my $z_steps					= int($z * $z_steps_per_mm);
my $e_steps					= int($e * $e_steps_per_mm);

printf "X:%gmm (%i steps)\tY:%gmm (%i steps)\tZ:%gmm (%i steps)\tE:%gmm (%i steps)\tD:%5.2fmm\tF:%imm/min (%1.2gmm/sec)\ttime: %i (%g seconds)\n", $x, $x_steps, $y, $y_steps, $z, $z_steps, $e, $e_steps, $distance, $f, $f / 60, $time, $time / $f_cpu;

my $ct = 0;

my $xd = ($x_steps > 0)?int($time / $x_steps):$time;
my $yd = ($y_steps > 0)?int($time / $y_steps):$time;
my $zd = ($z_steps > 0)?int($time / $z_steps):$time;
my $ed = ($e_steps > 0)?int($time / $e_steps):$time;
my $xc = $xd;
my $yc = $yd;
my $zc = $zd;
my $ec = $ed;

my $td;

my $td;
# if ($xc < $td) {
	$td = $xc;
# }
if ($yc < $td) {
	$td = $yc;
}
if ($zc < $td) {
	$td = $zc;
}
if ($ec < $td) {
	$td = $ec;
}

my $last_x_step_time = 0;
my $last_y_step_time = 0;
my $last_z_step_time = 0;
my $last_e_step_time = 0;

while (($x_steps + $y_steps + $z_steps + $e_steps) > 0) {
	my $st = "";

	printf "\t";
	if ($xc <= 0) {
		printf "[X Speed: %5.2fmm/sec] ", 1 / (($ct - $last_x_step_time) / $f_cpu) / $x_steps_per_mm;
		$last_x_step_time = $ct;
		$xc += $xd;
		$x_steps--;
		$st .= "X ";
	}
	if ($yc <= 0) {
		printf "[Y Speed: %5.2fmm/sec] ", 1 / (($ct - $last_y_step_time) / $f_cpu) / $y_steps_per_mm;
		$last_y_step_time = $ct;
		$yc += $yd;
		$y_steps--;
		$st .= "Y ";
	}
	if ($zc <= 0) {
		printf "[Z Speed: %5.2fmm/sec] ", 1 / (($ct - $last_z_step_time) / $f_cpu) / $z_steps_per_mm;
		$last_z_step_time = $ct;
		$zc += $zd;
		$z_steps--;
		$st .= "Z ";
	}
	if ($ec <= 0) {
# 		die Dumper ['$e_steps_per_mm' => $e_steps_per_mm, 'ct' => $ct, 'last_e_step_time' => $last_e_step_time]
# 			if ((($ct - $last_e_step_time) / $f_cpu) / $e_steps_per_mm) == 0;
		printf "[E Speed: %5.2fmm/sec] ", 1 / (($ct - $last_e_step_time) / $f_cpu) / $e_steps_per_mm;
		$last_e_step_time = $ct;
		$ec += $ed;
		$e_steps--;
		$st .= "E ";
	}
	printf "\n";

	printf "X:%6i (%2.2fmm)\tY:%6i (%2.2fmm)\tZ:%6i (%2.2fmm)\tE:%6i (%2.2fmm)\txcd:%4i/%4i  ycd:%4i/%4i  zcd:%4i/%4i ecd:%4i/%4i\t%s\n", $x_steps, $x_steps / $x_steps_per_mm, $y_steps, $y_steps / $y_steps_per_mm, $z_steps, $z_steps / $z_steps_per_mm, $e_steps, $e_steps / $e_steps_per_mm, $xc, $xd, $yc, $yd, $zc, $zd, $ec, $ed, $st;

	my $x_actual_pos = $x_steps / $x_steps_per_mm;
	my $x_ideal_pos = $x - ($x * $ct / $time);
	my $x_error = $x_ideal_pos - $x_actual_pos;

	my $y_actual_pos = $y_steps / $y_steps_per_mm;
	my $y_ideal_pos = $y - ($y * $ct / $time);
	my $y_error = $y_ideal_pos - $y_actual_pos;

	my $z_actual_pos = $z_steps / $z_steps_per_mm;
	my $z_ideal_pos = $z - ($z * $ct / $time);
	my $z_error = $z_ideal_pos - $z_actual_pos;

	my $e_actual_pos = $e_steps / $e_steps_per_mm;
	my $e_ideal_pos = $e - ($e * $ct / $time);
	my $e_error = $e_ideal_pos - $e_actual_pos;

	printf stderr "%g %s %s %s %s %s %s %s %s\n", $ct / $f_cpu, (($st =~ /X/)?sprintf("%g", $x_actual_pos):"no-st"), sprintf("%+g", $x_error), (($st =~ /Y/)?sprintf("%g", $y_actual_pos):"no-st"), sprintf("%+g", $y_error), (($st =~ /Z/)?sprintf("%g", $z_actual_pos):"no-st"), sprintf("%+g", $z_error), (($st =~ /E/)?sprintf("%g", $e_actual_pos):"no-st"), sprintf("%+g", $e_error);

# 	$td = $time;
# 	if ($xc < $td) {
		$td = $xc;
# 	}
	if ($yc < $td) {
		$td = $yc;
	}
	if ($zc < $td) {
		$td = $zc;
	}
	if ($ec < $td) {
		$td = $ec;
	}

	$xc -= $td
		if $x_steps;
	$yc -= $td
		if $y_steps;
	$zc -= $td
		if $z_steps;
	$ec -= $td
		if $e_steps;

	printf "\t%5i (%5.3fs) + %3i = ", $ct, $ct / $f_cpu, $td;
	$ct += $td;
	printf "%5i (%5.3fs)\n", $ct, $ct / $f_cpu;
}
