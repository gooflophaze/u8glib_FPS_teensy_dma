#include <Arduino.h>
#include "u8glib.h"

#include <SPI.h>

//ssd1351
//#define MOSI 11
//#define SCLK 13
#define A0 15
#define CS 10
#define RESET 14
#define SPICLOCK 24000000

uint8_t u8g_com_hw_spi_dma_paul_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr){
  uint8_t 		_cs,_dc,_sid,_sclk,_rst;
  uint8_t 		pcs_data, pcs_command;
  uint32_t 		ctar;
  volatile uint8_t      *datapin, *clkpin, *cspin, *rspin;
  bool _inited;

  _cs = CS;
  _sid = MOSI; //mosi
//  _sclk = SCLK;
  _rst = RESET;
  _dc = A0; //dc

  switch(msg)
  {
  case U8G_COM_MSG_STOP:
    //STOP THE DEVICE
    // can't stop, won't stop.
    break;

  case U8G_COM_MSG_INIT:
    SPI.begin();
    if (SPI.pinIsChipSelect(_cs, _dc)) {
      pcs_data = SPI.setCS(_cs);
      pcs_command = pcs_data | SPI.setCS(_dc);
    }  
    else {
      pcs_data=0;
      pcs_command=0;
      return 0;
    }
    SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
    pinMode(_rst, OUTPUT);
    pinMode(_dc, OUTPUT);
    pinMode(_cs, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(1);
    digitalWrite(_rst, LOW);
    delay(10);
    digitalWrite(_rst, HIGH);
    delay(1);
    SPI.endTransaction();

    break;

  case U8G_COM_MSG_ADDRESS:  
    //SWITCH FROM DATA TO COMMAND MODE (arg_val == 0 for command mode)
    if (arg_val != 0)
    {
      digitalWrite(_dc, HIGH);
    }
    else
    {
      digitalWrite(_dc, LOW);
    }
    break;

  case U8G_COM_MSG_CHIP_SELECT:
    if(arg_val == 0)
    {
      digitalWrite(_cs, HIGH);
    }
    else{
      digitalWrite(_cs, LOW);
    }
    break;

  case U8G_COM_MSG_RESET:
    //TOGGLE THE RESET PIN ON THE DISPLAY BY THE VALUE IN arg_val
    digitalWrite(_rst, arg_val);
    break;

  case U8G_COM_MSG_WRITE_BYTE:
    SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
    SPI0.PUSHR = arg_val | (pcs_command << 16) | SPI_PUSHR_CTAS(0);
    while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
    SPI.endTransaction();
    break;

  case U8G_COM_MSG_WRITE_SEQ:
  case U8G_COM_MSG_WRITE_SEQ_P:
    {
      //WRITE A SEQUENCE OF BYTES TO THE DEVICE
      SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
      register uint8_t *ptr = static_cast<uint8_t *>(arg_ptr);
      while(arg_val > 0){
        SPI0.PUSHR = *ptr++ | (pcs_command << 16) | SPI_PUSHR_CTAS(0);
        while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
        arg_val--;
      }
    }
    SPI.endTransaction();
    break;
  }
  return 1;
}



