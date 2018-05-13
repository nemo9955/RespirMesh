#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>   

#define MAXPACKETSIZE 4096

class TCPServer {
public:
  int sockfd, newsockfd;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
  uint8_t msg[MAXPACKETSIZE];
  int msgLen;

  void setup(int port);
  int receive();

  int getMsgLen();
  void detach();
  void clean();
  void receiveMesage();
};

#endif
