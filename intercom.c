#include	"intercom.h"

#include	"arduino.h"
#include	"pinout.h"

#define		BAUD			19200

//#define		EXTRUDER
#define		HOST

/*
 Defines a super simple intercom interface using the RS485 modules

 Host will say: START1 START2 PWM_CMD PWM_CHK 
 Extruder will reply: START1 START2 TMP_CMD TMP_CHK 

 CHK = 255-CMD, if they match do the work, if not, ignore this packet

 in a loop
*/


#define		START1	0xAA
#define		START2	0x55

typedef enum {
	SEND_START1,
	SEND_START2,
	SEND_CMD,
	SEND_CHK,


	READ_START1,
	READ_START2,
	READ_CMD,
	READ_CHK,
} intercom_state_e;


intercom_state_e state = READ_START1;
uint8_t cmd, chk, send_cmd, read_cmd;

void intercom_init()
{
#ifdef HOST
	#if BAUD > 38401
		UCSR1A = MASK(U2X1);
	#else
		UCSR1A = 0;
	#endif
	#if BAUD > 38401
		UBRR1 = (((F_CPU / 8) / BAUD) - 0.5);
	#else
		UBRR1 = (((F_CPU / 16) / BAUD) - 0.5);
	#endif
		UCSR1B = MASK(RXEN1) | MASK(TXEN1);
		UCSR1C = MASK(UCSZ11) | MASK(UCSZ10);

		UCSR1B |= MASK(RXCIE1) | MASK(UDRIE1);
#else
	#if BAUD > 38401
		UCSR0A = MASK(U2X0);
	#else
		UCSR0A = 0;
	#endif
	#if BAUD > 38401
		UBRR0 = (((F_CPU / 8) / BAUD) - 0.5);
	#else
		UBRR0 = (((F_CPU / 16) / BAUD) - 0.5);
	#endif
		UCSR0B = MASK(RXEN0) | MASK(TXEN0);
		UCSR0C = MASK(UCSZ01) | MASK(UCSZ00);

		UCSR0B |= MASK(RXCIE0) | MASK(UDRIE0);
#endif
}

void update_send_cmd(uint8_t new_send_cmd) {
	send_cmd = new_send_cmd;
}

uint8_t get_read_cmd(void) {
	return read_cmd;
}

static void write_byte(uint8_t val) {
#ifdef HOST
	UDR1 = val;
#else
	UDR0 = val;
#endif
}

static uint8_t read_byte(void) {
#ifdef HOST
	return UDR1;
#else
	return UDR0;
#endif
}

static void start_send(void) {
	enable_transmit();
	state = SEND_START1;
	write_byte(START1);
	//Enable interrupts so we can send next characters
#ifdef HOST
	UCSR1B |= MASK(UDRIE1);
#else
	UCSR0B |= MASK(UDRIE0);
#endif
}

static void finish_send(void) {
	disable_transmit();
#ifdef HOST
	UCSR1B &= ~MASK(UDRIE1);
#else
	UCSR0B &= ~MASK(UDRIE0);
#endif
	state = READ_START1;
}


/*
	Interrupts, UART 0 for mendel
*/
#ifdef HOST
ISR(USART1_RX_vect)
#else
ISR(USART_RX_vect)
#endif
{
	uint8_t c = read_byte();

	switch(state) {
	case READ_START1:
		if (c == START1) state = READ_START2;
		break;
	case READ_START2:
		if (c == START2) state = READ_CMD;
		else			 state = READ_START1;
		break;
	case READ_CMD:
		cmd = c;
		state = READ_CHK;
		break;
	case READ_CHK:
		chk = c;
					
		if (chk == 255 - cmd) {	
			//Values are correct, do something useful
			read_cmd = cmd;
			start_send();
		}
		else
		{
			state = READ_START1;
		}
		break;
	default:
		break;
	}
}

#ifdef HOST
ISR(USART1_UDRE_vect)
#else
ISR(USART_UDRE_vect)
#endif
{
	switch(state) {
	case SEND_START1:
		write_byte(START2);
		state = SEND_START2;
		break;
	case SEND_START2:
		write_byte(send_cmd);
		state = SEND_CMD;
		break;
	case SEND_CMD:
		write_byte(255 - send_cmd);
		state = SEND_CHK;
		break;
	case SEND_CHK:
		finish_send();
		break;
	default:
		break;
	}
}

