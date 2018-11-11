#ifndef REMORCHESTRATOR_HPP_
#define REMORCHESTRATOR_HPP_

#include "RemRouter.hpp"
#include "RemConnectionScanner.hpp"
#include <RemChannel.hpp>
#include <RemLogger.hpp>
#include "RemHeaderTypes.h"
#include <list>
#include <stdint.h>
#include <stdio.h>

class RemRouter;
class RemLogger;
class RemConnectionScanner;

class Hardware
{
  public:
    virtual uint32_t device_id() = 0;
    virtual uint32_t time_milis() = 0;
    virtual uint32_t sleep_milis(uint32_t ) = 0;
};

class RemOrchestrator
{
  public:
    RemOrchestrator(){

    };

    ~RemOrchestrator(){

    };

    void set_hardware(Hardware *);
    void set_router(RemRouter *);
    void set_scanner(RemConnectionScanner *);
    void set_logger(RemLogger *);
    void init();
    void start();
    void stop();
    void update();

    Hardware *basicHardware;
    RemRouter *remRouter;
    RemLogger *logs;
    RemConnectionScanner *remScanner;
    std::list<RemChannel *> channels;
    void add_channel(RemChannel *channel);
    void clean_channels();


  private:
    static void receive_fn(uint8_t *data, uint16_t size, void *arg);

  protected:
};

#endif /* !REMORCHESTRATOR_HPP_ */
