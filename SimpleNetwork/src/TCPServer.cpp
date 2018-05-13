#include "TCPServer.h"
// #include <functional>

void TCPServer::setup(int port) {
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons(port);
  bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  listen(sockfd, 5);
}

int TCPServer::receive() {

  socklen_t sosize = sizeof(clientAddress);
  newsockfd = accept(sockfd, (struct sockaddr *)&clientAddress, &sosize);
  // str = inet_ntoa(clientAddress.sin_addr);
  // auto ptrFun = std::mem_fn<TCPServer>(&TCPServer::Task) ;
  // receiveMesage();
  // ThreadContainer con {this,newsockfd};

  return newsockfd;
}
void TCPServer::receiveMesage() {
  int n = recv(newsockfd, msg, MAXPACKETSIZE, 0);
  if (n == 0) {
    close(newsockfd);
  }
  msg[n] = '\0';
  // send(newsockfd,msg,n,0);
  msgLen = n;
}

int TCPServer::getMsgLen() { return msgLen; }
void TCPServer::clean() {
  memset(msg, 0, MAXPACKETSIZE);
  msgLen = 0;
}

void TCPServer::detach() {
  close(sockfd);
  close(newsockfd);
}
