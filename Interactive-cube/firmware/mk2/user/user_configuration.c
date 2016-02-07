
#include "platform.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "user_interface.h"

#include "ets_sys.h"
#include "mem.h"

#include "flash_api.h"
#include "spi_flash.h"

#define WANTED_STATE 3

struct configblock {
  uint16_t state;
  uint16_t counter;

  char ssid[64];
  char password[64];
  char cubeid[16];

  uint8_t remoteip[4];
  uint16_t remoteport;
  uint16_t localport;
};

struct configblock configdata;

int flash_sector;

// extern uint32_t _SPIFFS_end;

void config_empty() {
  memset(configdata.cubeid, 0, 16);
  memset(configdata.ssid, 0, 64);
  memset(configdata.password, 0, 64);
  memset(configdata.remoteip, 0, 4);
  configdata.remoteport = 0;
  configdata.localport = 0;

}

void config_init() {

  flash_sector = 0x3c;// (((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE);

  // default config
  config_empty();

  // cli();
  if (flash_safe_read(flash_sector * SPI_FLASH_SEC_SIZE, (uint32 *)&configdata, sizeof(struct configblock)) != SPI_FLASH_RESULT_OK) {
    os_printf("Failed to read config from flash.\n");
  } else {
    os_printf("Successfully read config from flash.\n");
  }

  // interrupts();

  if (configdata.state != WANTED_STATE) {
    // firmware resetted reset.
    config_empty();
  }

  if (strlen(configdata.cubeid) == 0) {
    // strcpy(configdata.cubeid, "cube1");
    // TODO: default to cubeXYZW based on mac unique address
    // bool wifi_get_macaddr(0, uint8 *macaddr) 

    uint8_t mac[6] = { 0, };
    wifi_get_macaddr(STATION_IF, (uint8_t *)&mac);
    c_sprintf(configdata.cubeid, "cube%02X%02X", mac[0], mac[1]);
  }

  if (strlen(configdata.ssid) == 0) {
    strcpy(configdata.ssid, "possan");
  }

  if (strlen(configdata.password) == 0) {
    strcpy(configdata.password, "kebabkebab");
  }

  if (configdata.localport == 0) {
    configdata.localport = 3333;
  }

  if (configdata.remoteip[0] == 0) {
    configdata.remoteip[0] = 192;
    configdata.remoteip[1] = 168;
    configdata.remoteip[2] = 1;
    configdata.remoteip[3] = 107;
  }

  if (configdata.remoteport == 0) {
    configdata.remoteport = 3333;
  }
















  // load from EPROM
}

void config_save() {
  // save to EPROM
  os_printf("Saving configuration.\n");

  configdata.state = WANTED_STATE;
  configdata.counter ++;

  // cli();

  if(spi_flash_erase_sector(flash_sector) != SPI_FLASH_RESULT_OK) {
    os_printf("Failed to erase flash sector.\n");
  } else {
    os_printf("Successfully erased flash sector.\n");
  }

  if(spi_flash_write(flash_sector * SPI_FLASH_SEC_SIZE, (uint32 *)&configdata, sizeof(struct configblock)) != SPI_FLASH_RESULT_OK) {
    os_printf("Failed to write flash sector.\n");
  } else {
    os_printf("Successfully wrote flash sector.\n");
  }
}



const char *config_get_cubeid() {
  return (const char *)&configdata.cubeid;
}

const char *config_get_ssid() {
  return (const char *)&configdata.ssid;
}

const char *config_get_password() {
  return (const char *)&configdata.password;
}


uint16_t config_get_localport() {
  return configdata.localport;
}

uint16_t config_get_remoteport() {
  return configdata.remoteport;
}

uint8_t *config_get_remoteip() {
  return (uint8_t *)&configdata.remoteip;
}



void config_set_cubeid(const char *value) {
  memset(configdata.cubeid, 0, 16);
  strcpy(configdata.cubeid, value);
}

void config_set_ssid(const char *value) {
  memset(configdata.ssid, 0, 64);
  strcpy(configdata.ssid, value);
}

void config_set_password(const char *value) {
  memset(configdata.password, 0, 64);
  strcpy(configdata.password, value);
}

void config_set_localport(uint16_t value) {
  configdata.localport = value;
}

void config_set_remoteip(uint8_t *value) {
  memcpy(configdata.remoteip, value, 4);
}

void config_set_remoteport(uint16_t value) {
  configdata.remoteport = value;
}


