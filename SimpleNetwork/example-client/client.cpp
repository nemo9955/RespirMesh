// #include <iostream>
#include <signal.h>
#include "TCPClient.h"
#include <stdio.h>

#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>
#include "mesh-packet.pb.h"
#include "RemHeaderTypes.h"

int action_counter = 0;
char pb_buffer[64];

TCPClient tcp;

void sig_exit(int s)
{
    tcp.exit();
    exit(0);
}

uint32_t chipID = 123456789;

void send_ping_to_server()
{
}

void send_mesh_topo_to_server()
{
    RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
    uint16_t offsetHeader = sizeof(RemBasicHeader);
    header->ForwardingType = ForwardingType_TO_ROOT;
    header->HeaderType = HeaderType_BASIC;

    RespirMeshInfo remPingPong = RespirMeshInfo_init_default;
    remPingPong.source_id = chipID;

    header->ForwardingType = ForwardingType_PARENT_TO_ROOT;
    header->ProtobufType = ProtobufType_MESH_TOPOLOGY;
    remPingPong.type = ProtobufType_MESH_TOPOLOGY;

    pb_ostream_t ostream = pb_ostream_from_buffer((uint8_t*)pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
    bool pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &remPingPong);

    if (!pb_status)
    {
        printf("Encoding PingPong failed: %s\n", PB_GET_ERROR(&ostream));
        return;
    }

    printf(" HEADER size %d \n", offsetHeader);
    printf(" Protobuf size %d \n", ostream.bytes_written);

    pb_buffer[ostream.bytes_written + offsetHeader] = '\n';
    pb_buffer[ostream.bytes_written + offsetHeader + 1] = '\r';
    pb_buffer[ostream.bytes_written + offsetHeader + 2] = '\0';

    for (uint8_t i = 0; i < ostream.bytes_written + offsetHeader + 2; i++)
        printf("%d ", pb_buffer[i]);
    printf("\n");

    tcp.Send((void*)pb_buffer , ostream.bytes_written + offsetHeader);
    // tcp.Send(string(((char *)pb_buffer), ostream.bytes_written + offsetHeader + 1));
    // parClient.write((const char *)(pb_buffer), ostream.bytes_written + offsetHeader);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sig_exit);

    tcp.setup("127.0.0.1", 9995);

    while (1)
    {
        action_counter++;
        srand(time(NULL));

        if (action_counter % 4 == 1)
        {
            send_mesh_topo_to_server();
        }
        else if (action_counter % 3 == 0)
        {
            send_ping_to_server();
        }
        else
        {
            send_ping_to_server();
        }

        // tcp.Send(to_string(rand() % 25000));
        // string rec = tcp.receive();
        // if (rec != "")
        // {
        //     cout << "Server Response:" << rec << endl;
        // }
        sleep(1);
    }
    return 0;
}
