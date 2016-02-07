
#include "platform.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "user_interface.h"

#include "user_display.h"
#include "user_comm.h"
#include "user_configuration.h"

#include "ets_sys.h"
#include "driver/uart.h"
#include "mem.h"

#include "espconn.h"
#include "lwip/udp.h"

#include "spiffs.h"
#include "spi_flash.h"

// wifi connection variables
// const char* ssid = "possan";
// const char* password = "kebabkebab";
bool wifiConnected = false;
// const unsigned int localPort = 3333;
// const char *remoteIP = "192.168.1.107";
bool udpConnected = false;
long totalbytes = 0;
long totalpackets = 0;
struct udp_pcb *ptel_pcb;
static esp_tcp global_udp;                                  // udp connect var (see espconn.h)
static struct espconn *last_conn;
uint8_t *last_message;
uint8_t last_message_len;


int parse_osc(unsigned char *bufptr, int siz) {
  int i;

  const char *name = config_get_cubeid();

  if (siz < 6)
    return 0;

  // check name
  int namelen = strlen(name);
  if (namelen > 8) namelen = 8;
  if (memcmp(bufptr + 1, name, namelen) != 0) {
    // osc name didn't match.
    return 0;
  }


  if (memcmp(bufptr + 9, "/led\0\0", 6) == 0 && siz == 28) {
    #ifdef USE_SERIAL
    // Serial.println("Looks like /cube1/led command!");
    #endif

    //
    // 0000   2f 63 75 62 65 31 2f 6c 65 64 00 00 2c 69 69 00  /cube1/led..,ii.
    // 0010   00 00 00 06 00 00 00 01                          ........
    //

    if (bufptr[16] == 0x2C && bufptr[17] == 'i' && bufptr[18] == 'i') {
      // Serial.println("Has correct argument signature");

      unsigned int1value = bufptr[19+4];
      unsigned int2value = bufptr[23+4];

      display_set_pixel(int1value, int2value);
    }

    return 24;
  } else if (memcmp(bufptr + 9, "/side\0", 6) == 0 && siz == 36) {
    #ifdef USE_SERIAL
    // Serial.println("Looks like /cube1/side command!");
    #endif

    //
    // 0000   2f 63 75 62 65 30 2f 73 69 64 65 00 2c 69 69 69  /cube0/side.,iii
    // 0010   00 00 00 __ 00 00 00 xx 00 00 00 37 00 00 00 4d                                                  ...........7...M
    //


    if (bufptr[16] == 0x2C && bufptr[17] == 'i' && bufptr[18] == 'i' && bufptr[19] == 'i') {
      #ifdef USE_SERIAL
      // Serial.println("Has correct argument signature");
      #endif

      unsigned side = bufptr[23+4];
      unsigned b1 = bufptr[27+4];
      unsigned b2 = bufptr[31+4];

      display_set_side(side, (b1 << 8) + b2);
    }

    return 32;
  } else if (memcmp(bufptr + 9, "/leds\0", 6) == 0 && siz == 80) {
    #ifdef USE_SERIAL
    // Serial.println("Looks like /cube1/leds command!");
    #endif

    //
    // 0000   2f 63 75 62 65 31 2f 6c 65 64 ?? 00 2c 69 69 69  /cube1/leds.,iii
    // 0010   69 69 69 69 69 69 69 69 69 00 00 00 00 00 00 0a  iiiiiiiii.......
    // 0020   00 00 00 14 00 00 00 1e 00 00 00 28 00 00 00 32  ...........(...2
    // 0030   00 00 00 3c 00 00 00 0a 00 00 00 14 00 00 00 1e  ...<............
    // 0040   00 00 00 28 00 00 00 32 00 00 00 3c              ...(...2...<
    //

    if (bufptr[16] == 0x2C && bufptr[17] == 'i' && bufptr[18] == 'i' && bufptr[19] == 'i' &&
        bufptr[20] == 'i' && bufptr[21] == 'i' && bufptr[22] == 'i' && bufptr[23] == 'i' &&
        bufptr[24] == 'i' && bufptr[25] == 'i' && bufptr[26] == 'i' && bufptr[27] == 'i' &&
        bufptr[28] == 'i') {
      #ifdef USE_SERIAL
      // Serial.println("Has correct argument signature");
      #endif

      uint8_t b[12];

      for(i=0; i<12; i++) {
        b[i] = bufptr[31 + (4 * i) + 4];
      }

      display_set_all((uint8_t *)&b);
    }

    return 76;
  }
  return 0;
}





static void ICACHE_FLASH_ATTR network_received(void *arg, char *data, unsigned short len) {
  os_printf("udp packet #%d, %d bytes.\n", totalpackets, len);
  parse_osc(data, len);
  totalbytes += len;
  totalpackets ++;
}

struct espconn *pUdpServer;

void udp_listen() {
  pUdpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
  pUdpServer->type = ESPCONN_UDP;
  pUdpServer->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
  pUdpServer->proto.udp->local_port = config_get_localport();

  espconn_regist_recvcb(pUdpServer, network_received);
  if (espconn_create(pUdpServer) == 0) {
    os_printf("UDP OK\n\r");
  }
}

void wifi_callback(System_Event_t *evt) {
  os_printf( "%s: %d\n", __FUNCTION__, evt->event );
  switch (evt->event) {
    case EVENT_STAMODE_CONNECTED: {
      os_printf("connect to ssid %s, channel %d\n",
                  evt->event_info.connected.ssid,
                  evt->event_info.connected.channel);
      break;
    }

    case EVENT_STAMODE_DISCONNECTED: {
      os_printf("disconnect from ssid %s, reason %d\n",
                  evt->event_info.disconnected.ssid,
                  evt->event_info.disconnected.reason);
      break;
    }

    case EVENT_STAMODE_GOT_IP: {
      os_printf("Got IP! " IPSTR ", netmask: " IPSTR ", gateway:" IPSTR,
                  IP2STR(&evt->event_info.got_ip.ip),
                  IP2STR(&evt->event_info.got_ip.mask),
                  IP2STR(&evt->event_info.got_ip.gw));
      os_printf("\n");
      break;
    }
  }
}

static inline void tcp_random_port_init (void) {
  extern uint16_t _tcp_new_port_port; // provided by the linker script
  _tcp_new_port_port += xthal_get_ccount () % 4096;
}

bool connectWifi2() {
  bool state = true;
  int i = 0;

  ptel_pcb = udp_new();
  
  os_printf("");
  os_printf("Connecting to WiFi");
  os_printf("Connecting...");

  struct station_config conf;
  conf.bssid_set = 0;
  strcpy((char *)conf.ssid, config_get_ssid());
  strcpy((char *)conf.password, config_get_password());

  // ETS_UART_INTR_DISABLE();
  wifi_set_event_handler_cb( wifi_callback );
  wifi_set_opmode(STATION_MODE);
  // wifi_station_set_auto_connect(0);
  wifi_station_set_reconnect_policy(true);
  wifi_station_set_config(&conf);
  // wifi_station_connect();
  // wifi_station_dhcpc_start();
  // ETS_UART_INTR_ENABLE();

  while ((wifi_get_opmode() & 1) == 0) {
    os_delay_us(500 * 1000);
    os_printf(".");

    if (i > 100) {
      state = false;
      break;
    }

    i++;
  }

  if (state) {
    os_printf("");
    os_printf("Connected to ");
    os_printf(conf.ssid);
    // os_printf("IP address: ");
    // os_printf(WiFi.localIP());
  } else {
    os_printf("");
    os_printf("Connection failed.");
  }

  return state;
}








void comm_init() {
	// tcp_random_port_init ();
  connectWifi2();
  udp_listen();
}

void comm_update() {
}



void comm_debuginfo() {
  os_printf("Comm total bytes: %d.\n", totalbytes);
}

static void ICACHE_FLASH_ATTR udpNetworkConnectedCb(void *arg) {
  struct espconn *conn = (struct espconn *)arg;
  // espconn_regist_recvcb(conn, tcpNetworkRecvCb);

  os_printf("UDP connected\n\r"); 
  // espconn_sent(conn, last_message, last_message_len);
  // UDP_conn_ok = TRUE;
}

static void ICACHE_FLASH_ATTR udpNetworkReconCb(void *arg, sint8 err) {
  os_printf("UDP reconnect\n\r");
  // UDP_conn_ok = FALSE;
  // network_init();
}

static void ICACHE_FLASH_ATTR udpNetworkDisconCb(void *arg) {
  os_printf("UDP disconnect\n\r");
  // UDP_conn_ok = FALSE;
  // network_init();
}

static void ICACHE_FLASH_ATTR udpNetworkSentCb(void *arg) {
  os_printf("UDP sent\n\r");
  // tcp_conn_ok = FALSE;
  // network_init();
}

int16_t comm_fromUnsigned16(uint16_t in) {
  return (int16_t)in;
}

uint16_t comm_fromSigned16(int16_t in) {
  return (uint16_t)in;
}

uint32_t comm_fromSigned16_toSigned32(int16_t in)  {
  return (uint32_t)(int32_t)in;
}

void comm_send_osc(uint8_t *bytes, uint8_t len) {

  last_message = bytes;
  last_message_len = len;

  struct espconn tmpconn;
  esp_udp tmpudp;
  memset(&tmpconn, 0, sizeof(struct espconn));
  memset(&tmpudp, 0, sizeof(esp_udp));

  uint16_t localport = config_get_localport();
  uint16_t remoteport = config_get_remoteport();
  uint8_t *remoteip = config_get_remoteip();

  tmpconn.type = ESPCONN_UDP;
  tmpconn.state = ESPCONN_NONE;
  tmpconn.proto.udp = &tmpudp;
  tmpconn.proto.udp->local_port = espconn_port();
  tmpconn.proto.udp->remote_port = remoteport;
  tmpconn.proto.udp->remote_ip[0] = remoteip[0];
  tmpconn.proto.udp->remote_ip[1] = remoteip[1];
  tmpconn.proto.udp->remote_ip[2] = remoteip[2];
  tmpconn.proto.udp->remote_ip[3] = remoteip[3];

  espconn_create(&tmpconn);
  espconn_sent(&tmpconn, bytes, len);
  espconn_disconnect(&tmpconn);
}

void comm_send_key(uint8_t btn, uint8_t down) {
  uint8_t message[28];

  os_printf("send OSC key event; btn=%d, down=%d\n", btn, down);

  const char *name = config_get_cubeid();

  //
  // 0000   2f 63 75 62 65 31 2f 6c 65 64 73 00 2c 69 69 00  /cube1/leds.,ii.
  // 0010   00 00 00 06 00 00 00 01                          ........
  //
  // 0000   2f 63 75 62 65 4e 4e 4e 4e 2f 6c 65 64 73 00 00  /cubeNNNN/leds..
  // 0010   2c 69 69 69 69 00 00 00 00 00 00 01 00 00 00 02  ,iiii...........
  // 0020   00 00 00 03 00 00 00 04                          ........
  //

  message[0] = '/';
  message[1] = '_';
  message[2] = '_';
  message[3] = '_';
  message[4] = '_';
  message[5] = '_';
  message[6] = '_';
  message[7] = '_';
  message[8] = '_';
  message[9] = '/';
  message[10] = 'b';
  message[11] = 't';
  message[12] = 'n';
  message[13] = 0;
  message[14] = 0;
  message[15] = 0;

  int namelen = strlen(name);
  if (namelen > 8) namelen = 8;
  memcpy(message + 1, name, namelen); // inject name

  message[16] = 0x2C;
  message[17] = 'i';
  message[18] = 'i';
  message[19] = 0;

  message[20] = 0;
  message[21] = 0;
  message[22] = 0;
  message[23] = btn;
  message[24] = 0;
  message[25] = 0;
  message[26] = 0;
  message[27] = down;

  comm_send_osc(message, sizeof(message));
}


void comm_send_acceleration(int16_t ax, int16_t ay, int16_t az) {
  uint8_t message[36];

  // os_printf("send OSC acceleration event; ax=%d, ay=%d, az=%d\n", ax, ay, az);

  //
  // 0000   2f 63 75 62 65 31 2f 6c 65 64 73 00 2c 69 69 00  /cube1/leds.,ii.
  // 0010   00 00 00 06 00 00 00 01                          ........
  //

  uint32_t uax = comm_fromSigned16_toSigned32(ax);
  uint32_t uay = comm_fromSigned16_toSigned32(ay);
  uint32_t uaz = comm_fromSigned16_toSigned32(az);

  const char *name = config_get_cubeid();

  message[0] = '/';
  message[1] = '_';
  message[2] = '_';
  message[3] = '_';
  message[4] = '_';
  message[5] = '_';
  message[6] = '_';
  message[7] = '_';
  message[8] = '_';
  message[9] = '/';
  message[10] = 'a';
  message[11] = 'c';
  message[12] = 'c';
  message[13] = 0;
  message[14] = 0;
  message[15] = 0;

  int namelen = strlen(name);
  if (namelen > 8) namelen = 8;
  memcpy(message + 1, name, namelen); // inject name

  message[16] = 0x2C;
  message[17] = 'i';
  message[18] = 'i';
  message[19] = 'i';
  message[20] = 0;
  message[21] = 0;
  message[22] = 0;
  message[23] = 0;

  message[24] = (uax >> 24) & 255;
  message[25] = (uax >> 16) & 255;
  message[26] = (uax >> 8) & 255;
  message[27] = (uax >> 0) & 255;

  message[28] = (uay >> 24) & 255;
  message[29] = (uay >> 16) & 255;
  message[30] = (uay >> 8) & 255;
  message[31] = (uay >> 0) & 255;

  message[32] = (uaz >> 24) & 255;
  message[33] = (uaz >> 16) & 255;
  message[34] = (uaz >> 8) & 255;
  message[35] = (uaz >> 0) & 255;

  comm_send_osc(message, sizeof(message));
}


void comm_send_rotation(int16_t rx, int16_t ry, int16_t rz) {
  uint8_t message[36];

  os_printf("send OSC rotation event; rx=%d, ry=%d, rz=%d\n", rx, ry, rz);

  //
  // 0000   2f 63 75 62 65 31 2f 6c 65 64 73 00 2c 69 69 00  /cube1/leds.,ii.
  // 0010   00 00 00 06 00 00 00 01                          ........
  //

  uint32_t urx = comm_fromSigned16_toSigned32(rx);
  uint32_t ury = comm_fromSigned16_toSigned32(ry);
  uint32_t urz = comm_fromSigned16_toSigned32(rz);

  const char *name = config_get_cubeid();

  message[0] = '/';
  message[1] = '_';
  message[2] = '_';
  message[3] = '_';
  message[4] = '_';
  message[5] = '_';
  message[6] = '_';
  message[7] = '_';
  message[8] = '_';
  message[9] = '/';
  message[10] = 'r';
  message[11] = 'o';
  message[12] = 't';
  message[13] = 0;
  message[14] = 0;
  message[15] = 0;

  int namelen = strlen(name);
  if (namelen > 8) namelen = 8;
  memcpy(message + 1, name, namelen); // inject name

  message[16] = 0x2C;
  message[17] = 'i';
  message[18] = 'i';
  message[19] = 'i';
  message[20] = 0;
  message[21] = 0;
  message[22] = 0;
  message[23] = 0;

  message[24] = (urx >> 24) & 255;
  message[25] = (urx >> 16) & 255;
  message[26] = (urx >> 8) & 255;
  message[27] = (urx >> 0) & 255;

  message[28] = (ury >> 24) & 255;
  message[29] = (ury >> 16) & 255;
  message[30] = (ury >> 8) & 255;
  message[31] = (ury >> 0) & 255;

  message[32] = (urz >> 24) & 255;
  message[33] = (urz >> 16) & 255;
  message[34] = (urz >> 8) & 255;
  message[35] = (urz >> 0) & 255;

  comm_send_osc(message, sizeof(message));
}



