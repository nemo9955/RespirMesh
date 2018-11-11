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

#include <ESP8266mDNS.h>

void setup();
void espSetHost();
void loop();
String macToString(const unsigned char *mac);

const char __rem_pass[] = "generic-pass";
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

bool wifi_scanning_now = false;
String ssidToPass(String qssid);

#define MAX_INDEXED_SCANNED_AP 15
#define MINIMUM_VALID_AP_RSSI (-70)
#define MEDIUM_VALID_AP_RSSI (-50)

uint16_t succesfullPongFromServer = 1;
uint16_t chipWIFIChannel = 0;

typedef struct _ApConnData
{
    uint8_t netIndex;
    int16_t rssi;
} ApConnData;

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

template <class CLIENT, class SERVER>
class SimpleListScanner : public RemConnectionScanner
{
  private:
    static bool is_client_connected;
    static bool is_server_started;
    static ApConnData sortedValidAPs[MAX_INDEXED_SCANNED_AP];
    static uint8_t validAPsCount;
    static RemOrchestrator *remOrch;
    static bool wifi_ap_connecting;
    static bool needToCreateAccessPoint;
    static bool accessPointCrated;

    WiFiEventHandler stationGotIPHandler;
    WiFiEventHandler stationConnectedHandler;
    WiFiEventHandler stationDisconnectedHandler;

  public:
    SimpleListScanner()
    {
        is_client_connected = false;
        is_server_started = false;
        validAPsCount = 0;

        stationGotIPHandler = WiFi.onStationModeGotIP(&SimpleListScanner::onStationGotIP);
        stationConnectedHandler = WiFi.onStationModeConnected(&SimpleListScanner::onStationConnected);
        stationDisconnectedHandler = WiFi.onStationModeDisconnected(&SimpleListScanner::onStationDisconnected);
    };

    void set_orchestrator(RemOrchestrator *remOrch_)
    {
        logf("  RemConnectionScanner : set_orchestrator \n");
        SimpleListScanner::remOrch = remOrch_;
        this->remOrch = remOrch_;
    };

    void begin()
    {
        wifi_scan_beggin();
    };

    // void add_client_host(char *_host, char *_port)
    // {

    //     remOrch->logs->info("adding client  %s : %s ", _host, _port);
    //     clients_list.push_back(std::make_pair(_host, _port));
    // };

    // void add_server_host(char *_host, char *_port)
    // {
    //     servers_list.push_back(std::make_pair(std::string(_host), std::string(_port)));
    // };

    void update()
    {

        // remOrch->logs->info("  RemConnectionScanner update ");
        if (is_client_connected == false)
        {
            if (wifi_scanning_now == false)
            {
                wifi_scan_beggin();
            }

            if (wifi_ap_connecting == false && validAPsCount > 0)
            {
                wifi_connect_beggin();
            }
        }

        if (is_server_started == false)
        {
            // start_servers();
        }
    };

    // void start_servers()
    // {
    //     remOrch->logs->info(" SimpleListScanner::start_servers servers_list  size  %u  \n", servers_list.size());
    //     for (auto it = servers_list.begin(); it != servers_list.end(); ++it)
    //     {
    //         remOrch->logs->info("\n\n\nTrying to start server %s:%s   \n", it->first.c_str(), it->second.c_str());
    //         SERVER *server_ = new SERVER();
    //         server_->init(
    //             const_cast<char *>(it->first.c_str()),
    //             const_cast<char *>(it->second.c_str()),
    //             remOrch);

    //         if (server_->is_connected == true)
    //         {
    //             remOrch->add_channel(move(server_));
    //             is_server_started = true;
    //         }
    //     }
    // };

    static void wifi_connect_beggin()
    {
        remOrch->logs->info(" SimpleListScanner::wifi_connect_beggin   ");

        if (sortedValidAPs[0].netIndex != 255)
        {
            delay(1);
            uint8_t index = sortedValidAPs[0].netIndex;

            chipWIFIChannel = WiFi.channel(index);
            logf(">W   Connecting to %d %s  : %s \n", index, WiFi.SSID(index).c_str(), ssidToPass(WiFi.SSID(index)).c_str());
            logf("     Connecting to channel %d  \n", WiFi.channel(index));

            // WiFi.begin(WiFi.SSID(index).c_str(), ssidToPass(WiFi.SSID(index)).c_str());
            WiFi.begin(WiFi.SSID(index).c_str(), ssidToPass(WiFi.SSID(index)).c_str(), chipWIFIChannel);
            wifi_ap_connecting = true;
        }
    };

    static void onStationConnected(const WiFiEventStationModeConnected &evt)
    {
        Serial.print("WiFi connected to ");
        Serial.println(evt.ssid.c_str());
        Serial.print("Station mac: ");
        Serial.println(macToString(evt.bssid));
    };

    static void onStationGotIP(const WiFiEventStationModeGotIP &evt)
    {
        Serial.print("WiFi got IP from ");
        Serial.println(WiFi.SSID().c_str());

        Serial.print("IP address: ");
        Serial.println(evt.ip);

        Serial.print("ROOT address: ");
        Serial.println(rootIP);

        is_client_connected = true;

        // //  if (evt.gw[2] == 1 && evt.gw[3] == 254)
        // // if (evt.gw[2] == 1 && evt.gw[3] == 13)
        // if (evt.gw[0] == _GW_ROOT_0 && evt.gw[1] == _GW_ROOT_1 && evt.gw[2] == _GW_ROOT_2 && evt.gw[3] == _GW_ROOT_3)
        // {
        //     Serial.println(" +++++++++++++   ");
        //     gateway = rootIP;
        //     parentServerIP = rootIP;
        //     IS_LEVEL_R1 = true;
        // }
        // else
        // {
        //     Serial.println(" --------------------   ");
        //     gateway = (uint32_t)evt.gw;
        //     parentServerIP = (uint32_t)evt.gw;
        // }

        // APlocal_IP[3] = random(250);
        // APlocal_IP[2] = random(250);
        // APgateway = (uint32_t)APlocal_IP;
        // APgateway[3] = APgateway[3] + 1;

        // Serial.print("Gateway: ");
        // gateway.printTo(Serial);
        // Serial.println("");
        // // Serial.println(gateway);

        wifi_ap_connecting = false;
        // wifiCheckTask.setInterval(WIFI_CHECK_INTERVAL_SLOW * TASK_SECOND);

        // remPingPongTask.setInterval(PING_SEND_INTERVAL_FAST * TASK_SECOND);
        // remPingPongTask.enable();
        // Serial.println("Enabled remPingPongTask");
    };

    static void onStationDisconnected(const WiFiEventStationModeDisconnected &evt)
    {
        Serial.print("Station disconnected: ");
        Serial.println(evt.reason);
        // Serial.println(macToString(evt.bssid));

        wifi_ap_connecting = false;
        is_client_connected = false;

        // wifiCheckTask.setInterval(WIFI_CHECK_INTERVAL_FAST * TASK_SECOND);
        // wifiCheckTask.forceNextIteration();

        // // remPingPongTask.disable();
        // remPacketSendTestTask.disable();
        // remTimeSyncTask.disable();
    };

    static void wifi_scan_beggin()
    {
        remOrch->logs->info("wifi_scan_beggin");

        if (((WiFi.getMode() & WIFI_STA) != 0))
            WiFi.enableSTA(true);
        WiFi.scanNetworksAsync(&SimpleListScanner::wifi_scan_done, true);
        wifi_scanning_now = true;
    };

    static void resetIndexedValidAps()
    {
        remOrch->logs->info("resetIndexedValidAps");
        validAPsCount = 0;
        for (uint8_t i = 0; i < MAX_INDEXED_SCANNED_AP; i++)
        {
            sortedValidAPs[i].netIndex = 255;
            sortedValidAPs[i].rssi = 255;
        }
    };

    static bool apValidSSID(String qssid)
    {
        remOrch->logs->info("apValidSSID");

        if (qssid.startsWith(_S_WIFI_SSID))
            return true;
        return qssid.startsWith("rspm");
    }

    static void wifi_scan_done(int networksCount)
    {
        remOrch->logs->info("wifi_scan_done");
        resetIndexedValidAps();
        wifi_scanning_now = true;

        Serial.printf("\n%d network(s) found\n", networksCount);
        for (uint8_t i = 0; i < networksCount; i++)
        {
            if (validAPsCount < MAX_INDEXED_SCANNED_AP && WiFi.RSSI(i) > MINIMUM_VALID_AP_RSSI && apValidSSID(WiFi.SSID(i)))
            {
                sortedValidAPs[validAPsCount].netIndex = i;
                sortedValidAPs[validAPsCount].rssi = WiFi.RSSI(i);
                // Serial.printf("Valid network : %d  %s \n", sortedValidAPs[validAPsCount].netIndex, WiFi.SSID(sortedValidAPs[validAPsCount].netIndex).c_str());
                validAPsCount++;
            }
            // Serial.printf("_%d: %s, Ch:%d (%ddBm) EncrT:%d MAC: %s    \n", i, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i), WiFi.BSSIDstr(i).c_str());
        }

        if (validAPsCount > 0)
        {
            ApConnData tmp = {0, 0};
            for (uint8_t i = 0; i < validAPsCount - 1; i++)
                for (uint8_t j = i + 1; j < validAPsCount; j++)
                    if (sortedValidAPs[j].rssi > sortedValidAPs[i].rssi)
                    {
                        tmp.netIndex = sortedValidAPs[j].netIndex;
                        tmp.rssi = sortedValidAPs[j].rssi;
                        sortedValidAPs[j].netIndex = sortedValidAPs[i].netIndex;
                        sortedValidAPs[j].rssi = sortedValidAPs[i].rssi;
                        sortedValidAPs[i].netIndex = tmp.netIndex;
                        sortedValidAPs[i].rssi = tmp.rssi;
                    }

            Serial.printf("SORTED valid APs %d :\n", validAPsCount);
            for (uint8_t i = 0; i < validAPsCount; i++)
            {

                Serial.printf("     %d: %s, %ddBm\n", sortedValidAPs[i].netIndex, WiFi.SSID(sortedValidAPs[i].netIndex).c_str(), WiFi.RSSI(sortedValidAPs[i].netIndex));
            }

            // if (validAPsCount > 0 && WiFi.status() != WL_CONNECTED)
            // {
            //     wifi_ap_connecting = true;
            // }

            Serial.println();
        }
    };
};

template <class CLIENT, class SERVER>
bool SimpleListScanner<CLIENT, SERVER>::is_client_connected;
template <class CLIENT, class SERVER>
bool SimpleListScanner<CLIENT, SERVER>::is_server_started;
template <class CLIENT, class SERVER>
ApConnData SimpleListScanner<CLIENT, SERVER>::sortedValidAPs[MAX_INDEXED_SCANNED_AP];
template <class CLIENT, class SERVER>
uint8_t SimpleListScanner<CLIENT, SERVER>::validAPsCount;
template <class CLIENT, class SERVER>
RemOrchestrator *SimpleListScanner<CLIENT, SERVER>::remOrch;
template <class CLIENT, class SERVER>
bool SimpleListScanner<CLIENT, SERVER>::wifi_ap_connecting;
// template <class CLIENT, class SERVER>
// bool SimpleListScanner<CLIENT, SERVER>::needToCreateAccessPoint;
// template <class CLIENT, class SERVER>
// bool SimpleListScanner<CLIENT, SERVER>::accessPointCrated;

SimpleListScanner<ESP8266_TCPASYNC_CLIENT, ESP8266_TCPASYNC_SERVER> parentScanner;

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

    WiFi.mode(WIFI_OFF);
    WiFi.disconnect(true);
    delay(1);
    // WiFi.reconnect();
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

String ssidToPass(String qssid)
{
    if (qssid.startsWith(_S_WIFI_SSID))
        return String(_S_WIFI_PASS);
    return String(__rem_pass);
}

String macToString(const unsigned char *mac)
{
    char buf[20];
    snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}
