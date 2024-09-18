/*
 * usart.h
 *
 * Created: 2022/9/1 星期四 15:31:46
 *  Author: wjc
 */ 


#ifndef USART_H_
#define USART_H_
#define  uchar unsigned char
#define  uint  unsigned int

void USART1_Init();
void USART2_Init();

void USART1_Transmit(uchar data);
void USART2_Transmit(uchar data);
void USART1_Send_Bytes(uchar *data, int length);
void puts1(char *s);
void puts2(char *s);
// char USART1_Receive(void);
// void USART1_Flush(void);
// void USART2_Flush(void);

 typedef struct SqQueue{
	 char base[100];
	 int front;
	 int rear;
 }Queue;

void initqueue(Queue *Q);

int enQueue(Queue *Q,char e);
int DeQueue( Queue *Q,char *e);
#endif /* USART_H_ */