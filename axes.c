#include	"axes.h"

#include	"config.h"

#include	"pins.h"

axis_block axes[NUM_AXES] = {
	{ 'X', 0, 200000 },
	{ 'Y', 0, 200000 },
	{ 'Z', 0, 200000 },
	{ 'E', 0, 200000 },
	{ 'F', 0, 200000 }
};

axis_runtime_block axis_runtime[NUM_AXES];

const uint8_t designator_to_index(uint8_t designator) {
	if (designator < NUM_AXES)
		return designator;

	uint8_t i;
	for (i = 0; i < NUM_AXES; i++) {
		if (axes[i].designator == designator)
			return i;
	}

	// TODO: throw an error or something
	return 0;
}

void axis_enable(uint8_t designator) {
	uint8_t i = designator_to_index(designator);
	assert_pin(&axis_pins[i].enable);
}

void axis_disable(uint8_t designator) {
	uint8_t i = designator_to_index(designator);
	deassert_pin(&axis_pins[i].enable);
}

void axis_set_direction(uint8_t designator, uint8_t direction) {
	uint8_t i = designator_to_index(designator);
	if (direction)
		assert_pin(&axis_pins[i].dir);
	else
		deassert_pin(&axis_pins[i].dir);
}

void axis_step(uint8_t designator) {
	uint8_t i = designator_to_index(designator);
	assert_pin(&axis_pins[i].step);
	deassert_pin(&axis_pins[i].step);
}

uint8_t	axis_hit_endstop(uint8_t designator) {
	uint8_t i = designator_to_index(designator);
	return read_pin(&axis_pins[i].min) | read_pin(&axis_pins[i].max);
}
