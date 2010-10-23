#if !defined _SIMULATION_H && defined SIMULATION
#define	_SIMULATION_H

#include <stdint.h>
#include <stdbool.h>

/* make arduino.h happy */
#define DIO0_PIN

#undef HEATER_PIN
#undef FAN_PIN
#undef HEATER_PWM
#undef FAN_PWM

typedef enum {
	X_STEP_PIN,
	X_DIR_PIN,
	X_MIN_PIN,
	Y_STEP_PIN,
	Y_DIR_PIN,
	Y_MIN_PIN,
	Z_STEP_PIN,
	Z_DIR_PIN,
	Z_MIN_PIN,
	E_STEP_PIN,
	E_DIR_PIN,

	STEPPER_ENABLE_PIN,

	SCK,
	MOSI,
	MISO,
	SS,

	PIN_NB
} pin_t;

#undef TEMP_PIN_CHANNEL
#define TEMP_PIN_CHANNEL 0

extern volatile bool sim_interrupts;
extern char *sim_serial_port;

#undef WRITE
#undef SET_OUTPUT
#undef SET_INPUT
void WRITE(pin_t pin, bool on);
void SET_OUTPUT(pin_t pin);
void SET_INPUT(pin_t pin);

#ifdef USE_WATCHDOG
#define wd_init()
#define wd_reset()
#endif

void sim_info(const char fmt[], ...);
void sim_error(const char msg[]);
void sim_assert(bool cond, const char msg[]);

#endif /* _SIMULATION_H */

