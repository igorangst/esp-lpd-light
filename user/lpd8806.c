#include "lpd8806.h"
#include "mem.h"

LPD8806 lpd8806;

#define LPD_CPIN  lpd8806.cpin
#define LPD_DPIN  lpd8806.dpin
#define LPD_DELAY ets_delay_us(1)
#define LPD_TICK  GPIO_OUTPUT_SET(LPD_CPIN, 1); \
  LPD_DELAY;					\
  GPIO_OUTPUT_SET(LPD_CPIN, 0);			\
  LPD_DELAY
#define LPD_SEND_BIT(b) GPIO_OUTPUT_SET(LPD_DPIN, b);	\
  LPD_TICK

void lpd8806_start_bitbang() 
{
  GPIO_OUTPUT_SET(LPD_DPIN, 0);
  uint16 i;
  for (i = ((lpd8806.n+31)/32)*8; i>0; i--) {
    LPD_TICK;
  }
}

void lpd8806_init(uint16 n, uint16 dpin, uint16 cpin)
{
  lpd8806.dpin = dpin;
  lpd8806.cpin = cpin;

  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
 
  gpio_output_set(0, 0, (1 << dpin) | (1 << cpin), 0);
  
  uint8 latchBytes;
  uint16 dataBytes, totalBytes;

  dataBytes = n * 3;
  latchBytes = (n + 31) / 32;
  totalBytes = dataBytes + latchBytes;
  if ((lpd8806.pixels = (uint8 *)malloc(totalBytes))){
    lpd8806.n = n;
    lpd8806.nbytes = totalBytes;
    unsigned i = 0;
    for (i=0; i<dataBytes; ++i){
      lpd8806.pixels[i] = 0x80;
    }
    for (i=0; i<latchBytes; ++i){
      lpd8806.pixels[dataBytes+i] = 0x00;
    }
  }
  
  lpd8806_start_bitbang();
}

void lpd8806_send_byte(uint8 byte)
{
  LPD_SEND_BIT(0x1 & (byte >> 7));
  LPD_SEND_BIT(0x1 & (byte >> 6));
  LPD_SEND_BIT(0x1 & (byte >> 5));
  LPD_SEND_BIT(0x1 & (byte >> 4));
  LPD_SEND_BIT(0x1 & (byte >> 3));
  LPD_SEND_BIT(0x1 & (byte >> 2));
  LPD_SEND_BIT(0x1 & (byte >> 1));
  LPD_SEND_BIT(0x1 & (byte >> 0));
}

void lpd8806_set_pixel_rgb(uint16 i, uint8 r, uint8 g, uint8 b)
{
  if (i < lpd8806.n) {
    uint8 *p = &lpd8806.pixels[i * 3];
    *p++ = g | 0x80;
    *p++ = r | 0x80;
    *p++ = b | 0x80;
  }
}

void lpd8806_set_pixel_color(uint16 i, Color c)
{
  if (i < lpd8806.n) {
    uint8 *p = &lpd8806.pixels[i * 3];
    *p++ = c.g | 0x80;
    *p++ = c.r | 0x80;
    *p++ = c.b | 0x80;
  }
}

void lpd8806_show()
{
  uint8 *ptr = lpd8806.pixels;
  uint16 i   = lpd8806.nbytes;

  uint8 p, bit;  
  while(i--) {
    p = *ptr++;
    for (bit=0x80; bit; bit >>= 1) {
      if (p & bit){
	LPD_SEND_BIT(1);
      } else {
	LPD_SEND_BIT(0);
      }
    }
  }
}

