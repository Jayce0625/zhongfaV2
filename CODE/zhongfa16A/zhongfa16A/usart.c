/*
 * usart.c
 *
 * Created: 2022/9/1 星期四 14:38:56
 *  Author: Administrator
 */ 
#include <avr/io.h>
#include <avr/signal.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>
#include <usart.h>
#define  MCLK  16000000// ATmega16A的晶振大小为16MHz


Queue bufQueue;
void initqueue(Queue *Q){
	for(int i=0;i<100;i++){
		Q->base[i]=0;
	}
	Q->front=Q->rear=0;
}

int enQueue(Queue *Q,char e)
{
	if ((Q->rear+1)%100==Q->front)
	return 0;
	Q->base[Q->rear]=e;
	Q->rear=(Q->rear+1)%100;
	return 1;
}
int DeQueue(Queue *Q,char *e){
	if(Q->front==Q->rear) return 0;
	*e=Q->base[Q->front];
	Q->front=(Q->front+1)%100;
	return 1;
}
void delay(uint ms)
{
	uint i,j;
	for(i=0;i<ms;i++)
	{
		for(j=0;j<1141;j++);
	}
}

// 异步串行通信初始化
void USART_Init()
{
	UCSRB = 0X00;
	UCSRA = 0X00;  // 控制寄存器清零
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);  // 选择UCSRC寄存器、8位数据位
	UCSRC &=~(1<<USBS);//1位停止位，
	UBRRL = 3;  // 波特率2400低八位存入UBRRL
	UBRRH = 0;  // 剩余高位存入UBRRH
	UCSRB = (1<<TXEN)|(1<<RXEN)|(1<<RXCIE);  // 接受、发送使能，接收结束中断使能
	SREG |= (1<<7);  // 全局中断开放
	DDRD |= 0X02;  // PD0和PD1分别是串口输入和串口输出，配置TX为输出RX为输入

	/*	bufnum=0;for (int i = 0; i < 100 ; i++)
	dbgBuf[i]=0X00;*/
	//USART_Transmit(UDR);
	
}

// 数据发送函数
void USART_Transmit(uchar data)
{
	while(!(UCSRA & (1<<UDRE)));  // 查询数据寄存器是否为空
	UDR = data;  // 将数据写入UDR数据寄存器
	//led2flashing();//red
}

// 接收结束中断函数
void __attribute((signal))   __vector_11(void)
{
	char cdata;
	cdata=UDR;
	enQueue(&bufQueue,cdata);
}

void puts0(char *s)//发送字符串
{
	while(*s)
	{
		USART_Transmit(*s);
		*s=0;
		s++;
	}
	
}

