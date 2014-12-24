#include <Arduino.h>
#include "u8glib.h"

#include <SPI.h>

//ssd1351
#define MOSI 11
#define SCLK 13
#define DC 15
#define CS 10
#define RESET 14
#define SPISPEED CTAR_24MHz

#define CTAR_24MHz   (SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0) | SPI_CTAR_DBR)
#define CTAR_16MHz   (SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0) | SPI_CTAR_DBR)
#define CTAR_12MHz   (SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0))
#define CTAR_8MHz    (SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0))
#define CTAR_6MHz    (SPI_CTAR_PBR(0) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1))
#define CTAR_4MHz    (SPI_CTAR_PBR(1) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1))


static bool spi_pin_is_cs(uint8_t pin){
  if (pin == 2 || pin == 6 || pin == 9) return true;
  if (pin == 10 || pin == 15) return true;
  if (pin >= 20 && pin <= 23) return true;
  return false;
}

static uint8_t spi_configure_cs_pin(uint8_t pin){
  switch (pin) {
  case 10: CORE_PIN10_CONFIG = PORT_PCR_MUX(2); return 0x01; // PTC4
  case 2:  CORE_PIN2_CONFIG  = PORT_PCR_MUX(2); return 0x01; // PTD0
  case 9:  CORE_PIN9_CONFIG  = PORT_PCR_MUX(2); return 0x02; // PTC3
  case 6:  CORE_PIN6_CONFIG  = PORT_PCR_MUX(2); return 0x02; // PTD4
  case 20: CORE_PIN20_CONFIG = PORT_PCR_MUX(2); return 0x04; // PTD5
  case 23: CORE_PIN23_CONFIG = PORT_PCR_MUX(2); return 0x04; // PTC2
  case 21: CORE_PIN21_CONFIG = PORT_PCR_MUX(2); return 0x08; // PTD6
  case 22: CORE_PIN22_CONFIG = PORT_PCR_MUX(2); return 0x08; // PTC1
  case 15: CORE_PIN15_CONFIG = PORT_PCR_MUX(2); return 0x10; // PTC0
  }
  return 0;
}

uint8_t u8g_com_hw_spi_dma_sumotoy_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr){
  uint8_t 		_cs,_dc,_sid,_sclk,_rst;
  uint8_t 		pcs_data, pcs_command;
  uint32_t 		ctar;
  volatile uint8_t      *datapin, *clkpin, *cspin, *rspin;
  bool _inited;

  _cs = CS;
  _sid = MOSI; 
  _sclk = SCLK;
  _rst = RESET;
  _dc = DC; 

  switch(msg)
  {
  case U8G_COM_MSG_STOP:
    //STOP THE DEVICE
    //can't stop, won't stop.
    break;

  case U8G_COM_MSG_INIT:
    if (spi_pin_is_cs(_cs) && spi_pin_is_cs(_dc)
      && (_sid == 7 || _sid == 11) && (_sclk == 13 || _sclk == 14)
      && !(_cs ==  2 && _dc == 10) && !(_dc ==  2 && _cs == 10)
      && !(_cs ==  6 && _dc ==  9) && !(_dc ==  6 && _cs ==  9)
      && !(_cs == 20 && _dc == 23) && !(_dc == 20 && _cs == 23)
      && !(_cs == 21 && _dc == 22) && !(_dc == 21 && _cs == 22)) {
      if (_sclk == 13) {
        CORE_PIN13_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_DSE;
        SPCR.setSCK(13);
      } 
      else {
        CORE_PIN14_CONFIG = PORT_PCR_MUX(2);
        SPCR.setSCK(14);
      }
      if (_sid == 11) {
        CORE_PIN11_CONFIG = PORT_PCR_MUX(2);
        SPCR.setMOSI(11);
      } 
      else {
        CORE_PIN7_CONFIG = PORT_PCR_MUX(2);
        SPCR.setMOSI(7);
      }
      ctar = SPISPEED;
      pcs_data = spi_configure_cs_pin(_cs);
      pcs_command = pcs_data | spi_configure_cs_pin(_dc);
      SIM_SCGC6 |= SIM_SCGC6_SPI0;
      SPI0.MCR = SPI_MCR_MDIS | SPI_MCR_HALT;
      SPI0.CTAR0 = ctar | SPI_CTAR_FMSZ(7);
      SPI0.CTAR1 = ctar | SPI_CTAR_FMSZ(15);
      SPI0.MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F) | SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF;
      _inited = true;
    } 
    else {
      _inited = false;
    }
    if (_rst && _inited) {
      pinMode(_rst, OUTPUT);
      pinMode(_dc, OUTPUT);
      pinMode(_cs, OUTPUT);
      digitalWrite(_rst, HIGH);
      delay(1);
      digitalWrite(_rst, LOW);
      delay(10);
      digitalWrite(_rst, HIGH);
      delay(1);
    }
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
    SPI0.PUSHR = arg_val | (pcs_command << 16) | SPI_PUSHR_CTAS(0);
    while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
    break;

  case U8G_COM_MSG_WRITE_SEQ:
  case U8G_COM_MSG_WRITE_SEQ_P:
    {
      //WRITE A SEQUENCE OF BYTES TO THE DEVICE
      register uint8_t *ptr = static_cast<uint8_t *>(arg_ptr);
      while(arg_val > 0){
        SPI0.PUSHR = *ptr++ | (pcs_command << 16) | SPI_PUSHR_CTAS(0);
        while (((SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
        arg_val--;
      }
    }
    break;
  }
  return 1;
}

