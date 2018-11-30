#ifndef REMCONNECTIONCONTROLLER_HPP_
#define REMCONNECTIONCONTROLLER_HPP_

#include "RemOrchestrator.hpp"
#include "RemHeaderTypes.hpp"

class RemOrchestrator;

class RemConnectionController
{
  public:
    // virtual void add_client_host(char *_host, char *_port) = 0;
    // virtual void scan_clients() = 0;
    virtual void update() = 0;
    virtual void begin() = 0;

    virtual void set_orchestrator(RemOrchestrator *remOrch_) = 0;
    // {
    //     logf("  RemConnectionController : set_orchestrator \n");
    //     remOrch = remOrch_;
    // };

  private:
  protected:
    // RemOrchestrator *remOrch;
};

#endif /* !REMCONNECTIONCONTROLLER_HPP_ */
