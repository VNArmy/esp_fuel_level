/*
* ----------------------------------------------------------------------------
* "THE BEER-WARE LICENSE" (Revision 42):
* Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain
* this notice you can do whatever you want with this stuff. If we meet some day,
* and you think this stuff is worth it, you can buy me a beer in return.
* ----------------------------------------------------------------------------
* Heavily modified and enhanced by Thorsten von Eicken in 2015
* ----------------------------------------------------------------------------
*/

#include <esp8266.h>
#include "httpd.h"
#include "httpdespfs.h"
#include "cgi.h"
#include "cgiwifi.h"
#include "cgiflash.h"
#include "cgitcp.h"
#include "cgimqtt.h"
#include "auth.h"
#include "espfs.h"
#include "uart.h"
#include "cgiuart.h"
#include "config.h"
#include "gpio.h"
#include "cgiservices.h"
#include "levelmeasure.h"

#ifdef NOTICE_DBG

#define NOTICE(format, ...) do {	                                          \
	os_printf(format "\n", ## __VA_ARGS__);                                   \
} while ( 0 )

#else
#define NOTICE(format, ...) do{ } while ( 0 )
#endif
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
HttpdBuiltInUrl builtInUrls[] = {
  { "/", cgiRedirect, "/index.html" },
  { "/menu", cgiMenu, NULL },
  { "/flash/next", cgiGetFirmwareNext, NULL },
    { "/flash/upload", cgiUploadFirmware, NULL },
    { "/flash/reboot", cgiRebootFirmware, NULL },
  //Enable the line below to protect the WiFi configuration with an username/password combo.
  //    {"/wifi/*", authBasic, myPassFn},
  { "/wifi", cgiRedirect, "/wifi/wifi.html" },
  { "/wifi/", cgiRedirect, "/wifi/wifi.html" },
  { "/wifi/info", cgiWifiInfo, NULL },
  { "/wifi/scan", cgiWiFiScan, NULL },
  { "/wifi/connect", cgiWiFiConnect, NULL },
  { "/wifi/connstatus", cgiWiFiConnStatus, NULL },
  { "/wifi/setmode", cgiWiFiSetMode, NULL },
  { "/wifi/special", cgiWiFiSpecial, NULL },
  { "/wifi/apinfo", cgiApSettingsInfo, NULL },
  { "/wifi/apchange", cgiApSettingsChange, NULL },
  { "/uart/info", cgiUartInfo, NULL },
  { "/uart/config", cgiUartConfig, NULL },
  { "/sensor/info", cgiSensorInfo, NULL },
  { "/sensor/config", cgiSensorConfig, NULL },
  { "/system/info", cgiSystemInfo, NULL },
  { "/system/update", cgiSystemSet, NULL },

#ifdef MQTT
  { "/mqtt", cgiMqtt, NULL },
#endif  
  { "*", cgiEspFsHook, NULL }, //Catch-all cgi function for the filesystem
  { NULL, NULL, NULL }
};

#ifdef SHOW_HEAP_USE
static ETSTimer prHeapTimer;
static void ICACHE_FLASH_ATTR prHeapTimerCb(void *arg) {
  os_printf("Heap: %ld\n", (unsigned long)system_get_free_heap_size());
}
#endif

# define VERS_STR_STR(V) #V
# define VERS_STR(V) VERS_STR_STR(V)
char* esp_link_version = VERS_STR(VERSION);

// address of espfs binary blob
extern uint32_t _binary_espfs_img_start;

extern void app_init(void);
extern void mqtt_client_init(void);

bool istcpclient = true;
void user_rf_pre_init(void) {
  //default is enabled
  system_set_os_print(DEBUG_SDK);
}

// Main routine to initialize esp-link.
void user_init(void) {
  // get the flash config so we know how to init things
//  configWipe(); // uncomment to reset the config for testing purposes
  bool restoreOk = configRestore();
  // Init gpio pin registers
  gpio_init();
  gpio_output_set(0, 0, 0, (1<<15)); // some people tie it to GND, gotta ensure it's disabled
  // init UART
  uart_init(flashConfig.baud_rate,flashConfig.data_bits,flashConfig.parity,flashConfig.stop_bits, 115200);
  // Say hello (leave some time to cause break in TX after boot loader's msg
  os_delay_us(10000L);
  NOTICE("\n\n** %s\n", esp_link_version);
  NOTICE("Flash config restore %s\n", restoreOk ? "ok" : "*FAILED*");
  // Wifi
  wifiInit();
  // init the flash filesystem with the html stuff
  espFsInit(&_binary_espfs_img_start);
  //EspFsInitResult res = espFsInit(&_binary_espfs_img_start);
  //NOTICE("espFsInit %s\n", res?"ERR":"ok");
  // mount the http handlers
  httpdInit(builtInUrls, 80);
//  serclient_init();
//  // init the wifi-serial transparent bridge (port 23)
//  if(istcpclient)
//  {
//	  user_initserbridgeclient();
//
//  }
//  else
//  {
//	  serbridgeInit(23, 2323);
//  }

  msgInit();

  #ifdef SHOW_HEAP_USE
  os_timer_disarm(&prHeapTimer);
  os_timer_setfn(&prHeapTimer, prHeapTimerCb, NULL);
  os_timer_arm(&prHeapTimer, 10000, 1);
#endif

//  struct rst_info *rst_info = system_get_rst_info();
//  NOTICE("Reset cause: %d=%s", rst_info->reason, rst_codes[rst_info->reason]);
//  NOTICE("exccause=%d epc1=0x%x epc2=0x%x epc3=0x%x excvaddr=0x%x depc=0x%x",
//    rst_info->exccause, rst_info->epc1, rst_info->epc2, rst_info->epc3,
//    rst_info->excvaddr, rst_info->depc);
////  uint32_t fid = spi_flash_get_id();
////  NOTICE("Flash map %s, manuf 0x%02lX chip 0x%04lX", flash_maps[system_get_flash_size_map()],
////      fid & 0xff, (fid&0xff00)|((fid>>16)&0xff));
//  NOTICE("** esp-link ready");
    
  // Init SNTP service
//  cgiServicesSNTPInit();
#ifdef MQTT
  NOTICE("initializing MQTT");
  mqtt_client_init();
#endif
  NOTICE("initializing user application");
  app_init();
  NOTICE("Waiting for work to do...");
}
