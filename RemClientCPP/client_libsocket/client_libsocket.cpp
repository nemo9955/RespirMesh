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
#include <list>
#include <utility> // std::pair, std::make_pair
#include <fcntl.h>

#include <unistd.h>

#include "mesh-packet.pb.h"
#include "RemHeaderTypes.h"
#include "RemChannel.hpp"
#include "RemOrchestrator.hpp"
#include "RemRouter.hpp"
#include "RemConnectionScanner.hpp"
#include <RemLogger.hpp>

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
    x86LinuxHardware(){};
    uint32_t device_id() { return chipID; };

    uint32_t sleep_milis(uint32_t duration)
    {
        usleep(duration);
    };

    uint32_t time_milis()
    {
        uint32_t milliseconds_since_epoch =
            std::chrono::system_clock::now().time_since_epoch() /
            std::chrono::milliseconds(1);
        return milliseconds_since_epoch;
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
    int ch_id;
    bool managed_to_send;
    RemOrchestrator *remOrch;

  public:
    bool is_connected;

  public:
    x86LinuxClientChannel()
    // : cli_sock(make_unique<libsocket::inet_stream>())
    {
        is_connected = false;
    };

    ~x86LinuxClientChannel()
    {
        stop();
    };

    void init(char *_host, char *_port, RemOrchestrator *remOrch_)
    {
        remOrch = remOrch_;
        // strcpy(host, _host.c_str());
        // strcpy(port, _port.c_str());
        managed_to_send = true;
        try
        {
            connected_to_root = true;
            remOrch->logs->info("Local TCP started %s:%s \n", _host, _port);
            cli_sock = make_unique<libsocket::inet_stream>();
            cli_sock->connect(_host, _port, LIBSOCKET_IPv4);
            ch_id_common++;
            ch_id = ch_id_common + chipID;
            is_connected = true;
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

    void init(unique_ptr<libsocket::inet_stream> _sock_client, RemOrchestrator *remOrch_)
    {
        remOrch = remOrch_;
        managed_to_send = true;
        try
        {
            ch_id = ch_id_common + chipID;
            ch_id_common++;
            connected_to_root = false;
            // cli_sock = _sock_client;
            cli_sock = move(_sock_client);
            is_connected = true;
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

        return managed_to_send;
    };

    bool send(uint8_t *data, uint16_t size)
    {
        // logf("\t cli send  \n");
        ssize_t act_size;

        try
        {
            // logf("SENd ch %d [%d] %s \n", this->ch_info(), size, (uint8_t *)data);
            // funcf("chn %d send client [size %zu] \t", ch_info(), size);
            // for (uint8_t i = 0; i < size; i++)
            //     funcf("%d ", data[i]);

            act_size = cli_sock->snd((void *)data, size);
            // if the server receives to fast ... it combines the 2 packets
            remOrch->basicHardware->sleep_milis(5);

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
        remOrch->logs->info("x86LinuxClientChannel::stop \n");
        if (is_connected == true)
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
                    remOrch->logs->error(" x86LinuxClientChannel :       void receive_loop() STOPPED the socket ");
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

                    // funcf("chn %d recv client [size %zu]  \t", ch_info(), recvbyt);
                    // for (uint8_t i = 0; i < recvbyt; i++)
                    //     funcf("%d ", buf[i]);
                    // funcf("\n");

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
    // RemRouter *rem_srv;
    RemOrchestrator *remOrch;
    // char host[32];
    // char port[8];

  public:
    bool is_connected;

  public:
    x86LinuxServerChannel() : serv_sock()
    {
        is_connected = false;
    };

    ~x86LinuxServerChannel()
    {
        stop();
    };

    int ch_info() { return chipID; };

    void init(char *_host, char *_port, RemOrchestrator *remOrch_)
    {
        // strcpy(host, _host);
        // strcpy(port, _port);

        connected_to_root = false;

        remOrch = remOrch_;

        try
        {
            remOrch->logs->info("Server TCP setup %s:%s \n", _host, _port);
            serv_sock.setup(_host, _port, LIBSOCKET_IPv4);
            is_connected = true;
        }
        catch (const libsocket::socket_exception &exc)
        {
            cout << " x86LinuxServerChannel :  void init(char *_host, char *_port, RemRouter *_rem_srv)    " << endl;
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
        remOrch->logs->info("x86LinuxServerChannel::stop \n");
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
                chan_client_->init(move(client), remOrch);

                // funcf("\n");
                // funcf("\n");
                // funcf("Server accepted client %d \n", chan_client_->ch_info());
                remOrch->add_channel(move(chan_client_));
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

template <class CLIENT, class SERVER>
class SimpleListScanner : public RemConnectionScanner
{
  private:
    std::list<std::pair<std::string, std::string>> clients_list;
    std::list<std::pair<std::string, std::string>> servers_list;

    bool is_client_connected;
    bool is_server_started;
    RemOrchestrator *remOrch;

  public:


    void set_orchestrator(RemOrchestrator *remOrch_)
    {
        logf("  SimpleWiFiScanner : set_orchestrator \n");
        remOrch = remOrch_;
    };

    SimpleListScanner()
    {
        is_client_connected = false;
        is_server_started = false;
    };

    void add_client_host(char *_host, char *_port)
    {

        remOrch->logs->info("adding client  %s : %s ", _host, _port);
        clients_list.push_back(std::make_pair(_host, _port));
    };

    void add_server_host(char *_host, char *_port)
    {
        servers_list.push_back(std::make_pair(std::string(_host), std::string(_port)));
    };

    void update()
    {
        if (is_client_connected == false)
        {
            scan_clients();
        }

        if (is_server_started == false)
        {
            start_servers();
        }
    };

    void start_servers()
    {
        remOrch->logs->info(" SimpleListScanner::start_servers servers_list  size  %u  \n", servers_list.size());
        for (auto it = servers_list.begin(); it != servers_list.end(); ++it)
        {
            remOrch->logs->info("\n\n\nTrying to start server %s:%s   \n", it->first.c_str(), it->second.c_str());
            SERVER *server_ = new SERVER();
            server_->init(
                const_cast<char *>(it->first.c_str()),
                const_cast<char *>(it->second.c_str()),
                remOrch);

            if (server_->is_connected == true)
            {
                remOrch->add_channel(move(server_));
                is_server_started = true;
            }
        }
    };

    void scan_clients()
    {
        remOrch->logs->info(" SimpleListScanner::scan_clients  clients_list  size  %u  ", clients_list.size());

        for (auto it = clients_list.begin(); it != clients_list.end(); ++it)
        {
            remOrch->logs->info("\n\n\nTrying to connect to  %s:%s   \n", it->first.c_str(), it->second.c_str());

            CLIENT *client_ = new CLIENT();
            client_->init(
                const_cast<char *>(it->first.c_str()),
                const_cast<char *>(it->second.c_str()),
                remOrch);

            if (client_->is_connected == true)
            {
                remOrch->add_channel(move(client_));
                is_client_connected = true;
            }
        }
    };
};

SimpleListScanner<x86LinuxClientChannel, x86LinuxServerChannel> parentScanner;

x86LinuxHardware hardware_;
RemRouter remRouter;
RemOrchestrator remOrch;
RemLogger logs;

void sig_exit(int s)
{
    remOrch.stop();
    exit(0);
}

int main(int argc, char *argv[])
{

    remOrch.set_router(&remRouter);
    remOrch.set_scanner(&parentScanner);
    remOrch.set_hardware(&hardware_);
    remOrch.set_logger(&logs);

    // for (size_t i = 0; i < argc; i++)
    // {
    //     printf(" %d %s \n", i, argv[i]);
    // }
    // return 0;

    logs.info("_ STARTING !!!!!!!!!!!!!!!!!!!!! _");

    if (argc < 6)
    {
        printf("First 2 arguments specify host and port of the server \n");
        printf("3th arg, the ID of the device, random if 0 \n");
        printf("Next sets of 2 arguments specify host and port of client \n");
        exit(1);
    }

    srand(time(NULL));

    if (atoi(argv[3]) != 0)
        chipID = atoi(argv[3]);
    else
        chipID = rand() % 640000;

    char *server_host = const_cast<char *>(argv[1]);
    char *server_port = const_cast<char *>(argv[2]);
    parentScanner.add_server_host(server_host, server_port);

    for (size_t i = 4; i < argc; i += 2)
    {
        char *client_host = const_cast<char *>(argv[i]);
        char *client_port = const_cast<char *>(argv[i + 1]);
        parentScanner.add_client_host(client_host, client_port);
    }

    signal(SIGINT, sig_exit);

    logf("chipID %d %x \n", chipID, chipID);
    logf("Time : %u  \n", remOrch.basicHardware->time_milis());
    logf("devID: %d == 0x%x \n", remOrch.basicHardware->device_id(), remOrch.basicHardware->device_id());

    remOrch.start();
    while (1)
    {
        remOrch.update();
        sleep(1);
        // logs.trace("main loop update");
    }
    // sleep(1);
    // remOrch.update();

    remOrch.stop();
    return 0;
}
