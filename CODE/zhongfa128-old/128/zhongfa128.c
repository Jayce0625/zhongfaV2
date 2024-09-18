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
char ip[19],port[8];  //WiFi密码最多16位，WiFi名字英文最多32位，中文10位，即20个ASIC码


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
	int tcp_ok_flag;
	//char ip[19],port[8];  //WiFi密码最多16位，WiFi名字英文最多32位，中文10位，即20个ASIC码
	int ipnum=0,portnum=0,charnum=0;  //字符串位数、特殊字符数量
	char address=0x00; 
	char addressread=0x00; 
	char char1;
	SREG |= (1<<7);  // 全局中断开启
	//ip[0]='"';
	//ip[1]=',';
	//ip[2]='"';
	
    while(1)
    {	
		delay(100);
		key = PINE & (1<<3);
		if (!key)  // ----------------------------------------------------如果按下配网按钮，开始配网
		{
			//--------------------------------------------灯光
			LedGreenOff();
			LedRedOff();
            wifi_ok_flag = 0;
			tcp_ok_flag = 0;
            LedYellowOff();
			delay(1000);
			LedYellowOn();
			puts1("+++");
			delay(2000);
			
			
			//-------------------------------------------从EEPROM读取字符
			addressread = 0X00;
			char1=EEPROM_READ(addressread++);
		    while (charnum<2)  //读完停止
			{
				if (char1=='@'){charnum++;}
				else
				{
					if(charnum==0){ip[ipnum++]=char1;}
					else{port[portnum++]=char1;}
				}
				char1=EEPROM_READ(addressread++);
				//USART1_Transmit(char1);
			}
			ip[ipnum]='\0';
			port[portnum]='\0';
			addressread=0x00;
			ipnum=0;portnum=0;charnum=0;
			SPIputs(ip);
			SPIputs(port);
			
			dianping=PINE&(1<<7);
			if(dianping)//--------------------------------------------------------高电平配置WiFi
			{	
				while (DeQueue(&UARTbufQueue,&c)); //排空队列
				puts1("AT+CIPSTATUS\r\n");
				delay(1000);
				while (!DeQueue(&UARTbufQueue,&c)); //等待队列非空
			    a=b;
				b=c;
				SPI_MasterInitInit();
				SPI_MasterTransmit(c);
				SPI_Init();
				while (DeQueue(&UARTbufQueue,&c))
				{
					SPI_MasterInitInit();
					SPI_MasterTransmit(c);
					SPI_Init();
					if (((a == 'S')&(b == ':')&(c == '2'))|((a == 'S')&(b == ':')&(c == '4')))
					{
						wifi_ok_flag = 1;
					}
					else if ((a == 'S')&(b == ':')&(c == '3')) 
					{
						tcp_ok_flag = 1;
					}
					a = b;
					b = c;
				}
				if (tcp_ok_flag)  //-------------------------------------------------------------- 若TCP连接已存在则直接开启透传
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
						LedGreenOn();
						LedRedOn();
						puts1("wifimodule ok");
						SPCR &= ~(1<<7);  // 关闭SPI中断SPIE
						while (DeQueue(&SPIbufQueue,&f)); 
						SPCR |= (1<<7);  // 开启SPI中断SPIE
					}
				}
				else
				{
					//----------------------------------------------------------查询wifi是否连接成功
					if (!wifi_ok_flag)//------------------------------------------若wifi连接不成功，则从头配置
					{
						while (DeQueue(&UARTbufQueue,&f)); //排空队列
						puts1("AT+CWMODE=1\r\n");  // 8266设置为STA模式
						delay(1000);
						while (!DeQueue(&UARTbufQueue,&f));//等待非空
						e=f;
						SPI_MasterInitInit();
						SPI_MasterTransmit(f);
						SPI_Init();
						if (rx_ok(e,f))
						{
							while (DeQueue(&UARTbufQueue,&f));//排空
							puts1("AT+CWAUTOCONN=1\r\n");  //上电自动连接AP
							delay(1000);
							while (!DeQueue(&UARTbufQueue,&f));
							e=f;
							SPI_MasterInitInit();
							SPI_MasterTransmit(f);
							SPI_Init();
						    //while(1);
							if (rx_ok(e,f))
							{
								while (DeQueue(&UARTbufQueue,&f));//排空
								puts1("AT+CWSTARTSMART=3\r\n");   //开启ESP-TOUCH+Airkiss配网
								delay(1000);
								while (!DeQueue(&UARTbufQueue,&f));
								e=f;
								SPI_MasterInitInit();
								SPI_MasterTransmit(f);
								SPI_Init();
								//while(1);
								if (rx_ok(e,f))
								{
									while (DeQueue(&UARTbufQueue,&f));//排空
									puts1("AT+CWSTARTDISCOVER=\"gh_4248324a4d02\",\"aithinker\",10\r\n");
									delay(1000);
									while (!DeQueue(&UARTbufQueue,&c));  // 等待回复
									a=b;
									b=c;
									SPI_MasterInitInit();
									SPI_MasterTransmit(c);
									SPI_Init();
									while (1)  // 因为此时会回传多条信息，为防止时差过长，设死循环
									{   
										while (DeQueue(&UARTbufQueue,&c))
										{
											SPI_MasterInitInit();
											SPI_MasterTransmit(c);
											SPI_Init();
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
					}
			
			
					if (wifi_ok_flag)  //-------------------------------------------------------------- 若wifi已配好开始配置服务器
					{
						set_server(e,f);
					}
				}				
				
				
			}
			
			else//----------------------------------------------------------------------------------低电平配置4G
			{
				while (DeQueue(&UARTbufQueue,&f)); 
				puts2("config,set,paramsrc,1\r\n");
				delay(1000);
				while (!DeQueue(&UARTbufQueue,&f));
				e=f;
				SPI_MasterInitInit();
				SPI_MasterTransmit(f);
				SPI_Init();
				if (rx_smallok(e,f))
				{
					while (DeQueue(&UARTbufQueue,&f)); 
				    puts2("config,set,tcp,1,ttluart,1,1,,1,");
				    puts2(ip);
				    puts2(",");
				    puts2(port);
				    puts2(",0,0,0,0,0,0\r\n");
				    delay(1000);
				    while (!DeQueue(&UARTbufQueue,&f));
				    e=f;
				    SPI_MasterInitInit();
				    SPI_MasterTransmit(f);
				    SPI_Init();
					if (rx_smallok(e,f)){
						while (DeQueue(&UARTbufQueue,&f)); 
				        puts2("config,set,save\r\n");
				        delay(1000);
				        while (!DeQueue(&UARTbufQueue,&f));
				        e=f;
				        SPI_MasterInitInit();
				        SPI_MasterTransmit(f);
				        SPI_Init();
						if (rx_smallok(e,f))
						{
							delay(5000);
						
				            puts2("4Gmodule ok");
							SPCR &= ~(1<<7);  // 关闭SPI中断SPIE
						    while (DeQueue(&SPIbufQueue,&f)); 
						    SPCR |= (1<<7);  // 开启SPI中断SPIE
							LedGreenOn();
				            LedRedOn();
						}
					   
					}
				}
			}
		}
		
		//-------------------------------------------------------------------------发送给16
		if(UARTbufQueue.front!=UARTbufQueue.rear){
			SPI_MasterInitInit();
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
	
	
		//------------------------------------------------------------------------------------发给4G/wifi
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
						address=0x00;
						flagnum=0;
						LedYellowOff();
						delay(500);
						LedYellowOn();
						delay(500);
						LedYellowOff();
						delay(500);
						LedYellowOn();
						delay(500);
						LedYellowOff();
						delay(500);
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







      