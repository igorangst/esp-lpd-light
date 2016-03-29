#ifndef LPD8806_H
#define LPD8806_H 

#include <esp8266.h>
#include "gpio.h"

typedef struct {
  uint16 n;       // number of LEDs
  uint16 dpin;    // data pin
  uint16 cpin;    // clock pin
  uint16 nbytes;  // size of pixel buffer
  uint8  *pixels; // color and latch bytes
} LPD8806;

typedef struct {
  uint8 r;
  uint8 g;
  uint8 b;
} Color;

extern LPD8806 lpd8806;

void lpd8806_init(uint16 n, uint16 dpin, uint16 cpin);

void lpd8806_set_pixel_color(uint16 i, Color c);

void lpd8806_set_pixel_rgb(uint16 i, uint8 r, uint8 g, uint8 b);

void lpd8806_show();

#endif
