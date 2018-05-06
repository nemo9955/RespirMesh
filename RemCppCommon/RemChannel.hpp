#ifndef REMCHANNEL_HPP_
#define REMCHANNEL_HPP_

#include <stdint.h>

typedef void (*ReceiveDataCallback)(uint8_t *data, uint16_t size, void *arg);

struct AbstractChannel
{
    // virtual ~AbstractChannel() = 0;
    virtual void send(uint8_t *data, uint16_t size) = 0;
    virtual void set_recv_cb(ReceiveDataCallback cb, void *arg) = 0;
    virtual int ch_info() = 0;
};

template <class Hardware>
class RemChannel : public AbstractChannel
{

  protected:

  public:
    Hardware channel_;
    RemChannel(){};
    // ~RemChannel(){};

    void init(){channel_.init();};
    void init(char* address, int port){channel_.init(address, port);};
    void update(){channel_.update();};

    void send(uint8_t *data, uint16_t size) { channel_.send(data, size); };
    int ch_info() { return 1; };
    void stop() { channel_.stop(); };

    void recv(uint8_t *data, uint16_t size) { on_recv_cb(data, size,on_recv_arg); };
    void set_recv_cb(ReceiveDataCallback cb, void *arg )
    {
        on_recv_cb = cb;
        on_recv_arg = arg;
    }; //on successful connect

  private:
    ReceiveDataCallback on_recv_cb;
    void *on_recv_arg;

  protected:
};

#endif /* !REMCHANNEL_HPP_ */
