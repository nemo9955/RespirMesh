#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXPACKETSIZE 4096

class TCPClient {
private:
  int sock;
  struct sockaddr_in server;

public:
  TCPClient();
  void setup(int sockc);
  bool setup(const char *address, int port);
  bool Send(void *data, int len);
  bool receive();
  void exit();
  void clean();
  uint8_t msg[MAXPACKETSIZE];
  int msgLen;
};

#endif
