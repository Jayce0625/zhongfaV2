/*
 * led.h
 *
 * Created: 2022/9/1 星期四 15:30:39
 *  Author: Administrator
 */ 


#ifndef LED_H_
#define LED_H_

void delay(uint ms);
void Led_Init(void);
void LedRedOn(void);
void LedGreenOn(void);
void LedYellowOn(void);
void LedRedOff();

void LedGreenOff();

void LedYellowOff();
#endif /* LED_H_ */