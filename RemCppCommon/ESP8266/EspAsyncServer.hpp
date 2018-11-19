#ifndef ESPASYNCSERVER_HPP_
#define ESPASYNCSERVER_HPP_

#include <ESPAsyncTCP.h>
#include <ESP8266WiFi.h>

#include "RemOrchestrator.hpp"
#include "RemChannel.hpp"

class EspAsyncServer : public RemChannel
{
  public:
    EspAsyncServer();
    ~EspAsyncServer();

    uint16_t ch_id();
    void init(int socket, RemOrchestrator *remOrch_);
    void init(IPAddress *address, int port, RemOrchestrator *remOrch_);
    uint16_t send(uint8_t *data, uint16_t size);
    bool is_ok();
    void stop();

  protected:
  private:
    RemOrchestrator *remOrch;
};

#endif /* !ESPASYNCSERVER_HPP_ */
