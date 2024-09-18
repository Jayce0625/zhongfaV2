/*
 * utils.h
 *
 * Created: 2022/9/29 19:35:00
 *  Author: wjc
 */ 


#ifndef UTILS_H_
#define UTILS_H_
#define  uchar unsigned char
#define  uint  unsigned int

char rx_ok(char a,char b);
char rx_smallok(char a,char b);
char rx_enter(char a);
void esp8266_send_at_cmd(char *cmd);
void socks5_init(char e, char f);
void set_server(char e, char f);


#endif /* UTILS_H_ */