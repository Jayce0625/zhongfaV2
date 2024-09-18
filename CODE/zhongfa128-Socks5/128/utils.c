/*
 * utils.c
 *
 * Created: 2022/9/29 18:44:22
 *  Author: wjc
 */ 


#include <avr/io.h>
#include <avr/signal.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <avr/delay.h>
#include <usart.h>
#include <utils.h>
#include <string.h>
#include <stdlib.h>
extern Queue SPIbufQueue;
extern Queue UARTbufQueue;
// 目标服务器的IP地址和端口号
extern char ip[19],port[8],proxy_ip[19],proxy_port[8],user[255],password[255];
/*// CCProxy代理服务器IP地址和端口号
#define PROXY_SERVER_IP "10.120.235.33"
#define PROXY_SERVER_PORT 1080*/
/*// CCProxy代理服务器用户名和密码
#define PROXY_USERNAME "Jayce"
#define PROXY_PASSWORD "114834766"*/
// ESP8266模块的AT指令缓存
char esp8266_at_buf[256];
// ESP8266模块接收缓存
uchar usart_recv_buf[256];
// socks5配置标志位，0失败，1成功
extern int socks5_ok_flag;

// 匹配返回值是否为大写OK
char rx_ok(char a,char b)
{
	while (DeQueue(&UARTbufQueue,&b))
	{
		SPI_MasterInitInit();
		SPI_MasterTransmit(b);
		SPI_Init();
		if ((a == 'O') && (b == 'K'))
		{
			return 1;
		}
		a = b;
	}
	return 0;
}

// 匹配返回值是否为小写ok
char rx_smallok(char a,char b)
{
	while (DeQueue(&UARTbufQueue,&b))
	{
		SPI_MasterInitInit();
		SPI_MasterTransmit(b);
		SPI_Init();
		if ((a == 'o') && (b == 'k'))
		{
			return 1;
		}
		a = b;
	}
	return 0;
}

// 匹配返回值是否为'>'，>为允许发送数据的标志
char rx_enter(char a)
{
	if (a == '>')
	{
		return 1;
	}
	else
	{
		while (DeQueue(&UARTbufQueue,&a))
		{
			SPI_MasterInitInit();
			SPI_MasterTransmit(a);
			SPI_Init();
			if (a == '>')
			{
				return 1;
			}
		}
	}
	return 0;
}

// 发送AT指令并在其后自动补上另起新行，并等待返回值
void esp8266_send_at_cmd(char *cmd)
{
	// USART1_Flush();
	puts1(cmd);
	USART1_Transmit('\r');
	USART1_Transmit('\n');
	delay(1000);
}

// 配置Socks5代理
void socks5_init(char e, char f)
{
	// 发送Socks5协议版本和认证方法等信息
	uchar socks5_auth_methods[] = {0x05,0x02,0x00,0x02};
	USART1_Send_Bytes(socks5_auth_methods,sizeof(socks5_auth_methods));
	delay(1000);  // 等待Socks5服务器的回复
	while (!DeQueue(&UARTbufQueue,&f));  // 等待队列非空
	e=f;
	DeQueue(&UARTbufQueue,&f);
	SPI_MasterInitInit();
	SPI_MasterTransmit(e);  // 0x05
	SPI_MasterTransmit(f);  // 0x02
	SPI_Init();
	if ((e == 0x05) && (f == 0x02)) {
		// 发送用于认证的用户名和密码
		uchar socks5_auth_req[] = {0x01,strlen(user)};
		uchar socks5_auth_req2[] = {strlen(password)};
		USART1_Send_Bytes(socks5_auth_req,sizeof(socks5_auth_req));
		USART1_Send_Bytes((uchar *)user,strlen(user));
		USART1_Send_Bytes(socks5_auth_req2,sizeof(socks5_auth_req2));
		USART1_Send_Bytes((uchar *)password,strlen(password));
		delay(1000);  // 等待Socks5服务器的回复
		while (!DeQueue(&UARTbufQueue,&f));  // 等待队列非空
		e=f;
		DeQueue(&UARTbufQueue,&f);
		SPI_MasterInitInit();
		SPI_MasterTransmit(e);  // 0x01
		SPI_MasterTransmit(f);  // 0x00
		SPI_Init();
		// 若返回的STATUS不为0x00则代表鉴权失败
		if ((e == 0x01) && (f == 0x00)) {
			// 发送真实目标服务器IP地址和端口号等信息
			uchar socks5_auth_server[] = {0x05, 0x01, 0x00, 0x01};
			// 将字符串IPV4地址转换为共占4个字节的整型数值
			uchar a,b,c,d;
			sscanf(ip,"%hhu.%hhu.%hhu.%hhu",&a,&b,&c,&d);
			uchar true_ipv4_hex[] = {a,b,c,d};
			// 将字符串端口号转换为共占2个字节的整型数值
			uint true_port = atoi(port);  // 将字符串类型的端口号转换为整形数值
			uchar true_port_hex[] = {(true_port >> 8),(true_port & 0xFF)};  // 十六进制端口号大端在前小端在后
			USART1_Send_Bytes(socks5_auth_server,sizeof(socks5_auth_server));
			USART1_Send_Bytes(true_ipv4_hex,sizeof(true_ipv4_hex));
			USART1_Send_Bytes(true_port_hex,sizeof(true_port_hex));
			delay(1000);  // 等待Socks5服务器的回复
			while (!DeQueue(&UARTbufQueue,&f));  // 等待队列非空
			e=f;
			DeQueue(&UARTbufQueue,&f);
			SPI_MasterInitInit();
			SPI_MasterTransmit(e);  // 0x05
			SPI_MasterTransmit(f);  // 0x00
			SPI_Init();
			if ((e == 0x05) && (f == 0x00)) {
				socks5_ok_flag = 1;  // 成功标志位置1
				while (DeQueue(&UARTbufQueue,&f));  // 排空队列
			}
		}
	}
}

// 配置服务器，包含代理服务器和目标服务器
void set_server(char e, char f)
{
	LedGreenOn();
	while (DeQueue(&UARTbufQueue,&f)); // 排空队列
	esp8266_send_at_cmd("AT+CIPMUX=0");
	while (!DeQueue(&UARTbufQueue,&f));  // 等待缓冲区内有数据
	e=f;
	SPI_MasterInitInit();
	SPI_MasterTransmit(f);
	SPI_Init();
	if (rx_ok(e,f))
	{
		while (DeQueue(&UARTbufQueue,&f)); // 排空队列
		sprintf((char *)esp8266_at_buf, "AT+CIPSTART=\"TCP\",\"%s\",%s", proxy_ip, proxy_port);  // TCP链接代理服务器IP地址和对应端口号
		esp8266_send_at_cmd(esp8266_at_buf);
		while (!DeQueue(&UARTbufQueue,&f));  // 等待队列非空
		e=f;
		SPI_MasterInitInit();
		SPI_MasterTransmit(f);
		SPI_Init();
		if (rx_ok(e,f))
		{
			while (DeQueue(&UARTbufQueue,&f));  // 排空队列
			esp8266_send_at_cmd("AT+CIPMODE=1");  // 开始透传
			while (!DeQueue(&UARTbufQueue,&f));
			e=f;
			SPI_MasterInitInit();
			SPI_MasterTransmit(f);
			SPI_Init();
			if (rx_ok(e,f))
			{
				while (DeQueue(&UARTbufQueue,&f)); //排空队列
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
						//---------------开始建立Socks5代理，此时已与代理服务器建立TCP连接---------------
						socks5_init(e,f);
						//---------------已建立Socks5代理---------------
						if (socks5_ok_flag != 1) {  // 失败
							SPIputs("Socks5 proxy service failed to build!");
							return;
						} else {  // 成功
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
	}
}