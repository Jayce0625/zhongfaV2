/*
 * spi_slave.h
 *
 * Created: 2022/9/1 星期四 14:59:16
 *  Author: Administrator
 */ 
#define  uchar unsigned char

#ifndef SPI_SLAVE_H_
#define SPI_SLAVE_H_
void SPI_Init(void);
void SPI_MasterInit(void);


void SPI_MasterTransmit(uchar rdata);
//void SPI_Puts(char *s);
//void SPI_Slavecommuninquire()


#endif /* SPI_SLAVE_H_ */