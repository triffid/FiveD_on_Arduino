#include <signal.h>
#include <stdlib.h>
#include <time.h>

#include "../clock.h"
#include "simulation.h"

#define SIM_CLOCK_SLOWDOWN 1

#ifdef	GLOBAL_CLOCK
static volatile uint32_t gclock = 0;
#endif

static uint8_t clock_counter_250ms = 0;
static uint8_t clock_counter_1s    = 0;
volatile uint8_t clock_flag = 0;

static bool clock_initialised = false;
#define CLOCK_SIG (SIGRTMIN + 2)

/* 1/4 second tick */
static void timer2_isr(int cause, siginfo_t *HowCome, void *ucontext)
{
	if (!sim_interrupts) return;

	sim_interrupts = false;

	// global clock
#ifdef	GLOBAL_CLOCK
	gclock += SIM_CLOCK_SLOWDOWN;
#endif
	// 1/4 second tick
	if (++clock_counter_250ms == 250 / SIM_CLOCK_SLOWDOWN) {
		clock_flag |= CLOCK_FLAG_250MS;
		clock_counter_250ms = 0;
		if (++clock_counter_1s == 4) {
			clock_flag |= CLOCK_FLAG_1S;
			clock_counter_1s = 0;
		}
	}

	sim_interrupts = true;
}

void clock_setup(void)
{
	timer_t timerid;
	struct sigevent sev;
	struct itimerspec its;
	struct sigaction sa;
	const long long nsec = SIM_CLOCK_SLOWDOWN * 1000 * 1000;

	/* Establish handler for timer signal */
	sa.sa_sigaction = timer2_isr;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	if (sigaction(CLOCK_SIG, &sa, NULL) == -1) {
		sim_error("sigaction");
	}

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = CLOCK_SIG;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
	{
		sim_error("timer_create");
	}

	its.it_value.tv_sec = nsec / 1000000000;
	its.it_value.tv_nsec = nsec % 1000000000;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	if (timer_settime(timerid, 0, &its, NULL) == -1)
	{
		sim_error("timer_settime");
	}

	clock_initialised = true;
}

#ifdef GLOBAL_CLOCK
uint32_t clock_read()
{
	return gclock;
}
#endif

