/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

#include <esp8266.h>
#include "httpd.h"
#include "io.h"
#include "httpdespfs.h"
#include "cgiwifi.h"
#include "stdout.h"
#include "auth.h"
#include "espfs.h"
#include "captdns.h"
#include "webpages-espfs.h"
#include "cgiwebsocket.h"
#include "lpd8806.h"
#include "color.h"

static ETSTimer lightTimer;

#define NLEDS 24

uint16 numColors = 1;   // number of colors in the queue, between 1 and 5
Color  colors[5];       // color queue used for fading or wheel effect
uint32 time = 0;        // global time
uint16 deltaT = 64;     // step size by which time advances at each update
uint16 brightness = 64; // brightness value in [0,128]
uint16 fx = 0;          // effect 0: fade, 1: wheel
uint8  status = 0;      // main switch 0:off, 1:on

void lightTimerfunc(void *arg)
{
  // advance time
  time += deltaT;
  
  // we ignore the LSBs for later calculations 
  uint16 t = time >> 4;

  if (status==0){ // set all LEDs to black
    uint16 i;
    for (i=0; i<NLEDS; ++i){
      lpd8806_set_pixel_rgb(i,0,0,0);
    }
    lpd8806_show();
  } else {
    if (numColors==1){ // single color with brightness
      uint16 i;
      Color c = dim(colors[0], brightness);
      for (i=0; i<NLEDS; ++i){
	lpd8806_set_pixel_color(i, c);
      }
      lpd8806_show();
    } else {
      if (fx == 1){ // color wheel
	uint16 i;
	Color c;
	for (i=0; i<NLEDS; ++i){
	  c = color_interpolate_list(colors, numColors, t + i*(numColors*128/NLEDS));
	  c = dim(c, brightness);
	  lpd8806_set_pixel_color(i, c);
	}
	lpd8806_show();
      } else if (fx == 0){ // fade
	uint16 i;
	Color c = color_interpolate_list(colors, numColors, t);
	c = dim(c, brightness);
	for (i=0; i<NLEDS; ++i){
	  lpd8806_set_pixel_color(i, c);
	}
	lpd8806_show();
      }
    }
  }
}

bool startsWith(const char *str, const char *pre)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

void myWebsocketRecv(Websock *ws, char *data, int len, int flags) {
  char *val = strstr(data, "=");
  char buff[128];
  if (startsWith(data, "SWITCH")){
    if (val != NULL){
      status = atoi(val+1);
    }
    os_sprintf(buff, "SWITCH OK"); 
  } else if (startsWith(data, "NCOLORS")){
    if (val != NULL){
      numColors = atoi(val+1);
      if (numColors < 1){
	numColors = 1;
      }
      if (numColors > 5){
	numColors = 5;
      }
    }
    os_sprintf(buff, "SET NCOLORS OK"); 
  } else if (startsWith(data, "BRIGHT")){
    if (val != NULL){
      brightness = atoi(val+1);
    }
    os_sprintf(buff, "SET BRIGHTNESS OK"); 
  } else if (startsWith(data, "SPEED")){
    if (val != NULL){
      deltaT = atoi(val+1);
    }
    os_sprintf(buff, "SET SPEED OK"); 
  } else if (startsWith(data, "FX")){
    if (val != NULL){
      fx = atoi(val+1);
    }
    os_sprintf(buff, "SET FX OK"); 
  } else if (startsWith(data, "COLORS")){
    uint16 i;
    for (i=0; i<5; ++i){
      uint32 hex = (uint32)strtol(val+1+7*i, NULL, 16);
      Color c;
      c.r = ((hex >> 16) & 0xff) >> 1;
      c.g = ((hex >> 8 ) & 0xff) >> 1;
      c.b = ((hex >> 0 ) & 0xff) >> 1;
      colors[i] = c;
    }
    os_sprintf(buff, "SET COLORS OK"); 
  } else {
    os_sprintf(buff, "ILLEGAL COMMAND"); 
  }
  cgiWebsocketSend(ws, buff, os_strlen(buff), WEBSOCK_FLAG_NONE);
}

//Websocket connected. Install reception handler and send welcome message.
void myWebsocketConnect(Websock *ws) {
	ws->recvCb=myWebsocketRecv;
	cgiWebsocketSend(ws, "HELLO WORLD!", 14, WEBSOCK_FLAG_NONE);
}

/*
This is the main url->function dispatching data struct.
In short, it's a struct with various URLs plus their handlers. The handlers can
be 'standard' CGI functions you wrote, or 'special' CGIs requiring an argument.
They can also be auth-functions. An asterisk will match any url starting with
everything before the asterisks; "*" matches everything. The list will be
handled top-down, so make sure to put more specific rules above the more
general ones. Authorization things (like authBasic) act as a 'barrier' and
should be placed above the URLs they protect.
*/
HttpdBuiltInUrl builtInUrls[]={
	{"*", cgiRedirectApClientToHostname, "esp8266.nonet"},
	{"/", cgiRedirect, "/light.html"},
	//Routines to make the /wifi URL and everything beneath it work.

//Enable the line below to protect the WiFi configuration with an username/password combo.
//	{"/wifi/*", authBasic, myPassFn},

	{"/wifi", cgiRedirect, "/wifi/wifi.tpl"},
	{"/wifi/", cgiRedirect, "/wifi/wifi.tpl"},
	{"/wifi/wifiscan.cgi", cgiWiFiScan, NULL},
	{"/wifi/wifi.tpl", cgiEspFsTemplate, tplWlan},
	{"/wifi/connect.cgi", cgiWiFiConnect, NULL},
	{"/wifi/setmode.cgi", cgiWiFiSetMode, NULL},

	{"/websocket/ws.cgi", cgiWebsocket, myWebsocketConnect},

	{"/test", cgiRedirect, "/test/index.html"},
	{"/test/", cgiRedirect, "/test/index.html"},

	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL}
};

//Main routine. Initialize stdout, the I/O, filesystem and the webserver and we're done.
void user_init(void) {
	stdoutInit();
	ioInit();
	captdnsInit();

	colors[0] = COLOR_RED;
	colors[1] = COLOR_BLUE;
	colors[2] = COLOR_GREEN;
	colors[3] = COLOR_PURPLE;
	colors[4] = COLOR_CYAN;

	gpio_init();
	lpd8806_init(32, 2, 0);
	os_timer_disarm(&lightTimer);
	os_timer_setfn(&lightTimer, (os_timer_func_t *)lightTimerfunc, NULL);
	os_timer_arm(&lightTimer, 50, 1);

	// 0x40200000 is the base address for spi flash memory mapping, ESPFS_POS is the position
	// where image is written in flash that is defined in Makefile.
#ifdef ESPFS_POS
	espFsInit((void*)(0x40200000 + ESPFS_POS));
#else
	espFsInit((void*)(webpages_espfs_start));
#endif
	httpdInit(builtInUrls, 80);
	os_printf("\nReady\n");
}

void user_rf_pre_init() {
	//Not needed, but some SDK versions want this defined.
}
