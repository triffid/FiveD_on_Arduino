#include	"move.h"

#include	"axes.h"
#include	"timer.h"

uint32_t	default_cnst(move *m) {
	return m->step_time;
}

uint32_t	(*choose_next_step_time)(move *m) = default_cnst;

void move_create(move *m) {
	// TODO: initialise variables
	// start and end have already been filled in by gcode_parse
	// flags should be zeroed
	// step time isn't yet set, derive from start.F
	m->flags = 0;
	m->step_time = (F_CPU * 60) / m->start.axis[designator_to_index('F')];
}

void move_start(move *m) {
	// set direction pins
	uint8_t i;
	for (i = 0; i < NUM_AXES; i++) {
		if ((m->end.axis[i] - m->start.axis[i]) >= 0) {
			assert_pin(&axis_pins[i].dir);
		}
		else {
			deassert_pin(&axis_pins[i].dir);
		}
	}

	// mark this move as alive
	m->live = 1;

	// do our first step
	move_step(m);
}

void move_step(move *m) {
	// step relevant axes
	uint8_t i;
	for (i = 0; i < NUM_AXES; i++) {
		if (axis_runtime[i].pos != m->end.axis[i]) {
			assert_pin(&axis_pins[i].step);
			if (read_pin(&axis_pins[i].step))
				axis_runtime[i].pos++;
			else
				axis_runtime[i].pos--;
		}
	}

	// set timeout for next step
	setTimer(choose_next_step_time(m));

	// reset pins
	for (i = 0; i < NUM_AXES; i++) {
		deassert_pin(&axis_pins[i].step);
	}
}
