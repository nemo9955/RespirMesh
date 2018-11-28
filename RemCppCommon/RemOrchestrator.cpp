

#include "RemOrchestrator.hpp"

void RemOrchestrator::begin()
{
    remScanner->begin();
    remRouter->begin();
}

void RemOrchestrator::update()
{
    remScanner->update();
    remRouter->update();
}

void RemOrchestrator::stop()
{
    logs->warn("RemOrchestrator::stop() \n");
    remRouter->stop();

    for (std::list<RemChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        printf(" -------------   %d    %u  \n", (*it)->ch_id(), (*it)->ch_id());
        logs->warn("STOPPING CHANNEL  %u  \n", (*it)->ch_id());
        (*it)->stop();
        // channels.erase(it);
    }
}

void RemOrchestrator::add_channel(RemChannel *channel)
{
    channel->set_receiver(receive_fn, this);

    printf(" -------------   %d    %u  \n", channel->ch_id(), channel->ch_id());

    logs->info("ADDED ch %d  \n", channel->ch_id());
    channels.push_back(std::move(channel));
    // clean_channels();
}

void RemOrchestrator::clean_channels()
{

    for (std::list<RemChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if ((*it)->is_ok() == false)
        {
            logs->info("DELETING CHANNEL  %d  \n", (*it)->ch_id());
            (*it)->stop();
            channels.erase(it);
        }
    }

    logs->info("CHs size  %d  \n", channels.size());
}

void RemOrchestrator::set_logger(RemLogger *remLogger_)
{
    logs = remLogger_;
    logs->set_orchestrator(this);
}

void RemOrchestrator::set_hardware(RemHardware *hardware_)
{
    basicHardware = hardware_;
}

void RemOrchestrator::set_router(RemRouter *remRouter_)
{
    remRouter = remRouter_;
    remRouter->set_orchestrator(this);
}

void RemOrchestrator::set_scanner(RemConnectionController *remScanner_)
{
    remScanner = remScanner_;
    remScanner->set_orchestrator(this);
}

void RemOrchestrator::receive_fn(uint8_t *data, uint16_t size, void *arg)
{

    // funcf("data  receive_fn :      \t");
    // for (uint8_t i = 0; i < size; i++)
    //     funcf("%d ", data[i]);
    // funcf("\n");

    // RemRouter *rm = (RemRouter *)arg;
    RemOrchestrator *rorch = (RemOrchestrator *)arg;
    rorch->remRouter->route_packet(data, size);
}
