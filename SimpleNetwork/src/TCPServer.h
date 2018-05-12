#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "TCPClient.h"

using namespace std;

#define MAXPACKETSIZE 4096

// class TCPServer;
// typedef  struct ThreadContainer
// {
//     TCPServer *mee;
//     long payload;
// };

class TCPServer
{
  public:
    int sockfd, newsockfd, n, pid;
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    pthread_t serverThread;
    uint8_t msg[MAXPACKETSIZE];
    int msgLen;
    // uint8_t outt[MAXPACKETSIZE];
    string Message;

    void setup(int port);
    TCPClient* receive();
    string getMessage();
    int getMsgLen();
    void Send(string msg);
    void Send(uint8_t* data, uint8_t len);
    void detach();
    void clean();
    void receiveMesage();

  private:
    void Task();
};

#endif
