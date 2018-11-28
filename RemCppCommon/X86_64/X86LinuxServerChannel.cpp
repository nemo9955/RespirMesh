#include "X86LinuxServerChannel.hpp"
#include "X86LinuxClientChannel.hpp"

#include <libsocket/inetclientstream.hpp>
// #include <libsocket/inetserverstream.hpp>
#include <libsocket/exception.hpp>
// #include <libsocket/socket.hpp>
// #include <libsocket/select.hpp>

#include <iostream>
// #include <memory>



uint16_t X86LinuxServerChannel::ch_id_common = 0;

X86LinuxServerChannel::X86LinuxServerChannel() : serv_sock()
{
    is_connected = false;
    ch_id_common++;
};

X86LinuxServerChannel::~X86LinuxServerChannel()
{
    stop();
};

void X86LinuxServerChannel::init(char *_host, char *_port, RemOrchestrator *remOrch_)
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
        std::cout << " X86LinuxServerChannel :  void init(char *_host, char *_port, RemRouter *_rem_srv)    " << std::endl;
        std::cerr << exc.mesg;
        exit(60);
        return;
    }

    pthread_t listen_thread;
    pthread_create(&listen_thread, NULL, &X86LinuxServerChannel::accept_starter, this);
    pthread_detach(listen_thread);
};

bool X86LinuxServerChannel::is_ok()
{
    return true;
};

uint16_t X86LinuxServerChannel::send(uint8_t *data, uint16_t size)
{
    return 0;
};

void X86LinuxServerChannel::stop()
{
    remOrch->logs->info("X86LinuxServerChannel::stop \n");
    serv_sock.destroy();
};

void X86LinuxServerChannel::accept_loop()
{
    while (1)
    {
        try
        {

            std::unique_ptr<libsocket::inet_stream> client = serv_sock.accept2();
            X86LinuxClientChannel *chan_client_ = new X86LinuxClientChannel();
            chan_client_->init(std::move(client), remOrch);

            // funcf("\n");
            // funcf("\n");
            // funcf("Server accepted client %d \n", chan_client_->ch_id());
            remOrch->add_channel(std::move(chan_client_));
        }
        catch (const libsocket::socket_exception &exc)
        {
            std::cout << " X86LinuxServerChannel :       void accept_loop()" << std::endl;
            std::cerr << exc.mesg;
            return;
        }
    }
};

