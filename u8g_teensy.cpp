#include <Arduino.h>
#include "u8glib.h"

#include <SPI.h>

//ssd1351
#define A0 15
#define CS 10
#define RESET 14

#define SPISPEED 24000000

uint8_t u8g_com_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
  switch(msg)
  {
  case U8G_COM_MSG_STOP:
    //STOP THE DEVICE
    break;

  case U8G_COM_MSG_INIT:
    pinMode(A0, OUTPUT);
    pinMode(CS, OUTPUT);
    SPI.begin();
    SPI.beginTransaction(SPISettings(SPISPEED, MSBFIRST, SPI_MODE0)); 
    pinMode(RESET, OUTPUT);
    //bounce reset pin
    digitalWrite(RESET, HIGH);
    delay(1);
    digitalWrite(RESET, LOW);
    delay(10);
    digitalWrite(RESET, HIGH);
    SPI.endTransaction();
    break;

  case U8G_COM_MSG_ADDRESS:  
    //SWITCH FROM DATA TO COMMAND MODE (arg_val == 0 for command mode)
    if (arg_val != 0) {
      digitalWrite(A0, HIGH);
    } 
    else
    {
      digitalWrite(A0, LOW);
    }
    break;

  case U8G_COM_MSG_CHIP_SELECT:
    if(arg_val == 0)
    {
      digitalWrite(CS, HIGH);
    }
    else{
      digitalWrite(CS, LOW);
    }
    break;

  case U8G_COM_MSG_RESET:
    //TOGGLE THE RESET PIN ON THE DISPLAY BY THE VALUE IN arg_val
    digitalWrite(RESET, arg_val);
    break;

  case U8G_COM_MSG_WRITE_BYTE:
    //WRITE BYTE TO DEVICE
    SPI.beginTransaction(SPISettings(SPISPEED, MSBFIRST, SPI_MODE0)); 
    SPI.transfer(arg_val);
    SPI.endTransaction();
    break;

  case U8G_COM_MSG_WRITE_SEQ:
  case U8G_COM_MSG_WRITE_SEQ_P:
    {
      //WRITE A SEQUENCE OF BYTES TO THE DEVICE
      register uint8_t *ptr = static_cast<uint8_t *>(arg_ptr);
      SPI.beginTransaction(SPISettings(SPISPEED, MSBFIRST, SPI_MODE0)); 
      while(arg_val > 0){
        SPI.transfer(*ptr++);
        arg_val--;
      }
      SPI.endTransaction();
    }
    break;

  }
  return 1;
}


