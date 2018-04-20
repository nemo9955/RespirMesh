// // #include <stdint.h>
// // #include <inttypes.h>

// #include <TaskScheduler.h>
// #include <ESPAsyncTCP.h>

// #include <WiFiClient.h>
// #include <ESP8266WebServer.h>
// #include <ESP8266mDNS.h>
// #include <ESP8266HTTPUpdateServer.h>


// #include <pb_encode.h>
// #include <pb_decode.h>
// #include <pb.h>
// #include "mesh-packet.pb.h"

#include "RespirMesh.h"





RespirMesh mesh(_SERVER_PORT);

void setup()
{
    Serial.begin(115200);
    mesh.begin();
}

void loop()
{
    mesh.update();
}
