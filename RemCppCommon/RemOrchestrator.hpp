#ifndef REMORCHESTRATOR_HPP_
#define REMORCHESTRATOR_HPP_

#include "RemChannel.hpp"
#include "RemHardware.hpp"
#include "RemRouter.hpp"
#include "RemConnectionController.hpp"
#include "RemChannel.hpp"
#include "RemLogger.hpp"
#include "RemHeaderTypes.h"
#include <list>
#include <stdint.h>
#include <stdio.h>

class RemRouter;
class RemLogger;
class RemConnectionController;
class RemHardware;
class RemChannel;

class RemOrchestrator
{
  public:
    RemOrchestrator(){

    };

    ~RemOrchestrator(){

    };

    void set_hardware(RemHardware *);
    void set_router(RemRouter *);
    void set_scanner(RemConnectionController *);
    void set_logger(RemLogger *);
    void begin();
    void stop();
    void update();

    RemHardware *basicHardware;
    RemRouter *remRouter;
    RemLogger *logs;
    RemConnectionController *remScanner;
    std::list<RemChannel *> channels;
    void add_channel(RemChannel *channel);
    void clean_channels();

  private:
    static void receive_fn(uint8_t *data, uint16_t size, void *arg);

  protected:
};

#endif /* !REMORCHESTRATOR_HPP_ */
