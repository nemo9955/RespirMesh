#ifndef REMORCHESTRATOR_HPP_
#define REMORCHESTRATOR_HPP_

#include "RemHardware.hpp"
#include "RemRouter.hpp"
#include "RemConnectionScanner.hpp"
#include "RemChannel.hpp"
#include "RemLogger.hpp"
#include "RemHeaderTypes.h"
#include <list>
#include <stdint.h>
#include <stdio.h>

class RemRouter;
class RemLogger;
class RemConnectionScanner;
class RemHardware;

class RemOrchestrator
{
  public:
    RemOrchestrator(){

    };

    ~RemOrchestrator(){

    };

    void set_hardware(RemHardware *);
    void set_router(RemRouter *);
    void set_scanner(RemConnectionScanner *);
    void set_logger(RemLogger *);
    void begin();
    void stop();
    void update();

    RemHardware *basicHardware;
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
