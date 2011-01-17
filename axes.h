#ifndef	_AXES_H
#define	_AXES_H

#include	"config.h"

typedef struct {
	uint8_t	designator;
	int32_t	min;
	int32_t	max;
} axis_block;

extern axis_block axes[NUM_AXES];

typedef struct {
	int32_t	pos;
} axis_runtime_block;

extern axis_runtime_block axis_runtime[NUM_AXES];

const uint8_t designator_to_index(uint8_t designator);

void axis_enable(uint8_t designator);
void axis_disable(uint8_t designator);

void axis_set_direction(uint8_t designator, uint8_t direction);
void axis_step(uint8_t designator);

#endif	/* _AXES_H */
