/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/

// #include "lua.h"
#include "platform.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "user_interface.h"
#include "user_exceptions.h"
#include "user_modules.h"

#include "user_configuration.h"
#include "user_display.h"
#include "user_comm.h"
#include "user_orient.h"

#include "ets_sys.h"
#include "driver/uart.h"
#include "mem.h"

#include "espconn.h"
#include "lwip/udp.h"


// #ifdef LUA_USE_MODULES_RTCTIME
// #include "rtc/rtctime.h"
// #endif

// #define SIG_LUA 0
#define SIG_UARTINPUT 1
// #define TASK_QUEUE_LEN 1

// static os_event_t *taskQueue;
long loopcounter = 0;

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void loop(os_event_t *events);



/* Important: no_init_data CAN NOT be left as zero initialised, as that
 * initialisation will happen after user_start_trampoline, but before
 * the user_init, thus clobbering our state!
 */
static uint8_t no_init_data = 0xff;

void TEXT_SECTION_ATTR user_start_trampoline (void)
{
  __real__xtos_set_exception_handler(EXCCAUSE_LOAD_STORE_ERROR, load_non_32_wide_handler);

  SPIFlashInfo sfi;
  SPIRead (0, &sfi, sizeof (sfi)); // Cache read not enabled yet, safe to use
  if (sfi.size < 2) // Compensate for out-of-order 4mbit vs 2mbit values
    sfi.size ^= 1;
  uint32_t flash_end_addr = (256 * 1024) << sfi.size;
  uint32_t init_data_hdr = 0xffffffff;
  SPIRead (flash_end_addr - 4 * SPI_FLASH_SEC_SIZE, &init_data_hdr, sizeof (init_data_hdr));
  no_init_data = (init_data_hdr == 0xffffffff);

  call_user_start();
}


/* To avoid accidentally losing the fix for the TCP port randomization
 * during an LWIP upgrade, we've implemented most it outside the LWIP
 * source itself. This enables us to test for the presence of the fix
 * /at link time/ and error out if it's been lost.
 * The fix itself consists of putting the function-static 'port' variable
 * into its own section, and get the linker to provide an alias for it.
 * From there we can then manually randomize it at boot.
 */
static inline void tcp_random_port_init (void)
{
  extern uint16_t _tcp_new_port_port; // provided by the linker script
  _tcp_new_port_port += xthal_get_ccount () % 4096;
}

static const uint32_t i2c_endpoint_id = 0;

static void ICACHE_FLASH_ATTR loop(os_event_t *events)
{
    if (loopcounter % 1000 == 0) {
      os_printf("Hello loop %d\n", loopcounter);

      comm_debuginfo();
    }

    loopcounter++;

    // change GPIO 2 level
    //    level = !level;
    //    GPIO_OUTPUT_SET(GPIO_ID_PIN(2), ((level) ? 1 : 0));
    // Delay

    display_update();
    orient_update();
    comm_update();

    os_delay_us(1000);

    // turn again
    system_os_post(user_procTaskPrio, 0, 0 );
}

void nodemcu_init(void)
{
    NODE_DBG("Hello World!.\n");

    NODE_ERR("\n");
    // Initialize platform first for lua modules.
    if( platform_init() != PLATFORM_OK )
    {
        // This should never happen
        NODE_DBG("Can not init platform for modules.\n");
        return;
    }

#if defined(FLASH_SAFE_API)
    if( flash_safe_get_size_byte() != flash_rom_get_size_byte()) {
        NODE_ERR("Self adjust flash size.\n");
        // Fit hardware real flash size.
        flash_rom_set_size_byte(flash_safe_get_size_byte());
        // Write out init data at real location.
        no_init_data = true;
    }
#endif // defined(FLASH_SAFE_API)

        NODE_DBG("Yoink 1.\n");

    if (no_init_data)
    {
        NODE_ERR("Restore init data.\n");
        // Flash init data at FLASHSIZE - 0x04000 Byte.
        flash_init_data_default();
        // Flash blank data at FLASHSIZE - 0x02000 Byte.
        flash_init_data_blank();
        // Reboot to make the new data come into effect
        system_restart ();
    }

    NODE_DBG("Yoink 2.\n");
    tcp_random_port_init ();

    NODE_DBG("Yoink 3.\n");

    system_os_post(LUA_TASK_PRIO, 0, 0);
}




/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    os_delay_us(1000*1000);
    uart_init (BIT_RATE_115200, BIT_RATE_115200, USER_TASK_PRIO_0, 0);
    os_delay_us(100*1000);

    os_printf("Interactive Cube starting..\n");

    config_init();

    platform_i2c_setup(i2c_endpoint_id, 1, 2, PLATFORM_I2C_SPEED_SLOW);
    display_init();

    NODE_DBG("SDK version: %s\n", system_get_sdk_version());
    system_print_meminfo();

    os_printf("Heap size: %d.\n", system_get_free_heap_size());

#ifndef NODE_DEBUG
    system_set_os_print(1);
#endif

    int b;
    for(b=0; b<4; b++) {
        display_set_pixel(0, 1);
        display_update_inner();

        os_delay_us(500*1000);
        display_update_inner();

        display_set_pixel(0, 0);
        display_update_inner();

        os_delay_us(500*1000);
        display_update_inner();
    }

    if (display_is_key_down(0)) {
        // enter setup mode, show as blinking...!
        configui_init();
        return;
    }

    display_set_pixel(0, 1);
    display_set_pixel(1, 1);
    display_set_pixel(2, 1);
    display_update_inner();

    // uart_init(BIT_RATE_9600,0);
    os_delay_us(100*1000);

    os_delay_us(1000*1000);

    display_anim();
    //  os_delay_us(500*1000);

    NODE_DBG("Hello Wurst.\n");
    // os_delay_us(500*1000);

    // Set up I2C
    // Wire.begin(D1, D2); // sda, scl

    display_set_pixel(0, 0);
    display_set_pixel(1, 0);
    display_set_pixel(2, 0);
    display_set_pixel(3, 0);
    display_update();

    comm_init();
    orient_init();

    // system_init_done_cb(nodemcu_init);

    system_os_task(loop, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
    system_os_post(user_procTaskPrio, 0, 0);

    NODE_DBG("End of user_init.\n");
}
