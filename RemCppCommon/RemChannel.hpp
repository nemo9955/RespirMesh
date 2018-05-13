#ifndef REMCHANNEL_HPP_
#define REMCHANNEL_HPP_

#include <stdint.h>

typedef void (*ReceiveDataCallback)(uint8_t *data, uint16_t size, void *arg);

    struct SelfReference{
         void* _this;
         void* argv;
    };
class RemChannel
{
  public:
    RemChannel() { connected_to_root = false; };
    // virtual void init(char *address, int port)=0;
    virtual bool send(uint8_t *data, uint16_t size) = 0;
    virtual void stop() = 0;

    virtual int ch_info() { return 1; };
    virtual void recv(uint8_t *data, uint16_t size) { 
      on_recv_cb(data, size, on_recv_arg); 
      };
    virtual void set_recv_cb(ReceiveDataCallback cb, void *arg)
    {
        argv_recv._this=this;
        argv_recv.argv=arg;
        on_recv_cb = cb;
        on_recv_arg = (void*)&argv_recv;
    };

    bool connected_to_root;

  protected:
    ReceiveDataCallback on_recv_cb;
    void *on_recv_arg;
    SelfReference argv_recv;
};

#endif /* !REMCHANNEL_HPP_ */
