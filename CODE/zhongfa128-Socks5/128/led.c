/*
 * led.c
 *
 * Created: 2022/9/1 星期四 15:30:49
 *  Author: wjc
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include <avr/signal.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#define  uchar unsigned char
#define  uint  unsigned int
void delay(uint ms)  // 带形参的延时函数，函数中传入一个希望延时多少毫秒的形参
{
	uint i,j;
	for(i=0;i<ms;i++)
	for(j=0;j<1141;j++);
}

void Led_Init(void)
{
	DDRA |= 0X07;  // PA0/PA1/PA2设为输出
	PORTA &= 0XFE;  // 黄灯亮
	PORTA |= 0X06;  // 红灯绿灯灭
}

void LedRedOn()
{
		PORTA &= ~(1<<2);
}

void LedGreenOn(uint i)
{
		PORTA &= ~(1<<1);
}

void LedYellowOn()
{
		PORTA &= ~(1<<0);
}

void LedRedOff(){
	PORTA |= (1<<2);
}

void LedGreenOff(){
	PORTA |= (1<<1);
}

void LedYellowOff(){
	PORTA |= (1<<0);
}
