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

uint8_t EEPROM_READ(uint16_t address);                    //在EEPROM中读取指定位置一个字节
void EEPROM_WRITE(uint16_t address, uint8_t data);        //向EEPROM中写入一个字节
void write_double_toEEPROM(uint16_t address,double data); //向指定位置连续写4个字节，即一个double数据
double read_double_fromEEPROM(uint16_t address);          //从指定位置连续读4个字节，组成一个double数据





#endif /* EEPORM_H_ */