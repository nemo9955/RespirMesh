#ifndef RESPIRWIFI_HPP_
#define RESPIRWIFI_HPP_

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

#define MAX_AP_CLIENTS 6
#define MICROtoSEC 1.0 / 1000000.0

#define MAX_INDEXED_SCANNED_AP 15
#define MINIMUM_VALID_AP_RSSI (-70)
#define MEDIUM_VALID_AP_RSSI (-50)

class RespirWifi
{
    private:
        const char __rem_pass[] = "generic-pass";
        const char __wifi_ssid[] = _S_WIFI_SSID;
        const char __wifi_pass[] = _S_WIFI_PASS;

        const char __ota_http_user[] = _S_OTA_HTTP_USER;
        const char __ota_http_pass[] = _S_OTA_HTTP_PASS;
        const char __ota_http_path[] = _S_OTA_HTTP_PATH;

        char __esp_host[10] = "rspm-";
        const size_t __server_port = _SERVER_PORT;
    
        
    public:
        void wifiCheckCallback();
        void wifiScanCallback(int);


        
};

#endif /*End respir wifi*/