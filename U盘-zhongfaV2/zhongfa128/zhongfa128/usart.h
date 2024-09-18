/*
 * usart.h
 *
 * Created: 2022/9/1 星期四 15:31:46
 *  Author: Administrator
 */ 


#ifndef USART_H_
#define USART_H_
#define  uchar unsigned char
#define  uint  unsigned int
Queue UARTbufQueue;
void USART1_Init();
void USART2_Init();

void USART1_Transmit(uchar data);
void USART2_Transmit(uchar data);
void puts1(char *s);
void puts2(char *s);

 typedef struct SqQueue{
	 char base[100];
	 int front;
	 int rear;
 }Queue;

void initqueue(Queue *Q);

int enQueue(Queue *Q,char e);
int DeQueue( Queue *Q,char *e);
char rx_ok(char a,char b);
#endif /* USART_H_ */