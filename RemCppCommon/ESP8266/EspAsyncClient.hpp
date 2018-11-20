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

    void init(AsyncClient *client, RemOrchestrator *remOrch_);
    void init(IPAddress *address, uint16_t port, RemOrchestrator *remOrch_);
    uint16_t send(uint8_t *data, uint16_t size);
    uint16_t ch_id() { return channel_id; };
    bool is_ok();
    void stop();

    void onConnect(AsyncClient *client);
    void cliectDisconnected(AsyncClient *client);
    void dataRecv(AsyncClient *c, void *data, size_t len);

    static void onConnect_wrapper(void *arg, AsyncClient *client)
    {
        ((EspAsyncClient *)arg)->onConnect(client);
    };
    static void cliectDisconnected_wrapper(void *arg, AsyncClient *client)
    {
        ((EspAsyncClient *)arg)->cliectDisconnected(client);
    };
    static void dataRecv_wrapper(void *arg, AsyncClient *c, void *data, size_t len)
    {
        ((EspAsyncClient *)arg)->dataRecv(c, data, len);
    };

  protected:
  private:
    AsyncClient ascl_core;
    RemOrchestrator *remOrch;
    uint16_t channel_id = 0;
    static uint16_t channel_global_id;
};

#endif /* !ESPASYNCCLIENT_HPP_ */
