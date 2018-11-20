#include "EspAsyncClient.hpp"

uint16_t EspAsyncClient::channel_global_id = 0;

EspAsyncClient::EspAsyncClient()
{
    // parClient = AsyncClient();
    channel_id = channel_global_id++;

    ascl_core.onConnect(onConnect_wrapper, this);
    ascl_core.onDisconnect(cliectDisconnected_wrapper, this);
    ascl_core.onData(dataRecv_wrapper, this);
    ascl_core.setNoDelay(true);
};

EspAsyncClient::~EspAsyncClient(){

};

void EspAsyncClient::init(AsyncClient *client, RemOrchestrator *remOrch_)
{
    remOrch = remOrch_;
    remOrch->logs->info("Local TCP started from SERVER !!! ");
    ascl_core = *client;
};

void EspAsyncClient::init(IPAddress *address, uint16_t port, RemOrchestrator *remOrch_)
{
    remOrch = remOrch_;
    remOrch->logs->info("Local TCP started %u.%u.%u.%u:%d ", (*address)[0], (*address)[1], (*address)[2], (*address)[3], port);

    // if (!ascl_core.connected() && !ascl_core.connecting())
    // {
    ascl_core.connect((*address), port);

    // Serial.print("(*address): ");
    // (*address).printTo(Serial);
    // Serial.printf("\n port %d \n", port);

    // remOrch->basicHardware->sleep_milis(1);
    // }
};

uint16_t EspAsyncClient::send(uint8_t *data, uint16_t size)
{
    //logf("Client TCP sending .... \n");

    // funcf("chn %d send client [size %zu] \t", ch_id(), size);
    // for (uint8_t i = 0; i < size; i++)
    //     funcf("%d ", data[i]);
    // funcf(" \n");
    remOrch->basicHardware->sleep_milis(1);

    uint16_t act_size;
    act_size = ascl_core.write(((const char *)data), size);
    // funcf(" act_size = %u \n", act_size);
    return act_size;
};

bool EspAsyncClient::is_ok()
{
    return ascl_core.connected();
};

void EspAsyncClient::stop()
{
    remOrch->logs->info("Local TCP exiting \n");
};

void EspAsyncClient::onConnect(AsyncClient *client)
{
    remOrch->logs->info("void onConnect(AsyncClient *client){   ");
};
void EspAsyncClient::cliectDisconnected(AsyncClient *client)
{
    remOrch->logs->info("void cliectDisconnected(AsyncClient *client){   ");
};
void EspAsyncClient::dataRecv(AsyncClient *c, void *data, size_t len)
{
    remOrch->logs->info("void dataRecv(AsyncClient *c, void *data, size_t len){   ");
};
