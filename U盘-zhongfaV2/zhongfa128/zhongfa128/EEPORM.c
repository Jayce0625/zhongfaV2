/*
 * EEPORM.c
 *
 * Created: 2018/1/26 22:04:24
 *  Author: Administrator
 */ 

#include "EEPORM.h"

uint8_t EEPROM_READ(uint16_t address){
    eeprom_busy_wait();
	return eeprom_read_byte((uint8_t*)(address));
}

void EEPROM_WRITE(uint16_t address, uint8_t data){
    eeprom_busy_wait();
	eeprom_write_byte((uint8_t*)(address),data);
	
}

/********************************************************************
 *�������ܣ���һ��double����д�뵽��address��ʼ��������4���ֽ���
 * 
*********************************************************************/
void write_double_toEEPROM(uint16_t address,double data){
	uint8_t *p;
	p = &data;
	EEPROM_WRITE(address,*p);
	EEPROM_WRITE(address+1,*(p+1));
	EEPROM_WRITE(address+2,*(p+2));
	EEPROM_WRITE(address+3,*(p+3));
}

/********************************************************************
 *�������ܣ�����address��ʼ��������4���ֽ��ж���һ��double����
 * 
*********************************************************************/
double read_double_fromEEPROM(uint16_t address){
	double result;
	double *p;
	uint8_t temp[8];
	p = &temp;
	temp[0] = EEPROM_READ(address);
	temp[1] = EEPROM_READ(address+1);
	temp[2] = EEPROM_READ(address+2);
	temp[3] = EEPROM_READ(address+3);
	
	result = *p;
	return result;
}

