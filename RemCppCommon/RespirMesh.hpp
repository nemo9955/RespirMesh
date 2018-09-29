#ifndef RESPIRMESH_HPP_
#define RESPIRMESH_HPP_

#include "RemHeaderTypes.h"
#include <RemChannel.hpp>
#include <list>
#include <stdint.h>

#include "mesh-packet.pb.h"
#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include <stdio.h>

#define errorf printf
#define debugf //printf
#define infof printf
#define logf printf
#define funcf printf

#define INTERNAL_BUFFER_ZISE 64

class Hardware
{
  public:
    virtual uint32_t device_id() = 0;
    virtual uint32_t time_milis() = 0;
};

class RespirMesh
{
  private:
    struct RemChannelAndId
    {
        RemChannel *rc;
        uint32_t id;
    };
    std::list<RemChannelAndId *> MapedChannels;
    std::list<RemChannel *> channels;
    Hardware *hardware_;
    uint8_t pb_buffer[INTERNAL_BUFFER_ZISE];
    int action_counter = 0;
    int32_t tmili;
    uint8_t nrping;
    uint8_t nrpingSendTopo;

  public:
    RespirMesh()
    {
        nrpingSendTopo = 2;
    };
    //   RespirMesh(Hardware *hardware) {
    //     hardware_ = hardware;
    //     nrpingSendTopo = 2;
    //   };
    ~RespirMesh(){};

    void set_hardware(Hardware *_hard);
    uint32_t time_milis();
    uint32_t device_id();

    static void receive_fn(uint8_t *data, uint16_t size, void *arg);
    void add_channel(RemChannel *channel);
    void clean_channels();

    void send_packet(uint8_t *data, uint16_t size);
    void route_packet(uint8_t *data, uint16_t size);
    void process_packet(uint8_t *data, uint16_t size);
    void update();

    void handle_mesh_topo(uint8_t *data, size_t len);
    void send_mesh_topo();

    void handle_ping(uint8_t *data, uint16_t siz );
    void send_ping(ForwardingType TO);

    void stop();

    //   void handle_pong(uint8_t *data, uint16_t size);
    //   void send_info(RemChannel *rc);
    // void sendPing();

    // void sendPingToNode(RemChannel* c);
    // void sendPongToNode(RemChannel* , uint8_t *data, size_t len, RemBasicHeader
    // *header);

    // void handle_pong(RemChannel *c, uint8_t *data, size_t len, RemBasicHeader
    // *header);
};

#endif /* !RESPIRMESH_HPP_ */