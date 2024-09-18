/*
 * zhongfa128.c
 *
 * Created: 2022/9/1 星期四 15:13:04
 *  Author: Administrator
 */ 


#include <avr/io.h>
#include <avr/signal.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <avr/delay.h>
#include <usart.h>
extern Queue SPIbufQueue;
extern Queue UARTbufQueue;


int main(void)
{
    delay(2000);
	SREG &= (~(1<<7));  // 全局中断关闭
    Led_Init();  // LED灯初始化,黄灯常量，红绿亮灯不亮
    SPI_Init();  // SPI通信初始化
	//拨动按钮初始化
	DDRE  &= ~(1<<7);  // PE7输入
	PORTE |= (1<<7);  // PE7输出高电平
	//独立按键初始化
	DDRE  &= ~(1<<3);  // PE7输入
	PORTE |= (1<<3);  // PE7输出高电平
	//SPI_MasterInitInit();
	USART1_Init();
    USART2_Init();
    initqueue(&UARTbufQueue);
	initqueue(&SPIbufQueue);
	char dianping,key;
    // 协议标志位
    char a = 0;
    char b = 0;
	char c = 0;
	char d;
	char e = 0;
	char f = 0;
	int flagnum,flag;  // 存EEPROM标志位
	int wifi_ok_flag;  // WIFI是否配置成功标志位
	char ip[19],port[8];  //WiFi密码最多16位，WiFi名字英文最多32位，中文10位，即20个ASIC码
	int ipnum=0,portnum=0,charnum=0;  //字符串位数、特殊字符数量
	char address=0x00; 
	char char1;
	SREG |= (1<<7);  // 全局中断开启
	//ip[0]='"';
	//ip[1]=',';
	//ip[2]='"';
	
    while(1)
    {	
		delay(100);
		key = PINE & (1<<3);
		if (!key)  // 如果按下配网按钮，电平为低
		{
			
			address = 0X00;
			//从EEPROM读取字符
			char1=EEPROM_READ(address++);
			
		    while (charnum<2)  //读完停止
			{
				if (char1=='@'){charnum++;}
				else
				{
					if(charnum==0){ip[ipnum++]=char1;}
					else{port[portnum++]=char1;}
				}
				char1=EEPROM_READ(address++);
				//USART1_Transmit(char1);
			}
			ip[ipnum]='\0';
			port[portnum]='\0';
			
			address=0x00;
			ipnum=0;portnum=0;charnum=0;
			dianping=PINE&(1<<7);
			if(dianping)//高电平配置WiFi
			{	
				//puts1("AT\r\n");
				//delay(10000);
				wifi_ok_flag = 0;
				
				puts1("AT+CIPSTATUS");
				while (!DeQueue(&UARTbufQueue,&c));  // 等待WIFI回复
			    a=b,b=c;
				SPI_MasterTransmit(c);
				while (DeQueue(&UARTbufQueue,&c))
				{
					SPI_MasterTransmit(c);
					if ((a == 'S')&(b == ':')&(c == '2'))
					{
						wifi_ok_flag = 1;
						// break;   // 不是死循环，无需break，正好将其回复的信息完全传回PC？？？？？
					}
					a = b;
					b = c;
				}
				
				if (wifi_ok_flag)  // 若wifi已配好
				{
					LedGreenOn();
					
					puts1("AT+CIPMUX=0\r\n");
					while (!DeQueue(&UARTbufQueue,&f));
					e=f;
					SPI_MasterTransmit(f);
					if (rx_ok(e,f))
					{
						puts1("AT+CIPSTART=\"TCP\",");
						USART1_Transmit(0x22);
						puts1(ip);
						USART1_Transmit(0x22);
						USART1_Transmit(',');
						puts1(port);
						USART1_Transmit('\r');
						USART1_Transmit('\n');
						while (!DeQueue(&UARTbufQueue,&f));
						e=f;
						SPI_MasterTransmit(f);
						if (rx_ok(e,f))
						{
							puts1("AT+CIPMODE=1\r\n");
							while (!DeQueue(&UARTbufQueue,&f));
							e=f;
							SPI_MasterTransmit(f);
							if (rx_ok(e,f))
							{
								puts1("AT+CIPSEND\r\n");
								while (!DeQueue(&UARTbufQueue,&f));
								e=f;
								SPI_MasterTransmit(f);
								if (rx_ok(e,f))
								{
									LedRedOn();
									puts1("wifimodule ok");
								}
							}
						}
					}
				}
				else  // 从头配网
				{
					puts1("AT+CWMODE=1\r\n");  // 8266设置为STA模式
					while (!DeQueue(&UARTbufQueue,&f));
					e=f;
					SPI_MasterTransmit(f);
					if (rx_ok(e,f))
					{
						puts1("AT+CWAUTOCONN=1\r\n");  //上电自动连接AP
						while (!DeQueue(&UARTbufQueue,&f));
						e=f;
						SPI_MasterTransmit(f);
						if (rx_ok(e,f))
						{
							puts1("AT+CWSTARTSMART=3\r\n");   //开启ESP-TOUCH+Airkiss配网
							while (!DeQueue(&UARTbufQueue,&f));
							e=f;
							SPI_MasterTransmit(f);
							if (rx_ok(e,f))
							{
								puts1("AT+CWSTARTDISCOVER="gh_4248324a4d02","aithinker",10");
								while (!DeQueue(&UARTbufQueue,&c));  // 等待回复
								a=b,b=c;
								SPI_MasterTransmit(c);
								while (1)  // 因为此时会回传多条信息，为防止时差过长，设死循环
								{
									while (DeQueue(&UARTbufQueue,&c))
									{
										SPI_MasterTransmit(c);
										if ((a == 't')&(b == 'e')&(c == 'd'))
										{
											wifi_ok_flag = 1;
										}
										a = b;
										b = c;
									}
									if (wifi_ok_flag) break;
								}
							}
						}
					}
					if (wifi_ok_flag)  // 微信配网成功，开始配置服务器
					{
						LedGreenOn();
						
						puts1("AT+CIPMUX=0\r\n");
						while (!DeQueue(&UARTbufQueue,&f));
						e=f;
						SPI_MasterTransmit(f);
						if (rx_ok(e,f))
						{
							puts1("AT+CIPSTART=\"TCP\",");
							USART1_Transmit(0x22);
							puts1(ip);
							USART1_Transmit(0x22);
							USART1_Transmit(',');
							puts1(port);
							USART1_Transmit('\r');
							USART1_Transmit('\n');
							while (!DeQueue(&UARTbufQueue,&f));
							e=f;
							SPI_MasterTransmit(f);
							if (rx_ok(e,f))
							{
								puts1("AT+CIPMODE=1\r\n");
								while (!DeQueue(&UARTbufQueue,&f));
								e=f;
								SPI_MasterTransmit(f);
								if (rx_ok(e,f))
								{
									puts1("AT+CIPSEND\r\n");
									while (!DeQueue(&UARTbufQueue,&f));
									e=f;
									SPI_MasterTransmit(f);
									if (rx_ok(e,f))
									{
										LedRedOn();
										puts1("wifimodule ok");
									}
								}
							}
						}
					}
				}
			}
			else//低电平配置4G
			{
				puts2("config,set,paramsrc,1\r\n");
				delay(2000);
				puts2("config,set,tcp,1,ttluart,1,1,,1,");
				puts2(ip);
				puts2(",");
				puts2(port);
				puts2(",0,0,0,0,0,0\r\n");
				delay(2000);
				puts2("config,set,save\r\n");
				delay(2000);
				delay(2000);
				puts2("4Gmodule ok");
				LedGreenOn();
				LedRedOn();
			}
		}
		
		//发送给16
		
		if(UARTbufQueue.front!=UARTbufQueue.rear){
			while(!(PORTB&(1<<0)));//等待SS电平为高
			delay(10);
			PORTB &= (~(1<<0));// 拉低ss
			SPCR &= ~(1<<7);  // 关闭SPI中断SPIE
			//delay(1000);
			//SREG&=~1<<7;
			SPI_MasterInitInit();
			delay(10);
			while(UARTbufQueue.front!=UARTbufQueue.rear)
			{ 
				DeQueue(&UARTbufQueue,&d);
				//USART2_Transmit(d);
				SPI_MasterTransmit(d);
				//delay(10);
			}
			d = SPDR; //读取SPDR保证SPIF不会自动置位
		   	SPI_Init();
		}
	  
		//发给4G/wifi
		flagnum = 0;
		flag = 0;
		while (DeQueue(&SPIbufQueue,&c))
		{
			//发送
			dianping = PINE & (1<<7);
			if(dianping)//高电平
			{
				USART1_Transmit(c);
			}
			else
			{
				USART2_Transmit(c);//发给4G
			}
			if (flag)
			{
				EEPROM_WRITE(address++,c);
				if (c == '@')
				{
					flagnum++;
					flag = 0;
					
					if (flagnum == 2)
					{
						LedYellowOff();
						delay(1000);
						LedYellowOn();
						delay(1000);
						LedYellowOff();
						delay(1000);
						LedYellowOn();
						delay(1000);
						LedYellowOff();
						delay(1000);
						LedYellowOn();
					}
				}
			}
			
			if ((a == 'I') & (b == 'P') & (c == ':')) flag = 1;
			if ((a == 'R') & (b == 'T') & (c == ':')) flag = 1;
			a = b;
			b = c;
		}
    }
}







      