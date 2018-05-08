// #include <inttypes.h>
#include "RemHeaderTypes.h"


#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>
#include "mesh-packet.pb.h"

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include <ESPAsyncTCP.h>
#include <TaskScheduler.h>

#define MAX_AP_CLIENTS 6
#define MICROtoSEC 1.0 / 1000000.0

#define MAX_INDEXED_SCANNED_AP 15
#define MINIMUM_VALID_AP_RSSI (-70)
#define MEDIUM_VALID_AP_RSSI (-50)
// _S_WIFI_SSID
// _S_WIFI_PASS
// _S_OTA_HTTP_USER
// _S_OTA_HTTP_PASS
// _S_OTA_HTTP_PATH
// _S_ESP_HOST
// _SERVER_PORT

const char __rem_pass[] = "generic-pass";
const char __wifi_ssid[] = _S_WIFI_SSID;
const char __wifi_pass[] = _S_WIFI_PASS;
const char __ota_http_user[] = _S_OTA_HTTP_USER;
const char __ota_http_pass[] = _S_OTA_HTTP_PASS;
const char __ota_http_path[] = _S_OTA_HTTP_PATH;
char __esp_host[10] = "rspm-";
const size_t __server_port = _SERVER_PORT;

bool IS_LEVEL_R1 = false;
IPAddress rootIP(_SERVER_IP);
IPAddress gateway;
IPAddress parentServerIP;

IPAddress APlocal_IP(192, 168, 4, 22);
IPAddress APgateway(192, 168, 4, 9);
IPAddress APsubnet(255, 255, 255, 0);

AsyncClient *chClients[MAX_AP_CLIENTS];
uint8_t conectedCClientsCnt = 0;
AsyncClient parClient;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
AsyncServer myTCPServer(__server_port);

uint8_t pb_buffer[64];

bool needToConnectToStation = false;
bool needToCreateAccessPoint = false;
bool accessPointCrated = false;
uint8_t chClientsCnt = 0;

WiFiEventHandler stationGotIPHandler;
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;

void ensureParentConnected();
// void handleMeshTopology(AsyncClient *c, uint8_t *data, size_t len, RemBasicHeader *header);
void handleMeshTopology(AsyncClient *c, uint8_t *data, size_t len);
void handlePingPong(AsyncClient *c, uint8_t *data, size_t len, RemBasicHeader *header);
void handleTimeSync(AsyncClient *c, uint8_t *data, size_t len, RemBasicHeader *header);
void remPacketSendTestCallback();
void remPingPongCallback();
void remTimeSyncCallback();
void wifiCheckCallback();
void wifiScanCallback(int);

void dataRecv(void *s, AsyncClient *c, void *data, size_t len);
String macToString(const unsigned char *mac);
void cliectDisconnected(void *s, AsyncClient *c);
void espSetHost();
String ssidToPass(String qssid);
void resetIndexedValidAps();

Scheduler runner;
#define WIFI_CHECK_INTERVAL_FAST 10
#define WIFI_CHECK_INTERVAL_SLOW 60
Task wifiCheckTask(WIFI_CHECK_INTERVAL_FAST *TASK_SECOND, TASK_FOREVER, &wifiCheckCallback, &runner, false);
Task remPingPongTask(1 * TASK_SECOND, TASK_FOREVER, &remPingPongCallback, &runner, false);
Task remPacketSendTestTask(3 * TASK_SECOND, TASK_FOREVER, &remPacketSendTestCallback, &runner, false);
Task remTimeSyncTask(30 * TASK_SECOND, TASK_FOREVER, &remTimeSyncCallback, &runner, false);
#define EVERY_N_PING_A_TOPO 2
#define PING_SEND_INTERVAL_SLOW 5
#define PING_SEND_INTERVAL_MED 2
#define PING_SEND_INTERVAL_FAST 1

uint16_t succesfullPongFromServer = 1;
uint16_t chipWIFIChannel = 0;

typedef struct _ApConnData
{
    uint8_t netIndex;
    int16_t rssi;
} ApConnData;

ApConnData sortedValidAPs[MAX_INDEXED_SCANNED_AP];
uint8_t validAPsCount = 0;

void onStationConnected(const WiFiEventStationModeConnected &evt)
{
    Serial.print("WiFi connected to ");
    Serial.println(evt.ssid.c_str());
    Serial.print("Station mac: ");
    Serial.println(macToString(evt.bssid));
}

void onStationGotIP(const WiFiEventStationModeGotIP &evt)
{
    Serial.print("WiFi got IP from ");
    Serial.println(WiFi.SSID().c_str());

    Serial.print("IP address: ");
    Serial.println(evt.ip);

    Serial.print("ROOT address: ");
    Serial.println(rootIP);

    //  if (evt.gw[2] == 1 && evt.gw[3] == 254)
    // if (evt.gw[2] == 1 && evt.gw[3] == 13)
    if (evt.gw[0] == _GW_ROOT_0 && evt.gw[1] == _GW_ROOT_1 && evt.gw[2] == _GW_ROOT_2 && evt.gw[3] == _GW_ROOT_3)
    {
        Serial.println(" +++++++++++++   ");
        gateway = rootIP;
        parentServerIP = rootIP;
        IS_LEVEL_R1 = true;
    }
    else
    {
        Serial.println(" --------------------   ");
        gateway = (uint32_t)evt.gw;
        parentServerIP = (uint32_t)evt.gw;
    }

    APlocal_IP[3] = random(250);
    APlocal_IP[2] = random(250);
    APgateway = (uint32_t)APlocal_IP;
    APgateway[3] = APgateway[3] + 1;

    Serial.print("Gateway: ");
    gateway.printTo(Serial);
    Serial.println("");
    // Serial.println(gateway);

    needToConnectToStation = false;
    wifiCheckTask.setInterval(WIFI_CHECK_INTERVAL_SLOW * TASK_SECOND);

    remPingPongTask.setInterval(PING_SEND_INTERVAL_FAST * TASK_SECOND);
    remPingPongTask.enable();
    Serial.println("Enabled remPingPongTask");
}

void onStationDisconnected(const WiFiEventStationModeDisconnected &evt)
{
    Serial.print("Station disconnected: ");
    Serial.println(evt.reason);
    // Serial.println(macToString(evt.bssid));

    bool needToConnectToStation = true;

    wifiCheckTask.setInterval(WIFI_CHECK_INTERVAL_FAST * TASK_SECOND);
    wifiCheckTask.forceNextIteration();

    // remPingPongTask.disable();
    remPacketSendTestTask.disable();
    remTimeSyncTask.disable();
}

void clientConnected(void *s, AsyncClient *c)
{
    if (c == NULL)
        return;

    if ((*c) == parClient)
    {
        Serial.println("Parent Client is Connected ");
        remPingPongTask.setInterval(PING_SEND_INTERVAL_FAST * TASK_SECOND);
        remPingPongTask.restart();
    }
    else if (chClientsCnt < MAX_AP_CLIENTS)
    {
        Serial.printf("Child #%d Client is Connected \n", chClientsCnt);
        c->write("HELO!!!");

        chClients[chClientsCnt] = c;

        chClients[chClientsCnt]->onData(dataRecv, NULL);
        chClients[chClientsCnt]->onDisconnect(cliectDisconnected, NULL);

        chClientsCnt++;
    }
}

void cliectDisconnected(void *s, AsyncClient *c)
{
    Serial.println("Client Disconnected from server");
    if ((*c) == parClient)
    {
        Serial.println("Parent Client is DISCONECTED to server");
        // WiFi.mode(WIFI_OFF);
        // parClient = NULL;
        bool needToConnectToStation = true;
        // WiFi.enableAP(false);
        WiFi.softAPdisconnect();
    }

    for (uint8_t i = 0; i < MAX_AP_CLIENTS; i++)
    {
        if (chClients[i] == c)
        {
            for (uint8_t s = i; s < chClientsCnt - 1; s++)
                chClients[s] = chClients[s + 1];
            chClientsCnt--;
            break;
        }
    }

    // c->close(true);
    // c->free();
}

void dataRecv(void *s, AsyncClient *c, void *data, size_t len)
{

    // for (uint8_t i = 0; i < len; i++)
    //     Serial.printf("%c", ((char *)data)[i]);
    // Serial.println();

    // for (uint8_t i = 0; i < len; i++)
    //     Serial.printf("%d ", ((char *)data)[i]);
    // Serial.println();

    RemBasicHeader *header = (RemBasicHeader *)data;
    uint16_t offsetHeader = sizeof(RemBasicHeader);
    // memcpy(&header, data, offsetHeader);

    Serial.printf("  ForwardingType %d \n", header->ForwardingType);
    Serial.printf("      HeaderType %d \n", header->HeaderType);
    Serial.printf("    ProtobufType %d \n", header->ProtobufType);

    uint8_t *packetData = ((uint8_t *)(data)) + offsetHeader;
    uint16_t packetDataLen = len - offsetHeader;

    if (header->ForwardingType == ForwardingType_TO_ROOT)
    {
        parClient.write(((const char *)data), len);
    }
    else if (header->ForwardingType == ForwardingType_PARENT_TO_ROOT)
    {
        switch (header->ProtobufType)
        {
        case ProtobufType_MESH_TOPOLOGY:
            Serial.printf("putting data in  TOPO to root : \n");
            handleMeshTopology(c, (uint8_t *)(data), len);
            break;
        }
    }
    else
    {

        switch (header->ProtobufType)
        {
        case ProtobufType_TIMESYNC:
            handleTimeSync(c, packetData, packetDataLen, header);
            break;
        case ProtobufType_PONG:
        case ProtobufType_PING:
            handlePingPong(c, packetData, packetDataLen, header);
            break;
            // default:
            //     break;
        }
    }
    // Serial.printf("mesh_packet.message_type   %d !\n", packetType);
    delay(1);
}

// void handleMeshTopology(AsyncClient *c, uint8_t *data, size_t len, RemBasicHeader *header)
void handleMeshTopology(AsyncClient *c, uint8_t *data, size_t len)
{

    for (uint8_t i = 0; i < len + 10; i++)
        Serial.printf("%d ", ((char *)data)[i]);
    Serial.println();
    for (uint8_t i = 0; i < len + 10; i++)
        Serial.printf("%d ", ((char *)pb_buffer)[i]);
    Serial.println();

    bool pb_status = false;
    uint16_t offsetHeader = sizeof(RemBasicHeader);
    uint8_t *packetData = (uint8_t *)(data) + offsetHeader;
    // uint8_t *packetData = ((uint8_t *)(data)) + offsetHeader;
    uint16_t packetDataLen = len - offsetHeader;
    memcpy(pb_buffer, data, offsetHeader);
    RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
    // header = (RemBasicHeader *)pb_buffer;

    RespirMeshInfo meshTopo;

    pb_istream_t istream = pb_istream_from_buffer(packetData, packetDataLen);
    pb_status = pb_decode(&istream, RespirMeshInfo_fields, &meshTopo);
    if (!pb_status)
    {
        printf("Decoding meshTopo failed %s with type %d\n", PB_GET_ERROR(&istream), header->ProtobufType);
        return;
    }

    for (uint8_t i = 0; i < len + 10; i++)
        Serial.printf("%d ", ((char *)pb_buffer)[i]);
    Serial.println();

    if (header->ForwardingType == ForwardingType_PARENT_TO_ROOT)
    {

        header->ForwardingType = ForwardingType_TO_ROOT;
        meshTopo.type = ProtobufType_MESH_TOPOLOGY;
        meshTopo.target_id = ESP.getChipId();

        pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
        pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &meshTopo);

        if (!pb_status)
        {
            Serial.printf("Encoding meshTopo failed: %s\n", PB_GET_ERROR(&ostream));
            return;
        }

        // Serial.printf(" target_id %x \n", pingpong.target_id);
        Serial.printf("Sending completed TOPO to root %d %d \n", ostream.bytes_written, offsetHeader);
        Serial.printf("         src %x  tar %x \n", meshTopo.source_id, meshTopo.target_id);
        Serial.printf("         Forward %d \n", header->ForwardingType);
        Serial.printf("             Hea %d \n", header->HeaderType);
        Serial.printf("           Proto %d \n", header->ProtobufType);

        for (uint8_t i = 0; i < ostream.bytes_written + offsetHeader + 3; i++)
            Serial.printf("%d ", ((char *)pb_buffer)[i]);
        Serial.println();

        parClient.write((const char *)(pb_buffer), ostream.bytes_written + offsetHeader);
        delay(1);
    }
}

void remTimeSyncCallback()
{
    ensureParentConnected();

    RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
    uint16_t offsetHeader = sizeof(RemBasicHeader);
    header->ForwardingType = ForwardingType_TO_ROOT;
    header->HeaderType = HeaderType_BASIC;
    header->ProtobufType = ProtobufType_TIMESYNC;
    // memcpy(pb_buffer, &header, offsetHeader);

    RespirMeshTimeSync remTimeSync = RespirMeshTimeSync_init_default;
    remTimeSync.info.type = ProtobufType_TIMESYNC;
    remTimeSync.info.source_id = ESP.getChipId();
    remTimeSync.request_sent_time = micros();
    pb_ostream_t stream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
    bool pb_status = pb_encode(&stream, RespirMeshTimeSync_fields, &remTimeSync);

    if (!pb_status)
    {
        Serial.printf("Encoding TimeSync failed: %s\n", PB_GET_ERROR(&stream));
        return;
    }

    parClient.write((const char *)(pb_buffer), stream.bytes_written + offsetHeader);
    Serial.printf(" > Sending TimeSync size of %d at %d \n", stream.bytes_written, micros());
    delay(1);
}

void handleTimeSync(AsyncClient *c, uint8_t *data, size_t len, RemBasicHeader *header)
{
    Serial.printf("\n TimeSync size is %d \n", len);
    RespirMeshTimeSync timesync;
    // uint32_t us_start = micros();
    pb_istream_t stream = pb_istream_from_buffer(data, len);
    bool pb_status = pb_decode(&stream, RespirMeshTimeSync_fields, &timesync);

    if (!pb_status)
    {
        printf("Decoding TimeSync failed %s with type %d\n", PB_GET_ERROR(&stream), header->ProtobufType);
        return;
    }

    uint32_t request_duration;
    uint32_t response_duration;
    uint32_t round_trip_duration;

    timesync.request_arrive_time = micros();
    request_duration = (timesync.request_arrive_time - timesync.request_sent_time);
    response_duration = (timesync.response_sent_time - timesync.response_arrive_time);
    round_trip_duration = (request_duration - response_duration);
    // Serial.printf("          Request_sent_time %d \n", timesync.request_sent_time);
    // Serial.printf("       Response_arrive_time %d \n", timesync.response_arrive_time);
    // Serial.printf("         Response_sent_time %d \n", timesync.response_sent_time);
    // Serial.printf("        Request_arrive_time %d \n", timesync.request_arrive_time);
    Serial.printf("      server_timestamp_unix %d sec \n", timesync.server_timestamp_unix);
    Serial.printf("--->      request_duration: %f sec \n", request_duration * MICROtoSEC);
    // Serial.printf("<---     response_duration: %d \n", response_duration);
    // Serial.printf("<-->               average: %d sec \n", ((response_duration + request_duration) / 2) * MICROtoSEC);
    // Serial.printf(">-->   round_trip_duration: %f sec \n", round_trip_duration * MICROtoSEC);
    // Serial.printf(">-->    round_trip average: %d \n", round_trip_duration / 2);
    // Serial.printf("====   PB decoding duration %d us \n", micros() - us_start);
    // Serial.printf("\nRecv size        %d \n", len);
}

void remPingPongCallback()
{
    ensureParentConnected();

    RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
    uint16_t offsetHeader = sizeof(RemBasicHeader);
    header->ForwardingType = ForwardingType_TO_ROOT;
    header->HeaderType = HeaderType_BASIC;

    RespirMeshInfo remPingPong = RespirMeshInfo_init_default;
    remPingPong.source_id = ESP.getChipId();

    if (remPingPongTask.getRunCounter() % EVERY_N_PING_A_TOPO == 1)
    {
        header->ForwardingType = ForwardingType_PARENT_TO_ROOT;
        header->ProtobufType = ProtobufType_MESH_TOPOLOGY;
        remPingPong.type = ProtobufType_MESH_TOPOLOGY;
    }
    else
    {
        header->ProtobufType = ProtobufType_PING;
        remPingPong.type = ProtobufType_PING;
    }

    pb_ostream_t stream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
    bool pb_status = pb_encode(&stream, RespirMeshInfo_fields, &remPingPong);

    if (!pb_status)
    {
        Serial.printf("Encoding PingPong failed: %s\n", PB_GET_ERROR(&stream));
        return;
    }
    parClient.write((const char *)(pb_buffer), stream.bytes_written + offsetHeader);

    delay(1);

    if (remPingPongTask.getRunCounter() % EVERY_N_PING_A_TOPO == 1)
        Serial.printf(">>> Sending TOPOLOGY #%d size of %d at %u \n", remPingPongTask.getRunCounter(), stream.bytes_written, micros());
    else
        Serial.printf(">>> Sending PING #%d size of %d at %u \n", remPingPongTask.getRunCounter(), stream.bytes_written, micros());

    if (remPingPongTask.getRunCounter() == 10)
    {
        remPingPongTask.setInterval(PING_SEND_INTERVAL_MED * TASK_SECOND);
        // if (IS_LEVEL_R1)
        //     remPingPongTask.setInterval(PING_SEND_INTERVAL_SLOW * TASK_SECOND);
    }

    if (accessPointCrated == false && remPingPongTask.getRunCounter() > 10)
    {
        // && ((float)succesfullPongFromServer / remPingPongTask.getRunCounter()) > 0.70
        needToCreateAccessPoint = true;
    }

    delay(1);
}

void handlePingPong(AsyncClient *c, uint8_t *data, size_t len, RemBasicHeader *header)
{
    // Serial.printf("\n PingPong size is %d \n", len);
    RespirMeshInfo pingpong;
    pb_istream_t stream = pb_istream_from_buffer(data, len);
    bool pb_status = pb_decode(&stream, RespirMeshInfo_fields, &pingpong);

    if (!pb_status)
    {
        printf("Decoding PingPong failed %s with type %d\n", PB_GET_ERROR(&stream), header->ProtobufType);
        return;
    }

    if (pingpong.type == ProtobufType_PONG)
    {
        succesfullPongFromServer++;

        Serial.printf("<<< Got PONG #%d size %d at %d \n", succesfullPongFromServer, len, micros());
        // Serial.printf(" target_id %x \n", pingpong.target_id);
        // Serial.printf("      source_id %x \n", pingpong.source_id);
        return;
    }
}

void ensureParentConnected()
{
    if (!parClient.connected() && !parClient.connecting())
    {
        parClient.connect(parentServerIP, __server_port);

        Serial.print("parentServerIP: ");
        parentServerIP.printTo(Serial);
        Serial.printf("\n __server_port %d \n", __server_port);

        delay(1);
    }
}

void setup(void)
{
    Serial.begin(115200);
    Serial.println();
    espSetHost();

    WiFi.mode(WIFI_OFF);
    WiFi.disconnect(true);
    delay(1);
    // WiFi.reconnect();
    randomSeed(analogRead(0));

    stationGotIPHandler = WiFi.onStationModeGotIP(&onStationGotIP);
    stationConnectedHandler = WiFi.onStationModeConnected(&onStationConnected);
    stationDisconnectedHandler = WiFi.onStationModeDisconnected(&onStationDisconnected);
    MDNS.begin(__esp_host);

    httpUpdater.setup(&server);
    // httpUpdater.setup(&server, __ota_http_path);
    // httpUpdater.setup(&server, __ota_http_path, __ota_http_user, __ota_http_pass);
    server.begin();

    MDNS.addService("http", "tcp", 80);
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", __esp_host, __ota_http_path, __ota_http_user, __ota_http_pass);
    Serial.println("  YOOOOOOOOOOOOOO ");

    myTCPServer.setNoDelay(true);
    myTCPServer.onClient(clientConnected, NULL);
    myTCPServer.begin();

    parClient.onConnect(clientConnected, NULL);
    parClient.onDisconnect(cliectDisconnected, NULL);
    parClient.onData(dataRecv, NULL);

    // remPacketSendTestTask.enableDelayed();
    // Serial.println("Enabled remPacketSendTestTask");

    // Serial.println("Enabled remPingPongTask");

    // remTimeSyncTask.enableDelayed();
    // Serial.println("Enabled remTimeSyncTask");

    wifiCheckTask.enable();
    Serial.println("Enabled wifiCheckTask");
}

void loop(void)
{
    server.handleClient();
    runner.execute();

    if (needToConnectToStation && validAPsCount > 0)
    {
        if (sortedValidAPs[0].netIndex != 255)
        {
            delay(1);
            uint8_t index = sortedValidAPs[0].netIndex;

            chipWIFIChannel = WiFi.channel(index);
            Serial.printf(">W   Connecting to %d %s  : %s \n", index, WiFi.SSID(index).c_str(), ssidToPass(WiFi.SSID(index)).c_str());
            Serial.printf("     Connecting to channel %d  \n", WiFi.channel(index));

            // WiFi.begin(WiFi.SSID(index).c_str(), ssidToPass(WiFi.SSID(index)).c_str());
            WiFi.begin(WiFi.SSID(index).c_str(), ssidToPass(WiFi.SSID(index)).c_str(), chipWIFIChannel);
            needToConnectToStation = false;
        }
        // for (uint8_t i = 0; needToConnectToStation && i < validAPsCount; i++)
        // {
        //     uint8_t index = sortedValidAPs[i].netIndex;
        //     if (index == 255)
        //         break;
        //     // Serial.printf("%d: %s, %ddBm\n", index, WiFi.SSID(index).c_str(), WiFi.RSSI(index));
        //     Serial.printf(" > Connecting to %d %s  : %s \n", index, WiFi.SSID(index).c_str(), ssidToPass(WiFi.SSID(index)).c_str());
        //     WiFi.begin(WiFi.SSID(index).c_str(), ssidToPass(WiFi.SSID(index)).c_str());
        // }
    }

    if (needToCreateAccessPoint)
    {
        needToCreateAccessPoint = false;

        // APlocal_IP = (uint32_t)APgateway;
        // APlocal_IP[3] = APlocal_IP[3] + 1;

        Serial.print("APlocal_IP: ");
        APlocal_IP.printTo(Serial);
        Serial.println("");
        Serial.print("APgateway: ");
        APgateway.printTo(Serial);
        Serial.println("");

        WiFi.softAPConfig(APlocal_IP, APgateway, APsubnet);
        Serial.println("Starting as AP + station");
        WiFi.mode(WIFI_AP_STA);
        __esp_host[4] = '_';
        WiFi.softAP(__esp_host, __rem_pass, chipWIFIChannel);
        __esp_host[4] = '-';

        Serial.print("Soft-AP IP address = ");
        Serial.println(WiFi.softAPIP());
        accessPointCrated = true;
    }
}

String ssidToPass(String qssid)
{
    if (qssid.startsWith(_S_WIFI_SSID))
        return String(_S_WIFI_PASS);
    return String(__rem_pass);
}

void wifiCheckCallback()
{

    // if (WiFi.RSSI() > MEDIUM_VALID_AP_RSSI)
    // {
    //     wifiCheckTask.setInterval(60 * TASK_SECOND);
    // }
    // else
    // {
    //     wifiCheckTask.setInterval(20 * TASK_SECOND);
    // }

    if (WiFi.status() != WL_CONNECTED || (needToConnectToStation == true && WiFi.status() == WL_CONNECTED))
    {
        Serial.print("Checking WiFi Status, needToConnectToStation ");
        Serial.println(needToConnectToStation);
        if (((WiFi.getMode() & WIFI_STA) != 0))
            WiFi.enableSTA(true);
        WiFi.scanNetworksAsync(wifiScanCallback, true);
    }
}

bool apValidSSID(String qssid)
{
    if (qssid.startsWith(_S_WIFI_SSID))
        return true;
    return qssid.startsWith("rspm");
}

void resetIndexedValidAps()
{
    validAPsCount = 0;
    for (uint8_t i = 0; i < MAX_INDEXED_SCANNED_AP; i++)
    {
        sortedValidAPs[i].netIndex = 255;
        sortedValidAPs[i].rssi = 255;
    }
}

void wifiScanCallback(int networksCount)
{
    resetIndexedValidAps();

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

        if (validAPsCount > 0 && WiFi.status() != WL_CONNECTED)
        {
            needToConnectToStation = true;
        }

        Serial.println();
    }
}

void remPacketSendTestCallback()
{
}

String macToString(const unsigned char *mac)
{
    char buf[20];
    snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

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
