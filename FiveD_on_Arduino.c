/*############################################################################
#                                                                            #
# FiveD on Arduino - alternative firmware for repraps                        #
#                                                                            #
# by Triffid Hunter, Traumflug, jakepoz                                      #
#                                                                            #
#                                                                            #
# This firmware is Copyright (C) 2009-2010 Michael Moon aka Triffid_Hunter   #
#                                                                            #
# This program is free software; you can redistribute it and/or modify       #
# it under the terms of the GNU General Public License as published by       #
# the Free Software Foundation; either version 2 of the License, or          #
# (at your option) any later version.                                        #
#                                                                            #
# This program is distributed in the hope that it will be useful,            #
# but WITHOUT ANY WARRANTY; without even the implied warranty of             #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              #
# GNU General Public License for more details.                               #
#                                                                            #
# You should have received a copy of the GNU General Public License          #
# along with this program; if not, write to the Free Software                #
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA #
#                                                                            #
############################################################################*/

#include	<stdint.h>
#include	<avr/interrupt.h>

#include	"serial.h"
#include	"timer.h"
#include	"gcode_parse.h"

uint8_t		input_fd;

void mainloop(void);
void mainloop() {
	/*
		process gcode
	*/
	if (input_fd == 0) {
		if (serial_rxchars()) {
			gcode_parse_char(serial_popchar());
		}
	}
	else {
		// TODO: alternate gcode sources
	}

	/*
		check timers
	*/
	ifclock(CLOCK_FLAG_10MS) {
	}
	ifclock(CLOCK_FLAG_250MS) {
	}
	ifclock(CLOCK_FLAG_1S) {
	}
}

void main(void);
void main() {
	serial_init();

	timer_init();

	sei();

	input_fd = 0;
	
	for (;;)
		mainloop();
}
