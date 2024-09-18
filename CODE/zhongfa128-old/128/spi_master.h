/*
 * spi_master.h
 *
 * Created: 2022/9/1 星期四 15:31:11
 *  Author: Administrator
 */ 


#ifndef SPI_MASTER_H_
#define SPI_MASTER_H_
#define  uchar unsigned char
#define  uint  unsigned int
void SPI_Init(void);
void SPI_MasterInitInit(void);
void SPI_MasterTransmit(uchar rdata);
void SPIputs(char *s);


#endif /* SPI_MASTER_H_ */