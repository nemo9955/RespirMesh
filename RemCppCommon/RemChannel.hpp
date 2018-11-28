#ifndef REMCHANNEL_HPP_
#define REMCHANNEL_HPP_

#include <stdint.h>

#include "RemOrchestrator.hpp"

typedef void (*ReceiveDataCallback)(uint8_t *data, uint16_t size, void *arg);

// struct SelfReference
// {
//     void *_this;
//     void *argv;
// };

// struct RemChannelAndId
// {
//     RemChannel *rc;
//     uint32_t id;
// };
// std::list<RemChannelAndId *> MapedChannels;

class RemOrchestrator ;

class RemChannel
{
  public:
    RemChannel()
    {
        connected_to_root = false;
        is_client = false;
        is_server = false;
    };

    // virtual ~RemChannel()
    // {
    //     stop();
    // };

    // virtual void init(char *address, int port)=0;

    // virtual void init(char *_host, char *_port, RemOrchestrator *remOrch_) = 0;
    // virtual void init(void *_clone, RemOrchestrator *remOrch_) = 0;

    virtual uint16_t send(uint8_t *data, uint16_t size) = 0;
    virtual void stop() = 0;
    virtual bool is_ok() = 0;

    virtual uint16_t ch_id() { return 1; };

    virtual void received(uint8_t *data, uint16_t size)
    {
        on_recv_cb(data, size, rem_pointer);
        // on_recv_cb(data, size, on_recv_arg);
    };

    virtual void set_receiver(ReceiveDataCallback cb, void *arg)
    {
        rem_pointer = arg;
        // argv_recv._this = this;
        // argv_recv.argv = arg;
        on_recv_cb = cb;
        // on_recv_arg = (void *)&argv_recv;
    };

    bool connected_to_root;
    bool is_client;
    bool is_server;

  protected:
    ReceiveDataCallback on_recv_cb;
    void *rem_pointer;
    // void *on_recv_arg;
    // SelfReference argv_recv;
};

#endif /* !REMCHANNEL_HPP_ */
