
#include "platform.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "user_interface.h"

#include "user_display.h"
#include "user_comm.h"

#include "ets_sys.h"
#include "driver/uart.h"
#include "mem.h"

// #include "platform/platform.h"
// #include "driver/i2c_master.h"







static const uint8_t ledLUT[16] = {
	0x3A, 0x37, 0x35, 0x34,
	0x28, 0x29, 0x23, 0x24,
	0x16, 0x1B, 0x11, 0x10,
	0x0E, 0x0D, 0x0C, 0x02
};

static const uint8_t buttonLUT[16] = {
	0x07, 0x04, 0x02, 0x22,
	0x05, 0x06, 0x00, 0x01,
	0x03, 0x10, 0x30, 0x21,
	0x13, 0x12, 0x11, 0x31
};



uint8_t pixels[6*4*4] = { 0, };
bool pixels_dirty = true;
uint8_t buttons[6*4*4] = { 0, };
uint8_t lastbuttons[6*4*4] = { 0, };
uint32_t buttondelay;
uint32_t updateinterval;



#define LED_ON  1
#define LED_OFF 0

#define HT16K33_BLINK_OFF    0
#define HT16K33_BLINK_2HZ    1
#define HT16K33_BLINK_1HZ    2
#define HT16K33_BLINK_HALFHZ 3


#define HT16K33_BLINK_CMD       0x80
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_CMD_BRIGHTNESS  0xE0

static const uint32_t endpoint_id = 0;

uint32_t trellis_read_keyboard(int addr) {

	uint8_t k, x;
	uint8_t keybuffer[6] = { 0, };
	uint16_t bitmask;
	uint32_t output = 0;

	/*
	k = pgm_read_byte(&buttonLUT[k]);
	return (lastkeys[k>>4] & _BV(k & 0x0F));

	boolean Adafruit_Trellis::readSwitches(void) {
	memcpy(lastkeys, keys, sizeof(keys));
	Wire.beginTransmission((byte)i2c_addr);
	Wire.write(0x40);
	Wire.endTransmission();
	Wire.requestFrom((byte)i2c_addr, (byte)6);
	for (uint8_t i=0; i<6; i++)
		keys[i] = Wire.read();
	}
	return false;
	}
	*/

	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, addr, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, 0x40);
	platform_i2c_send_stop(endpoint_id);

	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, addr, PLATFORM_I2C_DIRECTION_RECEIVER);
	for(k=0; k<6; k++) {
		keybuffer[k] = platform_i2c_recv_byte(endpoint_id, k < 5);
	}
	platform_i2c_send_stop(endpoint_id);

	for(k=0; k<16; k++) {
		x = buttonLUT[k];
		bitmask = 1 << (x & 0x0F);
		if ((keybuffer[x >> 4] & bitmask) == bitmask) {
			bitmask = 1 << k;
			output |= bitmask;
		}
	}

	return output;
}

void trellis_set_brightness(int addr, int b) {
	// Wire.beginTransmission(i2c_addr);
	// Wire.write(HT16K33_CMD_BRIGHTNESS | b);
	// Wire.endTransmission();

	if (b > 15) b = 15;
	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, addr, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, HT16K33_CMD_BRIGHTNESS | b);
	platform_i2c_send_stop(endpoint_id);

}

void trellis_set_blinkrate(int addr, int b) {
	// Wire.beginTransmission(i2c_addr);
	// if (b > 3) b = 0; // turn off if not sure
	// Wire.write(HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (b << 1));
	// Wire.endTransmission();

	if (b > 3) b = 3;
	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, addr, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (b << 1));
	platform_i2c_send_stop(endpoint_id);
}

void trellis_update_leds(int addr, uint32_t leds) {
	// Wire.beginTransmission(i2c_addr);
	// Wire.write((uint8_t)0x00); // start at address $00
	// for (uint8_t i=0; i<8; i++) {
	// Wire.write(displaybuffer[i] & 0xFF);
	// Wire.write(displaybuffer[i] >> 8);
	// }
	// Wire.endTransmission();
	// x = pgm_read_byte(&ledLUT[x]);
  	// displaybuffer[x >> 4] |= _BV(x & 0x0F);


	uint8_t i, x;
	uint16_t bitmask;
	uint16_t displaybuffer[8] = { 0, };

	for(i=0; i<16; i++) {
		bitmask = 1 << i;
		if ((leds & bitmask) == bitmask) {
			x = ledLUT[i];
			bitmask = 1 << (x & 0x0F);
			displaybuffer[x >> 4] |= bitmask;
		}
	}

	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, addr, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, 0x00);
	for(i=0; i<8; i++) {
		platform_i2c_send_byte(endpoint_id, displaybuffer[i] & 255);
		platform_i2c_send_byte(endpoint_id, displaybuffer[i] >> 8);
	}
	platform_i2c_send_stop(endpoint_id);
}

void trellis_init(int addr) {
	// Wire.begin();
	// Wire.beginTransmission(i2c_addr);
	// Wire.write(0x21);  // turn on oscillator
	// Wire.endTransmission();
	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, addr, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, 0x21);
	platform_i2c_send_stop(endpoint_id);

	// blinkRate(HT16K33_BLINK_OFF);
	trellis_set_blinkrate(addr, HT16K33_BLINK_OFF);

	// setBrightness(15); // max brightness
	trellis_set_brightness(addr, 15);

	// Wire.beginTransmission(i2c_addr);
	// Wire.write(0xA1);  // turn on interrupt, active low
	// Wire.endTransmission();
	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, addr, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, 0xA1);
	platform_i2c_send_stop(endpoint_id);
}





void display_set_pixel(uint8_t pixel, uint8_t value) {
	pixels[pixel] = value;
	pixels_dirty = true;
}

void display_set_side(uint8_t side, uint32_t value) {
	int k;
	for(k=0; k<16; k++) {
		int bitmask = 1 << k;
		pixels[side*16+0+k] = (value & bitmask) == bitmask;
	}
	pixels_dirty = true;
}

void display_set_all(uint8_t *values) {
	int k;
	for(k=0; k<8; k++) {
		int bitmask = 1 << k;
		pixels[0+k] = (values[0] & bitmask) == bitmask;
		pixels[8+k] = (values[1] & bitmask) == bitmask;
		pixels[16+k] = (values[2] & bitmask) == bitmask;
		pixels[24+k] = (values[3] & bitmask) == bitmask;
		pixels[32+k] = (values[4] & bitmask) == bitmask;
		pixels[40+k] = (values[5] & bitmask) == bitmask;
		pixels[48+k] = (values[6] & bitmask) == bitmask;
		pixels[56+k] = (values[7] & bitmask) == bitmask;
		pixels[64+k] = (values[8] & bitmask) == bitmask;
		pixels[72+k] = (values[9] & bitmask) == bitmask;
		pixels[80+k] = (values[10] & bitmask) == bitmask;
		pixels[88+k] = (values[11] & bitmask) == bitmask;
	}
	pixels_dirty = true;
}

uint16_t startanim[6] = {
	1,
	1+2,
	1+2+4,
	1+2+4+8,
	0xFFFF,
	0,
};

void display_anim() {

	uint8_t f, s, a;
	for(f=0; f<=7; f++) {
		system_soft_wdt_feed();
		for(a=0; a<6; a++) {
			for(s=0; s<6; s++) {
				if (f == s) {
					trellis_update_leds(0x70 + s, startanim[a]);
				} else {
					trellis_update_leds(0x70 + s, 0x0000);
				}
			}
		}
		os_delay_us(500 * 1000);
    }

	pixels_dirty = true;
	buttondelay = 0;
	updateinterval = 0;
}

void display_init() {

    os_printf("Initializing display...\n");

    memset(pixels, 0, 4*4*6);
    memset(lastbuttons, 0, 4*4*6);
    memset(buttons, 0, 4*4*6);

	trellis_init(0x70);
	trellis_init(0x71);
	trellis_init(0x72);
	trellis_init(0x73);
	trellis_init(0x74);
	trellis_init(0x75);

	uint8_t s;
	for(s=0; s<6; s++) {
		trellis_update_leds(0x70 + s, 0x0000);
	}

	pixels_dirty = true;
	buttondelay = 0;
	updateinterval = 0;
}

uint8_t display_is_key_down(uint8_t key) {
	return buttons[key] > 0 ? 1 : 0;
}

void display_update() {
	if (updateinterval % 10 == 0) {
		display_update_inner();
	}
	updateinterval ++;
}

void display_update_inner() {

	// scan keyboard.

	uint8_t i;
	uint32_t bitmask;

	uint32_t keyboardstate;

	keyboardstate = trellis_read_keyboard(0x70);
	for(i=0; i<16; i++) {
		bitmask = 1 << i;
		buttons[0 + i] = ((keyboardstate & bitmask) == bitmask) > 0;
	}
	keyboardstate = trellis_read_keyboard(0x71);
	for(i=0; i<16; i++) {
		bitmask = 1 << i;
		buttons[16 + i] = ((keyboardstate & bitmask) == bitmask) > 0;
	}
	keyboardstate = trellis_read_keyboard(0x72);
	for(i=0; i<16; i++) {
		bitmask = 1 << i;
		buttons[32 + i] = ((keyboardstate & bitmask) == bitmask) > 0;
	}
	keyboardstate = trellis_read_keyboard(0x73);
	for(i=0; i<16; i++) {
		bitmask = 1 << i;
		buttons[48 + i] = ((keyboardstate & bitmask) == bitmask) > 0;
	}
	keyboardstate = trellis_read_keyboard(0x74);
	for(i=0; i<16; i++) {
		bitmask = 1 << i;
		buttons[64 + i] = ((keyboardstate & bitmask) == bitmask) > 0;
	}
	keyboardstate = trellis_read_keyboard(0x75);
	for(i=0; i<16; i++) {
		bitmask = 1 << i;
		buttons[80 + i] = ((keyboardstate & bitmask) == bitmask) > 0;
	}

	for(i=0; i<6*16; i++) {
		if (buttons[i] != lastbuttons[i]) {
			if (buttondelay > 100) {
				if (buttons[i]) {
					// key down
					comm_send_key(i, 1);
				} else {
					// key up
					comm_send_key(i, 0);
				}
			}
			lastbuttons[i] = buttons[i];
		}
	}

	if (buttondelay < 1000) {
		buttondelay ++;
	}

	if (pixels_dirty) {
		// send pixels to boards.
		uint32_t ledstate;
		ledstate = 0;
		for(i=0; i<16; i++) {
			bitmask = 1 << i;
			if (pixels[0 + i]) {
				ledstate |= bitmask;
			}
		}
		trellis_update_leds(0x70, ledstate);
		ledstate = 0;
		for(i=0; i<16; i++) {
			bitmask = 1 << i;
			if (pixels[16 + i]) {
				ledstate |= bitmask;
			}
		}
		trellis_update_leds(0x71, ledstate);
		ledstate = 0;
		for(i=0; i<16; i++) {
			bitmask = 1 << i;
			if (pixels[32 + i]) {
				ledstate |= bitmask;
			}
		}
		trellis_update_leds(0x72, ledstate);
		ledstate = 0;
		for(i=0; i<16; i++) {
			bitmask = 1 << i;
			if (pixels[48 + i]) {
				ledstate |= bitmask;
			}
		}
		trellis_update_leds(0x73, ledstate);
		ledstate = 0;
		for(i=0; i<16; i++) {
			bitmask = 1 << i;
			if (pixels[64 + i]) {
				ledstate |= bitmask;
			}
		}
		trellis_update_leds(0x74, ledstate);
		ledstate = 0;
		for(i=0; i<16; i++) {
			bitmask = 1 << i;
			if (pixels[80 + i]) {
				ledstate |= bitmask;
			}
		}
		trellis_update_leds(0x75, ledstate);
		pixels_dirty = false;
	}

	system_soft_wdt_feed();
}
