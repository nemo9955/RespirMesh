#include "EspAsyncServer.hpp"

EspAsyncServer::EspAsyncServer()
    : myTCPServer(_SERVER_PORT){
          // myTCPServer = AsyncServer(_SERVER_PORT);
      };

EspAsyncServer::~EspAsyncServer(){

};

uint16_t EspAsyncServer::ch_id() { return 500; };

void EspAsyncServer::init(int socket, RemOrchestrator *remOrch_)
{
    remOrch = remOrch_;
    remOrch->logs->info("   SERVER TCP started in socket:%d \n", socket);
    // myTCPServer.onClient(onClientConnected_wrapper, this);
};

void EspAsyncServer::init(IPAddress *address, int port, RemOrchestrator *remOrch_)
{
    remOrch = remOrch_;
    remOrch->logs->info("   SERVER TCP started %u.%u.%u.%u:%d \n", (*address)[0], (*address)[1], (*address)[2], (*address)[3], port);
};

// void EspAsyncServer::onClientConnected(AsyncClient *client){

// };

uint16_t EspAsyncServer::send(uint8_t *data, uint16_t size)
{
    //logf("Client TCP sending .... \n");
    return 0;
};

bool EspAsyncServer::is_ok()
{
    return 1;
};

void EspAsyncServer::stop()
{
    remOrch->logs->info("Local TCP exiting \n");
};