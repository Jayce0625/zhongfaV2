/*
 * usart.c
 *
 * Created: 2022/9/1 星期四 15:31:37
 *  Author: Administrator
 */ 
// ATmega16A的晶振大小为16MHz
#include <avr/io.h>

#define F_CPU 16000000UL
#include <util/delay.h>
#include <usart.h>
#define  uchar unsigned char
#define  uint  unsigned int
Queue UARTbufQueue;
char a;
// rdata用于保存接收到的数据


void initqueue(Queue *Q){
	for(int i=0;i<100;i++){
		Q->base[i]=0;
	}
	Q->front=Q->rear=0;
}

int enQueue(Queue *Q,char e)
{
	if ((Q->rear+1)%100==Q->front) return 0;
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

void USART1_Init()//UART1初始化
{
	UCSR0A = 0X00;
	UCSR0B = 0X00;  // 控制寄存器清零
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);  // 异步，禁止校验，1位停止位，8位数据位
    UCSR0C &= ~(1<<USBS0);
	UBRR0L = 8;
	UBRR0H = 0;
	UCSR0B = (1 << RXCIE0)|(1 << RXEN0)|(1 << TXEN0);  // 接收、发送使能，接收结束中断使能
	//SREG = (1<<7);  // 全局中断开启
	DDRE |= 0X02;  // PE0和PE1是USART1的输出和接收端口
}
// USART2初始化
void USART2_Init()
{
	UCSR1A = 0X00;
	UCSR1B = 0X00;  // 控制寄存器清零
	UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);  // |(1<<USBS1)异步，禁止校验，1位停止位，8位数据位
	UCSR1C &= ~(1<<USBS1);//1位停止位，
	UBRR1L = 103;
	UBRR1H = 0;
	UCSR1B = (1 << RXCIE1)|(1 << RXEN1)|(1 << TXEN1);  // 接收、发送使能，接收结束中断使能
	//SREG |= (1<<7);  // 全局中断开启
	DDRD |= 0X08;  // PD2和PD3是USART2的输出和接收端口
	
}
void USART1_Transmit(uchar data)// USART1数据发送函数
{
	
	while(!(UCSR0A & ((1<<UDRE0))));
	UDR0 = data;
	
}

void USART2_Transmit(uchar data)// USART2数据发送函数
{
	while(!(UCSR1A & ((1<<UDRE1))));
	UDR1 = data;
}
void __attribute((signal))   __vector_18(void){//uart1接收中断
	char cdata0;
	cdata0=UDR0;
	enQueue(&UARTbufQueue,cdata0);//接受到的UART数据放在缓冲区
}



void __attribute((signal))   __vector_30(void){//uart2接收中断
	char cdata1;
	cdata1=UDR1;
	enQueue(&UARTbufQueue,cdata1);//接受到的UART数据放在缓冲区
}
void puts1(char *s)//发送字符串
{

	
	while(*s)
	{
		USART1_Transmit(*s);
		//*s=0;
		s++;
	}
}

void puts2(char *s)//发送字符串
{
	while(*s)
	{
		USART2_Transmit(*s);
		//*s=0;
		s++;
	}
	
}

/*
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
}*/





