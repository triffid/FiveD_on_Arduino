#ifndef	_MOVE_H
#define	_MOVE_H

#include	"config.h"

typedef struct {
	int32_t	axis[NUM_AXES];
} position;

typedef struct {
	position start;
	position end;
	union {
		struct {
			uint8_t	live	:1;
		};
		uint8_t flags;
	};
} move;

void move_create(move *m);
void move_start(move *m);
void move_step(move *m);

#endif	/* _MOVE_H */
