/*
 * spi_slave.c
 *
 * Created: 2022/9/1 星期四 14:38:38
 *  Author: Administrator
 */ 
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <usart.h>
#define uint unsigned int
#define  uchar  unsigned char
// 等待串行通信数据接收结束
 Queue SPIbufQueue;


void SPI_Init(void)// SPI初始化，默认从机模式
{
	SPCR &= ~(1<<7);  // 关闭SPI中断SPIE
	SPSR &= ~(1<<7);  // 清空中断标志SPIF
	// 设置SPI为主机模式
	SPCR &= ~(1<<4);  // 将SPCR控制寄存器中的MSTR清零，代表选择从机模式
	// 仅MISO设置为输入，其他均设为输出
	DDRB &= ~(1<<4);  // ~SS，输入
	DDRB &= ~(1<<7);  // SCK，输入
	DDRB &= ~(1<<5);  // MOSI，输入
	DDRB |= (1<<6);  // MISO，输出
	PORTB |= (1<<4);  // 拉高SS的电平
	
	SPCR |=(1<<0)|(1<<1);//128分频
	SPCR |=(1<<6)|(1<<7);//SPI使能和中断使能
}
void SPI_MasterInit(void)//SPI主机初始化，谁发谁是主，默认全部是从机模式同时将SS设为低电平，谁想发数据就先配置为主机再拉高SS
{
    SPCR &= ~(1<<7);  // 关闭SPI中断SPIE
	SPSR &= ~(1<<7);  // 清空中断标志SPIF
	// 设置SPI为主机模式
	SPCR |= (1<<4);  // 将SPCR控制寄存器中的MSTR置位，代表选择主机模式
	// 仅MISO设置为输入，其他均设为输出
 	DDRB |= (1<<4);  // ~SS，输出
	DDRB |= (1<<7);  // SCK，输出
	DDRB |= (1<<5);  // MOSI，输出
	DDRB &= ~(1<<6);  // MISO，输入
	PORTB &= ~(1<<4);  // 拉低SS电平，以开始数据传输
	SPCR |=(1<<0)|(1<<1);//128分频
	//SPCR |=(1<<6);//SPI使能和中断
}


void SPI_MasterTransmit(uchar rdata)
{
   // 将待传输的数据存放进SPDR数据寄存器中
   SPDR = rdata;  // rdata为485通过USART传进ATmega16中的数据，通过SPI发送给ATmega128
   while (!(SPSR & (1<<SPIF))); // 等待数据发送结束
}

void __attribute((signal))   __vector_10(void){//SPI中断

	char flag3;
	flag3=SPDR;
	  enQueue(&SPIbufQueue,flag3);//发送
}

/*void SPI_Puts(char *s)//发送字符串
{
	while (*s)
	{
		SPI_MasterTransmit(*s);
		s++;
	}
}*/