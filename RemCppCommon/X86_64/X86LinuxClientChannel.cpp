#include "X86LinuxClientChannel.hpp"

#include <iostream>
#include <fcntl.h>

uint16_t X86LinuxClientChannel::ch_id_common = 0;

X86LinuxClientChannel::X86LinuxClientChannel()
// : cli_sock(make_unique<libsocket::inet_stream>())
{
    is_connected = false;
    ch_id_common++;
};

X86LinuxClientChannel::~X86LinuxClientChannel()
{
    stop();
};

void X86LinuxClientChannel::init(char *_host, char *_port, RemOrchestrator *remOrch_)
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
        chip_id = ch_id_common + remOrch->basicHardware->device_id();
        is_connected = true;
    }
    catch (const libsocket::socket_exception &exc)
    {
        managed_to_send = false;
        std::cout << " X86LinuxClientChannel :    void init(char *_host, char *_port)   " << std::endl;
        std::cerr << exc.mesg;
        return;
    }

    pthread_t listen_thread;
    pthread_create(&listen_thread, NULL, &X86LinuxClientChannel::receive_starter, this);
    pthread_detach(listen_thread);
};

void X86LinuxClientChannel::init(std::unique_ptr<libsocket::inet_stream> _sock_client, RemOrchestrator *remOrch_)
{
    remOrch = remOrch_;
    managed_to_send = true;
    try
    {
        chip_id = ch_id_common + remOrch->basicHardware->device_id();
        ch_id_common++;
        connected_to_root = false;
        // cli_sock = _sock_client;
        cli_sock = std::move(_sock_client);
        is_connected = true;
    }
    catch (const libsocket::socket_exception &exc)
    {
        managed_to_send = false;
        std::cout << " X86LinuxClientChannel :    void init(libsocket::inet_stream _sock_client)   " << std::endl;
        std::cerr << exc.mesg;
        return;
    }

    pthread_t listen_thread;
    pthread_create(&listen_thread, NULL, &X86LinuxClientChannel::receive_starter, this);
    pthread_detach(listen_thread);
};

bool X86LinuxClientChannel::is_ok()
{

    int r;
    r = fcntl(cli_sock->getfd(), F_GETFL);
    if (r == -1)
        managed_to_send = false;

    return managed_to_send;
};

uint16_t X86LinuxClientChannel::send(uint8_t *data, uint16_t size)
{
    // logf("\t cli send  \n");
    ssize_t act_size;

    try
    {
        // logf("SENd ch %d [%d] %s \n", this->ch_id(), size, (uint8_t *)data);
        // funcf("chn %d send client [size %zu] \t", ch_id(), size);
        // for (uint8_t i = 0; i < size; i++)
        //     funcf("%d ", data[i]);

        act_size = cli_sock->snd((void *)data, size);
        // if the server receives to fast ... it combines the 2 packets
        remOrch->basicHardware->sleep_milis(10);
    }
    catch (const libsocket::socket_exception &exc)
    {
        managed_to_send = false;
        std::cout << " X86LinuxClientChannel :    uint16_t send(uint8_t *data, uint16_t size)" << std::endl;
        std::cerr << exc.mesg;
        return false;
    }
    return act_size > 0;
};

void X86LinuxClientChannel::stop()
{
    remOrch->logs->info("X86LinuxClientChannel::stop \n");
    if (is_connected == true)
        cli_sock->shutdown();
};

void X86LinuxClientChannel::receive_loop()
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
                remOrch->logs->error(" X86LinuxClientChannel :       void receive_loop() STOPPED the socket ");
                stop();

                if (connected_to_root == true)
                {
                    // sig_exit(42);
                }
                return;
            }
            if (recvbyt > 0)
            {
                buf[recvbyt] = '\0';

                // funcf("chn %d recv client [size %zu]  \t", ch_id(), recvbyt);
                // for (uint8_t i = 0; i < recvbyt; i++)
                //     funcf("%d ", buf[i]);
                // funcf("\n");

                this->received((uint8_t *)buf, recvbyt);
            }
        }
        catch (const libsocket::socket_exception &exc)
        {
            std::cout << " X86LinuxClientChannel :       void receive_loop()" << std::endl;
            std::cerr << exc.mesg;
            managed_to_send = false;
            return;
        }
    }
};
