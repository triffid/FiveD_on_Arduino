#ifndef	_GCODE_PARSE_H
#define	_GCODE_PARSE_H

#include	<stdint.h>

extern float words[26];
extern uint32_t seen_mask;

void gcode_parse_char(uint8_t c);
void gcode_parse_line(uint8_t *c);

#endif	/* GCODE_PARSE_H	*/
