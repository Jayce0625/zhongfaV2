/*
 * zhongfa128.c
 *
 * Created: 2022/9/1 星期四 15:13:04
 * Author: wjc
 */ 


#include <avr/io.h>
#include <avr/signal.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <avr/delay.h>
#include <usart.h>
#include <stdio.h>
#include <utils.h>
extern Queue SPIbufQueue;
extern Queue UARTbufQueue;
char ip[19],port[8],proxy_ip[19],proxy_port[8];  // 储存目标服务器的IPV4地址和端口号
char user[255],password[255];  // 储存Socks5代理协议的用户名和密码，根据协议规定用户名和密码最长均为255个字符
// socks5配置标志位，0失败，1成功
int socks5_ok_flag = 0;

int main(void)
{
    delay(2000);
	SREG &= (~(1<<7));  // 全局中断关闭
    Led_Init();  // LED灯初始化,黄灯常量，红绿亮灯不亮
    SPI_Init();  // SPI通信初始化
	// 拨动按钮初始化
	DDRE  &= ~(1<<7);  // PE7输入
	PORTE |= (1<<7);  // PE7输出高电平
	// 独立按键初始化
	DDRE  &= ~(1<<3);  // PE7输入
	PORTE |= (1<<3);  // PE7输出高电平
	// 初始化USART
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
	int ipnum=0,portnum=0,proxy_ipnum=0,proxy_portnum=0,charnum=0;  //字符串位数、特殊字符数量
	int usernum=0,passwdnum=0;
	char address=0x00; 
	char addressread=0x00; 
	char char1;
	SREG |= (1<<7);  // 全局中断开启
	
    while(1)
    {	
		delay(100);
		key = PINE & (1<<3);
		dianping=PINE&(1<<7);
		if (!key)  // ----------------------------------------------------如果按下配网按钮，开始配网
		{
			// --------------------------------------------灯光
			LedGreenOff();
			LedRedOff();
            wifi_ok_flag = 0;
			tcp_ok_flag = 0;
            LedYellowOff();
			delay(1000);
			LedYellowOn();
			puts1("+++");
			delay(2000);
			
			// -------------------------------------------从EEPROM读取字符
			addressread = 0x00;
			
			char1=EEPROM_READ(addressread++);
			// WIFI模式，写六个字段读六个字段
			if (dianping) 
			{
				while (charnum<6)  //读完停止
				{
					if (char1=='@'){charnum++;}
					else
					{
						if(charnum==0) {ip[ipnum++]=char1;}
						else if(charnum==1) {port[portnum++]=char1;}
						else if(charnum==2) {proxy_ip[proxy_ipnum++]=char1;}
						else if(charnum==3) {proxy_port[proxy_portnum++]=char1;}
						else if(charnum==4) {user[usernum++]=char1;}
						else {password[passwdnum++]=char1;}
					}
					char1=EEPROM_READ(addressread++);
				}
			}
			// 4G模式，写两个字段读两个字段
			else 
			{
				while (charnum<2)  //读完停止
				{
					if (char1=='@'){charnum++;}
					else
					{
						if(charnum==0) {ip[ipnum++]=char1;}
						else {port[portnum++]=char1;}
					}
					char1=EEPROM_READ(addressread++);
				}
			}
			ip[ipnum]='\0';
			port[portnum]='\0';
			proxy_ip[proxy_ipnum]='\0';
			proxy_port[proxy_portnum]='\0';
			user[usernum]='\0';
			password[passwdnum]='\0';
			addressread=0x00;
			ipnum=0;portnum=0;proxy_ipnum=0;proxy_portnum=0;charnum=0;usernum=0;passwdnum=0;
			if (dianping)
			{
				SPIputs(ip);SPIputs("@");
				SPIputs(port);SPIputs("@");
				SPIputs(proxy_ip);SPIputs("@");
				SPIputs(proxy_port);SPIputs("@");
				SPIputs(user);SPIputs("@");
				SPIputs(password);SPIputs("@");
			}
			else 
			{
				SPIputs(ip);SPIputs("@");
				SPIputs(port);SPIputs("@");
			}
			
			if(dianping)  // --------------------------------------------------------高电平配置WiFi
			{	
				while (DeQueue(&UARTbufQueue,&c));  // 排空队列
				esp8266_send_at_cmd("AT+CIPSTATUS");
				while (!DeQueue(&UARTbufQueue,&c));  // 等待队列非空
			    a=b;
				b=c;
				// 每一个SPI通信流程均是，从机初始化，主机初始化，从机初始化。想发送数据先设置为主机再拉高SS(SPI_MasterInitInit())，发送完之后再恢复成从机并拉低SS(SPI_Init)
				// 谁发谁是主，默认全为从，在发送结束后重置为从机
				SPI_MasterInitInit();  // SPI主机初始化，使能SPI中断
				SPI_MasterTransmit(c);  // SPI通信传输数据
				SPI_Init();  // SPI从机初始化，使能SPI中断
				while (DeQueue(&UARTbufQueue,&c))
				{
					SPI_MasterInitInit();
					SPI_MasterTransmit(c);
					SPI_Init();
					if (((a == 'S')&(b == ':')&(c == '2'))|((a == 'S')&(b == ':')&(c == '4')))  // STATUS:2代表已连接AP, STATUS:4代表之前建立的TCP或UDP连接已断开
					{
						wifi_ok_flag = 1;
					}
					else if ((a == 'S')&(b == ':')&(c == '3'))  // STATUS:3代表TCP或UDP连接已建立
					{
						tcp_ok_flag = 1;
					}
					a = b;
					b = c;
				}
				if (tcp_ok_flag)  // -------------------------------------------------------------- 若TCP连接已存在则直接开启透传并建立Socks5代理连接
				{
					LedGreenOn();
					while (DeQueue(&UARTbufQueue,&f));  // 排空队列
					esp8266_send_at_cmd("AT+CIPMODE=1");  // 开始透传
					while (!DeQueue(&UARTbufQueue,&f));
					e=f;
					SPI_MasterInitInit();
					SPI_MasterTransmit(f);
					SPI_Init();
					if (rx_ok(e,f))
					{
						while (DeQueue(&UARTbufQueue,&f));  // 排空队列
						esp8266_send_at_cmd("AT+CIPSEND");  // 透传模式开始传送数据
						while (!DeQueue(&UARTbufQueue,&f));
						e=f;
						SPI_MasterInitInit();
						SPI_MasterTransmit(f);
						SPI_Init();
						if (rx_ok(e,f))
						{
							while (!DeQueue(&UARTbufQueue,&f));
							SPI_MasterInitInit();
							SPI_MasterTransmit(f);
							SPI_Init();
							if (rx_enter(f))
							{
								if (socks5_ok_flag != 1)
								{
									// 此时与Socks5代理服务器的TCP连接已经建立，但认证失败需重新认证
									socks5_init(e,f);
									if (socks5_ok_flag != 1) {  // 失败
										SPIputs("Socks5 proxy service failed to build!");
									} else {  // 成功
										LedRedOn();
										SPIputs("\r\nSocks5 proxy service build succeeded & wifimodule ok!\r\n");
										puts1("wifimodule ok");
										SPCR &= ~(1<<7);  // 关闭SPI中断SPIE
										while (DeQueue(&SPIbufQueue,&f));
										SPCR |= (1<<7);  // 开启SPI中断SPIE
									}
								} else {
									LedRedOn();
									SPIputs("\r\nSocks5 proxy service build succeeded & wifimodule ok!\r\n");
									puts1("wifimodule ok");
									SPCR &= ~(1<<7);  // 关闭SPI中断SPIE
									while (DeQueue(&SPIbufQueue,&f));
									SPCR |= (1<<7);  // 开启SPI中断SPIE
								}
							}	
						}
					}
				}
				else
				{
					// ----------------------------------------------------------查询wifi是否连接成功
					if (!wifi_ok_flag)// ------------------------------------------若wifi连接不成功，则从头配置
					{
						while (DeQueue(&UARTbufQueue,&f));  // 排空队列
						esp8266_send_at_cmd("AT+CWMODE=1");  // 8266设置为STA模式
						while (!DeQueue(&UARTbufQueue,&f));  // 等待回复
						e=f;
						SPI_MasterInitInit();
						SPI_MasterTransmit(f);
						SPI_Init();
						if (rx_ok(e,f))
						{
							while (DeQueue(&UARTbufQueue,&f));  // 排空
							esp8266_send_at_cmd("AT+CWAUTOCONN=1");  // 上电自动连接AP
							while (!DeQueue(&UARTbufQueue,&f));
							e=f;
							SPI_MasterInitInit();
							SPI_MasterTransmit(f);
							SPI_Init();
						    //while(1);
							if (rx_ok(e,f))
							{
								while (DeQueue(&UARTbufQueue,&f));  // 排空
								esp8266_send_at_cmd("AT+CWSTARTSMART=3");  // 开启ESP-TOUCH+Airkiss配网
								while (!DeQueue(&UARTbufQueue,&f));
								e=f;
								SPI_MasterInitInit();
								SPI_MasterTransmit(f);
								SPI_Init();
								//while(1);
								if (rx_ok(e,f))
								{
									while (DeQueue(&UARTbufQueue,&f));  // 排空
									esp8266_send_at_cmd("AT+CWSTARTDISCOVER=\"gh_4248324a4d02\",\"aithinker\",10");  // 开启可被局域⽹内微信探测模式
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
			
					if (wifi_ok_flag)  // -------------------------------------------------------------- 若wifi已配好开始配置服务器
					{
						set_server(e,f);
					}
				}
			}
			
			else  // ----------------------------------------------------------------------------------低电平配置4G
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
		
		// -------------------------------------------------------------------------发送给ATmega16
		if(UARTbufQueue.front!=UARTbufQueue.rear){
			SPI_MasterInitInit();
			while(UARTbufQueue.front!=UARTbufQueue.rear)
			{ 
				DeQueue(&UARTbufQueue,&d);
				//USART2_Transmit(d);
				SPI_MasterTransmit(d);
				//delay(10);
			}
			d = SPDR;  // 读取SPDR保证SPIF不会自动置位
		   	SPI_Init();
		}
	
	
		// ------------------------------------------------------------------------------------发给4G/WiFi/存入EEPROM
		flagnum = 0;
		flag = 0;
		while (DeQueue(&SPIbufQueue,&c))
		{
			// 发送
			dianping = PINE & (1<<7);
			if(dianping)  // 高电平
			{
				USART1_Transmit(c);  // 发给WiFi
			}
			else
			{
				USART2_Transmit(c);  // 发给4G
			}
			if (flag)
			{
				EEPROM_WRITE(address++,c);
				if (c == '@')
				{
					flagnum++;
					flag = 0;
					
					if (dianping && flagnum == 6)
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
					if (!dianping && flagnum == 2)
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
			
			// IP:112.125.89.8@PORT:33232@ip:192.168.1.110@port:1080@USER:Jayce@PASSWD:114834766@  大写代表目标服务器IP地址和端口号，小写代表代理服务器IP地址和端口号
			if ((a == 'I') & (b == 'P') & (c == ':')) flag = 1;
			if ((a == 'R') & (b == 'T') & (c == ':')) flag = 1;
			if ((a == 'i') & (b == 'p') & (c == ':')) flag = 1;
			if ((a == 'r') & (b == 't') & (c == ':')) flag = 1;
			if ((a == 'E') & (b == 'R') & (c == ':')) flag = 1;
			if ((a == 'W') & (b == 'D') & (c == ':')) flag = 1;
			a = b;
			b = c;
		}
    }
}