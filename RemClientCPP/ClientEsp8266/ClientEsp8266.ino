#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>
#include "mesh-packet.pb.h"

#include "RemRouter.hpp"
#include "RemChannel.hpp"

void setup();
void loop();


char __esp_host[10] = "rspm-";

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long interval = 1000;

class ESP8266_HARDWARE : public Hardware
{
  private:
  public:
    ESP8266_HARDWARE(){};

    uint32_t device_id()
    {
        return ESP.getChipId();
    }
    uint32_t time_milis()
    {
        return millis();
    };
};

class ESP8266_TCPASYNC_CLIENT : public RemChannel
{
  private:
    // AsyncClient parClient;

  public:
    ESP8266_TCPASYNC_CLIENT(){};

    int ch_info() { return 500; }
    void init(int socket)
    {
        logf("Local TCP started in socket:%d \n", socket);
    }
    void init(const char *address, int port)
    {
        logf("Local TCP started %s:%d \n", address, port);
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
        logf("Local TCP exiting \n");
    }
};

ESP8266_TCPASYNC_CLIENT espTcpClient;
// ESP8266_TCPASYNC_SERVER espTcpServer;

ESP8266_HARDWARE hardware_;
RemRouter mesh;


void espSetHost();

void espSetHost()
{
    String idStr = String(ESP.getChipId(), HEX);
    uint8_t len = idStr.length();
    __esp_host[0 + 5] = idStr.charAt(len - 1 - 3);
    __esp_host[1 + 5] = idStr.charAt(len - 1 - 2);
    __esp_host[2 + 5] = idStr.charAt(len - 1 - 1);
    __esp_host[3 + 5] = idStr.charAt(len - 1 - 0);

    __esp_host[9] = '\0';
    Serial.printf("ESP HOST IS : %s \n", __esp_host);
}


void setup()
{
    Serial.begin(115200);
    Serial.println();
    espSetHost();

    WiFi.mode(WIFI_OFF);
    WiFi.disconnect(true);
    delay(1);
    // WiFi.reconnect();
    randomSeed(analogRead(0));


    mesh.set_hardware(&hardware_);
    mesh.add_channel(&espTcpClient);
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
        mesh.update();
    }
}
