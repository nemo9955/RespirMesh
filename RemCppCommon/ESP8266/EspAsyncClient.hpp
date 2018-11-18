#ifndef ESPASYNCCLIENT_HPP_
#define ESPASYNCCLIENT_HPP_

#include <ESPAsyncTCP.h>
#include <ESP8266WiFi.h>

#include "RemOrchestrator.hpp"
#include "RemChannel.hpp"

class EspAsyncClient : public RemChannel
{
  public:
    EspAsyncClient();
    ~EspAsyncClient();

    int ch_info();
    void init(int socket, RemOrchestrator *remOrch_);
    void init(IPAddress *address, int port, RemOrchestrator *remOrch_);
    bool send(uint8_t *data, uint16_t size);
    bool is_ok();
    void stop();

  protected:
  private:
    AsyncClient parClient;
    RemOrchestrator *remOrch;
};

#endif /* !ESPASYNCCLIENT_HPP_ */
