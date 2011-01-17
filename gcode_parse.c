#include	"gcode_parse.h"

#include	<stdlib.h>

#include	"config.h"
#include	"axes.h"
#include	"sersendf.h"

#define	GCODE_LINE_BUFFER_LEN	64

uint8_t		gcode_line[GCODE_LINE_BUFFER_LEN];
uint8_t		gcode_line_pointer;

float			words[26];
uint32_t	seen_mask;

void gcode_parse_char(uint8_t c) {
	if (gcode_line_pointer < GCODE_LINE_BUFFER_LEN)
		gcode_line[gcode_line_pointer++] = c;
	if ((c == 13) || (c == 10)) {
		for (;gcode_line_pointer < GCODE_LINE_BUFFER_LEN; gcode_line_pointer++)
			gcode_line[gcode_line_pointer] = 0;
		gcode_parse_line(gcode_line);
		gcode_line_pointer = 0;
	}
}

void gcode_parse_line(uint8_t *c) {
	enum {
		STATE_FIND_WORD,
		STATE_FIND_VALUE,
		STATE_SEMICOLON_COMMENT,
		STATE_BRACKET_COMMENT,
	} state = STATE_FIND_WORD;

	uint8_t i;	// string index
	uint8_t w = 0;	// current gcode word

	seen_mask = 0;

	for (i = 0; c[i] != 0 && c[i] != 13 && c[i] != 10; i++) {
		switch (state) {
			case STATE_FIND_WORD:
				// start of word
				if (c[i] >= 'A' && c[i] <= 'Z') {
					w = c[i] - 'A';
					if (w < 26)
						state = STATE_FIND_VALUE;
				}
				// comment until end of line
				if (c[i] == ';') {
					state = STATE_SEMICOLON_COMMENT;
				}
				// comment until close bracket
				if (c[i] == '(') {
					state = STATE_BRACKET_COMMENT;
				}
				break;
			case STATE_FIND_VALUE:
				if ((c[i] >= '0' && c[i] <= '9') || c[i] == '-') {
					uint8_t	*ep;
					float v = strtod((const char *) &c[i], (char **) &ep);
					state = STATE_FIND_WORD;
					if (ep > &c[i]) {
						seen_mask |= (1 << w);
						words[w] = v;
						i = ep - c - 1;
					}
				}
				break;
			case STATE_BRACKET_COMMENT:
				if (c[i] == ')')
					state = STATE_FIND_WORD;
				break;
			case STATE_SEMICOLON_COMMENT:
				// dummy entry to suppress compiler warning
				break;
		} // switch (state)
	} // for i=0 .. newline

	// TODO: process line just read

	// first, limit axis words to hard limits
	// TODO: better to throw an error, or silently limit?
	uint8_t j;
	for (j = 0; j < NUM_AXES; j++) {
		uint8_t d = axes[j].designator - 'A';
		if (words[d] > axes[j].max)
			words[d] = axes[j].max;
		if (words[d] < axes[j].min)
			words[d] = axes[j].min;
	}

	// work out what the command is
	// seen G?
	if (seen_mask & (1 << ('G' - 'A'))) {
		uint8_t G = words['G' - 'A'];
		switch (G) {
			case 0:
				// rapid move
				break;
			case 1:
				// synchronised move
				break;
			case 4:
				// dwell
				break;
			case 20:
				// inches as units
				break;
			case 21:
				// mm as units
				break;
			case 28:
				// go to 0
				break;
			case 30:
				// move, then go home
				break;
			case 90:
				// absolute positioning
				break;
			case 91:
				// relative positioning
				break;
			case 92:
				// set current position
				break;
			default:
				sersendf_P(PSTR("Unsupported G-code: %d"), G);
				break;
		} // switch (G)
	} // if seen G

	// seen M?
	if (seen_mask & (1 << ('M' - 'A'))) {
		uint8_t M = words['M' - 'A'];
		switch (M) {
			case 2:
				// program end
				break;
			case 6:
				// TODO: M6 actually performs toolchange to tool selected by T command
				break;
			case 3:
			case 101:
				// extruder on
				break;
			case 5:
			case 103:
				// extruder off
				break;
			case 104:
				// set temperature
				break;
			case 105:
				// print temperature
				break;
			case 7:
			case 106:
				// heater on
				break;
			case 9:
			case 107:
				// heater off
				break;
			case 109:
				// set temperature and wait
				break;
			case 110:
				// set expected line number
				break;
			case 112:
				// immediate stop
				break;
			case 114:
				// report current position
				break;
			case 115:
				// report firmware capabilities
				break;
			case 190:
				// power on
				break;
			case 191:
				// power off
				break;
			default:
				sersendf_P(PSTR("Unsupported M-code: %d"), M);
				break;
		} // switch (M)
	} // if seen M
} // gcode_parse_line()
