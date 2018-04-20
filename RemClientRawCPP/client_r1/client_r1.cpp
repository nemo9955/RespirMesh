// #include <iostream>
#include <signal.h>
#include "TCPClient.h"
#include "TCPServer.h"
#include <stdio.h>
#include <string> // std::string, std::stoi

#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>
#include "mesh-packet.pb.h"
#include "RemHeaderTypes.h"

int action_counter = 0;
uint8_t pb_buffer[64];

TCPClient tcpParent;
TCPServer tcpServer;

void sig_exit(int s)
{
    tcpParent.exit();
    exit(0);
}

uint32_t chipID = 0;

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

    pb_ostream_t ostream = pb_ostream_from_buffer((uint8_t *)pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
    bool pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &remPingPong);

    if (!pb_status)
    {
        printf("Encoding PingPong failed: %s\n", PB_GET_ERROR(&ostream));
        return;
    }

    printf(" HEADER size %d \n", offsetHeader);
    printf(" Protobuf size %d \n", ostream.bytes_written);

    // pb_buffer[ostream.bytes_written + offsetHeader] = '\n';
    // pb_buffer[ostream.bytes_written + offsetHeader + 1] = '\r';
    // pb_buffer[ostream.bytes_written + offsetHeader + 2] = '\0';

    for (uint8_t i = 0; i < ostream.bytes_written + offsetHeader; i++)
        printf("%d ", pb_buffer[i]);
    printf("\n");

    tcpParent.Send((void *)pb_buffer, ostream.bytes_written + offsetHeader);
    // tcpParent.Send(string(((char *)pb_buffer), ostream.bytes_written + offsetHeader + 1));
    // parClient.write((const char *)(pb_buffer), ostream.bytes_written + offsetHeader);
}

void *recvServ(void *m)
{
    pthread_detach(pthread_self());
    tcpServer.receive();
}

void handleMeshTopology(uint8_t *data, size_t len)
{

    for (uint8_t i = 0; i < len + 10; i++)
        printf("%d ", ((char *)data)[i]);
    printf("\n");
    for (uint8_t i = 0; i < len + 10; i++)
        printf("%d ", ((char *)pb_buffer)[i]);
    printf("\n");

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
        printf("%d ", ((char *)pb_buffer)[i]);
    printf("\n");

    if (header->ForwardingType == ForwardingType_PARENT_TO_ROOT)
    {

        header->ForwardingType = ForwardingType_TO_ROOT;
        meshTopo.type = ProtobufType_MESH_TOPOLOGY;
        meshTopo.target_id = chipID;

        pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
        pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &meshTopo);

        if (!pb_status)
        {
            printf("Encoding meshTopo failed: %s\n", PB_GET_ERROR(&ostream));
            return;
        }

        // printf(" target_id %x \n", pingpong.target_id);
        printf("Sending completed TOPO to root %d %d \n", ostream.bytes_written, offsetHeader);
        printf("         src %x  tar %x \n", meshTopo.source_id, meshTopo.target_id);
        printf("         Forward %d \n", header->ForwardingType);
        printf("             Hea %d \n", header->HeaderType);
        printf("           Proto %d \n", header->ProtobufType);

        for (uint8_t i = 0; i < ostream.bytes_written + offsetHeader + 3; i++)
            printf("%d ", ((char *)pb_buffer)[i]);
        printf("\n");

        // parClient.write((const char *)(pb_buffer), ostream.bytes_written + offsetHeader);
                tcpParent.Send((void *)(pb_buffer), ostream.bytes_written + offsetHeader);
    }
}

void *loop(void *m)
{
    pthread_detach(pthread_self());
    while (1)
    {
        srand(time(NULL));
        // char ch = 'a' + rand() % 26;
        // string s(1, ch);
        int len = tcpServer.getMsgLen();
        if (len > 0)
        {
            uint8_t *data = tcpServer.msg;
            // char *mes = tcpServer.msg;
            // cout << "Message:" << str << endl;
            // string str = tcpServer.getMessage();

            RemBasicHeader *header = (RemBasicHeader *)data;
            uint16_t offsetHeader = sizeof(RemBasicHeader);
            // memcpy(&header, data, offsetHeader);

            printf("  ForwardingType %d \n", header->ForwardingType);
            printf("      HeaderType %d \n", header->HeaderType);
            printf("    ProtobufType %d \n", header->ProtobufType);

            uint8_t *packetData = ((uint8_t *)(data)) + offsetHeader;
            uint16_t packetDataLen = len - offsetHeader;

            if (header->ForwardingType == ForwardingType_TO_ROOT)
            {
                // parClient.write(((const char *)data), len);
                tcpParent.Send((void *)data, len);
            }
            else if (header->ForwardingType == ForwardingType_PARENT_TO_ROOT)
            {
                switch (header->ProtobufType)
                {
                case ProtobufType_MESH_TOPOLOGY:
                    printf("putting data in  TOPO to root : \n");
                    handleMeshTopology((uint8_t *)(data), len);
                    break;
                }
            }
            else
            {

                switch (header->ProtobufType)
                {
                case ProtobufType_TIMESYNC:
                    // handleTimeSync(c, packetData, packetDataLen, header);
                    break;
                case ProtobufType_PONG:
                case ProtobufType_PING:
                    // handlePingPong(c, packetData, packetDataLen, header);
                    break;
                    // default:
                    //     break;
                }
            }

            printf("GOT  \n");
            for (uint8_t i = 0; i < len; i++)
                printf(" %d", tcpServer.msg[i]);
            // printf("%d ", str.c_str()[i]);
            printf("\n");

            // tcpServer.Send(" [client message: " + str + "] " + s);
            tcpServer.clean();
        }
        usleep(1000);
    }
    tcpServer.detach();
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("First argument specify the port to connect to \n");
        printf("Second argument specify a port to open the server to\n");
        exit(1);
    }

    srand(time(NULL));
    chipID = rand() % 60000;
    signal(SIGINT, sig_exit);

    pthread_t msg;
    pthread_t serv;
    tcpServer.setup(atoi(argv[2]));
    if (pthread_create(&msg, NULL, loop, (void *)0) == 0)
    {
        pthread_create(&serv, NULL, recvServ, (void *)0);
    }

    tcpParent.setup("127.0.0.1", atoi(argv[1]));

    while (1)
    {
        action_counter++;

        if (action_counter % 2 == 1)
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

        // tcpParent.Send(to_string(rand() % 25000));
        // string rec = tcpParent.receive();
        // if (rec != "")
        // {
        //     cout << "Server Response:" << rec << endl;
        // }
        sleep(1);
    }
    return 0;
}
