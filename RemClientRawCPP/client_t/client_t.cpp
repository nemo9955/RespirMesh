#include <chrono>

#include <signal.h>
#include "TCPClient.h"
#include "TCPServer.h"
#include <stdio.h>
#include <string> // std::string, std::stoi

#include "RespirMesh.hpp"
#include "RemChannel.hpp"

void sig_exit(int s);

uint32_t chipID = 0;


class x86LinuxHardware : public Hardware
{
  private:
  public:
    x86LinuxHardware(){};

    uint32_t device_id(){
        return chipID;
    }
    uint32_t time_milis()
    {
        unsigned long milliseconds_since_epoch = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
        return milliseconds_since_epoch - 900000000;
    };


};

class x86LinuxClientChannel:public RemChannel
{
  private:
    static void*recvParent(void *_this){
        ((x86LinuxClientChannel*)_this)->recvParent();
    }

  public:
    TCPClient tcpParent;
    x86LinuxClientChannel(){};

    void init(char *address, int port)
    {
        logf("Local TCP started %s:%d \n", address, port);

        tcpParent.setup(std::string(address), port);
        // tcpParent.setup("127.0.0.1", atoi(argv[1]));
        pthread_t paren;
        pthread_create(&paren, NULL, &x86LinuxClientChannel::recvParent,this);
        pthread_detach(paren);
    }

    void send(uint8_t *data, uint16_t size)
    {
        logf("Local TCP sending .... \n");
        tcpParent.Send((void *)data, size);
    }


    void recvParent()
    {
    // pthread_detach(pthread_self());
    // tcpServer.receive();
    // logf("*****");

        while (1)
        {
            // logf(".");
            tcpParent.receive();
            // logf(" r:%s ", rec);
            if (tcpParent.msgLen > 0)
            {
            // cout << "Server Response:" << rec << endl;
            recv((uint8_t *)tcpParent.msg, tcpParent.msgLen);
            tcpParent.clean();
            }
            // logf(".");
        }
    };


    void stop()
    {
        logf("Local TCP exiting \n");
        tcpParent.exit();
    }
};



class x86LinuxServerChannel:public RemChannel
{
  private:
    static void*recvParent(void *_this){
        ((x86LinuxServerChannel*)_this)->recvParent();
    }

  public:     
    TCPServer tcpSrv;
    x86LinuxServerChannel(){};

    void init( int port)
    {
        logf("Local TCP started:%d \n", port);

        tcpSrv.setup( port);
        // tcpParent.setup("127.0.0.1", atoi(argv[1]));
        pthread_t paren;
        pthread_create(&paren, NULL, &x86LinuxServerChannel::recvParent,this);
        pthread_detach(paren);
        
    }

    void send(uint8_t *data, uint16_t size)
    {
        logf("Local TCP sending .... \n");
        tcpSrv.Send(data, size);
    }


    void recvParent()
    {
        while (1)
        {
            // logf(".");
            tcpSrv.receive();
            // logf(" r:%s ", rec);
            if (tcpSrv.msgLen > 0)
            {
            // cout << "Server Response:" << rec << endl;
            recv((uint8_t *)tcpSrv.msg, tcpSrv.msgLen);
            tcpSrv.clean();
            }
            // logf(".");
        }

    };


    void stop()
    {
        logf("Local TCP exiting \n");
        tcpSrv.detach();
    }
};

int action_counter = 0;

x86LinuxClientChannel clientTcp;
x86LinuxServerChannel serverTcp;
x86LinuxHardware hardware;
RespirMesh mesh(&hardware);

int main(int argc, char *argv[])
{
    PRINTF("STARTING CLIENT \n");

    mesh.add_channel(&clientTcp);
    //mesh.add_channel(&serverTcp);
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
    clientTcp.init("127.0.0.1", atoi(argv[1]));
    serverTcp.init(atoi(argv[2]));

    serverTcp.set_recv_cb(RespirMesh::receive_fn, &mesh);

    while (1)
    {
        action_counter++;
        mesh.update();
        sleep(1);
    }

    PRINTF("CLIENT DONE \n");
    return 0;
}



// void *recvServ(void *m)
// {
//     pthread_detach(pthread_self());
//     tcpServer.receive();
// }

void sig_exit(int s)
{
    logf("CLIENT CLEANING STUFF \n");
    clientTcp.stop();
    exit(0);
}

/*

// #include <iostream>

#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>
#include "mesh-packet.pb.h"
#include "RemHeaderTypes.h"

uint8_t pb_buffer[64];

TCPClient tcpParent;
TCPServer tcpServer;


void send_ping_to_server()
{
}

void *recvServ(void *m)
{
    pthread_detach(pthread_self());
    tcpServer.receive();
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