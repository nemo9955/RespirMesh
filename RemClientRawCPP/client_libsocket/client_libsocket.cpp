#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>

#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <chrono>
#include <signal.h>
#include <memory>

#include <fcntl.h>

#include "mesh-packet.pb.h"
#include "RemHeaderTypes.h"
#include "RemChannel.hpp"
#include "RespirMesh.hpp"

#include <libsocket/inetclientstream.hpp>
#include <libsocket/inetserverstream.hpp>
#include <libsocket/exception.hpp>
#include <libsocket/socket.hpp>
#include <libsocket/select.hpp>

uint32_t chipID = 0;

using namespace std;
void sig_exit(int s);

class x86LinuxHardware : public Hardware
{
  public:
    x86LinuxHardware(){
        // hard_id = chipID;
    };
    uint32_t device_id() { return chipID; };

    uint32_t time_milis()
    {
        unsigned long milliseconds_since_epoch =
            std::chrono::system_clock::now().time_since_epoch() /
            std::chrono::milliseconds(1);
        return milliseconds_since_epoch - 0;
    };

  private:
    // uint32_t hard_id;
};

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

int ch_id_common = 0;
class x86LinuxClientChannel : public RemChannel
{
  private:
    unique_ptr<libsocket::inet_stream> cli_sock;
    // libsocket::inet_dgram_client cli_sock;
    // char host[32];
    // char port[8];
    int ch_id;
    bool managed_to_send;

  public:
    x86LinuxClientChannel() : cli_sock(make_unique<libsocket::inet_stream>()){};

    void init(char *_host, char *_port)
    {
        // strcpy(host, _host.c_str());
        // strcpy(port, _port.c_str());
        managed_to_send = true;
        try
        {
            connected_to_root = true;
            logf("Local TCP started %s:%s \n", _host, _port);
            // cli_sock = make_unique<libsocket::inet_stream>();
            cli_sock->connect(_host, _port, LIBSOCKET_IPv4);
            ch_id = ch_id_common + chipID;
            ch_id_common++;
        }
        catch (const libsocket::socket_exception &exc)
        {
            managed_to_send = false;
            cout << " x86LinuxClientChannel :    void init(char *_host, char *_port)   " << endl;
            cerr << exc.mesg;
            return;
        }

        pthread_t listen_thread;
        pthread_create(&listen_thread, NULL, &x86LinuxClientChannel::receive_starter, this);
        pthread_detach(listen_thread);
    };

    void init(unique_ptr<libsocket::inet_stream> _sock_client)
    {
        managed_to_send = true;
        try
        {
            ch_id = ch_id_common + chipID;
            ch_id_common++;
            connected_to_root = false;
            // cli_sock = _sock_client;
            cli_sock = move(_sock_client);
        }
        catch (const libsocket::socket_exception &exc)
        {
            managed_to_send = false;
            cout << " x86LinuxClientChannel :    void init(libsocket::inet_stream _sock_client)   " << endl;
            cerr << exc.mesg;
            return;
        }

        pthread_t listen_thread;
        pthread_create(&listen_thread, NULL, &x86LinuxClientChannel::receive_starter, this);
        pthread_detach(listen_thread);
    };

    bool is_ok()
    {

        int r;
        r = fcntl(cli_sock->getfd(), F_GETFL);
        if (r == -1)
            managed_to_send = false;

        // logf(" ... fd  %d     %d \n",cli_sock->getfd() , r);

        return managed_to_send;
    };

    bool send(uint8_t *data, uint16_t size)
    {
        // logf("\t cli send  \n");
        ssize_t act_size;

        try
        {
            // logf("SENd ch %d [%d] %s \n", this->ch_info(), size, (uint8_t *)data);
            funcf("[ CHAN ] %d send client [size %zu]   ", ch_info(), size);
            for (uint8_t i = 0; i < size; i++)
                funcf("%d ", data[i]);
            act_size = cli_sock->snd((void *)data, size);
            funcf(" ===> actual %d ", act_size);
            funcf("\n");
            funcf("\n");
            funcf("\n");
            funcf("\n");
        }
        catch (const libsocket::socket_exception &exc)
        {
            managed_to_send = false;
            cout << " x86LinuxClientChannel :    bool send(uint8_t *data, uint16_t size)" << endl;
            cerr << exc.mesg;
            return false;
        }
        return act_size > 0;
    };

    int ch_info() { return ch_id; };

    void stop()
    {
        cli_sock->shutdown();
    };

    void receive_loop()
    {
        char buf[64];
        ssize_t recvbyt;
        while (1)
        {
            try
            {
                recvbyt = cli_sock->rcv(&buf, sizeof(buf));
                if (recvbyt == 0)
                {
                    managed_to_send = false;
                    logf(" x86LinuxClientChannel :       void receive_loop()  \n");
                    logf(" ^ STOPPED the socket  \n");
                    stop();

                    if (connected_to_root == true)
                    {
                        sig_exit(42);
                    }
                    return;
                }
                if (recvbyt > 0)
                {
                    buf[recvbyt] = '\0';

                    funcf("\n");
                    funcf("\n");
                    funcf("\n");
                    funcf("\n");
                    funcf("[ CHAN ] %d recv client [size %zu]   ", ch_info(), recvbyt);
                    for (uint8_t i = 0; i < recvbyt; i++)
                        funcf("%d ", buf[i]);
                    funcf("\n");

                    this->received((uint8_t *)buf, recvbyt);
                }
            }
            catch (const libsocket::socket_exception &exc)
            {
                cout << " x86LinuxClientChannel :       void receive_loop()" << endl;
                cerr << exc.mesg;
                managed_to_send = false;
                return;
            }
        }
    };

  private:
    static void *receive_starter(void *_this)
    {
        ((x86LinuxClientChannel *)_this)->receive_loop();
        return NULL;
    };
};

class x86LinuxServerChannel : public RemChannel
{

  private:
    libsocket::inet_stream_server serv_sock;
    RespirMesh *rem_srv;
    // char host[32];
    // char port[8];

  public:
    x86LinuxServerChannel() : serv_sock(){};

    void init(char *_host, char *_port, RespirMesh *_rem_srv)
    {
        // strcpy(host, _host);
        // strcpy(port, _port);

        connected_to_root = false;

        rem_srv = _rem_srv;

        try
        {
            logf("Server TCP setup %s:%s \n", _host, _port);
            serv_sock.setup(_host, _port, LIBSOCKET_IPv4);
        }
        catch (const libsocket::socket_exception &exc)
        {
            cout << " x86LinuxServerChannel :  void init(char *_host, char *_port, RespirMesh *_rem_srv)    " << endl;
            cerr << exc.mesg;
            exit(60);
            return;
        }

        pthread_t listen_thread;
        pthread_create(&listen_thread, NULL, &x86LinuxServerChannel::accept_starter, this);
        pthread_detach(listen_thread);
    };

    bool is_ok()
    {
        return true;
    };

    bool send(uint8_t *data, uint16_t size)
    {
        return true;
    };

    void stop()
    {
        serv_sock.destroy();
    };

    void accept_loop()
    {
        while (1)
        {
            try
            {

                unique_ptr<libsocket::inet_stream> client = serv_sock.accept2();
                x86LinuxClientChannel *chan_client_ = new x86LinuxClientChannel();
                chan_client_->init(move(client));

                funcf("\n");
                funcf("\n");
                funcf("Server accepted client %d \n", chan_client_->ch_info());
                rem_srv->add_channel(move(chan_client_));
                funcf("\n");
                funcf("\n");
            }
            catch (const libsocket::socket_exception &exc)
            {
                cout << " x86LinuxServerChannel :       void accept_loop()" << endl;
                cerr << exc.mesg;
                return;
            }
        }
    };

  private:
    static void *accept_starter(void *_this)
    {
        ((x86LinuxServerChannel *)_this)->accept_loop();
        return NULL;
    }
};

x86LinuxHardware hardware_;
x86LinuxClientChannel clientTcp;
x86LinuxServerChannel serverTcp;
RespirMesh mesh;

void sig_exit(int s)
{
    clientTcp.stop();
    serverTcp.stop();
    exit(0);
}

int main(int argc, char *argv[])
{
    printf(" \n\n\n\n  STARTING !!!!!!!!!!!!!!!!!!!!! \n\n\n\n\n");
    if (argc < 3)
    {
        printf("First argument specify the port to connect to \n");
        printf("Second argument specify a port to open the server to\n");
        exit(1);
    }
    if (argc == 4)
        chipID = atoi(argv[3]);
    else
        chipID = rand() % 640000;

    // char the_host[] = "192.168.1.60";
    char the_host_local[] = "localhost";
    char *the_host_client = the_host_local;

    if (argc == 5)
        the_host_client = const_cast<char *>(argv[4]);

    char the_host_server[] = "0.0.0.0";
    char *root_port = const_cast<char *>(argv[1]);
    char *server_port = const_cast<char *>(argv[2]);
    srand(time(NULL));

    signal(SIGINT, sig_exit);

    logf(" chipID %d %x \n", chipID, chipID);
    logf("testing host:port 1 %s: %s <- %s \n", the_host_client, root_port, server_port);

    clientTcp.init(the_host_client, root_port);
    serverTcp.init(the_host_server, server_port, &mesh);
    mesh.add_channel(&clientTcp);

    // uint8_t test[] = {0, 0, 5};
    // logf(" ... snd %d \n", clientTcp.send(test, 3));

    mesh.set_hardware(&hardware_);

    logf("Time: %d  \n", mesh.time_milis());
    logf("d ID: %d = 0x %x \n", mesh.device_id(), mesh.device_id());

    while (1)
    {
        mesh.update();
        sleep(1);
    }

    return 0;
}
