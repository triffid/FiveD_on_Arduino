#ifndef	_DDA_QUEUE
#define	_DDA_QUEUE

#include	"dda.h"

/*
	variables
*/

// this is the ringbuffer that holds the current and pending moves.
extern uint8_t	mb_head;
extern uint8_t	mb_tail;
extern DDA movebuffer[MOVEBUFFER_SIZE];

/*
	methods
*/

// queue status methods
uint8_t queue_full(void);
uint8_t queue_empty(void);

// take one step
void queue_step(void);

// add a new target to the queue
// t == NULL means add a wait for target temp to the queue
void enqueue(TARGET *t);

// called from step timer when current move is complete
void next_move(void) __attribute__ ((hot));

// print queue status
void print_queue(void);

// flush the queue for eg; emergency stop
void queue_flush(void);

#endif	/* _DDA_QUEUE */
