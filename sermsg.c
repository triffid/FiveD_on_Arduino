#include	"sermsg.h"

#include	"serial.h"

void serwrite_hex4(uint8_t v) {
	v &= 0xF;
	if (v < 10)
		serial_writechar('0' + v);
	else
		serial_writechar('A' - 10 + v);
}

void serwrite_hex8(uint8_t v) {
	serwrite_hex4(v >> 4);
	serwrite_hex4(v & 0x0F);
}

void serwrite_hex16(uint16_t v) {
	serwrite_hex8(v >> 8);
	serwrite_hex8(v & 0xFF);
}

void serwrite_hex32(uint32_t v) {
	serwrite_hex8(v >> 16);
	serwrite_hex8(v & 0xFFFF);
}

const uint32_t powers[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

void serwrite_uint32(uint32_t v) {
	uint8_t e, t;

	for (e = 9; e > 0; e--) {
		if (v >= powers[e])
			break;
	}

	do
	{
		for (t = 0; v >= powers[e]; v -= powers[e], t++);
		serial_writechar(t + '0');
	}
	while (e--);
}

void serwrite_int32(int32_t v) {
	if (v < 0) {
		serial_writechar('-');
		v = -v;
	}

	serwrite_uint32(v);
}
