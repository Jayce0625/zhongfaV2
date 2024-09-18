/*
 * EEPORM.h
 *
 * Created: 2022/9/3 22:04:41
 *  Author: wjc
 */ 


#ifndef EEPORM_H_
#define EEPORM_H_

#include <avr/io.h>
#include <avr/eeprom.h>

uint8_t EEPROM_READ(uint16_t address);                    //��EEPROM�ж�ȡָ��λ��һ���ֽ�
void EEPROM_WRITE(uint16_t address, uint8_t data);        //��EEPROM��д��һ���ֽ�
void write_double_toEEPROM(uint16_t address,double data); //��ָ��λ������д4���ֽڣ���һ��double����
double read_double_fromEEPROM(uint16_t address);          //��ָ��λ��������4���ֽڣ����һ��double����





#endif /* EEPORM_H_ */