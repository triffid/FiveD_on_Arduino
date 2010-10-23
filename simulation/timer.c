#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "../dda_queue.h"
#include "../timer.h"
#include "simulation.h"

#define TIMER_SIG (SIGRTMIN + 1)

static bool timer_initialised = false;
static timer_t timerid;

static void timer1_isr(int cause, siginfo_t *HowCome, void *ucontext)
{
	if (!sim_interrupts || !timerInterruptIsEnabled()) return;

	sim_interrupts = false;

	WRITE(SCK, 1);
	queue_step();
	WRITE(SCK, 0);

	sim_interrupts = true;
}

void setupTimerInterrupt(void)
{
	disableTimerInterrupt();
	sim_info("setupTimerInterrupt");

	struct sigevent sev;
	struct sigaction sa;

	/* Establish handler for timer signal */
	sa.sa_sigaction = timer1_isr;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	if (sigaction(TIMER_SIG, &sa, NULL) == -1) {
		sim_error("sigaction");
	}

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = TIMER_SIG;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
	{
		sim_error("timer_create");
	}

	timer_initialised = true;
}

void setTimer(uint32_t delay)
{
	struct itimerspec its;
	long long nsec = (long)delay * 8000000 / F_CPU * 1000;

	its.it_value.tv_sec = nsec / 1000000000;
	its.it_value.tv_nsec = nsec % 1000000000;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	if (timer_settime(timerid, 0, &its, NULL) == -1)
	{
		sim_error("timer_settime");
	}
}

