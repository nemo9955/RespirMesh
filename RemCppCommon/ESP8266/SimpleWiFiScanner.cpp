#import "SimpleWiFiScanner.hpp"

// client_state
// enum WiFIClientState
// {
//     unknown,
//     wifi_off,
//     none_available,
//     scanning,
//     filtering_aps,
//     selecting_ap,
//     connecting_ap,
//     getting_ip_ap,
//     full_available,
// };

SimpleWiFiScanner::SimpleWiFiScanner()
{
    is_client_connected = false;
    is_server_started = false;
    validAPsCount = 0;
    singleton = this;
    client_state = unknown;

    stationGotIPHandler = WiFi.onStationModeGotIP(&SimpleWiFiScanner::onStationGotIP_wrapper);
    stationConnectedHandler = WiFi.onStationModeConnected(&SimpleWiFiScanner::onStationConnected_wrapper);
    stationDisconnectedHandler = WiFi.onStationModeDisconnected(&SimpleWiFiScanner::onStationDisconnected_wrapper);
};

SimpleWiFiScanner::~SimpleWiFiScanner(){};

SimpleWiFiScanner *SimpleWiFiScanner::singleton;

void SimpleWiFiScanner::begin()
{
    WiFi.mode(WIFI_OFF);
    client_state = wifi_off;

    WiFi.disconnect(true);
    client_state = none_available;

    delay(1);
    // WiFi.reconnect();
    wifi_scan_beggin();
};

void SimpleWiFiScanner::set_orchestrator(RemOrchestrator *remOrch_)
{
    logf("  SimpleWiFiScanner : set_orchestrator \n");
    remOrch = remOrch_;
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

void SimpleWiFiScanner::update()
{

    // remOrch->logs->info("  RemConnectionScanner update ");
    // if (client_state < scanning)
    // {
    //     // if (wifi_scanning_now == false)
    //     // {
    //     //     wifi_scan_beggin();
    //     // }

    // }
    if (client_state == selecting_ap)
    {
        wifi_connect_beggin();
    }

    if (is_server_started == false)
    {
        // start_servers();
    }
};

String SimpleWiFiScanner::ssidToPass(String qssid)
{
    if (qssid.startsWith(_S_WIFI_SSID))
        return String(_S_WIFI_PASS);
    return String("generic-pass");
}

void SimpleWiFiScanner::wifi_connect_beggin()
{
    remOrch->logs->info(" SimpleWiFiScanner::wifi_connect_beggin   ");

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
        client_state = connecting_ap;
    }
};

void SimpleWiFiScanner::onStationConnected(const WiFiEventStationModeConnected &evt)
{
        client_state = getting_ip_ap;
    Serial.print("WiFi connected to ");
    Serial.println(evt.ssid.c_str());
    Serial.print("Station mac: ");
    logf("%02x:%02x:%02x:%02x:%02x:%02x\n", evt.bssid[0], evt.bssid[1], evt.bssid[2], evt.bssid[3], evt.bssid[4], evt.bssid[5]);
};

void SimpleWiFiScanner::onStationGotIP(const WiFiEventStationModeGotIP &evt)
{
        client_state = full_available;
    Serial.print("WiFi got IP from ");
    Serial.println(WiFi.SSID().c_str());

    Serial.print("IP address: ");
    Serial.println(evt.ip);

    // Serial.print("ROOT address: ");
    // Serial.println(rootIP);

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

    // wifi_ap_connecting = false;
    // wifiCheckTask.setInterval(WIFI_CHECK_INTERVAL_SLOW * TASK_SECOND);

    // remPingPongTask.setInterval(PING_SEND_INTERVAL_FAST * TASK_SECOND);
    // remPingPongTask.enable();
    // Serial.println("Enabled remPingPongTask");
};

void SimpleWiFiScanner::onStationDisconnected(const WiFiEventStationModeDisconnected &evt)
{
    Serial.print("Station disconnected: ");
    Serial.println(evt.reason);
    // Serial.println(macToStr(evt.bssid));

    // wifi_ap_connecting = false;
    // is_client_connected = false;

    // wifiCheckTask.setInterval(WIFI_CHECK_INTERVAL_FAST * TASK_SECOND);
    // wifiCheckTask.forceNextIteration();

    // // remPingPongTask.disable();
    // remPacketSendTestTask.disable();
    // remTimeSyncTask.disable();
};

void SimpleWiFiScanner::wifi_scan_beggin()
{
    remOrch->logs->info("wifi_scan_beggin");

    if (((WiFi.getMode() & WIFI_STA) != 0))
        WiFi.enableSTA(true);
    WiFi.scanNetworksAsync(&SimpleWiFiScanner::wifi_scan_done_wrapper, true);
    // wifi_scanning_now = true;

    client_state = scanning;
};

void SimpleWiFiScanner::resetIndexedValidAps()
{
    remOrch->logs->info("resetIndexedValidAps");
    validAPsCount = 0;
    for (uint8_t i = 0; i < MAX_INDEXED_SCANNED_AP; i++)
    {
        sortedValidAPs[i].netIndex = 255;
        sortedValidAPs[i].rssi = 255;
    }
};

bool SimpleWiFiScanner::apValidSSID(String qssid)
{
    remOrch->logs->info("apValidSSID");

    if (qssid.startsWith(_S_WIFI_SSID))
        return true;
    return qssid.startsWith("rspm");
}

void SimpleWiFiScanner::wifi_scan_done(int networksCount)
{
    remOrch->logs->info("wifi_scan_done");
    resetIndexedValidAps();
    // wifi_scanning_now = true;
    client_state = filtering_aps;

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
            // wifi_ap_connecting = true;
            client_state = selecting_ap;
        }

        Serial.println();
    }
};
