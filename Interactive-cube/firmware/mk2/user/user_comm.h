#ifndef _USER_COMM_H_
#define _USER_COMM_H_

void comm_init();
void comm_update();
void comm_debuginfo();

int16_t comm_fromUnsigned16(uint16_t in);
uint16_t comm_fromSigned16(int16_t in);
uint32_t comm_fromSigned16_toSigned32(int16_t in);

void comm_send_key(uint8_t btn, uint8_t down);

void comm_send_acceleration(int16_t ax, int16_t ay, int16_t az);
void comm_send_rotation(int16_t rx, int16_t ry, int16_t rz);

void comm_send_osc(uint8_t *bytes, uint8_t len);

#endif