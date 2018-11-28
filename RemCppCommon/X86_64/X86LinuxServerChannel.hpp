#ifndef X86LINUXSERVERCHANNEL_HPP_
#define X86LINUXSERVERCHANNEL_HPP_

#include <libsocket/inetserverstream.hpp>
#include "RemChannel.hpp"

class X86LinuxServerChannel : public RemChannel
{

  private:
    static uint16_t ch_id_common;
    libsocket::inet_stream_server serv_sock;
    // RemRouter *rem_srv;
    RemOrchestrator *remOrch;
    // char host[32];
    // char port[8];

  public:
    bool is_connected;

  public:
    X86LinuxServerChannel();
    ~X86LinuxServerChannel();
    void init(char *_host, char *_port, RemOrchestrator *remOrch_);
    bool is_ok();
    uint16_t send(uint8_t *data, uint16_t size);
    void stop();
    void accept_loop();
    uint16_t ch_id() { return ch_id_common; };

    // static RemChannel *instantiate()
    // {
    //     // return new X86LinuxServerChannel();
    //     return nullptr;
    // };

  private:

    template <typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args &&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    };

    static void *accept_starter(void *_this)
    {
        ((X86LinuxServerChannel *)_this)->accept_loop();
        return NULL;
    };
};

#endif /* !X86LINUXSERVERCHANNEL_HPP_ */
