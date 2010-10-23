#ifndef	_AVR_PGMSPACE_H
#define	_AVR_PGMSPACE_H

#define PGM_P const char *
#define PROGMEM
#define PSTR(x) (x)
#define pgm_read_byte(x) (*((uint8_t *)(x)))
#define pgm_read_word(x) (*((uint16_t *)(x)))

#endif /* _AVR_PGMSPACE_H */

