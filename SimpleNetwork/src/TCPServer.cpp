#include "TCPServer.h"
#include <functional>

// string TCPServer::Message;

void TCPServer::Task()
{

        // memccpy(outt,msg, 0, n );
        // Message = string(msg);

}

void TCPServer::setup(int port)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);
    bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    listen(sockfd, 5);
}

TCPClient* TCPServer::receive()
{

        socklen_t sosize = sizeof(clientAddress);
        newsockfd = accept(sockfd, (struct sockaddr *)&clientAddress, &sosize);
        // str = inet_ntoa(clientAddress.sin_addr);
        // auto ptrFun = std::mem_fn<TCPServer>(&TCPServer::Task) ;
        // receiveMesage();
        // ThreadContainer con {this,newsockfd};

        return new TCPClient( newsockfd);

}
void TCPServer::receiveMesage(){
    int n= recv(newsockfd, msg, MAXPACKETSIZE, 0);
        if (n == 0)
        {
            close(newsockfd);
        }
        msg[n] = '\0';
        // send(newsockfd,msg,n,0);
        msgLen = n;
}
string TCPServer::getMessage()
{
    return Message;
}

int TCPServer::getMsgLen()
{
    return msgLen;
}

void TCPServer::Send(string msg)
{
    send(newsockfd, msg.c_str(), msg.length(), 0);
}

void TCPServer::Send(uint8_t* data, uint8_t len){
     send(newsockfd, data,len, 0);
}
void TCPServer::clean()
{
    Message = "";
    memset(msg, 0, MAXPACKETSIZE);
    msgLen = 0;
}

void TCPServer::detach()
{
    close(sockfd);
    close(newsockfd);
}
