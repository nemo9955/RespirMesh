#ifndef SIMPLELISTCHNCTRL_HPP_
#define SIMPLELISTCHNCTRL_HPP_


#include "RemConnectionController.hpp"
#include "RemOrchestrator.hpp"
#include "RemChannel.hpp"

// typedef RemChannel *(*instantiate_channel)();

class X86LinuxServerChannel ;
class X86LinuxClientChannel ;

class SimpleListChnCtrl : public RemConnectionController
{
  public:
    SimpleListChnCtrl();
    ~SimpleListChnCtrl();

    void update();
    void begin();
    void set_orchestrator(RemOrchestrator *remOrch_);

    void add_client_host(char *_host, char *_port);
    void add_server_host(char *_host, char *_port);
    void start_servers();
    void scan_clients();

    // void set_inst_server(instantiate_channel inst_fun)
    // {
    //     instantiate_server = inst_fun;
    // };
    // void set_inst_client(instantiate_channel inst_fun)
    // {
    //     instantiate_client = inst_fun;
    // };

  private:
    // instantiate_channel instantiate_server;
    // instantiate_channel instantiate_client;

    std::list<std::pair<std::string, std::string>> clients_list;
    std::list<std::pair<std::string, std::string>> servers_list;

    bool is_client_connected;
    bool is_server_started;
    RemOrchestrator *remOrch;
};

#endif /* !SIMPLELISTCHNCTRL_HPP_ */
