
#include "SimpleListChnCtrl.hpp"

#include "X86_64/X86LinuxClientChannel.hpp"
#include "X86_64/X86LinuxServerChannel.hpp"

SimpleListChnCtrl::SimpleListChnCtrl()
{
    is_client_connected = false;
    is_server_started = false;
};

SimpleListChnCtrl::~SimpleListChnCtrl(){

};

void SimpleListChnCtrl::set_orchestrator(RemOrchestrator *remOrch_)
{
    logf("  SimpleWiFiScanner : set_orchestrator \n");
    remOrch = remOrch_;
};

void SimpleListChnCtrl::begin(){};

void SimpleListChnCtrl::add_client_host(char *_host, char *_port)
{

    remOrch->logs->info("adding client  %s : %s ", _host, _port);
    clients_list.push_back(std::make_pair(_host, _port));
};

void SimpleListChnCtrl::add_server_host(char *_host, char *_port)
{
    servers_list.push_back(std::make_pair(std::string(_host), std::string(_port)));
};

void SimpleListChnCtrl::update()
{
    if (is_client_connected == false)
    {
        scan_clients();
    }

    if (is_server_started == false)
    {
        start_servers();
    }
};

void SimpleListChnCtrl::start_servers()
{
    remOrch->logs->info(" SimpleListScanner::start_servers servers_list  size  %u  \n", servers_list.size());
    for (auto it = servers_list.begin(); it != servers_list.end(); ++it)
    {
        remOrch->logs->info("\n\n\nTrying to start server %s:%s   \n", it->first.c_str(), it->second.c_str());
        // RemChannel *server_ = instantiate_server();
        X86LinuxServerChannel *server_ = new X86LinuxServerChannel();

        server_->set_conn_listener(this);

        server_->init(
            const_cast<char *>(it->first.c_str()),
            const_cast<char *>(it->second.c_str()),
            remOrch);

        if (server_->is_connected == true)
        {
            remOrch->add_channel(std::move(server_));
            is_server_started = true;
        }
    }
};

void SimpleListChnCtrl::scan_clients()
{
    remOrch->logs->info(" SimpleListScanner::scan_clients  clients_list  size  %u  ", clients_list.size());

    for (auto it = clients_list.begin(); it != clients_list.end(); ++it)
    {
        remOrch->logs->info("\n\n\nTrying to connect to  %s:%s   \n", it->first.c_str(), it->second.c_str());

        // CLIENT *client_ = new CLIENT();
        // RemChannel *client_ = instantiate_client();
        X86LinuxClientChannel *client_ = new X86LinuxClientChannel();

        client_->set_conn_listener(this);

        client_->init(
            const_cast<char *>(it->first.c_str()),
            const_cast<char *>(it->second.c_str()),
            remOrch);

        if (client_->is_connected == true)
        {
            remOrch->add_channel(std::move(client_));
            is_client_connected = true;
        }
    }
};

void SimpleListChnCtrl::on_conn_lost(void *_chan)
{

    remOrch->logs->info("  SimpleListChnCtrl::on_conn_lost() ");
    remOrch->logs->info("  %d : connected_to_root ", ((RemChannel *)_chan)->connected_to_root);
    remOrch->logs->info("  %d : is_client ", ((RemChannel *)_chan)->is_client);
    remOrch->logs->info("  %d : is_server ", ((RemChannel *)_chan)->is_server);
};

void SimpleListChnCtrl::on_conn_start(void *_chan)
{
    remOrch->logs->info("  SimpleListChnCtrl::on_conn_start() ");
    remOrch->logs->info("  %d : connected_to_root ", ((RemChannel *)_chan)->connected_to_root);
    remOrch->logs->info("  %d : is_client ", ((RemChannel *)_chan)->is_client);
    remOrch->logs->info("  %d : is_server ", ((RemChannel *)_chan)->is_server);
};
