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
    Hardware hardware_;

  public:
    RemChannel(){};
    // ~RemChannel(){};

    void init(){hardware_.init();};
    void init(char* address, int port){hardware_.init(address, port);};
    void update(){hardware_.update();};

    void send(uint8_t *data, uint16_t size) { hardware_.send(data, size); };
    int ch_info() { return 1; };
    void stop() { hardware_.stop(); };

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
