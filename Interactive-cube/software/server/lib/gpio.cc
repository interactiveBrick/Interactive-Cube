// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2013 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio_port_+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio_port_+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

/*static*/ const uint32_t ::rgb_matrix::GPIO::kValidBits 
= ((1 <<  0) | (1 <<  1) | // Revision 1 accessible
   (1 <<  2) | (1 <<  3) | // Revision 2 accessible
   (1 <<  4) | (1 <<  7) | (1 << 8) | (1 <<  9) |
   (1 << 10) | (1 << 11) | (1 << 14) | (1 << 15)| (1 <<17) | (1 << 18)|
   (1 << 22) | (1 << 23) | (1 << 24) | (1 << 25)| (1 << 27)
// Add support for A+/B+!
 | (1 <<  5) | (1 <<  6) | (1 << 12) | (1 << 13) | (1 << 16) |
   (1 << 19) | (1 << 20) | (1 << 21) | (1 << 26)  
);
   

namespace rgb_matrix {
GPIO::GPIO() : output_bits_(0), gpio_port_(NULL) {
}
   
uint32_t GPIO::InitOutputs(uint32_t outputs) {
  if (gpio_port_ == NULL) {
    fprintf(stderr, "Attempt to init outputs but initialized.\n");
    return 0;
  }
  outputs &= kValidBits;   // Sanitize input.
  output_bits_ = outputs;
  for (uint32_t b = 0; b <= 27; ++b) {
    if (outputs & (1 << b)) {
      INP_GPIO(b);   // for writing, we first need to set as input.
      OUT_GPIO(b);
    }
  }
  return output_bits_;
}

// Detect Pi board type.  Doesn't return super-granular details,
// just the most basic distinction needed for GPIO compatibility:
// 0: Pi 1 Model B revision 1
// 1: Pi 1 Model B revision 2, Model A, Model B+, Model A+
// 2: Pi 2 Model B

int boardType(void) {
	FILE *fp;
	char  buf[1024], *ptr;
	int   n, board = 1; // Assume Pi1 Rev2 by default

	// Relies on info in /proc/cmdline.  If this becomes unreliable
	// in the future, alt code below uses /proc/cpuinfo if any better.
#if 1
	if((fp = fopen("/proc/cmdline", "r"))) {
		while(fgets(buf, sizeof(buf), fp)) {
			if((ptr = strstr(buf, "mem_size=")) &&
			   (sscanf(&ptr[9], "%x", &n) == 1) &&
			   (n == 0x3F000000)) {
				board = 2; // Appears to be a Pi 2
				break;
			} else if((ptr = strstr(buf, "boardrev=")) &&
			          (sscanf(&ptr[9], "%x", &n) == 1) &&
			          ((n == 0x02) || (n == 0x03))) {
				board = 0; // Appears to be an early Pi
				break;
			}
		}
		fclose(fp);
	}
#else
	char s[8];
	if((fp = fopen("/proc/cpuinfo", "r"))) {
		while(fgets(buf, sizeof(buf), fp)) {
			if((ptr = strstr(buf, "Hardware")) &&
			   (sscanf(&ptr[8], " : %7s", s) == 1) &&
			   (!strcmp(s, "BCM2709"))) {
				board = 2; // Appears to be a Pi 2
				break;
			} else if((ptr = strstr(buf, "Revision")) &&
			          (sscanf(&ptr[8], " : %x", &n) == 1) &&
			          ((n == 0x02) || (n == 0x03))) {
				board = 0; // Appears to be an early Pi
				break;
			}
		}
		fclose(fp);
	}
#endif

	return board;
}

// Based on code example found in http://elinux.org/RPi_Low-level_peripherals
bool GPIO::Init() {
  int mem_fd, gpio_base;

  if(boardType() == 2) { // Raspberry Pi 2?

    // Nasty kludge for timing on Pi 2, see notes in framebuffer.cc
    extern volatile uint32_t *freeRunTimer;

    // On Pi2, before we mmap GPIO, let's get the timer peripheral...
    if((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      perror("can't open /dev/mem: ");
      return 1;
    }

    char *timer_map =
      (char*)mmap(NULL,
      BLOCK_SIZE,
      PROT_READ | PROT_WRITE,
      MAP_SHARED,
      mem_fd,
      0x3F003000); // Offset to free-running 1 MHz timer

    close(mem_fd); // No need to keep mem_fd open after mmap

    if(timer_map == MAP_FAILED) {
      fprintf(stderr, "mmap error %ld\n", (long)timer_map);
      return 1;
    }

    freeRunTimer = &((volatile uint32_t *)timer_map)[1];
    // Okay then, getting back to GPIO...

    gpio_base = 0x3F000000 + 0x200000; // GPIO base addr for Pi 2
  } else {
    gpio_base = 0x20000000 + 0x200000; // " for Pi 1
  }

  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    perror("can't open /dev/mem: ");
    return false;
  }

  char *gpio_map =
    (char*) mmap(NULL,             //Any adddress in our space will do
         BLOCK_SIZE,       //Map length
         PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
         MAP_SHARED,       //Shared with other processes
         mem_fd,           //File to map
         gpio_base);       //Offset to GPIO peripheral

  close(mem_fd); //No need to keep mem_fd open after mmap

  if (gpio_map == MAP_FAILED) {
    fprintf(stderr, "mmap error %ld\n", (long)gpio_map);
    return false;
  }

  gpio_port_ = (volatile uint32_t *)gpio_map;

  return true;
}
}  // namespace rgb_matrix
