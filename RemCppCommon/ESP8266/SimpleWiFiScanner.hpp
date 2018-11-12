#ifndef SIMPLEWIFISCANNER_HPP_
#define SIMPLEWIFISCANNER_HPP_

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include "RemOrchestrator.hpp"
#include "RemHeaderTypes.h"

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

typedef struct _ApConnData
{
    uint8_t netIndex;
    int16_t rssi;
} ApConnData;

class RemOrchestrator;

class SimpleWiFiScanner : public RemConnectionScanner
{

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

  public:
    static void wifi_scan_done_wrapper(int networksCount)
    {
        singleton->wifi_scan_done(networksCount);
    }

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
};

#endif /* !SIMPLEWIFISCANNER_HPP_ */
