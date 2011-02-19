#include	"gcode_parse.h"

#include	<stdlib.h>

#include	"config.h"
#include	"sersendf.h"
#include	"serial.h"
#include	"sermsg.h"
#include	"gcode_process.h"


#define	GCODE_LINE_BUFFER_LEN	64

#define iA 0
#define iB 1
#define iC 2
#define iD 3
#define iE 4
#define iF 5
#define iG 6
#define iH 7
#define iI 8
#define iJ 9
#define iK 10
#define iL 11
#define iM 12
#define iN 13
#define iO 14
#define iP 15
#define iQ 16
#define iR 17
#define iS 18
#define iT 19
#define iU 20
#define iV 21
#define iW 22
#define iX 23
#define iY 24
#define iZ 25
#define	iAsterisk	26

GCODE_COMMAND next_target;

uint8_t		gcode_line[GCODE_LINE_BUFFER_LEN];
uint8_t		gcode_line_pointer = 0;

float			words[32];
uint32_t	seen_mask = 0;

#define	SEEN(c)	(seen_mask & (1L << (c)))

uint32_t	line_number = 0;

const uint8_t char2index(uint8_t c) __attribute__ ((pure));
const uint8_t char2index(uint8_t c) {
	if (c >= 'a' && c <= 'z')
		return c - 'a';
	if (c >= 'A' && c <= 'Z')
		return c - 'A';
	if (c == '*')
		return 26;
	return 255;
}

void gcode_parse_char(uint8_t c) {
	if (gcode_line_pointer < (GCODE_LINE_BUFFER_LEN - 1))
		gcode_line[gcode_line_pointer++] = c;
	if ((c == 13) || (c == 10)) {
		uint8_t i;
		for (i = gcode_line_pointer; i < GCODE_LINE_BUFFER_LEN; i++)
			gcode_line[i] = 0;
		if (gcode_line_pointer > 2)
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
	uint8_t checksum = 0;
	
	seen_mask = 0;
	
	// calculate checksum
	for(i = 0; c[i] != '*' && c[i] != 0; i++)
		checksum = checksum ^ c[i];
	
	// extract gcode words from line
	for (i = 0; c[i] != 0 && c[i] != 13 && c[i] != 10; i++) {
		switch (state) {
			case STATE_FIND_WORD:
				// start of word
				if (char2index(c[i]) < 255) {
					w = char2index(c[i]);
					state = STATE_FIND_VALUE;
				}
				// comment until end of line
				if (c[i] == ';')
					state = STATE_SEMICOLON_COMMENT;
				// comment until close bracket
				if (c[i] == '(')
					state = STATE_BRACKET_COMMENT;
				break;
			case STATE_FIND_VALUE:
				if ((c[i] >= '0' && c[i] <= '9') || c[i] == '-') {
					uint8_t	*ep;
					float v = strtod((const char *) &c[i], (char **) &ep);
					state = STATE_FIND_WORD;
					if (ep > &c[i]) {
// 						sersendf_P(PSTR("[seen %c: %lx->"), w + 'A', seen_mask);
						seen_mask |= (1L << w);
// 						sersendf_P(PSTR("%lx]"), seen_mask);
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
	
	if (SEEN(iAsterisk)) {
		if (checksum != words[iAsterisk]) {
			if (SEEN(iN))
				sersendf_P(PSTR("rs N%lu "), ((uint32_t) words[iN]));
			sersendf_P(PSTR("Bad checksum, received %d, expected %d\n"), ((uint8_t) words[iAsterisk]), checksum);
			seen_mask = 0;
			return;
		}
	}
	
	if (SEEN(iN)) {
		if (((uint32_t) words[iN]) != line_number) {
			sersendf_P(PSTR("rs N%lu Bad line number, received %lu, expected %lu\n"), line_number, ((uint32_t) words[iN]), line_number);
			seen_mask = 0;
			return;
		}
		line_number++;
	}
	
	serial_writestr_P(PSTR("ok "));

	// patch words into next_target struct
	// TODO: eliminate next_target, update gcode_process to use words[] directly

	next_target.flags = 0;
	if (SEEN(iG)) {
		next_target.seen_G = 1;
		next_target.G = words[iG];
// 		sersendf_P(PSTR("G:%d/"), next_target.G);
	}
	if (SEEN(iM)) {
		next_target.seen_M = 1;
		next_target.M = words[iM];
// 		sersendf_P(PSTR("M:%d/"), next_target.M);
	}
	if (SEEN(iX)) {
		next_target.seen_X = 1;
		next_target.target.X = words[iX] * STEPS_PER_MM_X;
// 		sersendf_P(PSTR("X:%ld/"), next_target.target.X);
	}
	if (SEEN(iY)) {
		next_target.seen_Y = 1;
		next_target.target.Y = words[iY] * STEPS_PER_MM_Y;
// 		sersendf_P(PSTR("Y:%ld/"), next_target.target.Y);
	}
	if (SEEN(iZ)) {
		next_target.seen_Z = 1;
		next_target.target.Z = words[iZ] * STEPS_PER_MM_Z;
// 		sersendf_P(PSTR("Z:%ld/"), next_target.target.Z);
	}
	if (SEEN(iE)) {
		next_target.seen_E = 1;
		next_target.target.E = words[iE] * STEPS_PER_MM_E;
// 		sersendf_P(PSTR("E:%ld/"), next_target.target.E);
	}
	if (SEEN(iF)) {
		next_target.seen_F = 1;
		next_target.target.F = words[iF];
// 		sersendf_P(PSTR("F:%ld/"), next_target.target.F);
	}
	if (SEEN(iS)) {
		next_target.seen_S = 1;
		// if this is temperature, multiply by 4 to convert to quarter-degree units
		// cosmetically this should be done in the temperature section,
		// but it takes less code, less memory and loses no precision if we do it here instead
		if ((next_target.M == 104) || (next_target.M == 109))
			next_target.S = words[iS] * 4.0;
		// if this is heater PID stuff, multiply by PID_SCALE because we divide by PID_SCALE later on
		else if ((next_target.M >= 130) && (next_target.M <= 132))
			next_target.S = words[iS] * PID_SCALE;
		else
			next_target.S = words[iS];
// 		sersendf_P(PSTR("S:%d/"), next_target.S);
	}
	if (SEEN(iP)) {
		next_target.seen_P = 1;
		next_target.P = words[iP];
// 		sersendf_P(PSTR("P:%u/"), next_target.P);
	}
	if (SEEN(iT)) {
		next_target.seen_T = 1;
		next_target.T = words[iT];
// 		sersendf_P(PSTR("T:%d/"), next_target.T);
	}
	if (SEEN(iN)) {
		next_target.seen_N = 1;
		next_target.N = words[iN];
// 		sersendf_P(PSTR("N:%lu/"), next_target.N);
	}
	next_target.N_expected = line_number;
	if (SEEN(iAsterisk)) {
		next_target.seen_checksum = 1;
		next_target.checksum_read = words[iAsterisk];
	}
	next_target.checksum_calculated = checksum;

	process_gcode_command();
	serial_writechar('\n');

	seen_mask = 0;
}

/****************************************************************************
*                                                                           *
* Character Received - add it to our command                                *
*                                                                           *
****************************************************************************/

#if 0
void gcode_parse_char(uint8_t c) {
	#ifdef ASTERISK_IN_CHECKSUM_INCLUDED
	if (next_target.seen_checksum == 0)
		next_target.checksum_calculated = crc(next_target.checksum_calculated, c);
	#endif

	// uppercase
	if (c >= 'a' && c <= 'z')
		c &= ~32;

	// process previous field
	if (last_field) {
		// check if we're seeing a new field or end of line
		// any character will start a new field, even invalid/unknown ones
		if ((c >= 'A' && c <= 'Z') || c == '*' || (c == 10) || (c == 13)) {
			switch (last_field) {
				case 'G':
					next_target.G = read_digit.mantissa;
					if (debug_flags & DEBUG_ECHO)
						serwrite_uint8(next_target.G);
					break;
				case 'M':
					next_target.M = read_digit.mantissa;
					if (debug_flags & DEBUG_ECHO)
						serwrite_uint8(next_target.M);
					break;
				case 'X':
					if (next_target.option_inches)
						next_target.target.X = decfloat_to_int(&read_digit, STEPS_PER_IN_X, 1);
					else
						next_target.target.X = decfloat_to_int(&read_digit, STEPS_PER_M_X, 1000);
					if (debug_flags & DEBUG_ECHO)
						serwrite_int32(next_target.target.X);
					break;
				case 'Y':
					if (next_target.option_inches)
						next_target.target.Y = decfloat_to_int(&read_digit, STEPS_PER_IN_Y, 1);
					else
						next_target.target.Y = decfloat_to_int(&read_digit, STEPS_PER_M_Y, 1000);
					if (debug_flags & DEBUG_ECHO)
						serwrite_int32(next_target.target.Y);
					break;
				case 'Z':
					if (next_target.option_inches)
						next_target.target.Z = decfloat_to_int(&read_digit, STEPS_PER_IN_Z, 1);
					else
						next_target.target.Z = decfloat_to_int(&read_digit, STEPS_PER_M_Z, 1000);
					if (debug_flags & DEBUG_ECHO)
						serwrite_int32(next_target.target.Z);
					break;
				case 'E':
					if (next_target.option_inches)
						next_target.target.E = decfloat_to_int(&read_digit, STEPS_PER_IN_E, 1);
					else
						next_target.target.E = decfloat_to_int(&read_digit, STEPS_PER_M_E, 1000);
					if (debug_flags & DEBUG_ECHO)
						serwrite_uint32(next_target.target.E);
					break;
				case 'F':
					// just use raw integer, we need move distance and n_steps to convert it to a useful value, so wait until we have those to convert it
					if (next_target.option_inches)
						next_target.target.F = decfloat_to_int(&read_digit, 254, 10);
					else
						next_target.target.F = decfloat_to_int(&read_digit, 1, 1);
					if (debug_flags & DEBUG_ECHO)
						serwrite_uint32(next_target.target.F);
					break;
				case 'S':
					// if this is temperature, multiply by 4 to convert to quarter-degree units
					// cosmetically this should be done in the temperature section,
					// but it takes less code, less memory and loses no precision if we do it here instead
					if ((next_target.M == 104) || (next_target.M == 109) || (next_target.M == 140))
						next_target.S = decfloat_to_int(&read_digit, 4, 1);
					// if this is heater PID stuff, multiply by PID_SCALE because we divide by PID_SCALE later on
					else if ((next_target.M >= 130) && (next_target.M <= 132))
						next_target.S = decfloat_to_int(&read_digit, PID_SCALE, 1);
					else
						next_target.S = decfloat_to_int(&read_digit, 1, 1);
					if (debug_flags & DEBUG_ECHO)
						serwrite_uint16(next_target.S);
					break;
				case 'P':
					next_target.P = decfloat_to_int(&read_digit, 1, 1);
					if (debug_flags & DEBUG_ECHO)
						serwrite_uint16(next_target.P);
					break;
				case 'T':
					next_target.T = read_digit.mantissa;
					if (debug_flags & DEBUG_ECHO)
						serwrite_uint8(next_target.T);
					break;
				case 'N':
					next_target.N = decfloat_to_int(&read_digit, 1, 1);
					if (debug_flags & DEBUG_ECHO)
						serwrite_uint32(next_target.N);
					break;
				case '*':
					next_target.checksum_read = decfloat_to_int(&read_digit, 1, 1);
					if (debug_flags & DEBUG_ECHO)
						serwrite_uint8(next_target.checksum_read);
					break;
			}
			// reset for next field
			last_field = 0;
			read_digit.sign = read_digit.mantissa = read_digit.exponent = 0;
		}
	}

	// skip comments
	if (next_target.seen_semi_comment == 0 && next_target.seen_parens_comment == 0) {
		// new field?
		if ((c >= 'A' && c <= 'Z') || c == '*') {
			last_field = c;
			if (debug_flags & DEBUG_ECHO)
				serial_writechar(c);
		}

		// process character
		switch (c) {
			// each currently known command is either G or M, so preserve previous G/M unless a new one has appeared
			// FIXME: same for T command
			case 'G':
				next_target.seen_G = 1;
				next_target.seen_M = 0;
				next_target.M = 0;
				break;
			case 'M':
				next_target.seen_M = 1;
				next_target.seen_G = 0;
				next_target.G = 0;
				break;
			case 'X':
				next_target.seen_X = 1;
				break;
			case 'Y':
				next_target.seen_Y = 1;
				break;
			case 'Z':
				next_target.seen_Z = 1;
				break;
			case 'E':
				next_target.seen_E = 1;
				break;
			case 'F':
				next_target.seen_F = 1;
				break;
			case 'S':
				next_target.seen_S = 1;
				break;
			case 'P':
				next_target.seen_P = 1;
				break;
			case 'T':
				next_target.seen_T = 1;
				break;
			case 'N':
				next_target.seen_N = 1;
				break;
			case '*':
				next_target.seen_checksum = 1;
				break;

			// comments
			case ';':
				next_target.seen_semi_comment = 1;
				break;
			case '(':
				next_target.seen_parens_comment = 1;
				break;

			// now for some numeracy
			case '-':
				read_digit.sign = 1;
				// force sign to be at start of number, so 1-2 = -2 instead of -12
				read_digit.exponent = 0;
				read_digit.mantissa = 0;
				break;
			case '.':
				if (read_digit.exponent == 0)
					read_digit.exponent = 1;
				break;
			#ifdef	DEBUG
			case ' ':
			case '\t':
			case 10:
			case 13:
				// ignore
				break;
			#endif

			default:
				// can't do ranges in switch..case, so process actual digits here.
				if (    c >= '0'
				     && c <= '9'
				     && read_digit.mantissa < (DECFLOAT_MANT_MAX / 10)
				     && read_digit.exponent < DECFLOAT_EXP_MAX ) {
					// this is simply mantissa = (mantissa * 10) + atoi(c) in different clothes
					read_digit.mantissa = (read_digit.mantissa << 3) + (read_digit.mantissa << 1) + (c - '0');
					if (read_digit.exponent)
						read_digit.exponent++;
				}
				#ifdef	DEBUG
				else {
					// invalid
					serial_writechar('?');
					serial_writechar(c);
					serial_writechar('?');
				}
				#endif
		}
	} else if ( next_target.seen_parens_comment == 1 && c == ')')
		next_target.seen_parens_comment = 0; // recognize stuff after a (comment)


	#ifndef ASTERISK_IN_CHECKSUM_INCLUDED
	if (next_target.seen_checksum == 0)
		next_target.checksum_calculated = crc(next_target.checksum_calculated, c);
	#endif

	// end of line
	if ((c == 10) || (c == 13)) {
		if (debug_flags & DEBUG_ECHO)
			serial_writechar(c);

		if (
		#ifdef	REQUIRE_LINENUMBER
			(next_target.N >= next_target.N_expected) && (next_target.seen_N == 1)
		#else
			1
		#endif
			) {
			if (
				#ifdef	REQUIRE_CHECKSUM
				((next_target.checksum_calculated == next_target.checksum_read) && (next_target.seen_checksum == 1))
				#else
				((next_target.checksum_calculated == next_target.checksum_read) || (next_target.seen_checksum == 0))
				#endif
				) {
				// process
				serial_writestr_P(PSTR("ok "));
				process_gcode_command();
				serial_writestr_P(PSTR("\n"));

				// expect next line number
				if (next_target.seen_N == 1)
					next_target.N_expected = next_target.N + 1;
			}
			else {
				sersendf_P(PSTR("rs N%ld Expected checksum %d\n"), next_target.N_expected, next_target.checksum_calculated);
				request_resend();
			}
		}
		else {
			sersendf_P(PSTR("rs N%ld Expected line number %ld\n"), next_target.N_expected, next_target.N_expected);
			request_resend();
		}

		// reset variables
		next_target.seen_X = next_target.seen_Y = next_target.seen_Z = \
			next_target.seen_E = next_target.seen_F = next_target.seen_S = \
			next_target.seen_P = next_target.seen_T = next_target.seen_N = \
			next_target.seen_M = next_target.seen_checksum = next_target.seen_semi_comment = \
			next_target.seen_parens_comment = next_target.checksum_read = \
			next_target.checksum_calculated = 0;
		last_field = 0;
		read_digit.sign = read_digit.mantissa = read_digit.exponent = 0;

		// assume a G1 by default
		next_target.seen_G = 1;
		next_target.G = 1;

		if (next_target.option_relative) {
			next_target.target.X = next_target.target.Y = next_target.target.Z = 0;
		}
		// E always relative
		next_target.target.E = 0;
	}
}

/***************************************************************************\
*                                                                           *
* Request a resend of the current line - used from various places.          *
*                                                                           *
* Relies on the global variable next_target.N being valid.                  *
*                                                                           *
\***************************************************************************/

void request_resend(void) {
	serial_writestr_P(PSTR("rs "));
	serwrite_uint8(next_target.N);
	serial_writechar('\n');
}
#endif
