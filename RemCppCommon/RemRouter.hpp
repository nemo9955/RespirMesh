#ifndef RemRouter_HPP_
#define RemRouter_HPP_

#include "TaskLooper.hpp"
#include "RemOrchestrator.hpp"
#include "RemHeaderTypes.hpp"

#include <list>
#include <stdint.h>
#include <stdio.h>

#define INTERNAL_BUFFER_ZISE 64

class RemOrchestrator;
// class TaskLooper;

class RemRouter
{
  private:
    TaskLooper mesh_topo_looper;

  protected:
    // Hardware *hardware_;
    RemOrchestrator *remOrch;
    uint8_t pb_buffer[INTERNAL_BUFFER_ZISE];
    int32_t tmili;
    uint8_t nrping;
    uint8_t nrpingSendTopo;


  public:
    RemRouter()
    {
        nrpingSendTopo = 2;
    };
    //   RemRouter(Hardware *hardware) {
    //     hardware_ = hardware;
    //     nrpingSendTopo = 2;
    //   };
    ~RemRouter(){};

    // void set_hardware(Hardware *_hard);
    void set_orchestrator(RemOrchestrator *remOrch_);

    void send_packet(uint8_t *data, uint16_t size);
    void route_packet(uint8_t *data, uint16_t size);
    void process_packet(uint8_t *data, uint16_t size);
    void update();

    void handle_mesh_topo(uint8_t *data, size_t len);
    void send_mesh_topo();

    void handle_ping(uint8_t *data, uint16_t siz);
    // void send_ping(ForwardingType TO);

    void stop();
    void begin();

    //   void handle_pong(uint8_t *data, uint16_t size);
    //   void send_info(RemChannel *rc);
    // void sendPing();

    // void sendPingToNode(RemChannel* c);
    // void sendPongToNode(RemChannel* , uint8_t *data, size_t len, RemBasicHeader
    // *header);

    // void handle_pong(RemChannel *c, uint8_t *data, size_t len, RemBasicHeader
    // *header);
};

#endif /* !RemRouter_HPP_ */