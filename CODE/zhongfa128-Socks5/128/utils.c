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
// Ŀ���������IP��ַ�Ͷ˿ں�
extern char ip[19],port[8],proxy_ip[19],proxy_port[8],user[255],password[255];
/*// CCProxy���������IP��ַ�Ͷ˿ں�
#define PROXY_SERVER_IP "10.120.235.33"
#define PROXY_SERVER_PORT 1080*/
/*// CCProxy����������û���������
#define PROXY_USERNAME "Jayce"
#define PROXY_PASSWORD "114834766"*/
// ESP8266ģ���ATָ���
char esp8266_at_buf[256];
// ESP8266ģ����ջ���
uchar usart_recv_buf[256];
// socks5���ñ�־λ��0ʧ�ܣ�1�ɹ�
extern int socks5_ok_flag;

// ƥ�䷵��ֵ�Ƿ�Ϊ��дOK
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

// ƥ�䷵��ֵ�Ƿ�ΪСдok
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

// ƥ�䷵��ֵ�Ƿ�Ϊ'>'��>Ϊ���������ݵı�־
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

// ����ATָ�������Զ������������У����ȴ�����ֵ
void esp8266_send_at_cmd(char *cmd)
{
	// USART1_Flush();
	puts1(cmd);
	USART1_Transmit('\r');
	USART1_Transmit('\n');
	delay(1000);
}

// ����Socks5����
void socks5_init(char e, char f)
{
	// ����Socks5Э��汾����֤��������Ϣ
	uchar socks5_auth_methods[] = {0x05,0x02,0x00,0x02};
	USART1_Send_Bytes(socks5_auth_methods,sizeof(socks5_auth_methods));
	delay(1000);  // �ȴ�Socks5�������Ļظ�
	while (!DeQueue(&UARTbufQueue,&f));  // �ȴ����зǿ�
	e=f;
	DeQueue(&UARTbufQueue,&f);
	SPI_MasterInitInit();
	SPI_MasterTransmit(e);  // 0x05
	SPI_MasterTransmit(f);  // 0x02
	SPI_Init();
	if ((e == 0x05) && (f == 0x02)) {
		// ����������֤���û���������
		uchar socks5_auth_req[] = {0x01,strlen(user)};
		uchar socks5_auth_req2[] = {strlen(password)};
		USART1_Send_Bytes(socks5_auth_req,sizeof(socks5_auth_req));
		USART1_Send_Bytes((uchar *)user,strlen(user));
		USART1_Send_Bytes(socks5_auth_req2,sizeof(socks5_auth_req2));
		USART1_Send_Bytes((uchar *)password,strlen(password));
		delay(1000);  // �ȴ�Socks5�������Ļظ�
		while (!DeQueue(&UARTbufQueue,&f));  // �ȴ����зǿ�
		e=f;
		DeQueue(&UARTbufQueue,&f);
		SPI_MasterInitInit();
		SPI_MasterTransmit(e);  // 0x01
		SPI_MasterTransmit(f);  // 0x00
		SPI_Init();
		// �����ص�STATUS��Ϊ0x00������Ȩʧ��
		if ((e == 0x01) && (f == 0x00)) {
			// ������ʵĿ�������IP��ַ�Ͷ˿ںŵ���Ϣ
			uchar socks5_auth_server[] = {0x05, 0x01, 0x00, 0x01};
			// ���ַ���IPV4��ַת��Ϊ��ռ4���ֽڵ�������ֵ
			uchar a,b,c,d;
			sscanf(ip,"%hhu.%hhu.%hhu.%hhu",&a,&b,&c,&d);
			uchar true_ipv4_hex[] = {a,b,c,d};
			// ���ַ����˿ں�ת��Ϊ��ռ2���ֽڵ�������ֵ
			uint true_port = atoi(port);  // ���ַ������͵Ķ˿ں�ת��Ϊ������ֵ
			uchar true_port_hex[] = {(true_port >> 8),(true_port & 0xFF)};  // ʮ�����ƶ˿ںŴ����ǰС���ں�
			USART1_Send_Bytes(socks5_auth_server,sizeof(socks5_auth_server));
			USART1_Send_Bytes(true_ipv4_hex,sizeof(true_ipv4_hex));
			USART1_Send_Bytes(true_port_hex,sizeof(true_port_hex));
			delay(1000);  // �ȴ�Socks5�������Ļظ�
			while (!DeQueue(&UARTbufQueue,&f));  // �ȴ����зǿ�
			e=f;
			DeQueue(&UARTbufQueue,&f);
			SPI_MasterInitInit();
			SPI_MasterTransmit(e);  // 0x05
			SPI_MasterTransmit(f);  // 0x00
			SPI_Init();
			if ((e == 0x05) && (f == 0x00)) {
				socks5_ok_flag = 1;  // �ɹ���־λ��1
				while (DeQueue(&UARTbufQueue,&f));  // �ſն���
			}
		}
	}
}

// ���÷����������������������Ŀ�������
void set_server(char e, char f)
{
	LedGreenOn();
	while (DeQueue(&UARTbufQueue,&f)); // �ſն���
	esp8266_send_at_cmd("AT+CIPMUX=0");
	while (!DeQueue(&UARTbufQueue,&f));  // �ȴ���������������
	e=f;
	SPI_MasterInitInit();
	SPI_MasterTransmit(f);
	SPI_Init();
	if (rx_ok(e,f))
	{
		while (DeQueue(&UARTbufQueue,&f)); // �ſն���
		sprintf((char *)esp8266_at_buf, "AT+CIPSTART=\"TCP\",\"%s\",%s", proxy_ip, proxy_port);  // TCP���Ӵ��������IP��ַ�Ͷ�Ӧ�˿ں�
		esp8266_send_at_cmd(esp8266_at_buf);
		while (!DeQueue(&UARTbufQueue,&f));  // �ȴ����зǿ�
		e=f;
		SPI_MasterInitInit();
		SPI_MasterTransmit(f);
		SPI_Init();
		if (rx_ok(e,f))
		{
			while (DeQueue(&UARTbufQueue,&f));  // �ſն���
			esp8266_send_at_cmd("AT+CIPMODE=1");  // ��ʼ͸��
			while (!DeQueue(&UARTbufQueue,&f));
			e=f;
			SPI_MasterInitInit();
			SPI_MasterTransmit(f);
			SPI_Init();
			if (rx_ok(e,f))
			{
				while (DeQueue(&UARTbufQueue,&f)); //�ſն���
				esp8266_send_at_cmd("AT+CIPSEND");  // ͸��ģʽ��ʼ��������
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
						//---------------��ʼ����Socks5������ʱ����������������TCP����---------------
						socks5_init(e,f);
						//---------------�ѽ���Socks5����---------------
						if (socks5_ok_flag != 1) {  // ʧ��
							SPIputs("Socks5 proxy service failed to build!");
							return;
						} else {  // �ɹ�
							LedRedOn();
							SPIputs("\r\nSocks5 proxy service build succeeded & wifimodule ok!\r\n");
							puts1("wifimodule ok");
							SPCR &= ~(1<<7);  // �ر�SPI�ж�SPIE
							while (DeQueue(&SPIbufQueue,&f));
							SPCR |= (1<<7);  // ����SPI�ж�SPIE
						}
					}	
				}
			}
		}
	}
}