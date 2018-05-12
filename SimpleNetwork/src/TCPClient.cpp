#include "TCPClient.h"

TCPClient::TCPClient()
{
    sock = -1;
    port = 0;
    address = "";
}
TCPClient::TCPClient(int sockc)
{
    sock =  sockc;
}
bool TCPClient::setup(string address, int port)
{
    if (sock == -1)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1)
        {
            cout << "Could not create socket" << endl;
        }
    }

    /*
    cast to correct warning:
    comparison between signed and unsigned integer expressions
    */
    if ((int32_t)inet_addr(address.c_str()) == -1)
    {
        struct hostent *he;
        struct in_addr **addr_list;
        if ((he = gethostbyname(address.c_str())) == NULL)
        {
            herror("gethostbyname");
            cout << "Failed to resolve hostname\n";
            return false;
        }
        addr_list = (struct in_addr **)he->h_addr_list;
        for (int i = 0; addr_list[i] != NULL; i++)
        {
            server.sin_addr = *addr_list[i];
            break;
        }
    }
    else
    {
        server.sin_addr.s_addr = inet_addr(address.c_str());
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return false;
    }
    return true;
}

bool TCPClient::Send(void *data, int len)
{
    if (sock != -1)
    {
        if (send(sock, (void *)data, len, 0) < 0)
        {
            cout << "Send failed : " << data << endl;
            return false;
        }
    }
    else
        return false;
    return true;
}

void TCPClient::receive()
{
    // char buffer[size];
    int n;
    // cout << "1" << endl;
    // memset(&buffer[0], 0, sizeof(buffer));
    // cout << "2" << endl;

    // string reply;
    n = recv(sock, msg, MAXPACKETSIZE, 0);
    if (n == 0)
    {
        // msgLen = 0;
        cout << "receive failed!" << endl;
        return ;
    }

    msg[n] = '\0';
    msgLen = n;
    // buffer[size - 1] = '\0';
    // reply = buffer;
    return;
}

string TCPClient::read()
{
    char buffer[1] = {};
    string reply;
    while (buffer[0] != '\n')
    {
        if (recv(sock, buffer, sizeof(buffer), 0) < 0)
        {
            cout << "receive failed!" << endl;
            return nullptr;
        }
        reply += buffer[0];
    }
    return reply;
}

void TCPClient::clean()
{
    memset(msg, 0, MAXPACKETSIZE);
    msgLen = 0;
}


void TCPClient::exit()
{
    close(sock);
}
