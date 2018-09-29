

#include <WiFiClient.h>

#include "RespirMesh.hpp"
#include "RemChannel.hpp"

void setup();
void loop();

unsigned long previousMillis = 0;
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
AsyncClient parClient;
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
        return _______;
    }

    void recvParentObj(){
        // logf("*****");
    };

    void stop()
    {
        logf("Local TCP exiting \n");
    }
};

class ESP8266_TCPASYNC_SERVER : public RemChannel
{
  private:
  public:
    ESP8266_TCPASYNC_SERVER(){};

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
        return _______;
    }

    void recvParentObj(){
        // logf("*****");
    };

    void stop()
    {
        logf("Local TCP exiting \n");
    }
};

ESP8266_TCPASYNC_CLIENT espTcpClient;
ESP8266_TCPASYNC_SERVER espTcpServer;

ESP8266_HARDWARE hardware;
RespirMesh mesh(&hardware);

void setup()
{

    mesh.add_channel(&serverTcp);

    mesh.add_channel(&espTcpClient);
    mesh.add_channel(&espTcpServer);
}

bool sta_started=false;
bool ap_started=false;

void loop()
{

    if(sta_started==false && millis()>1000){
        sta_started=true;
        start_sta()
    }
    if(ap_started==false && millis()>15000){
        ap_started=true;
        start_ap()
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        mesh.update();
    }
}




