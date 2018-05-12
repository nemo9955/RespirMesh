#ifndef RESPIRMESH_HPP_
#define RESPIRMESH_HPP_

#include <RemChannel.hpp>
#include "RemHeaderTypes.h"
#include <list>
#include <stdint.h>

#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>
#include "mesh-packet.pb.h"

#include <stdio.h>
#define PRINTF printf
#define infof printf
#define logf printf

#define INTERNAL_BUFFER_ZISE  64

class Hardware
{
public:
    virtual uint32_t device_id() = 0;
    virtual uint32_t time_milis() = 0;
};

class RespirMesh
{
  private:
    std::list<RemChannel *> channels;
    Hardware *hardware_;
    uint8_t pb_buffer[INTERNAL_BUFFER_ZISE];
    int action_counter = 0;
    
  public:
    RespirMesh(Hardware *hardware) {hardware_ = hardware;};
    ~RespirMesh() {};

    static void receive_fn(uint8_t *data, uint16_t size, void *arg);
    void add_channel(RemChannel *channel);
   
    void send(uint8_t *data, uint16_t size);
    void recv(uint8_t *data, uint16_t size);
    void update();

    void handleMeshTopology(uint8_t *data, size_t len);
    void send_mesh_topo_to_server();

    void sendPing();

    void sendPingToNode(RemChannel* c);
    void sendPongToNode(RemChannel* , uint8_t *data, size_t len, RemBasicHeader *header);


    void HandlePong(RemChannel *c, uint8_t *data, size_t len, RemBasicHeader *header);

};

#endif /* !RESPIRMESH_HPP_ */