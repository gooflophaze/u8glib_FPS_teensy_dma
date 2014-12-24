#include "U8glib.h"
#include <SPI.h>

//uncomment one pair below for life in the faster lane

#include "u8g_teensy.h" // 24mhz SPI calls
U8GLIB u8g(&u8g_dev_ssd1351_128x128_hicolor_hw_spi, u8g_com_hw_spi_fn); //ssd1351 - s/hicolor/332 for speed

//#include "u8g_teensy_dma_sumotoy.h"
//U8GLIB u8g(&u8g_dev_ssd1351_128x128_hicolor_hw_spi, u8g_com_hw_spi_dma_sumotoy_fn); 

//#include "u8g_teensy_dma_paul.h"
//U8GLIB u8g(&u8g_dev_ssd1351_128x128_hicolor_hw_spi, u8g_com_hw_spi_dma_paul_fn); 


//U8GLIB u8g(&u8g_dev_ssd1306_adafruit_128x64_hw_spi, u8g_com_hw_spi_fn); //ssd1306 example - change $2 depending on header

/*
ssd1351    no dma    |   sumotoy   |     paul
.     |  hiC - 332   |  hiC - 332  |  hiC - 332  
clip  | 18.3 - 19.1  | 34.6 - 37.4 | 32.6 - 35.8
clear | 13.5 - 15.3  | 20.5 - 24.9 | 19.9 - 23.8
@     | 14.9 - 16.2  | 23.8 - 27.4 | 22.9 - 26.0
pixel | 12.0 - 15.0  | 17.1 - 24.4 | 16.0 - 24.1

ssd1306 - has some corruption on the high rows (126-128).
.       no dma |   sumo |  paul
clip  | 172.2  | 193.4  | 193.2
clear |  28.3  |  28.9  |  27.8
@     |  39.8  |  40.8  |  40.6
pixel |  13.8  |  14.4  |  13.2  
*/

#define SECONDS 10
uint8_t flip_color = 0;
uint8_t draw_color = 1;

void draw_set_screen(void) {
  // graphic commands to redraw the complete screen should be placed here  
  if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    if ( flip_color == 0 ) 
      u8g.setHiColorByRGB(0,0,0);
    else
      u8g.setHiColorByRGB(255,255,255);
  }
  else {
    u8g.setColorIndex(flip_color);
  }
  u8g.drawBox( 0, 0, u8g.getWidth(), u8g.getHeight() );
}

void draw_clip_test(void) {
  u8g_uint_t i, j, k;
  char buf[3] = "AB";
  k = 0;
  if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  else {
    u8g.setColorIndex(draw_color);
  }
  u8g.setFont(u8g_font_6x10);
  
  for( i = 0; i  < 6; i++ ) {
    for( j = 1; j  < 8; j++ ) {
      u8g.drawHLine(i-3, k, j);
      u8g.drawHLine(i-3+10, k, j);
      
      u8g.drawVLine(k+20, i-3, j);
      u8g.drawVLine(k+20, i-3+10, j);
      
      k++;
    }
  }
  u8g.drawStr(0-3, 50, buf);
  u8g.drawStr180(0+3, 50, buf);
  
  u8g.drawStr(u8g.getWidth()-3, 40, buf);
  u8g.drawStr180(u8g.getWidth()+3, 40, buf);

  u8g.drawStr90(u8g.getWidth()-10, 0-3, buf);
  u8g.drawStr270(u8g.getWidth()-10, 3, buf);

  u8g.drawStr90(u8g.getWidth()-20, u8g.getHeight()-3, buf);
  u8g.drawStr270(u8g.getWidth()-20, u8g.getHeight()+3, buf);
  
}

void draw_char(void) {
  char buf[2] = "@";
  u8g_uint_t i, j;
  // graphic commands to redraw the complete screen should be placed here  
  if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  else {
    u8g.setColorIndex(draw_color);
  }
  u8g.setFont(u8g_font_6x10);
  j = 8;
  for(;;) {
    i = 0;
    for(;;) {
      u8g.drawStr( i, j, buf);
      i += 8;
      if ( i > u8g.getWidth() )
        break;
    }
    j += 8;
    if ( j > u8g.getHeight() )
      break;
  }
  
}

void draw_pixel(void) {
  u8g_uint_t x, y, w2, h2;
  if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  else {
    u8g.setColorIndex(draw_color);
  }
  w2 = u8g.getWidth();
  h2 = u8g.getHeight();
  w2 /= 2;
  h2 /= 2;
  for( y = 0; y < h2; y++ ) {
    for( x = 0; x < w2; x++ ) {
      if ( (x + y) & 1 ) {
        u8g.drawPixel(x,y);
        u8g.drawPixel(x,y+h2);
        u8g.drawPixel(x+w2,y);
        u8g.drawPixel(x+w2,y+h2);
      }
    }
  }
}

// returns unadjusted FPS
uint16_t picture_loop_with_fps(void (*draw_fn)(void)) {
  uint16_t FPS10 = 0;
  uint32_t time;
  
  time = millis() + SECONDS*1000;
  
  // picture loop
  do {
    u8g.firstPage();  
    do {
      draw_fn();
    } while( u8g.nextPage() );
    FPS10++;
    flip_color = flip_color ^ 1;
  } while( millis() < time );
  return FPS10;  
}

const char *convert_FPS(uint16_t fps) {
  static char buf[6];
  strcpy(buf, u8g_u8toa( (uint8_t)(fps/10), 3));
  buf[3] =  '.';
  buf[4] = (fps % 10) + '0';
  buf[5] = '\0';
  return buf;
}

void show_result(const char *s, uint16_t fps) {
  // assign default color value
  if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  else {
    u8g.setColorIndex(draw_color);
  }
  u8g.setFont(u8g_font_8x13B);
  u8g.firstPage();  
  do {
    u8g.drawStr(0,12, s);
    u8g.drawStr(0,24, convert_FPS(fps));
  } while( u8g.nextPage() );
}

void setup(void) {
  // flip screen, if required
  // u8g.setRot180();
  
  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) 
    draw_color = 255;     // white
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT )
    draw_color = 3;         // max intensity
  else if ( u8g.getMode() == U8G_MODE_BW )
    draw_color = 1;         // pixel on
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
}

void loop(void) {
  uint16_t fps;
  fps = picture_loop_with_fps(draw_clip_test);
  show_result("draw clip test", fps);
  delay(5000);
  fps = picture_loop_with_fps(draw_set_screen);
  show_result("clear screen", fps);
  delay(5000);
  fps = picture_loop_with_fps(draw_char);
  show_result("draw @", fps);
  delay(5000);  
  fps = picture_loop_with_fps(draw_pixel);
  show_result("draw pixel", fps);
  delay(5000);
}

