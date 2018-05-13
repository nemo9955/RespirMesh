#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using namespace std;

#define MAXPACKETSIZE 4096

class TCPClient {
private:
  int sock;
  std::string address;
  int port;
  struct sockaddr_in server;

public:
  TCPClient();
  void setup(int sockc);
  bool setup(const char *address, int port);
  bool Send(void *data, int len);
  bool receive();
  string read();
  void exit();
  void clean();
  uint8_t msg[MAXPACKETSIZE];
  int msgLen;
};

#endif
