

#include "RemOrchestrator.hpp"

void RemOrchestrator::init()
{
}

void RemOrchestrator::start()
{
}

void RemOrchestrator::update()
{
    remScanner->update();
    remRouter->update();
}

void RemOrchestrator::stop()
{
    log->warn("RemOrchestrator::stop() \n");
    remRouter->stop();

    for (std::list<RemChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        log->warn("STOPPING CHANNEL  %d  \n", (*it)->ch_info());
        (*it)->stop();
        // channels.erase(it);
    }
}

void RemOrchestrator::add_channel(RemChannel *channel)
{
    channel->set_receiver(receive_fn, this);
    log->info("ADDED ch %u  \n", channel->ch_info());
    channels.push_back(std::move(channel));
    // clean_channels();
}

void RemOrchestrator::clean_channels()
{

    for (std::list<RemChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if ((*it)->is_ok() == false)
        {
            log->info("DELETING CHANNEL  %d  \n", (*it)->ch_info());
            (*it)->stop();
            channels.erase(it);
        }
    }

    log->info("CHs size  %d  \n", channels.size());
}

void RemOrchestrator::set_logger(RemLogger *remLogger_)
{
    log = remLogger_;
    log->set_orchestrator(this);
}

void RemOrchestrator::set_hardware(Hardware *hardware_)
{
    basicHardware = hardware_;
}

void RemOrchestrator::set_router(RemRouter *remRouter_)
{
    remRouter = remRouter_;
    remRouter->set_orchestrator(this);
}

void RemOrchestrator::set_scanner(RemConnectionScanner *remScanner_)
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
