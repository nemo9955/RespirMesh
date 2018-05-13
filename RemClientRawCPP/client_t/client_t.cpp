#include <chrono>

#include "TCPClient.h"
#include "TCPServer.h"
#include <signal.h>
#include <stdio.h>
// #include <string> // std::string, std::stoi
#include <list>

#include "RemChannel.hpp"
#include "RespirMesh.hpp"

void sig_exit(int s);

uint32_t chipID = 0;

class x86LinuxHardware : public Hardware {
private:
public:
  x86LinuxHardware(){};

  uint32_t device_id() { return chipID; }
  uint32_t time_milis() {
    unsigned long milliseconds_since_epoch =
        std::chrono::system_clock::now().time_since_epoch() /
        std::chrono::milliseconds(1);
    return milliseconds_since_epoch - 900000000;
  };
};

class x86LinuxClientChannel : public RemChannel {
private:
  static void *recvParentStat(void *_this) {
    ((x86LinuxClientChannel *)_this)->recvParentObj();
    return NULL;
  }

public:
  TCPClient tcpParent;
  x86LinuxClientChannel(){};

  int ch_info() { return 500; }
  void init(int socket) {
    logf("Local TCP started in socket:%d \n", socket);
    tcpParent.setup(socket);

    connected_to_root = false;
    // tcpParent.setup("127.0.0.1", atoi(argv[1]));
    pthread_t paren;
    pthread_create(&paren, NULL, &x86LinuxClientChannel::recvParentStat, this);
    pthread_detach(paren);
  }
  void init(const char *address, int port) {
    logf("Local TCP started %s:%d \n", address, port);
    connected_to_root = true;

    tcpParent.setup(address, port);
    // tcpParent.setup("127.0.0.1", atoi(argv[1]));
    pthread_t paren;
    pthread_create(&paren, NULL, &x86LinuxClientChannel::recvParentStat, this);
    pthread_detach(paren);
  }

  bool send(uint8_t *data, uint16_t size) {
    // logf("Client TCP sending .... \n");
    return tcpParent.Send((void *)data, size);
  }

  void recvParentObj() {
    // pthread_detach(pthread_self());
    // tcpServer.receive();
    // logf("*****");

    while (tcpParent.receive()) {
      // logf(" r:%s ", rec);
      if (tcpParent.msgLen > 0) {
        // cout << "Server Response:" << rec << endl;
        this->recv((uint8_t *)tcpParent.msg, tcpParent.msgLen);
        tcpParent.clean();
      }
      // logf(".");
    }
  };

  void stop() {
    logf("Local TCP exiting \n");
    tcpParent.exit();
  }
};

class x86LinuxServerChannel : public RemChannel {
private:
  static void *recvParent(void *_this) {
    ((x86LinuxServerChannel *)_this)->recvParent();
    return NULL;
  }

  std::list<RemChannel *> channels;

public:
  TCPServer tcpSrv;
  x86LinuxServerChannel(){};

  int ch_info() { return 100; }
  void init(int port) {
    logf("Local TCP started:%d \n", port);

    tcpSrv.setup(port);
    // tcpParent.setup("127.0.0.1", atoi(argv[1]));
    pthread_t paren;
    pthread_create(&paren, NULL, &x86LinuxServerChannel::recvParent, this);
    pthread_detach(paren);
  }

  bool send(uint8_t *data, uint16_t size) {
    // logf("Server TCP sending .... \n");
    for (std::list<RemChannel *>::iterator it = channels.begin();
         it != channels.end();) {
      if (!(*it)->send(data, size)) {
        (*it)->stop();
        channels.erase(it++);
      } else {
        ++it;
      }
    }
    return true;
  }
  void recvParent() {
    while (1) {
      x86LinuxClientChannel *c = new x86LinuxClientChannel();
      c->set_recv_cb(this->on_recv_cb, argv_recv.argv);
      channels.push_back((RemChannel *)c);
      int soc = tcpSrv.receive();
      c->init(soc);
    }
  };

  void stop() {
    logf("Local TCP exiting \n");
    tcpSrv.detach();
  }
};

int action_counter = 0;

x86LinuxClientChannel clientTcp;
x86LinuxServerChannel serverTcp;
x86LinuxHardware hardware;
RespirMesh mesh(&hardware);

int main(int argc, char *argv[]) {
  const char *IP = "127.0.0.1";
  signal(SIGINT, sig_exit);
  PRINTF("STARTING CLIENT \n");

  if (argc < 3) {
    PRINTF("First argument specify the port to connect to \n");
    PRINTF("Second argument specify a port to open the server to\n");
    exit(1);
  }

  srand(time(NULL));
  if (argc > 3)
    chipID = atoi(argv[3]);
  else
    chipID = rand() % 640000;

  clientTcp.init(IP, atoi(argv[1]));
  serverTcp.init(atoi(argv[2]));

  mesh.add_channel(&clientTcp);
  mesh.add_channel(&serverTcp);

  while (1) {
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

void sig_exit(int s) {
  logf("CLIENT CLEANING STUFF \n");
  clientTcp.stop();
  exit(0);
}

/*

// #include <iostream>

#include "RemHeaderTypes.h"
#include "mesh-packet.pb.h"
#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

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