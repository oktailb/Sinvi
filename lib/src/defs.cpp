#include "defs.h"
#include "pCapUtils.h"
#include "variables.h"
#include <iostream>
#include <thread>
#include <unistd.h>

#ifdef WIN32

HINSTANCE   hInstance;
HINSTANCE   hPrevInstance;
int         nShowCmd;

int __stdcall WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, char*, int _nShowCmd)
{
    hInstance = _hInstance;
    hPrevInstance = _hPrevInstance;
    nShowCmd = _nShowCmd;

    return main(__argc, __argv);
}
#endif

void usage(int argc, char **argv)
{
    if (argc == 1)
    {
        std::cerr << argv[0] << " config_file.ini" << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::string version()
{
    return (VERSION);
}

ThreadSafeQueue srvQueue;

void init(std::map<std::string, std::string> &configuration, ThreadSafeQueue &queue)
{

    std::string mode = configuration["Input/mode"];
    bool relay = configuration["Input/relayPcap"].compare("true") == 0?true:false;
    bool otherClient = configuration["Input/otherClient"].compare("true") == 0?true:false;
    std::string pcap_file = configuration["Input/file"];
    std::string pcap_device = configuration["Input/device"];
    std::string filter_expression = configuration["Input/filter"];
    std::string address = configuration["Input/address"];

    if (mode.compare("file") == 0)
    {
        if (relay)
        {
            std::vector<std::string> tokens = split(configuration.find("Input/address")->second, ':');
            read_pcap_file(pcap_file, filter_expression, srvQueue);
            std::thread server_thread(sendPcapTo, tokens[0], std::stoi(tokens[1]), std::ref(srvQueue));
            if (!otherClient)
            {
                server_thread.detach();
                mode = "socket";
            }
            else
                server_thread.join();
        }
        else
            read_pcap_file(pcap_file, filter_expression, queue);
    }
    if (mode.compare("spy") == 0)
    {
        std::thread reader_thread(read_device, pcap_device, filter_expression, std::ref(queue));
        reader_thread.detach();
    }
    if (mode.compare("socket") == 0)
    {
        std::vector<std::string> tokens = split(configuration.find("Input/address")->second, ':');
        std::thread reader_thread(read_socket, tokens[0], std::stoi(tokens[1]), std::ref(queue));
        reader_thread.detach();
    }
}
