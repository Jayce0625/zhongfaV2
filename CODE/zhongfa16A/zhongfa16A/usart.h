/*
 * usart.h
 *
 * Created: 2022/9/1 星期四 14:59:35
 *  Author: Administrator
 */ 


#ifndef USART_H_
#define USART_H_
#define  uchar unsigned char
#define  uint  unsigned int
#define  MAX  1000
void delay(uint ms);
void USART_Init();
void USART_Transmit(uchar data);
void USART_rx(void);
void puts0(char *s);
 typedef struct SqQueue{
	 char base[100];
	 int front;
	 int rear;
 }Queue;

 void initqueue(Queue *Q);

 int enQueue(Queue *Q,char e);
 int DeQueue( Queue *Q,char *e);
#endif /* USART_H_ */