/*
 * powersaving.h
 *
 * Created: 09/09/2019 16:24:15
 *  Author: Mike
 */ 


#ifndef POWERSAVING_H_
#define POWERSAVING_H_

#include <avr/io.h>

void init_lowpower();
void idle();

#endif /* POWERSAVING_H_ */