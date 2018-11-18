#include "EspAsyncClient.hpp"

EspAsyncClient::EspAsyncClient()
{
    // parClient = AsyncClient();
};

EspAsyncClient::~EspAsyncClient(){

};

int EspAsyncClient::ch_info() { return 500; };

void EspAsyncClient::init(int socket, RemOrchestrator *remOrch_)
{
    remOrch = remOrch_;
    remOrch->logs->info("Local TCP started in socket:%d \n", socket);
};

void EspAsyncClient::init(IPAddress *address, int port, RemOrchestrator *remOrch_)
{
    remOrch = remOrch_;
    remOrch->logs->info("Local TCP started %u.%u.%u.%u:%d \n", (*address)[0], (*address)[1], (*address)[2], (*address)[3], port);

    // if (!parClient.connected() && !parClient.connecting())
    // {
    parClient.connect((*address), port);

    Serial.print("(*address): ");
    (*address).printTo(Serial);
    Serial.printf("\n port %d \n", port);

    remOrch->basicHardware->sleep_milis(1);
    // }
};

bool EspAsyncClient::send(uint8_t *data, uint16_t size)
{
    //logf("Client TCP sending .... \n");
    return 0;
};

bool EspAsyncClient::is_ok()
{
    return 1;
};

void EspAsyncClient::stop()
{
    remOrch->logs->info("Local TCP exiting \n");
};