#include	"queue.h"

#include	<string.h>

#include	"move.h"

move movebuffer[NUM_MOVES];

uint8_t	head;
uint8_t	tail;

position finish;

void enqueue(position end) {
	if (((head + 1) & NUM_MOVES) != tail) {
		uint8_t	nh = head + 1;
		// copy start and finish to buffer
		memcpy(&movebuffer[nh].start, &finish, sizeof(position));
		memcpy(&movebuffer[nh].end, &end, sizeof(position));
		// copy new finish
		memcpy(&finish, &end, sizeof(position));
		// initialise move
		move_create(&movebuffer[nh]);
		// update head
		head = nh;
		// start move if we're not moving
		if (movebuffer[tail].live == 0)
			next_move();
	}
	else {
		// TODO: error: move buffer overflow
	}
}

void next_move() {
	if (head != tail) {
		tail++;
		move_start(&movebuffer[tail]);
	}
}

void queue_step() {
	if (movebuffer[tail].live)
		move_step(&movebuffer[tail]);
	
	if (movebuffer[tail].live == 0)
		next_move();
}
