#include	"axes.h"

#include	"config.h"

#include	"pins.h"

struct {
	uint8_t	designator;
	int32_t	min;
	int32_t	max;
} axes[NUM_AXES] = {
	{ 'X', 0, 200000 },
	{ 'Y', 0, 200000 },
	{ 'Z', 0, 200000 },
	{ 'E', 0, 200000 }
};

struct {
	int32_t	position;
} axes_runtime[NUM_AXES];

uint8_t designator_to_index(uint8_t designator) {
	if (designator < NUM_AXES)
		return designator;

	uint8_t i;
	for (i = 0; i < NUM_AXES; i++) {
		if (axes[i].designator == designator)
			return i;
	}

	# TODO: throw an error or something
}

void axis_enable(uint8_t designator) {
	uint8_t i = designator_to_index(designator);
	assert_pin(axes_pins[i].enable);
}

void axis_disable(uint8_t designator) {
	uint8_t i = designator_to_index(designator);
	deassert_pin(axes_pins[i].enable);
}

void axis_set_direction(uint8_t designator, uint8_t direction) {
	uint8_t i = designator_to_index(designator);
	if (direction)
		assert_pin(axes_pins[i].dir);
	else
		deassert_pin(axes_pins[i].dir);
}

void axis_step(uint8_t designator) {
	uint8_t i = designator_to_index(designator);
	assert_pin(axes_pins[i].step);
	deassert_pin(axes_pins[i].step);
}

uint8_t	axis_hit_endstop(uint8_t designator) {
	uint8_t i = designator_to_index(designator);
	return read_pin(axes_pins[i].min) | read_pin(axes_pins[i].max);
}
