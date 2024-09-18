/*
 * spi_master.c
 *
 * Created: 2022/9/1 星期四 15:31:19
 *  Author: Administrator
 */ 
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <usart.h>
// 等待串行通信数据接收结束
#define  uchar unsigned char
#define  uint  unsigned int
Queue SPIbufQueue;
extern Queue UARTbufQueue;
int result=0;
void SPI_Init(void)//SPI从机初始化
{
	 SPCR &= (~(1<<SPE));//关闭spi

	 SPCR &= ~(1<<7);  // 关闭SPI中断SPIE
	 SPSR &= ~(1<<7);  // 清空中断标志SPIF
	SPCR &= ~(1<<4);  // MSTR为0代表从机模式，末两位00代表默认分频（4分频）
	//SPCR |= (1<<4); 
	// 初始时SPI默认为从机，仅MISO为输出，其他引脚均为输入
	DDRB |= (1<<3);  // MISO，输出
	DDRB &= ~(1<<1);  // SCK，输入
	DDRB &= ~(1<<2);  // MOSI，输入
	DDRB &= ~(1<<0);  // ~SS，输入
	PORTB |= (1<<0);  // 00010000,PB4口对应的SS被拉高
	
	
	//使用默认分频
	SPCR |= ((1<<0) | (1<<1)); 
	SPSR &= ~(1<<7);  // 清空中断标志SPIF
	// 使能spi和中断
	SPCR |= ((1<<SPE) | (1<<SPIE));  // 11000000
	// 使能spi
	//SPCR |= (1<<SPE);  // 11000000
	
}
void SPI_MasterInitInit(void)//SPI主机初始化
{
	while(!(PORTB&(1<<0)));//等待SS电平为高
			delay(10);
			PORTB &= (~(1<<0));// 拉低ss
			SPCR &= ~(1<<7);  // 关闭SPI中断SPIE
			
			
	SPCR &= ~(1<<7);  // 关闭SPI中断SPIE
	SPSR &= ~(1<<7);  // 清空中断标志SPIF
	
	SPCR |= (1<<4);  // MSTR为0代表从机模式，末两位00代表默认分频（4分频）

	DDRB &= ~(1<<3);  // MISO，输入
	DDRB |= (1<<1);  // SCK，输出
	DDRB |= (1<<2);  // MOSI，输出
	DDRB |= (1<<0);  // ~SS，输出
	PORTB &= ~(1<<0);  // 00010000,PB4口对应的SS被拉低
	
	//使用默认分频
	SPCR |= ((1<<0) | (1<<1));//128分频
	SPSR &= ~(1<<7);  // 清空中断标志SPIF
	//使能spi
	SPCR |= (1<<SPE);
	// 使能spi和中断
	//SPCR |= ((1<<SPE) | (1<<SPIE));  // 11000000
	
	delay(10);
	
}
void SPI_MasterTransmit(uchar rdata)
{
	// 将待传输的数据存放进SPDR数据寄存器中
	SPDR = rdata;  // rdata为485通过USART传进ATmega16中的数据，通过SPI发送给ATmega128
	while (!(SPSR & (1<<SPIF))); // 等待数据发送结束
}

void __attribute((signal))   __vector_17(void){  //SPI接收中断
	//if( (SPCR & (1<<MSTR)) == 0 )
	//{
		char flag;
		flag=SPDR;
		enQueue(&SPIbufQueue,flag);		
	//}
	
}

void SPIputs(char *s)//发送字符串
{
    SPI_MasterInitInit();
	
	while(*s)
	{
		SPI_MasterTransmit(*s);
		//*s=0;
		s++;
	}
	SPI_Init();
}
