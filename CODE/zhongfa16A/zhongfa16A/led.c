/*
 * led.c
 *
 * Created: 2022/9/1 星期四 17:28:30
 *  Author: Administrator
 */ 
#include <avr/io.h>
void led1on(void){
	
	DDRA |= (1<<0);
	PORTA &=~(1<<0);
}
//所有灯灭
void led1off(void){
	
	DDRA |= (1<<0);
	PORTA |= (1<<0);	
}
void led1flashing (void){
	led1on();
	delay(500);
	led1off();
	delay(500);
}

void led2on(void){
	
	DDRA |= (1<<1);
	PORTA &=~(1<<1);
}
//所有灯灭
void led2off(void){
	
	DDRA |= (1<<1);
	PORTA |= (1<<1);
}
void led2flashing (void){
	led2on();
	delay(500);
	led2off();
	delay(500);
}