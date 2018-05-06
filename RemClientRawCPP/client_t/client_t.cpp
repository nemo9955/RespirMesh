
#define PRINTF  printf
#define infof  printf
#define logf  printf


#include <signal.h>
#include "TCPClient.h"
#include "TCPServer.h"
#include <stdio.h>
#include <string> // std::string, std::stoi

#include "RespirMesh.hpp"
#include "RemChannel.hpp"

void sig_exit(int s);

class LocalTcpChannel
{
  private:
    TCPClient tcpParent;

  public:
    LocalTcpChannel(){};

    void init(char* address, int port)
    {
        logf("Local TCP started %s:%d \n",address,port);

        tcpParent.setup(std::string(address), port);
        // tcpParent.setup("127.0.0.1", atoi(argv[1]));
    }

    void send(uint8_t *data, uint16_t size)
    {
        logf("Local TCP sending .... \n");
        tcpParent.Send((void *)data, size);
    }

    void recv()
    {
        // pthread_create(&serv, NULL, recvServ, (void *)this);
    }

    void stop()
    {
        logf("Local TCP exiting \n");
        tcpParent.exit();
    }
};

uint32_t chipID = 0;
RemChannel<LocalTcpChannel> lTcp;
RespirMesh mesh;

int main(int argc, char *argv[])
{
    PRINTF("STARTING CLIENT \n");

    mesh.add_channel(&lTcp);

    if (argc < 3)
    {
        PRINTF("First argument specify the port to connect to \n");
        PRINTF("Second argument specify a port to open the server to\n");
        exit(1);
    }

    srand(time(NULL));

    if (argc > 3)
        chipID = atoi(argv[3]);
    else
        chipID = rand() % 640000;

    signal(SIGINT, sig_exit);

    lTcp.init("127.0.0.1", atoi(argv[1]) );


    /*
    pthread_t msg;
    pthread_t serv;
    tcpServer.setup(atoi(argv[2]));
    if (pthread_create(&msg, NULL, loop, (void *)0) == 0)
    {
        pthread_create(&serv, NULL, recvServ, (void *)0);
    }


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
    */
    PRINTF("CLIENT DONE \n");
    return 0;
}

void sig_exit(int s)
{
    PRINTF("CLIENT CLEANING STUFF \n");
    lTcp.stop();
    exit(0);
}

/*

// #include <iostream>

#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>
#include "mesh-packet.pb.h"
#include "RemHeaderTypes.h"

int action_counter = 0;
uint8_t pb_buffer[64];

TCPClient tcpParent;
TCPServer tcpServer;


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
        PRINTF("Encoding PingPong failed: %s\n", PB_GET_ERROR(&ostream));
        return;
    }

    PRINTF(" HEADER size %d \n", offsetHeader);
    PRINTF(" Protobuf size %d \n", (int)ostream.bytes_written);

    // pb_buffer[ostream.bytes_written + offsetHeader] = '\n';
    // pb_buffer[ostream.bytes_written + offsetHeader + 1] = '\r';
    // pb_buffer[ostream.bytes_written + offsetHeader + 2] = '\0';

    for (uint8_t i = 0; i < ostream.bytes_written + offsetHeader; i++)
        PRINTF("%d ", pb_buffer[i]);
    PRINTF("\n");

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
        PRINTF("%d ", ((char *)data)[i]);
    PRINTF("\n");
    for (uint8_t i = 0; i < len + 10; i++)
        PRINTF("%d ", ((char *)pb_buffer)[i]);
    PRINTF("\n");

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
        PRINTF("Decoding meshTopo failed %s with type %d\n", PB_GET_ERROR(&istream), header->ProtobufType);
        return;
    }

    for (uint8_t i = 0; i < len + 10; i++)
        PRINTF("%d ", ((char *)pb_buffer)[i]);
    PRINTF("\n");

    if (header->ForwardingType == ForwardingType_PARENT_TO_ROOT)
    {

        header->ForwardingType = ForwardingType_TO_ROOT;
        meshTopo.type = ProtobufType_MESH_TOPOLOGY;
        meshTopo.target_id = chipID;

        pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
        pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &meshTopo);

        if (!pb_status)
        {
            PRINTF("Encoding meshTopo failed: %s\n", PB_GET_ERROR(&ostream));
            return;
        }

        // PRINTF(" target_id %x \n", pingpong.target_id);
        PRINTF("Sending completed TOPO to root %d %d \n", ostream.bytes_written, offsetHeader);
        PRINTF("         src %x  tar %x \n", meshTopo.source_id, meshTopo.target_id);
        PRINTF("         Forward %d \n", header->ForwardingType);
        PRINTF("             Hea %d \n", header->HeaderType);
        PRINTF("           Proto %d \n", header->ProtobufType);

        for (uint8_t i = 0; i < ostream.bytes_written + offsetHeader + 3; i++)
            PRINTF("%d ", ((char *)pb_buffer)[i]);
        PRINTF("\n");

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

            PRINTF("  ForwardingType %d \n", header->ForwardingType);
            PRINTF("      HeaderType %d \n", header->HeaderType);
            PRINTF("    ProtobufType %d \n", header->ProtobufType);

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
                    PRINTF("putting data in  TOPO to root : \n");
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

            PRINTF("GOT  \n");
            for (uint8_t i = 0; i < len; i++)
                PRINTF(" %d", tcpServer.msg[i]);
            // PRINTF("%d ", str.c_str()[i]);
            PRINTF("\n");

            // tcpServer.Send(" [client message: " + str + "] " + s);
            tcpServer.clean();
        }
        usleep(1000);
    }
    tcpServer.detach();
}

*/