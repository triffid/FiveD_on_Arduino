#ifndef	_AXES_H
#define	_AXES_H

void axis_enable(uint8_t designator);
void axis_disable(uint8_t designator);

void axis_set_direction(uint8_t designator, uint8_t direction);
void axis_step(uint8_t designator);

#endif	/* _AXES_H */
