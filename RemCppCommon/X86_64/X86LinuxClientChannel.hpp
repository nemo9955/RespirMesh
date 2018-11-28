#ifndef X86LINUXCLIENTCHANNEL_HPP_
#define X86LINUXCLIENTCHANNEL_HPP_

#include "RemChannel.hpp"
#include "ConnectionListener.hpp"

// #include <libsocket/inetclientstream.hpp>
#include <libsocket/inetserverstream.hpp>
// #include <libsocket/exception.hpp>
// #include <libsocket/socket.hpp>
// #include <libsocket/select.hpp>

#include <utility> // std::pair, std::make_pair
#include <memory>

// typedef void (*callback_conn_lost)();

class X86LinuxClientChannel : public RemChannel
{
  private:
    static uint16_t ch_id_common;
    std::unique_ptr<libsocket::inet_stream> cli_sock;
    int chip_id;
    bool managed_to_send;
    RemOrchestrator *remOrch;

    ConnectionListener *conn_listener;

  public:
    bool is_connected;

  public:
    X86LinuxClientChannel();
    ~X86LinuxClientChannel();

    void init(char *_host, char *_port, RemOrchestrator *remOrch_);
    void init(std::unique_ptr<libsocket::inet_stream> _sock_client, RemOrchestrator *remOrch_);
    bool is_ok();
    uint16_t send(uint8_t *data, uint16_t size);
    uint16_t ch_id() { return chip_id; };
    void stop();
    void receive_loop();

    void set_conn_listener(ConnectionListener *_conn_lsn)
    {
        conn_listener = _conn_lsn;
    };

    // template <class LISTEN>
    // void add_connection_lost_listener(callback_conn_lost _cb_listener)
    // {
    //     con_lost_func = _cb_listener;
    //     // ((LISTEN *)_this)->receive_loop();
    // };

    // static RemChannel *instantiate()
    // {
    //     // return new X86LinuxClientChannel();
    //     return nullptr;
    // };

  private:
    // callback_conn_lost con_lost_func;

    template <typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args &&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    };

    static void *receive_starter(void *_this)
    {
        ((X86LinuxClientChannel *)_this)->receive_loop();
        return NULL;
    };
};

#endif /* !X86LINUXCLIENTCHANNEL_HPP_ */
