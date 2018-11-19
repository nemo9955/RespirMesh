#ifndef SIMPLEWIFISCANNER_HPP_
#define SIMPLEWIFISCANNER_HPP_

#include <ESP8266WiFi.h>
// #include <WiFiClient.h>
// #include <IPAddress.h>
#include <Arduino.h>

#include "RemOrchestrator.hpp"
#include "RemHeaderTypes.h"
#include "TaskLooper.hpp"

#import "ESP8266/EspUtils.hpp"

#define MAX_INDEXED_SCANNED_AP 15
#define MINIMUM_VALID_AP_RSSI (-70)
#define MEDIUM_VALID_AP_RSSI (-50)

enum WiFIClientState
{
    unknown,
    wifi_off,
    none_available,
    scanning,
    filtering_aps,
    selecting_ap,
    connecting_ap,
    getting_ip_ap,
    full_available,
};

enum WiFIServerState
{
    unknown_ap,
    wifi_off_ap,
    server_off_ap,
    starting_server_ap,
    no_clients_ap,
    some_clients_ap,
    max_clients_ap,
};

typedef struct _ApConnData
{
    uint8_t netIndex;
    int16_t rssi;
} ApConnData;

class RemOrchestrator;
class TaskLooper;

typedef void (*client_func_def)(RemClientInfo *client_info);
typedef void (*server_func_def)();

class SimpleWiFiScanner : public RemConnectionScanner
{

  private:
    // uint32_t rescan_timestamp_milis = 0;
    // uint32_t ap_server_start_ts = 0;
    // uint32_t rescan_interval = 10;

    TaskLooper rescan_looper;
    TaskLooper servap_looper;

    client_func_def create_client_func_ptr;
    server_func_def create_server_func_ptr;

    IPAddress APlocal_IP;
    IPAddress APgateway;
    IPAddress APsubnet;
    char __esp_host[10];

  private:
    RemOrchestrator *remOrch;
    bool is_client_connected;
    bool is_server_started;
    ApConnData sortedValidAPs[MAX_INDEXED_SCANNED_AP];
    uint8_t validAPsCount;
    // RemOrchestrator *remOrch;
    bool wifi_ap_connecting;
    bool needToCreateAccessPoint;
    bool accessPointCrated;
    bool wifi_scanning_now = false;
    uint16_t chipWIFIChannel = 0;

    WiFiEventHandler stationGotIPHandler;
    WiFiEventHandler stationConnectedHandler;
    WiFiEventHandler stationDisconnectedHandler;

  private:
    static SimpleWiFiScanner *singleton;
    WiFIClientState client_state;
    WiFIServerState server_state;

  public:
    static void wifi_scan_done_wrapper(int networksCount)
    {
        singleton->wifi_scan_done(networksCount);
    };

    static void onStationGotIP_wrapper(const WiFiEventStationModeGotIP &evt)
    {
        singleton->onStationGotIP(evt);
    };

    static void onStationConnected_wrapper(const WiFiEventStationModeConnected &evt)
    {
        singleton->onStationConnected(evt);
    };

    static void onStationDisconnected_wrapper(const WiFiEventStationModeDisconnected &evt)
    {
        singleton->onStationDisconnected(evt);
    };

  public:
    ~SimpleWiFiScanner();
    SimpleWiFiScanner();

    String ssidToPass(String qssid);
    void begin();
    void set_orchestrator(RemOrchestrator *remOrch_);
    void update();
    void wifi_connect_beggin();
    void onStationConnected(const WiFiEventStationModeConnected &evt);
    void onStationGotIP(const WiFiEventStationModeGotIP &evt);
    void onStationDisconnected(const WiFiEventStationModeDisconnected &evt);
    void wifi_scan_beggin();
    void resetIndexedValidAps();
    bool apValidSSID(String qssid);
    void wifi_scan_done(int networksCount);
    void ap_server_start();
    void espSetHost();
    void client_status();
    void set_client_func(client_func_def);
    void set_server_func(server_func_def);
};

#endif /* !SIMPLEWIFISCANNER_HPP_ */
