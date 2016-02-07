
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

#include "user_configui_html.h"




struct HttpRequestInfo {
  char method[10];
  char url[256];
  char postbody[1024];
};

enum {
  PARSING_FIRSTLINE = 0,
  PARSING_HEADER = 1,
  PARSING_BODY = 2,
  PARSING_DONE = 3,

  POSTBODY_PARSING_KEYNAME = 4,
  POSTBODY_PARSING_VALUE = 5,
};

bool should_reboot = false;

void parse_http_request(char *data, int len, struct HttpRequestInfo *info) {
  int state = PARSING_FIRSTLINE;
  char headerline[200] = { 0, };
  char *c = data;
  char *firstchar = NULL;
  char *lastpostbody = 0;
  int i;

  os_printf("start = %X\n", c);

  for(i=0; i<len; i++) {
    if (firstchar == NULL) {
      firstchar = c;
    }

    // os_printf("Handle char 0x%02X (%c)\n", *c, *c);

    if (state == PARSING_HEADER || state == PARSING_FIRSTLINE) {
      if (*c == '\n') {
        int linelen = (c - firstchar) - 1;
        os_printf("Got newline; linelen=%d\n", linelen);
        if (linelen > 0) {
          // we are past the headers.
          if (linelen > 200) { linelen = 200; }
          memcpy(headerline, firstchar, linelen);
          firstchar = NULL;

          if (state == PARSING_FIRSTLINE) {
            os_printf("Handle first header line: %s\n", headerline);
            state = PARSING_HEADER;
            strncpy(info->method, headerline, strstr(headerline, " ") - headerline);
            os_printf("Handle method: \"%s\"\n", info->method);
          } else {
            os_printf("Handle other header line: %s\n", headerline);
          }

        } else {
          os_printf("Handle empty line.\n");
          state = PARSING_BODY;
          firstchar = NULL;
        }
      }
    } else if (state == PARSING_BODY) {
      os_printf("Handle postbody data 0x%02X %c\n", *c, *c);
      lastpostbody = c;
    }

    c ++;
  }

  os_printf("last postbody char = %X\n", lastpostbody);
  int postbodylen = lastpostbody - firstchar + 1;
  os_printf("last postbody len = %d\n", postbodylen);

  if (postbodylen > 1 && firstchar != NULL) {
    memcpy(info->postbody, firstchar, postbodylen);
  }

  os_printf("last postbody: \"%s\"\n", info->postbody);
}

void postbody_get_value(struct HttpRequestInfo *info, char *key, char *value) {
  int state = POSTBODY_PARSING_KEYNAME;
  char *c = (char *)&info->postbody;
  char *firstchar = c;
  char keybuffer[30] = { 0, };
  char valuebuffer[200] = { 0, };
  int i;
  for(i=0; i<strlen(info->postbody); i++) {
    // os_printf("Parsing postbody char %02X (%c)\n", *c, *c);

    if (*c == '=') {
      memset(keybuffer, 0, 30);
      memcpy(keybuffer, firstchar, (c - firstchar));

      os_printf("found postbody key \"%s\"\n", keybuffer);

      firstchar = c + 1;
      state = POSTBODY_PARSING_KEYNAME;
    } else if (*c == '&') {

      memset(valuebuffer, 0, 200);
      memcpy(valuebuffer, firstchar, (c - firstchar));

      os_printf("found postbody value \"%s\"\n", valuebuffer);

      if (strcmp(keybuffer, key) == 0) {
        strcpy(value, valuebuffer); // save value
      }

      // check last value if any

      firstchar = c + 1;
      state = POSTBODY_PARSING_VALUE;
    } else if (state == POSTBODY_PARSING_KEYNAME) {
    } else if (state == POSTBODY_PARSING_VALUE) {
    }

    c ++;
  }

  // end of input, check value.

  memset(valuebuffer, 0, 200);
  memcpy(valuebuffer, firstchar, (c - firstchar));

  os_printf("found postbody value \"%s\"\n", valuebuffer);

  if (strcmp(keybuffer, key) == 0) {
    strcpy(value, valuebuffer); // save value
  }
}




static void replacestr(char *line, const char *search, const char *replace)
{
     char *sp;

     if ((sp = strstr(line, search)) == NULL) {
         return;
     }

     int search_len = strlen(search);
     int replace_len = strlen(replace);
     int tail_len = strlen(sp+search_len);

     memmove(sp+replace_len,sp+search_len,tail_len+1);
     memcpy(sp, replace, replace_len);
}

char http_response_buffer[5000] = { 0, };

os_event_t    reboot_taskQueue[1];

void handle_http_request(struct HttpRequestInfo *req, char **output, int *output_len) {

  char buf[20] = { 0, };

  if (strcmp(req->method, "POST") == 0) {

    char ssidvalue[30] = { 0, };
    postbody_get_value(req, "ssid", (char *)&ssidvalue);
    os_printf("Postbody: SSID = \"%s\".", ssidvalue);
    if (strlen(ssidvalue) > 0) {
      config_set_ssid(ssidvalue);
    }

    char passwordvalue[30] = { 0, };
    postbody_get_value(req, "password", (char *)&passwordvalue);
    os_printf("Postbody: Password = \"%s\".", passwordvalue);
    if (strlen(passwordvalue) > 0) {
      config_set_password(passwordvalue);
    }

    char cubeidvalue[30] = { 0, };
    postbody_get_value(req, "cubeid", (char *)&cubeidvalue);
    os_printf("Postbody: Cube ID = \"%s\".", cubeidvalue);
    if (strlen(cubeidvalue) > 0) {
      if (strlen(cubeidvalue) > 8) {
        cubeidvalue[8] = 0;
      }
      config_set_cubeid(cubeidvalue);
    }

    char remoteip[30] = { 0, };
    postbody_get_value(req, "remoteip", (char *)&remoteip);
    os_printf("Postbody: Remote IP = \"%s\".", remoteip);
    // TODO: Parse IP into bytes
    char *dot1,*dot2,*dot3;
    dot1 = strstr(remoteip, ".");
    if (dot1 != NULL) {
      dot2 = strstr(dot1 + 1, ".");
      if (dot2 != NULL) {
        dot3 = strstr(dot2 + 1, ".");
        if (dot3 != NULL) {
          uint8_t ip[4];

          // do some parsing here
          strncpy(buf, remoteip, dot1-remoteip);
          ip[0] = atoi(buf);
          strncpy(buf, dot1 + 1, dot2-dot1);
          ip[1] = atoi(buf);
          strncpy(buf, dot2 + 1, dot3-dot2);
          ip[2] = atoi(buf);
          strcpy(buf, dot3 + 1);
          ip[3] = atoi(buf);

          config_set_remoteip((uint8_t *)&ip);
        }
      }
    }

    char remoteport[30] = { 0, };
    postbody_get_value(req, "remoteport", (char *)&remoteport);
    os_printf("Postbody: Remote Port = \"%s\".", remoteport);
    if (strlen(remoteport) > 0) {
      config_set_remoteport(atoi(remoteport));
    }

    char localport[30] = { 0, };
    postbody_get_value(req, "localport", (char *)&localport);
    os_printf("Postbody: Local Port = \"%s\".", localport);
    if (strlen(localport) > 0) {
      config_set_localport(atoi(localport));
    }

    config_save();
    should_reboot = true;

    strcpy(http_response_buffer, reboot_html);

  } else {

    strcpy(http_response_buffer, configui_html);

    replacestr(http_response_buffer, "{ssid}", config_get_ssid());

    replacestr(http_response_buffer, "{password}", config_get_password());

    replacestr(http_response_buffer, "{cubeid}", config_get_cubeid());

    uint8_t *remoteipptr = config_get_remoteip();
    c_sprintf(buf, "%d.%d.%d.%d", remoteipptr[0], remoteipptr[1], remoteipptr[2], remoteipptr[3]);
    replacestr(http_response_buffer, "{remoteip}", buf);

    uint16_t remoteport = config_get_remoteport();
    c_sprintf(buf, "%d", remoteport);
    replacestr(http_response_buffer, "{remoteport}", buf);

    uint16_t localport = config_get_localport();
    c_sprintf(buf, "%d", localport);
    replacestr(http_response_buffer, "{localport}", buf);

  }

  *output = (char *)&http_response_buffer;
  *output_len = strlen(http_response_buffer);
}




void wifi_callback2 (System_Event_t *evt) {
  os_printf( "%s: %d\n", __FUNCTION__, evt->event );
  switch (evt->event) {
    case EVENT_SOFTAPMODE_STACONNECTED: {
      os_printf("connect to ssid %s, channel %d\n",
                  evt->event_info.connected.ssid,
                  evt->event_info.connected.channel);
      break;
    }

    case EVENT_SOFTAPMODE_STADISCONNECTED: {
      os_printf("disconnect from ssid %s, reason %d\n",
                  evt->event_info.disconnected.ssid,
                  evt->event_info.disconnected.reason);
      break;
    }

    case EVENT_SOFTAPMODE_PROBEREQRECVED: {
      os_printf("probe received.\n");
    }
  }
}



#define MAX_CONNECTIONS 8
#define HTTPD_PORT 80

LOCAL unsigned int mConnected = 0;
LOCAL unsigned int mConfigured = 0;
LOCAL struct espconn *mCurrentPost = 0;
LOCAL char *mPostBuf = 0;
LOCAL unsigned int mPostBufPos = 0;

#define RECONFIGURE_DELAY_MS 5000

LOCAL os_timer_t mReconfigureTimer;

LOCAL void ICACHE_FLASH_ATTR check_send_res(sint8 res) {
  if(res)
    os_printf("espconn_send returned: %d", res);
}

LOCAL void ICACHE_FLASH_ATTR dhap_httpd_disconnect_cb(void *arg) {
  mConnected--;
  os_printf("Client disconnected, %u left", mConnected);
}

LOCAL void ICACHE_FLASH_ATTR dhap_httpd_sent_cb(void *arg) {
  struct espconn *conn = arg;
  espconn_disconnect(conn);
  os_printf("Data sent");
}

LOCAL void ICACHE_FLASH_ATTR dhap_httpd_recv_cb(void *arg, char *data, unsigned short len) {
  struct espconn *conn = arg;

  os_printf("Received %d bytes", len);

  struct HttpRequestInfo reqinfo;
  memset(&reqinfo, 0, sizeof(struct HttpRequestInfo));
  parse_http_request(data, len, &reqinfo);

  os_printf("Request parsed.");


  char *output = NULL;
  int output_len = 0;
  handle_http_request(&reqinfo, &output, &output_len);

  os_printf("Send form, %u bytes", output_len);

  check_send_res(espconn_send(conn, output, output_len));

  /*
  if (should_reboot) {
    os_delay_us(500 * 1000);
    system_restart();
  }
  */
}

LOCAL void ICACHE_FLASH_ATTR dhap_httpd_reconnect_cb(void *arg, sint8 err) {
  mConnected--;
  os_printf("Connection error occurred %d", err);
}

LOCAL void ICACHE_FLASH_ATTR dhap_httpd_connect_cb(void *arg) {
  struct espconn *conn = arg;
  mConnected++;
  if(mConnected > MAX_CONNECTIONS) {
    espconn_disconnect(conn);
    os_printf("Refuse connection, already %u connections", mConnected);
    return;
  }
  espconn_regist_recvcb(conn, dhap_httpd_recv_cb);
  espconn_regist_disconcb(conn, dhap_httpd_disconnect_cb);
  espconn_regist_sentcb(conn, dhap_httpd_sent_cb);
  os_printf("Client connected");
}






























// #define MAX_CONNECTIONS 2
#define DNSD_PORT 53

LOCAL unsigned int mDNSdConnected = 0;
LOCAL unsigned char mSendingInProgress = 0;
#define DNS_ANSWER_BUF_SIZE 1024
LOCAL char *mDNSAnswerBuffer;

// struct declaration for little endian systems
// all fields should be filled with big endian
typedef struct __attribute__((packed)) {
  uint16_t id;
  unsigned recursionDesired : 1;
  unsigned truncatedMessage : 1;
  unsigned authoritiveAnswer : 1;
  unsigned opcode : 4;
  unsigned responseFlag : 1;
  unsigned rcode : 4;
  unsigned resevered : 2;
  unsigned primaryServer : 1;
  unsigned recursionAvailable : 1;
  uint16_t questionsNumber;
  uint16_t answersNumber;
  uint16_t authoritiesNumber;
  uint16_t resourcesNumber;
} DNS_HEADER;

typedef struct __attribute__((packed)) {
  uint16_t nameOffset;
  uint16_t type;
  uint16_t class;
  uint32_t ttl;
  uint16_t ipSize;
  uint32_t ip;
} DNS_ANSWER;

LOCAL uint16_t ICACHE_FLASH_ATTR htobe_16(uint16_t n) {
  uint32_t res;
  uint8_t *p = (uint8_t *) &res;
  p[1] = n % 0x100;
  n /= 0x100;
  p[0] = n % 0x100;
  return res;
}

LOCAL int ICACHE_FLASH_ATTR dnsd_answer(char *data, unsigned int len) {
  // always add response with host address data to the end
  DNS_HEADER *header = (DNS_HEADER *)data;
  DNS_ANSWER *answer = (DNS_ANSWER *)(&data[len]);
  header->answersNumber = htobe_16(1);
  header->authoritiesNumber = 0;
  header->resourcesNumber = 0;
  header->responseFlag = 1;
  header->authoritiveAnswer = 0;
  header->recursionAvailable = 1;
  header->rcode = 0;

  answer->nameOffset = htobe_16(0xC000 | sizeof(DNS_HEADER));
  answer->type = htobe_16(1);  // A - host address
  answer->class = htobe_16(1); // IN - class
  answer->ttl = 0;
  answer->ipSize = htobe_16(sizeof(answer->ip));
  answer->ip = 0;

  struct ip_info info;
  if(wifi_get_ip_info(SOFTAP_IF, &info))
    answer->ip = info.ip.addr;

  return len + sizeof(DNS_ANSWER);
}

LOCAL void ICACHE_FLASH_ATTR dhap_dnsd_disconnect_cb(void *arg) {
  mDNSdConnected--;
  os_printf("dnsd disconnect, %d left\n", mDNSdConnected);
}

LOCAL void ICACHE_FLASH_ATTR dhap_dnsd_sent_cb(void *arg) {
  struct espconn *conn = arg;
  mSendingInProgress = 0;
  if(conn->type & ESPCONN_TCP)
    espconn_disconnect(conn);
  os_printf("dnsd sent\n");
}

LOCAL void ICACHE_FLASH_ATTR dhap_dnsd_recv_cb(void *arg, char *data, unsigned short len) {
  struct espconn *conn = arg;
  if(mSendingInProgress || len + sizeof(DNS_ANSWER) > DNS_ANSWER_BUF_SIZE) {
    if(mSendingInProgress)
      os_printf("Got %u bytes, but dnsd is busy, drop\n", len);
    else
      os_printf("Got %u bytes, too big for this dnsd\n", len);
    if(conn->type & ESPCONN_TCP)
      espconn_disconnect(conn);
    return;
  }
  os_printf("dnsd received %d bytes\n", len);
  os_memcpy(mDNSAnswerBuffer, data, len);
  unsigned int rlen = dnsd_answer(mDNSAnswerBuffer, len);
  if(rlen) {
    mSendingInProgress = 1;
    if(espconn_send(conn, mDNSAnswerBuffer, rlen)) {
      os_printf("Failed to send response\n");
      mSendingInProgress = 0;
      if(conn->type & ESPCONN_TCP)
        espconn_disconnect(conn);
    }
  } else {
    os_printf("Wrong dns request\n");
    if(conn->type & ESPCONN_TCP)
      espconn_disconnect(conn);
  }
}

LOCAL void ICACHE_FLASH_ATTR dhap_dnsd_reconnect_cb(void *arg, sint8 err) {
  mDNSdConnected--;
  os_printf("dnsd connect error %d\n", err);
}

LOCAL void ICACHE_FLASH_ATTR dhap_dnsd_connect_cb(void *arg) {
  struct espconn *conn = arg;
  mDNSdConnected++;
  if(mDNSdConnected > MAX_CONNECTIONS) {
    espconn_disconnect(conn);
    os_printf("dnsd refuse\n");
    return;
  }
  espconn_regist_recvcb(conn, dhap_dnsd_recv_cb);
  espconn_regist_disconcb(conn, dhap_dnsd_disconnect_cb);
  espconn_regist_sentcb(conn, dhap_dnsd_sent_cb);
  os_printf("dnsd connected\n");
}

void ICACHE_FLASH_ATTR dhap_dnsd_init() {
  mDNSAnswerBuffer = (char *)os_malloc(DNS_ANSWER_BUF_SIZE);

  esp_tcp *dnsdTcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  dnsdTcp->local_port = DNSD_PORT;
  struct espconn *dnsdConnTCP = (struct espconn *)os_zalloc(sizeof(struct espconn ));
  dnsdConnTCP->type = ESPCONN_TCP;
  dnsdConnTCP->state = ESPCONN_NONE;
  dnsdConnTCP->proto.tcp = dnsdTcp;

  espconn_regist_connectcb(dnsdConnTCP, dhap_dnsd_connect_cb);
  espconn_regist_reconcb(dnsdConnTCP, dhap_dnsd_reconnect_cb);
  espconn_accept(dnsdConnTCP);

  esp_udp *dnsdUdp = (esp_udp *)os_zalloc(sizeof(esp_tcp));
  dnsdUdp->local_port = DNSD_PORT;
  struct espconn *dnsdConnUDP = (struct espconn *)os_zalloc(sizeof(struct espconn ));
  dnsdConnUDP->type = ESPCONN_UDP;
  dnsdConnUDP->state = ESPCONN_NONE;
  dnsdConnUDP->proto.udp = dnsdUdp;

  espconn_regist_recvcb(dnsdConnUDP, dhap_dnsd_recv_cb);
  espconn_regist_sentcb(dnsdConnUDP, dhap_dnsd_sent_cb);
  espconn_create(dnsdConnUDP);
}


















void setup_init() {
  os_printf("Starting Configuration AP...\n");

  should_reboot = false;

  struct softap_config apconfig;

  os_memset(apconfig, 0, sizeof(apconfig));

  uint8_t mac[6] = { 0, };
  wifi_get_macaddr(STATION_IF, (uint8_t *)&mac);

  c_sprintf(apconfig.ssid, "Cube Setup - %02X%02X%02X", mac[0], mac[1], mac[2]);

  apconfig.ssid_len = strlen(apconfig.ssid);
  apconfig.authmode = AUTH_OPEN;
  apconfig.ssid_hidden = 0;
  apconfig.channel = 1;
  apconfig.max_connection = 4;
  apconfig.beacon_interval = 100;

  wifi_set_event_handler_cb( wifi_callback2 );

  os_printf("Disconnecting...\n");
  wifi_station_disconnect();

  os_printf("Set opmode...\n");
  if (!wifi_set_opmode(SOFTAP_MODE)) os_printf("wifi_set_opmode failed.\n");

  if (!wifi_softap_dhcps_stop()) os_printf("wifi_softap_dhcps_stop failed.\n");

  os_printf("Setting config...\n");
  if (!wifi_softap_set_config(&apconfig)) os_printf("wifi_softap_set_config failed.\n");
  // if (!wifi_set_phy_mode(PHY_MODE_11G)) os_printf("wifi_set_phy_mode failed.\n");

  struct ip_info *ipinfo = (struct ip_info *)os_zalloc(sizeof(struct ip_info));
  if (!wifi_get_ip_info(SOFTAP_IF, ipinfo)) os_printf("wifi_get_ip_info failed.\n");
  IP4_ADDR(&ipinfo->ip, 192, 168, 0, 1);
  IP4_ADDR(&ipinfo->gw, 192, 168, 0, 1);
  IP4_ADDR(&ipinfo->netmask, 255, 255, 255, 0);
  espconn_dns_setserver(0, &ipinfo->ip);
  if (!wifi_set_ip_info(SOFTAP_IF, ipinfo)) os_printf("wifi_set_ip_info failed.\n");

  os_printf("Starting DHCP...\n");
  if (!wifi_softap_dhcps_start()) os_printf("wifi_softap_dhcps_start failed.\n");


  dhap_dnsd_init();


  struct espconn *httpdConn = (struct espconn *)os_zalloc(sizeof(struct espconn ));
  esp_tcp *httpdTcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  httpdConn->type = ESPCONN_TCP;
  httpdConn->state = ESPCONN_NONE;
  httpdTcp->local_port = HTTPD_PORT;
  httpdConn->proto.tcp = httpdTcp;

  espconn_regist_connectcb(httpdConn, dhap_httpd_connect_cb);
  espconn_regist_reconcb(httpdConn, dhap_httpd_reconnect_cb);
  sint8 res = espconn_accept(httpdConn);
  if(res) os_printf("espconn_accept failed.\n");


  os_printf("Listening for connections...\n");
}

bool setup_loop() {
  /*
  system_soft_wdt_feed();
  display_set_pixel(0, 1);
  display_set_pixel(1, 1);
  display_update_inner();
  os_delay_us(100*1000);

  display_set_pixel(0, 0);
  display_set_pixel(1, 0);
  display_update_inner();
  */
  os_delay_us(900*1000);
  system_soft_wdt_feed();

  os_printf("In setup ... opmode=%d \n", wifi_get_opmode());

  if (display_is_key_down(2)) {
      return false;
  }

  return true;
}

void setup_teardown() {

  wifi_station_disconnect();

}











//           1         2         3         4         5         6         7         8         9
// 0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0

const char *scroller =
  "......XX..XXX...Xxx.X..x......X..XXXx.XXX......X..X.....XXx......Xx......X.....x...x......"
  ".....X..X.X..X.XXx..Xx.X......X..X..X....X.....X..XXX..XX.X.....X..X.....X................"
  ".....X..X.XXx..X....X.xX......X..XXXx...X......X..X..X.X.XX.....X..X.....X.....x...x......"
  "......XX..X.....XxX.X..X......X.....X.XXXX..X..X...XX..XXX...X...XX...X..X.......xx.......";


void setup_draw_scroll() {
  uint16_t f, i, j, x, o;

  for(f=0; f<90-4; f++) {

    system_soft_wdt_feed();

    for(j=0; j<4; j++) {
      for(i=0; i<4; i++) {
        x = (i + f) % 90;
        o = j * 90 + x;

        display_set_pixel(j * 4 + i, (scroller[o] != '.') ? 1 : 0);
      }
    }

    display_update_inner();
    os_delay_us(60*1000);
  }
}









void configui_init() {

  // bring up setup network
  setup_init();

  setup_draw_scroll();

  /*
  // run ui
  while(true) {
      if(!setup_loop()) {
          break;
      }
  }

  // free up stuff
  setup_teardown();
  */

  // return and reboot
}




