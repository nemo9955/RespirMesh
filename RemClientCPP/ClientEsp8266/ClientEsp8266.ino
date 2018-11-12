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

#import "ESP8266/SimpleWiFiScanner.hpp"
// #import "ESP8266/EspUtils.hpp"

#include <ESP8266mDNS.h>

void setup();
void espSetHost();
void loop();

const char __wifi_ssid[] = _S_WIFI_SSID;
const char __wifi_pass[] = _S_WIFI_PASS;
const char __ota_http_user[] = _S_OTA_HTTP_USER;
const char __ota_http_pass[] = _S_OTA_HTTP_PASS;
const char __ota_http_path[] = _S_OTA_HTTP_PATH;
char __esp_host[10] = "rspm-";
// const size_t __server_port = _SERVER_PORT;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long interval = 1000;

bool IS_LEVEL_R1 = false;
IPAddress rootIP(_SERVER_IP);
IPAddress gateway;
IPAddress parentServerIP;

IPAddress APlocal_IP(192, 168, 4, 22);
IPAddress APgateway(192, 168, 4, 9);
IPAddress APsubnet(255, 255, 255, 0);


uint16_t succesfullPongFromServer = 1;


uint8_t chClientsCnt = 0;

class ESP8266_HARDWARE : public Hardware
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

class ESP8266_TCPASYNC_CLIENT : public RemChannel
{
  private:
    // AsyncClient parClient;
    RemOrchestrator *remOrch;

  public:
    ESP8266_TCPASYNC_CLIENT(){};

    int ch_info() { return 500; }
    void init(int socket, RemOrchestrator *remOrch_)
    {
        remOrch = remOrch_;
        remOrch->logs->info("Local TCP started in socket:%d \n", socket);
    }
    void init(const char *address, int port, RemOrchestrator *remOrch_)
    {
        remOrch = remOrch_;
        remOrch->logs->info("Local TCP started %s:%d \n", address, port);
    }

    bool send(uint8_t *data, uint16_t size)
    {
        //logf("Client TCP sending .... \n");
        return 0;
    }

    bool is_ok()
    {
        return 1;
    };

    void stop()
    {
        remOrch->logs->info("Local TCP exiting \n");
    }
};

class ESP8266_TCPASYNC_SERVER : public RemChannel
{
  private:
    // AsyncClient parClient;
    RemOrchestrator *remOrch;

  public:
    ESP8266_TCPASYNC_SERVER(){};

    int ch_info() { return 500; }

    void init(const char *address, int port, RemOrchestrator *remOrch_)
    {
        remOrch = remOrch_;
        remOrch->logs->info("Server TCP started %s:%d \n", address, port);
    }

    bool send(uint8_t *data, uint16_t size)
    {
        //logf("Client TCP sending .... \n");
        return 0;
    }

    bool is_ok()
    {
        return 1;
    };

    void stop()
    {
        remOrch->logs->info("Server TCP exiting \n");
    }
};


SimpleWiFiScanner parentScanner;
// SimpleWiFiScanner<ESP8266_TCPASYNC_CLIENT, ESP8266_TCPASYNC_SERVER> parentScanner;

ESP8266_HARDWARE hardware_;
RemRouter remRouter;
RemOrchestrator remOrch;
RemLogger logs;

void espSetHost()
{
    String idStr = String(ESP.getChipId(), HEX);
    uint8_t len = idStr.length();
    __esp_host[0 + 5] = idStr.charAt(len - 1 - 3);
    __esp_host[1 + 5] = idStr.charAt(len - 1 - 2);
    __esp_host[2 + 5] = idStr.charAt(len - 1 - 1);
    __esp_host[3 + 5] = idStr.charAt(len - 1 - 0);

    __esp_host[9] = '\0';
    logs.info("ESP HOST IS : %s \n", __esp_host);
}

void setup()
{
    remOrch.set_logger(&logs);
    remOrch.set_router(&remRouter);
    remOrch.set_scanner(&parentScanner);
    remOrch.set_hardware(&hardware_);

    Serial.begin(115200);
    Serial.println();
    espSetHost();

    randomSeed(analogRead(0));

    MDNS.begin(__esp_host);

    parentScanner.begin();

    // mesh.set_hardware(&hardware_);
    // mesh.add_channel(&espTcpClient);
    // mesh.add_channel(&espTcpServer);
}

bool sta_started = false;
bool ap_started = false;

void loop()
{
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        remOrch.update();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
