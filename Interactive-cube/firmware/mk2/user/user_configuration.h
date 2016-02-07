#ifndef _USER_CONFIGURATION_H_
#define _USER_CONFIGURATION_H_

void config_init();

void config_save();

const char *config_get_cubeid();
const char *config_get_ssid();
const char *config_get_password();

uint16_t config_get_localport();
uint16_t config_get_remoteport();
uint8_t *config_get_remoteip();

void config_set_cubeid(const char *value);
void config_set_ssid(const char *value);
void config_set_password(const char *value);

void config_set_localport(uint16_t value);
void config_set_remoteip(uint8_t *value);
void config_set_remoteport(uint16_t value);

#endif