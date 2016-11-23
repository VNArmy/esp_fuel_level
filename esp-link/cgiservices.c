#include <esp8266.h>
#include "cgiwifi.h"
#include "cgi.h"
#include "config.h"
#include "sntp.h"

#ifdef CGISERVICES_DBG
#define DBG(format, ...) do { os_printf(format, ## __VA_ARGS__); } while(0)
#else
#define DBG(format, ...) do { } while(0)
#endif

char* rst_codes[7] = {
  "normal", "wdt reset", "exception", "soft wdt", "restart", "deep sleep", "external",
};

char* flash_maps[7] = {
  "512KB:256/256", "256KB", "1MB:512/512", "2MB:512/512", "4MB:512/512",
  "2MB:1024/1024", "4MB:1024/1024"
};

static ETSTimer reassTimer;

// Cgi to update system info (name/description)
int ICACHE_FLASH_ATTR cgiSystemSet(HttpdConnData *connData) {
  if (connData->conn == NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.

  int8_t n = getStringArg(connData, "name", flashConfig.hostname, sizeof(flashConfig.hostname));
  int8_t d = getStringArg(connData, "description", flashConfig.sys_descr, sizeof(flashConfig.sys_descr));

  if (n < 0 || d < 0) return HTTPD_CGI_DONE; // getStringArg has produced an error response

  if (n > 0) {
    // schedule hostname change-over
    os_timer_disarm(&reassTimer);
    os_timer_setfn(&reassTimer, configWifiIP, NULL);
    os_timer_arm(&reassTimer, 1000, 0); // 1 second for the response of this request to make it
  }

  if (configSave()) {
    httpdStartResponse(connData, 204);
    httpdEndHeaders(connData);
  }
  else {
    httpdStartResponse(connData, 500);
    httpdEndHeaders(connData);
    httpdSend(connData, "Failed to save config", -1);
  }
  return HTTPD_CGI_DONE;
}

// Cgi to return various System information
int ICACHE_FLASH_ATTR cgiSystemInfo(HttpdConnData *connData) {
  char buff[1024];

  if (connData->conn == NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.

  uint8 part_id = system_upgrade_userbin_check();
  uint32_t fid = spi_flash_get_id();
  struct rst_info *rst_info = system_get_rst_info();

  os_sprintf(buff,
    "{ "
      "\"name\": \"%s\", "
      "\"reset cause\": \"%d=%s\", "
      "\"size\": \"%s\", "
      "\"id\": \"0x%02lX 0x%04lX\", "
      "\"partition\": \"%s\", "
      "\"baud\": \"%ld\", "
	  "\"sid\": \"%02X %02X%02X\", "
      "\"description\": \"%s\""
    " }",
    flashConfig.hostname,
    rst_info->reason,
    rst_codes[rst_info->reason],
    flash_maps[system_get_flash_size_map()],
    fid & 0xff, (fid & 0xff00) | ((fid >> 16) & 0xff),
    part_id ? "user2.bin" : "user1.bin",
    flashConfig.baud_rate,
    flashConfig.device_id[0],flashConfig.device_id[1],flashConfig.device_id[2],
    flashConfig.sys_descr
    );

  jsonHeader(connData, 200);
  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}

