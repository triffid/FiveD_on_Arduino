#include	"gcode_parse.h"

#include	<stdlib.h>

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
		}
	}

	// TODO: process line just read
}
