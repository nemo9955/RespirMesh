// #include <cstdarg>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>
#include "mesh-packet.pb.h"

#include "RemOrchestrator.hpp"
#include "RemLogger.hpp"
#include "RemRouter.hpp"
#include "RemChannel.hpp"
#include "TaskLooper.hpp"
#include "RemHardware.hpp"

#include "ESP8266/SimpleWiFiScanner.hpp"
#include "ESP8266/EspAsyncServer.hpp"
#include "ESP8266/EspAsyncClient.hpp"
#import "ESP8266/EspUtils.hpp"

#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>

void setup();
void loop();

TaskLooper update_looper;

const char __wifi_ssid[] = _S_WIFI_SSID;
const char __wifi_pass[] = _S_WIFI_PASS;
const char __ota_http_user[] = _S_OTA_HTTP_USER;
const char __ota_http_pass[] = _S_OTA_HTTP_PASS;
const char __ota_http_path[] = _S_OTA_HTTP_PATH;
char __esp_host[10] = "rspm-";
// const size_t __server_port = _SERVER_PORT;

bool IS_LEVEL_R1 = false;
IPAddress rootIP(_SERVER_IP);
IPAddress gateway;
IPAddress parentServerIP;

IPAddress APlocal_IP(192, 168, 4, 22);
IPAddress APgateway(192, 168, 4, 9);
IPAddress APsubnet(255, 255, 255, 0);

uint16_t succesfullPongFromServer = 1;

uint8_t chClientsCnt = 0;

class ESP8266_HARDWARE : public RemHardware
{
  private:
  public:
    ESP8266_HARDWARE(){};

    uint32_t device_id()
    {
        return ESP.getChipId();
    }

    uint32_t sleep_milis(uint32_t duration)
    {
        delay(duration);
    };

    uint32_t time_milis()
    {
        return millis();
    };
};

void async_server_init();
void async_client_init(RemClientInfo *client_info);

SimpleWiFiScanner parentScanner;

ESP8266_HARDWARE hardware_;
RemRouter remRouter;
RemOrchestrator remOrch;
RemLogger logs;

void setup()
{
    remOrch.set_hardware(&hardware_);
    remOrch.set_logger(&logs);
    remOrch.set_router(&remRouter);
    remOrch.set_scanner(&parentScanner);

    parentScanner.set_client_func(&async_client_init);
    parentScanner.set_server_func(&async_server_init);

    Serial.begin(115200);
    Serial.println();

    randomSeed(analogRead(0));

    MDNS.begin(__esp_host);

    remOrch.begin();
    update_looper.begin(remOrch.basicHardware);
    update_looper.set(1000);

    // mesh.set_hardware(&hardware_);
    // mesh.add_channel(&espTcpClient);
    // mesh.add_channel(&espTcpServer);
}

bool sta_started = false;
bool ap_started = false;

void loop()
{
    if (update_looper.check())
    {
        remOrch.update();
    }
}

#define MAX_AP_CLIENTS 8
EspAsyncClient chClients[MAX_AP_CLIENTS];
uint8_t conectedCClientsCnt = 0;

void async_server_init()
{
    remOrch.logs->info("async_server_init");
}

void async_client_init(RemClientInfo *client_info)
{
    remOrch.logs->info("async_client_init root %u  ip %u.%u.%u.%u : %u \n",
                       client_info->connected_to_root,
                       client_info->ip0, client_info->ip1, client_info->ip2, client_info->ip3,
                       client_info->port);

    // chClients[conectedCClientsCnt] = new EspAsyncClient();

    // IPAddress rem_server_ip(client_info->ip0 ,client_info->ip1 ,client_info->ip2 ,client_info->ip3 );
    // client_info->port

    IPAddress rem_server_ip(192, 168, 1, 17);
    chClients[conectedCClientsCnt].init(
        &rem_server_ip,
        9995,
        &remOrch);

    chClients[conectedCClientsCnt].connected_to_root = client_info->connected_to_root;

    remOrch.add_channel(&chClients[conectedCClientsCnt]);
    conectedCClientsCnt++;
    // if (client_->is_connected == true)
    // {
    //     remOrch->add_channel(move(client_));
    //     is_client_connected = true;
    // }
}
