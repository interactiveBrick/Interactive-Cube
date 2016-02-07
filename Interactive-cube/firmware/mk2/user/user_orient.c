#include "platform.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "user_interface.h"

#include "user_orient.h"
#include "user_comm.h"

#include "ets_sys.h"
#include "driver/uart.h"
#include "mem.h"

// From:
// https://github.com/adafruit/Adafruit_ADXL345
// https://github.com/adafruit/Adafruit_L3GD20

/*=========================================================================
I2C ADDRESS/BITS
-----------------------------------------------------------------------*/
#define ADXL345_ADDRESS                 (0x53)    // Assumes ALT address pin low
/*=========================================================================*/

/*=========================================================================
REGISTERS
-----------------------------------------------------------------------*/
#define ADXL345_REG_DEVID               (0x00)    // Device ID
#define ADXL345_REG_THRESH_TAP          (0x1D)    // Tap threshold
#define ADXL345_REG_OFSX                (0x1E)    // X-axis offset
#define ADXL345_REG_OFSY                (0x1F)    // Y-axis offset
#define ADXL345_REG_OFSZ                (0x20)    // Z-axis offset
#define ADXL345_REG_DUR                 (0x21)    // Tap duration
#define ADXL345_REG_LATENT              (0x22)    // Tap latency
#define ADXL345_REG_WINDOW              (0x23)    // Tap window
#define ADXL345_REG_THRESH_ACT          (0x24)    // Activity threshold
#define ADXL345_REG_THRESH_INACT        (0x25)    // Inactivity threshold
#define ADXL345_REG_TIME_INACT          (0x26)    // Inactivity time
#define ADXL345_REG_ACT_INACT_CTL       (0x27)    // Axis enable control for activity and inactivity detection
#define ADXL345_REG_THRESH_FF           (0x28)    // Free-fall threshold
#define ADXL345_REG_TIME_FF             (0x29)    // Free-fall time
#define ADXL345_REG_TAP_AXES            (0x2A)    // Axis control for single/double tap
#define ADXL345_REG_ACT_TAP_STATUS      (0x2B)    // Source for single/double tap
#define ADXL345_REG_BW_RATE             (0x2C)    // Data rate and power mode control
#define ADXL345_REG_POWER_CTL           (0x2D)    // Power-saving features control
#define ADXL345_REG_INT_ENABLE          (0x2E)    // Interrupt enable control
#define ADXL345_REG_INT_MAP             (0x2F)    // Interrupt mapping control
#define ADXL345_REG_INT_SOURCE          (0x30)    // Source of interrupts
#define ADXL345_REG_DATA_FORMAT         (0x31)    // Data format control
#define ADXL345_REG_DATAX0              (0x32)    // X-axis data 0
#define ADXL345_REG_DATAX1              (0x33)    // X-axis data 1
#define ADXL345_REG_DATAY0              (0x34)    // Y-axis data 0
#define ADXL345_REG_DATAY1              (0x35)    // Y-axis data 1
#define ADXL345_REG_DATAZ0              (0x36)    // Z-axis data 0
#define ADXL345_REG_DATAZ1              (0x37)    // Z-axis data 1
#define ADXL345_REG_FIFO_CTL            (0x38)    // FIFO control
#define ADXL345_REG_FIFO_STATUS         (0x39)    // FIFO status
/*=========================================================================*/

/*=========================================================================
REGISTERS
-----------------------------------------------------------------------*/
#define ADXL345_MG2G_MULTIPLIER (0.004)  // 4mg per lsb
/*=========================================================================*/


/* Used with register 0x2C (ADXL345_REG_BW_RATE) to set bandwidth */
typedef enum
{
  ADXL345_DATARATE_3200_HZ    = 0b1111, // 1600Hz Bandwidth   140µA IDD
  ADXL345_DATARATE_1600_HZ    = 0b1110, //  800Hz Bandwidth    90µA IDD
  ADXL345_DATARATE_800_HZ     = 0b1101, //  400Hz Bandwidth   140µA IDD
  ADXL345_DATARATE_400_HZ     = 0b1100, //  200Hz Bandwidth   140µA IDD
  ADXL345_DATARATE_200_HZ     = 0b1011, //  100Hz Bandwidth   140µA IDD
  ADXL345_DATARATE_100_HZ     = 0b1010, //   50Hz Bandwidth   140µA IDD
  ADXL345_DATARATE_50_HZ      = 0b1001, //   25Hz Bandwidth    90µA IDD
  ADXL345_DATARATE_25_HZ      = 0b1000, // 12.5Hz Bandwidth    60µA IDD
  ADXL345_DATARATE_12_5_HZ    = 0b0111, // 6.25Hz Bandwidth    50µA IDD
  ADXL345_DATARATE_6_25HZ     = 0b0110, // 3.13Hz Bandwidth    45µA IDD
  ADXL345_DATARATE_3_13_HZ    = 0b0101, // 1.56Hz Bandwidth    40µA IDD
  ADXL345_DATARATE_1_56_HZ    = 0b0100, // 0.78Hz Bandwidth    34µA IDD
  ADXL345_DATARATE_0_78_HZ    = 0b0011, // 0.39Hz Bandwidth    23µA IDD
  ADXL345_DATARATE_0_39_HZ    = 0b0010, // 0.20Hz Bandwidth    23µA IDD
  ADXL345_DATARATE_0_20_HZ    = 0b0001, // 0.10Hz Bandwidth    23µA IDD
  ADXL345_DATARATE_0_10_HZ    = 0b0000  // 0.05Hz Bandwidth    23µA IDD (default value)
} dataRate_t;

/* Used with register 0x31 (ADXL345_REG_DATA_FORMAT) to set g range */
typedef enum
{
  ADXL345_RANGE_16_G          = 0b11,   // +/- 16g
  ADXL345_RANGE_8_G           = 0b10,   // +/- 8g
  ADXL345_RANGE_4_G           = 0b01,   // +/- 4g
  ADXL345_RANGE_2_G           = 0b00    // +/- 2g (default value)
} range_t;

#define L3GD20_ADDRESS                (0x6B)        // 1101011
#define L3GD20_POLL_TIMEOUT           (100)         // Maximum number of read attempts
#define L3GD20_ID                     0xD4
#define L3GD20H_ID                    0xD7

#define L3GD20_SENSITIVITY_250DPS  (0.00875F)      // Roughly 22/256 for fixed point match
#define L3GD20_SENSITIVITY_500DPS  (0.0175F)       // Roughly 45/256
#define L3GD20_SENSITIVITY_2000DPS (0.070F)        // Roughly 18/256
#define L3GD20_DPS_TO_RADS         (0.017453293F)  // degress/s to rad/s multiplier

typedef enum
{                                               // DEFAULT    TYPE
  L3GD20_REGISTER_WHO_AM_I            = 0x0F,   // 11010100   r
  L3GD20_REGISTER_CTRL_REG1           = 0x20,   // 00000111   rw
  L3GD20_REGISTER_CTRL_REG2           = 0x21,   // 00000000   rw
  L3GD20_REGISTER_CTRL_REG3           = 0x22,   // 00000000   rw
  L3GD20_REGISTER_CTRL_REG4           = 0x23,   // 00000000   rw
  L3GD20_REGISTER_CTRL_REG5           = 0x24,   // 00000000   rw
  L3GD20_REGISTER_REFERENCE           = 0x25,   // 00000000   rw
  L3GD20_REGISTER_OUT_TEMP            = 0x26,   //            r
  L3GD20_REGISTER_STATUS_REG          = 0x27,   //            r
  L3GD20_REGISTER_OUT_X_L             = 0x28,   //            r
  L3GD20_REGISTER_OUT_X_H             = 0x29,   //            r
  L3GD20_REGISTER_OUT_Y_L             = 0x2A,   //            r
  L3GD20_REGISTER_OUT_Y_H             = 0x2B,   //            r
  L3GD20_REGISTER_OUT_Z_L             = 0x2C,   //            r
  L3GD20_REGISTER_OUT_Z_H             = 0x2D,   //            r
  L3GD20_REGISTER_FIFO_CTRL_REG       = 0x2E,   // 00000000   rw
  L3GD20_REGISTER_FIFO_SRC_REG        = 0x2F,   //            r
  L3GD20_REGISTER_INT1_CFG            = 0x30,   // 00000000   rw
  L3GD20_REGISTER_INT1_SRC            = 0x31,   //            r
  L3GD20_REGISTER_TSH_XH              = 0x32,   // 00000000   rw
  L3GD20_REGISTER_TSH_XL              = 0x33,   // 00000000   rw
  L3GD20_REGISTER_TSH_YH              = 0x34,   // 00000000   rw
  L3GD20_REGISTER_TSH_YL              = 0x35,   // 00000000   rw
  L3GD20_REGISTER_TSH_ZH              = 0x36,   // 00000000   rw
  L3GD20_REGISTER_TSH_ZL              = 0x37,   // 00000000   rw
  L3GD20_REGISTER_INT1_DURATION       = 0x38    // 00000000   rw
} l3gd20Registers_t;

typedef enum
{
  L3DS20_RANGE_250DPS,
  L3DS20_RANGE_500DPS,
  L3DS20_RANGE_2000DPS
} l3gd20Range_t;
/*



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
*/

uint32_t updateinterval;

static const uint32_t endpoint_id = 0;


void accelerometer_start() {
	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, ADXL345_ADDRESS, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, ADXL345_REG_POWER_CTL);
	platform_i2c_send_byte(endpoint_id, 0x08);
	platform_i2c_send_stop(endpoint_id);

	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, ADXL345_ADDRESS, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, ADXL345_REG_BW_RATE);
	platform_i2c_send_byte(endpoint_id, ADXL345_DATARATE_50_HZ);
	platform_i2c_send_stop(endpoint_id);
}

int16_t accelerometer_readSigned16(int reg) {

	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, ADXL345_ADDRESS, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, reg);
	platform_i2c_send_stop(endpoint_id);

	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, ADXL345_ADDRESS, PLATFORM_I2C_DIRECTION_RECEIVER);
	uint8_t b0 = platform_i2c_recv_byte(endpoint_id, 1);
	uint8_t b1 = platform_i2c_recv_byte(endpoint_id, 0);
	platform_i2c_send_stop(endpoint_id);

	uint16_t output = (b1 << 8) | b0;

	int16_t output2 = comm_fromUnsigned16(output);
	/*
	if (output < 32767)
		output2 = + (int)output;
	else
		output2 = - (int)(65536 - output);
	*/

	return output2;
}

void gyro_start() {
	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, L3GD20_ADDRESS, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, L3GD20_REGISTER_CTRL_REG1);
	platform_i2c_send_byte(endpoint_id, 0x0F);
	platform_i2c_send_stop(endpoint_id);

	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, L3GD20_ADDRESS, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, L3GD20_REGISTER_CTRL_REG2);
	platform_i2c_send_byte(endpoint_id, 0x08);
	platform_i2c_send_stop(endpoint_id);

	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, L3GD20_ADDRESS, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, L3GD20_REGISTER_CTRL_REG4);
	platform_i2c_send_byte(endpoint_id, 0x00);
	platform_i2c_send_stop(endpoint_id);

}

uint8_t gyro_read_all(int reg) {
	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, L3GD20_ADDRESS, PLATFORM_I2C_DIRECTION_TRANSMITTER);
	platform_i2c_send_byte(endpoint_id, reg);
	platform_i2c_send_stop(endpoint_id);

	platform_i2c_send_start(endpoint_id);
	platform_i2c_send_address(endpoint_id, L3GD20_ADDRESS, PLATFORM_I2C_DIRECTION_RECEIVER);
	uint8_t b0 = platform_i2c_recv_byte(endpoint_id, 0);
	platform_i2c_send_stop(endpoint_id);

	return b0;
}

void orient_init() {

	os_printf("Initializing accelerometer...\n");
	accelerometer_start();

	os_printf("Initializing gyro...\n");
	gyro_start();

	updateinterval = 0;
}

int16_t last_ax = 0;
int16_t last_ay = 0;
int16_t last_az = 0;

int16_t acc_gx = 0;
int16_t acc_gy = 0;
int16_t acc_gz = 0;

int16_t last_gx = 0;
int16_t last_gy = 0;
int16_t last_gz = 0;

void orient_update() {

	updateinterval ++;

	if (updateinterval % 20 == 0) {
		int16_t ax = accelerometer_readSigned16(ADXL345_REG_DATAX0);
		int16_t ay = accelerometer_readSigned16(ADXL345_REG_DATAY0);
		int16_t az = accelerometer_readSigned16(ADXL345_REG_DATAZ0);

		if (ax != last_ax ||
			ay != last_ay ||
			az != last_az) {

			comm_send_acceleration(ax, ay, az);

			last_ax = ax;
			last_ay = ay;
			last_az = az;
		}
	}

	if (updateinterval % 3 == 1) {

		platform_i2c_send_start(endpoint_id);
		platform_i2c_send_address(endpoint_id, L3GD20_ADDRESS, PLATFORM_I2C_DIRECTION_TRANSMITTER);
		platform_i2c_send_byte(endpoint_id, L3GD20_REGISTER_OUT_X_L | 0x80);
		platform_i2c_send_stop(endpoint_id);

		platform_i2c_send_start(endpoint_id);
		platform_i2c_send_address(endpoint_id, L3GD20_ADDRESS, PLATFORM_I2C_DIRECTION_RECEIVER);
		uint8_t b0 = platform_i2c_recv_byte(endpoint_id, 1);
		uint8_t b1 = platform_i2c_recv_byte(endpoint_id, 1);
		uint8_t b2 = platform_i2c_recv_byte(endpoint_id, 1);
		uint8_t b3 = platform_i2c_recv_byte(endpoint_id, 1);
		uint8_t b4 = platform_i2c_recv_byte(endpoint_id, 1);
		uint8_t b5 = platform_i2c_recv_byte(endpoint_id, 0);
		platform_i2c_send_stop(endpoint_id);

		int16_t gx = comm_fromUnsigned16((b1 << 8) | b0);
		int16_t gy = comm_fromUnsigned16((b3 << 8) | b2);
		int16_t gz = comm_fromUnsigned16((b5 << 8) | b4);

		acc_gx = ((acc_gx * 3) / 4) + (gx / 4);
		acc_gy = ((acc_gy * 3) / 4) + (gy / 4);
		acc_gz = ((acc_gz * 3) / 4) + (gz / 4);
	}

	if (updateinterval % 20 == 10) {
		if (acc_gx != last_gx ||
			acc_gy != last_gy ||
			acc_gz != last_gz) {

			comm_send_rotation(acc_gx, acc_gy, acc_gz);

			last_gx = acc_gx;
			last_gy = acc_gy;
			last_gz = acc_gz;
		}
	}
}
