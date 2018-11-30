#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>

#include <signal.h>

#include "mesh-packet.pb.h"
#include "RemHeaderTypes.hpp"
#include "RemChannel.hpp"
#include "RemOrchestrator.hpp"
#include "RemRouter.hpp"
#include "RemConnectionController.hpp"
#include "RemLogger.hpp"
#include "TaskLooper.hpp"
#include "RemHardware.hpp"

#include "X86_64/X86LinuxClientChannel.hpp"
#include "X86_64/X86LinuxServerChannel.hpp"
#include "X86_64/SimpleListChnCtrl.hpp"
#include "X86_64/X86LinuxHardware.hpp"

uint32_t chipID = 0;

void sig_exit(int s);

SimpleListChnCtrl parentScanner;

X86LinuxHardware hardware_;
RemRouter remRouter;
RemOrchestrator remOrch;
RemLogger logs;

TaskLooper update_looper;

void sig_exit(int s)
{
    remOrch.stop();
    exit(0);
}

int main(int argc, char *argv[])
{

    remOrch.set_router(&remRouter);
    remOrch.set_scanner(&parentScanner);
    remOrch.set_hardware(&hardware_);
    remOrch.set_logger(&logs);

    update_looper.begin(remOrch.basicHardware);
    update_looper.set(1 * 1000);

    // for (size_t i = 0; i < argc; i++)
    // {
    //     printf(" %d %s \n", i, argv[i]);
    // }
    // return 0;

    logs.info("_ STARTING !!!!!!!!!!!!!!!!!!!!! _");

    if (argc < 6)
    {
        printf("First 2 arguments specify host and port of the server \n");
        printf("3th arg, the ID of the device, random if 0 \n");
        printf("Next sets of 2 arguments specify host and port of client \n");
        exit(1);
    }

    srand(time(NULL));

    if (atoi(argv[3]) != 0)
        chipID = atoi(argv[3]);
    else
        chipID = rand() % 640000;

    hardware_.chip_id = chipID;

    char *server_host = const_cast<char *>(argv[1]);
    char *server_port = const_cast<char *>(argv[2]);
    parentScanner.add_server_host(server_host, server_port);

    for (size_t i = 4; i < argc; i += 2)
    {
        char *client_host = const_cast<char *>(argv[i]);
        char *client_port = const_cast<char *>(argv[i + 1]);
        parentScanner.add_client_host(client_host, client_port);
    }

    // parentScanner.set_inst_server(X86LinuxServerChannel::instantiate);
    // parentScanner.set_inst_client(X86LinuxClientChannel::instantiate);
    signal(SIGINT, sig_exit);

    logf("chipID %d %x \n", chipID, chipID);
    logf("Time : %u  \n", remOrch.basicHardware->time_milis());
    logf("devID: %d == 0x%x \n", remOrch.basicHardware->device_id(), remOrch.basicHardware->device_id());

    remOrch.begin();
    while (1)
    {
        if (update_looper.check())
        {
            remOrch.update();
        }
        sleep(0.5);
        // logs.trace("main loop update");
    }
    // sleep(1);
    // remOrch.update();

    remOrch.stop();
    return 0;
}
