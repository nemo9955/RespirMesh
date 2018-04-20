
#include "Arduino.h"

#include "RespirMesh.h"

#include "RemTCPChannel.h"



// TCP and UDP channels require a WIFI manager to be loaded in the mesh
RemTCPChannel tcpChanel;
RemI2CChannel i2cChannel(BOARD_I2C_UUID);

#ifdef ESP8266
    RemWiFiManager_ESP8266 remWiFi;
#endif

#ifdef ESP32
    RemWiFiManager_ESP32 remWiFi;
#endif

// mesh takes as parameters all the channels it will have to manage
RespirMesh mesh(&tcpChanel,&i2cChannel);

uint8_t buf[100];


setup(){
    remWiFi.enableAP(true);
    remWiFi.enableSTA(false);
    remWiFi.maxAPClients(2);

    mesh.addManager(remWiFi);

    // user should be able to listen for all packets
    // and for specific ones
    mesh.allCallback(&meshPackets);
    mesh.forHeaderTypeCb(&someOtherFun, LITTLE_BIG_ENDIAN_SYNC_TYPE);
    mesh.forPbufTypeCb(&someDataFun, PROTOBUF_USER_DEFINED);
    // maybe even like this ?
    mesh.forPbufTypeCb(&someDataFun, PROTOBUF_PING);

    // add callback directly to a channel maybe ?
    i2cChannel.allCallback(&specialWirePacket);

    mesh.begin();

}


loop(){
    mesh.update()

}


void SomePeriodicTask(){
    i2cChannel.disable();
    readDataFomSenzor();
    i2cChannel.enable();
    i2cChannel.broadcast(senzDataPacket);

}

void someDataFun(uint8_t* header,uint8_t* data, uint16_t len ){
}

void meshPackets(uint8_t* header,uint8_t* data, uint16_t len ){
    makePacket(buf,(uint8_t*)header,sizeof(header), data,len)
    mesh.write(buf, len+sizeof(header)) ;
}