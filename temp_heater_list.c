#include	"temp_heater_list.h"

#include	<avr/eeprom.h>
#include	<avr/pgmspace.h>

#include	"arduino.h"
#include	"timer.h"
#include	"machine.h"
#include	"debug.h"
#include	"sersendf.h"

typedef enum {
	THERMISTOR,
	MAX6675,
	AD595
} temp_types;

struct {
	uint8_t						temp_type;
	uint8_t						temp_pin;
	
	volatile uint8_t	*heater_port;
	uint8_t						heater_pin;
	volatile uint8_t	*heater_pwm;
} temp_heaters[NUM_TEMP_HEATERS] =
{
	{
		MAX6675,
		0,
		&PORTD,
		PIND0,
		&OCR0A
	},
};

typedef enum {
	PRESENT,
	TCOPEN
} temp_flags_enum;

struct {
	int32_t						p_factor;
	int32_t						i_factor;
	int32_t						d_factor;
	int16_t						i_limit;
} temp_heaters_pid[NUM_TEMP_HEATERS];

struct {
	temp_flags_enum		temp_flags;
	uint16_t					temp_history[TH_COUNT];
	uint8_t						temp_history_pointer;
	
	uint16_t					last_read_temp;
	uint16_t					target_temp;
	uint8_t						temp_residency;
	
	int16_t						heater_p;
	int16_t						heater_i;
	int16_t						heater_d;
	
	uint16_t					next_read_time;
} temp_heaters_runtime[NUM_TEMP_HEATERS];


#ifdef	TEMP_MAX6675
#endif

#ifdef	TEMP_THERMISTOR
#include	"analog.h"

#define NUMTEMPS 20
uint16_t temptable[NUMTEMPS][2] PROGMEM = {
	{1, 841},
	{54, 255},
	{107, 209},
	{160, 184},
	{213, 166},
	{266, 153},
	{319, 142},
	{372, 132},
	{425, 124},
	{478, 116},
	{531, 108},
	{584, 101},
	{637, 93},
	{690, 86},
	{743, 78},
	{796, 70},
	{849, 61},
	{902, 50},
	{955, 34},
	{1008, 3}
};
#endif

#ifdef	TEMP_AD595
#include	"analog.h"
#endif

#ifndef	ABSDELTA
#define	ABSDELTA(a, b)	(((a) >= (b))?((a) - (b)):((b) - (a)))
#endif

#define		DEFAULT_P				8192
#define		DEFAULT_I				512
#define		DEFAULT_D				-24576
#define		DEFAULT_I_LIMIT	384

typedef struct {
	int32_t		EE_p_factor;
	int32_t		EE_i_factor;
	int32_t		EE_d_factor;
	int16_t		EE_i_limit;
} EE_factor;

EE_factor EEMEM EE_factors[NUM_TEMP_HEATERS];

void temp_heater_init() {
	// read factors from eeprom
	uint8_t i;
	for (i = 0; i < NUM_TEMP_HEATERS; i++) {
		temp_heaters_pid[i].p_factor = eeprom_read_dword((uint32_t *) &EE_factors[i].EE_p_factor);
		temp_heaters_pid[i].i_factor = eeprom_read_dword((uint32_t *) &EE_factors[i].EE_i_factor);
		temp_heaters_pid[i].d_factor = eeprom_read_dword((uint32_t *) &EE_factors[i].EE_d_factor);
		temp_heaters_pid[i].i_limit = eeprom_read_word((uint16_t *) &EE_factors[i].EE_i_limit);
		
		if ((temp_heaters_pid[i].p_factor == 0) && (temp_heaters_pid[i].i_factor == 0) && (temp_heaters_pid[i].d_factor == 0) && (temp_heaters_pid[i].i_limit == 0)) {
			temp_heaters_pid[i].p_factor = DEFAULT_P;
			temp_heaters_pid[i].i_factor = DEFAULT_I;
			temp_heaters_pid[i].d_factor = DEFAULT_D;
			temp_heaters_pid[i].i_limit = DEFAULT_I_LIMIT;
		}
	}
}

void temp_heater_save_settings() {
	uint8_t i;
	for (i = 0; i < NUM_TEMP_HEATERS; i++) {
		eeprom_write_dword((uint32_t *) &EE_factors[i].EE_p_factor, temp_heaters_pid[i].p_factor);
		eeprom_write_dword((uint32_t *) &EE_factors[i].EE_i_factor, temp_heaters_pid[i].i_factor);
		eeprom_write_dword((uint32_t *) &EE_factors[i].EE_d_factor, temp_heaters_pid[i].d_factor);
		eeprom_write_word((uint16_t *) &EE_factors[i].EE_i_limit, temp_heaters_pid[i].i_limit);
	}
}

void temp_heater_tick() {
	uint8_t	i = 0, all_within_range = 1;
	for (; i < NUM_TEMP_HEATERS; i++) {
		if (temp_heaters_runtime[i].next_read_time) {
			temp_heaters_runtime[i].next_read_time--;
		}
		else {
			uint16_t	temp = 0;
			uint8_t		j;
			//time to deal with this temp/heater
			switch(temp_heaters[i].temp_type) {
				#ifdef	TEMP_MAX6675
				case MAX6675:
					#ifdef	PRR
						PRR &= ~MASK(PRSPI);
					#elif defined PRR0
						PRR0 &= ~MASK(PRSPI);
					#endif
					
					SPCR = MASK(MSTR) | MASK(SPE) | MASK(SPR0);
					
					// enable MAX6675
					WRITE(SS, 0);
					
					// ensure 100ns delay - a bit extra is fine
					delay(1);
					
					// read MSB
					SPDR = 0;
					for (;(SPSR & MASK(SPIF)) == 0;);
					temp = SPDR;
					temp <<= 8;
					
					// read LSB
					SPDR = 0;
					for (;(SPSR & MASK(SPIF)) == 0;);
					temp |= SPDR;
					
					// disable MAX6675
					WRITE(SS, 1);
					
					temp_heaters_runtime[i].temp_flags = 0;
					if ((temp & 0x8002) == 0) {
						// got "device id"
						temp_heaters_runtime[i].temp_flags |= PRESENT;
						if (temp & 4) {
							// thermocouple open
							temp_heaters_runtime[i].temp_flags |= TCOPEN;
						}
						else {
							temp = temp >> 3;
						}
					}
					
					// FIXME: placeholder number
					temp_heaters_runtime[i].next_read_time = 37;

					break;
				#endif	/* TEMP_MAX6675	*/

				#ifdef	TEMP_THERMISTOR
				case THERMISTOR:

					//Read current temperature
					temp = analog_read(temp_heaters[i].temp_pin);

					//Calculate real temperature based on lookup table
					for (j = 1; j < NUMTEMPS; j++) {
						if (pgm_read_word(&(temptable[j][0])) > temp) {
							// multiply by 4 because internal temp is stored as 14.2 fixed point
							temp = pgm_read_word(&(temptable[j][1])) + (pgm_read_word(&(temptable[j][0])) - temp) * 4 * (pgm_read_word(&(temptable[j-1][1])) - pgm_read_word(&(temptable[j][1]))) / (pgm_read_word(&(temptable[j][0])) - pgm_read_word(&(temptable[j-1][0])));
							break;
						}
					}

					//Clamp for overflows
					if (j == NUMTEMPS)
						temp = temptable[NUMTEMPS-1][1];

					// FIXME: placeholder number
					temp_heaters_runtime[i].next_read_time = 1;

					break;
				#endif	/* TEMP_THERMISTOR */

				#ifdef	TEMP_AD595
				case AD595:
					temp = analog_read(temp_pin);

					// convert
					// >>8 instead of >>10 because internal temp is stored as 14.2 fixed point
					temp = (temp * 500L) >> 8;

					// FIXME: placeholder number
					next_read_time = 1;

					break;
				#endif	/* TEMP_AD595 */
			}
			temp_heaters_runtime[i].last_read_temp = temp;

			if (ABSDELTA(temp, temp_heaters_runtime[i].target_temp) < TEMP_HYSTERESIS) {
				if (temp_heaters_runtime[i].temp_residency < TEMP_RESIDENCY_TIME)
					temp_heaters_runtime[i].temp_residency++;
			}
			else {
				temp_heaters_runtime[i].temp_residency = 0;
				all_within_range = 0;
			}

			// now for heater stuff
			int16_t	t_error = temp_heaters_runtime[i].target_temp - temp_heaters_runtime[i].last_read_temp;
			
			temp_heaters_runtime[i].temp_history[temp_heaters_runtime[i].temp_history_pointer++] = temp_heaters_runtime[i].last_read_temp;
			temp_heaters_runtime[i].temp_history_pointer &= (TH_COUNT - 1);
			
			// PID stuff
			// proportional
			temp_heaters_runtime[i].heater_p = t_error;
			
			// integral
			temp_heaters_runtime[i].heater_i += t_error;
			// prevent integrator wind-up
			if (temp_heaters_runtime[i].heater_i > temp_heaters_pid[i].i_limit)
				temp_heaters_runtime[i].heater_i = temp_heaters_pid[i].i_limit;
			else if (temp_heaters_runtime[i].heater_i < -temp_heaters_pid[i].i_limit)
				temp_heaters_runtime[i].heater_i = -temp_heaters_pid[i].i_limit;
			
			// derivative
			// note: D follows temp rather than error so there's no large derivative when the target changes
			temp_heaters_runtime[i].heater_d = temp_heaters_runtime[i].last_read_temp - temp_heaters_runtime[i].temp_history[temp_heaters_runtime[i].temp_history_pointer];
			
			// combine factors
			int32_t pid_output_intermed = (
			(
			(((int32_t) temp_heaters_runtime[i].heater_p) * temp_heaters_pid[i].p_factor) +
			(((int32_t) temp_heaters_runtime[i].heater_i) * temp_heaters_pid[i].i_factor) +
			(((int32_t) temp_heaters_runtime[i].heater_d) * temp_heaters_pid[i].d_factor)
			) / PID_SCALE
			);
			
			// rebase and limit factors
			uint8_t pid_output;
			if (pid_output_intermed > 255)
				pid_output = 255;
			else if (pid_output_intermed < 0)
				pid_output = 0;
			else
				pid_output = pid_output_intermed & 0xFF;
			
			if (debug_flags & DEBUG_PID)
				sersendf_P(PSTR("T{E:%d, P:%d * %ld = %ld / I:%d * %ld = %ld / D:%d * %ld = %ld # O: %ld = %u}\n"), t_error, temp_heaters_runtime[i].heater_p, temp_heaters_pid[i].p_factor, (int32_t) temp_heaters_runtime[i].heater_p * temp_heaters_pid[i].p_factor / PID_SCALE, temp_heaters_runtime[i].heater_i, temp_heaters_pid[i].i_factor, (int32_t) temp_heaters_runtime[i].heater_i * temp_heaters_pid[i].i_factor / PID_SCALE, temp_heaters_runtime[i].heater_d, temp_heaters_pid[i].d_factor, (int32_t) temp_heaters_runtime[i].heater_d * temp_heaters_pid[i].d_factor / PID_SCALE, pid_output_intermed, pid_output);
			
			if (temp_heaters[i].heater_pwm) {
				*temp_heaters[i].heater_pwm = pid_output;
			}
			else {
				if (pid_output >= 8)
					*temp_heaters[i].heater_port |= MASK(temp_heaters[i].heater_pin);
				else
					*temp_heaters[i].heater_port &= ~MASK(temp_heaters[i].heater_pin);
			}
		}
	}
}

uint8_t	temp_achieved() {
	uint8_t i, all_ok = 255;
	for (i = 0; i < NUM_TEMP_HEATERS; i++) {
		if (temp_heaters_runtime[i].temp_residency < TEMP_RESIDENCY_TIME)
			all_ok = 0;
	}
	return all_ok;
}

void temp_set(uint8_t index, uint16_t temperature) {
	temp_heaters_runtime[index].target_temp = temperature;
}

void temp_print(uint8_t index) {
	uint8_t c = 0;
	
	c = (temp_heaters_runtime[index].last_read_temp & 3) * 25;

	sersendf_P("T: %u.%u\n", temp_heaters_runtime[index].last_read_temp >> 2, c);
}

void pid_set_p(uint8_t index, int32_t p) {
	temp_heaters_pid[index].p_factor = p;
}

void pid_set_i(uint8_t index, int32_t i) {
	temp_heaters_pid[index].i_factor = i;
}

void pid_set_d(uint8_t index, int32_t d) {
	temp_heaters_pid[index].d_factor = d;
}

void pid_set_i_limit(uint8_t index, int32_t i_limit) {
	temp_heaters_pid[index].i_limit = i_limit;
}
