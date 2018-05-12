#ifndef REMCHANNEL_HPP_
#define REMCHANNEL_HPP_

#include <stdint.h>

typedef void (*ReceiveDataCallback)(uint8_t *data, uint16_t size, void *arg);

class RemChannel
{
  public:
    // virtual void init(char *address, int port)=0;
    virtual void send(uint8_t *data, uint16_t size)=0;
    virtual void stop()=0;

    virtual int ch_info(){return 1;};
    virtual void recv(uint8_t *data, uint16_t size) { on_recv_cb(data, size,on_recv_arg); };
    virtual void set_recv_cb(ReceiveDataCallback cb, void *arg )
    {
        on_recv_cb = cb;
        on_recv_arg = arg;
    };
    private:
    ReceiveDataCallback on_recv_cb;
    void *on_recv_arg;
};

#endif /* !REMCHANNEL_HPP_ */
