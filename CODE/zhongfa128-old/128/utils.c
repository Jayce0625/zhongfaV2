/*
 * utils.c
 *
 * Created: 2022/9/29 18:44:22
 *  Author: xcn
 */ 
#include <avr/io.h>
#include <avr/signal.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <avr/delay.h>
#include <usart.h>
#include <utils.h>
extern Queue SPIbufQueue;
extern Queue UARTbufQueue;
extern char ip[19],port[8];

char rx_ok(char a,char b)
{
	while (DeQueue(&UARTbufQueue,&b))
	{
		
		SPI_MasterInitInit();
		SPI_MasterTransmit(b);
		SPI_Init();
		if ((a == 'O')&(b == 'K'))
		{
			return 1;
		}
		a = b;
	}
	return 0;
}
char rx_smallok(char a,char b)
{
	while (DeQueue(&UARTbufQueue,&b))
	{
		
		SPI_MasterInitInit();
		SPI_MasterTransmit(b);
		SPI_Init();
		if ((a == 'o')&(b == 'k'))
		{
			return 1;
		}
		a = b;
	}
	return 0;
}

void set_server(char e, char f)
{
	LedGreenOn();
	while (DeQueue(&UARTbufQueue,&f)); //排空队列
	puts1("AT+CIPMUX=0\r\n");
	delay(1000);
	while (!DeQueue(&UARTbufQueue,&f));
	e=f;
	SPI_MasterInitInit();
	SPI_MasterTransmit(f);
	SPI_Init();
	if (rx_ok(e,f))
	{
		while (DeQueue(&UARTbufQueue,&f)); //排空队列
		puts1("AT+CIPSTART=\"TCP\",");
		USART1_Transmit(0x22);
		puts1(ip);
		USART1_Transmit(0x22);
		USART1_Transmit(',');
		puts1(port);
		USART1_Transmit('\r');
		USART1_Transmit('\n');
		delay(1000);
		while (!DeQueue(&UARTbufQueue,&f));
		e=f;
		SPI_MasterInitInit();
		SPI_MasterTransmit(f);
		SPI_Init();
		if (rx_ok(e,f))
		{
			while (DeQueue(&UARTbufQueue,&f)); //排空队列
			puts1("AT+CIPMODE=1\r\n");
			delay(1000);
			while (!DeQueue(&UARTbufQueue,&f));
			e=f;
			SPI_MasterInitInit();
			SPI_MasterTransmit(f);
			SPI_Init();
			if (rx_ok(e,f))
			{
				while (DeQueue(&UARTbufQueue,&f)); //排空队列
				puts1("AT+CIPSEND\r\n");
				delay(1000);
				while (!DeQueue(&UARTbufQueue,&f));
				e=f;
				SPI_MasterInitInit();
				SPI_MasterTransmit(f);
				SPI_Init();
				if (rx_ok(e,f))
				{
					LedRedOn();
					puts1("wifimodule ok");
					SPCR &= ~(1<<7);  // 关闭SPI中断SPIE
					while (DeQueue(&SPIbufQueue,&f)); 
					SPCR |= (1<<7);  // 开启SPI中断SPIE
				}
			}
		}
	}
}