#ifndef RESPIRMESH_HPP_
#define RESPIRMESH_HPP_

#include <RemChannel.hpp>
#include <list>
#include <stdint.h>

class RespirMesh
{
  public:
    RespirMesh(){};
    ~RespirMesh(){};



    static void recv(uint8_t *data, uint16_t size, void *arg)
    {
        logf("[ REM ] Received DATA \n");
    }; //on successful connect

    void add_channel(AbstractChannel *channel)
    {
        channel->set_recv_cb(recv,(void*)this);
        channels.push_back(channel);
    };

    void send(uint8_t *data, uint16_t size)
    {
        for (std::list<AbstractChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
        {
            (*it)->send(data, size);
        }
    };
    void update(){

    };

  private:
    std::list<AbstractChannel *> channels;

  protected:
};

#endif /* !RESPIRMESH_HPP_ */
