#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "../config.h"
#include "simulation.h"
#include "../clock.h"

uint8_t ACSR;
uint8_t TIMSK1;

char *sim_serial_port;

/* -- initialise simulation ------------------------------------------------ */

void sim_init(void) __attribute__((constructor));
void sim_init(void)
{
	/* hack to get argv and argc */
	extern char ** environ;
	int argc = 1;
	char **argv = environ - 3;

	while((int)*argv != argc)
	{
		++argc;
		--argv;
	}
	argv++;

	sim_assert(argc >= 2, "please specify a serial port device name");
	sim_serial_port = argv[1];
}


/* -- debugging ------------------------------------------------------------ */

void sim_info(const char fmt[], ...)
{
	va_list ap;
	fputs("\033[0;32m" , stderr);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputs("\033[m\n", stderr);
}

void sim_error(const char msg[])
{
	fprintf(stderr, "\033[0;31mERROR: %s\033[m\n", msg);
	exit(-1);
}

void sim_assert(bool cond, const char msg[])
{
	if (!cond)
	{
		sim_error(msg);
	}
}


/* -- interrupts ----------------------------------------------------------- */

volatile bool sim_interrupts = false;
void sei(void)
{
	sim_interrupts = true;
}


/* -- PIN I/O -------------------------------------------------------------- */

#define out true
#define in  false

static int x = 0, y = 0, z = 0, e = 0;

static bool direction[PIN_NB];
static bool state[PIN_NB];

static void print_pos(void)
{
	static int x_old = 0, y_old = 0, z_old = 0;
	float dx, dy, dz, d;
	static uint32_t clk_old;

	uint32_t clk = clock_read();
	if (clk - clk_old < 50) return; /* output every 50ms */

	/* calculate speeds in mm/minute */
	dx = 60000.0 * (x - x_old) / (clk - clk_old) / STEPS_PER_MM_X;
	dy = 60000.0 * (y - y_old) / (clk - clk_old) / STEPS_PER_MM_Y;
	dz = 60000.0 * (z - z_old) / (clk - clk_old) / STEPS_PER_MM_Z;
	d = sqrt(dx*dx + dy*dy + dz*dz);

	printf("%.3f  %.3f %.3f %.3f  %.4f %.4f %.4f  %.4f\n",
		clk / 1000.0,
		x / STEPS_PER_MM_X,
		y / STEPS_PER_MM_Y,
		z / STEPS_PER_MM_Z,
		dx, dy, dz, d);

	clk_old = clk;
	x_old = x; y_old = y; z_old = z;
}

void WRITE(pin_t pin, bool s)
{
	bool old_state = state[pin];

	if (direction[pin] == out)
	{
		state[pin] = s;
	}
	if (s && !old_state) /* rising edge */
	{
		switch (pin)
		{
		case X_STEP_PIN:
			x += state[X_DIR_PIN] ? 1 : -1;
			print_pos();
			break;
		case Y_STEP_PIN:
			y += state[Y_DIR_PIN] ? 1 : -1;
			print_pos();
			break;
		case Z_STEP_PIN:
			z += state[Z_DIR_PIN] ? 1 : -1;
			print_pos();
			break;
		case E_STEP_PIN:
			e += state[E_DIR_PIN] ? 1 : -1;
			print_pos();
			break;
		default:
			break;
		}
	}
}

void SET_OUTPUT(pin_t pin)
{
	direction[pin] = out;
}

void SET_INPUT(pin_t pin)
{
	direction[pin] = in;
}

