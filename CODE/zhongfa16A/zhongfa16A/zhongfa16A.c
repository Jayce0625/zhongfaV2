/*
 * zhongfa16A.c
 *
 * Created: 2022/9/1 星期四 14:36:27
 *  Author: Administrator
 */ 


#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <usart.h>
#include <avr/signal.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <avr/delay.h>
#define  uchar unsigned char
#define  uint  unsigned int
extern   Queue bufQueue;
extern   Queue SPIbufQueue;
int main(void)
{
	for(int i=0;i<5000;i++)
	{
		for(int j=0;j<1440;j++);
	}
	SPI_Init();
	USART_Init(); // USART异步串行通信
	
	initqueue(&bufQueue);
	initqueue(&SPIbufQueue);
	char e,a;
	while(1)
    { 
		delay(10);
		//发送给128
		if(bufQueue.front!=bufQueue.rear){
		while(!(PORTB&(1<<4)));//等待SS电平为高
		SPI_MasterInit();//转换为主机模式
			
		while(bufQueue.front!=bufQueue.rear)
		{
			
			DeQueue(&bufQueue,&e);
			//USART_Transmit(e);//发给PC
		    SPI_MasterTransmit(e);//发给128
		}
		 a = SPDR;
		  SPI_Init();
		
	}
		//发送给PC
			while(SPIbufQueue.front!=SPIbufQueue.rear)
			{   
				//USART_Transmit(0x11);
				DeQueue(&SPIbufQueue,&a);
				USART_Transmit(a);//发给PC
				//USART_Transmit(0x12);
				
			}
	}	
}