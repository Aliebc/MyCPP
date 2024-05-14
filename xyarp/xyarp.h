#pragma once
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <map>
#include <string>

#include "cmdline.h"

#include <pcap.h>
#include <libnet.h>
#include <netinet/ip.h>
#include <net/if_arp.h>

#include <sys/utsname.h>

#include <net/if_arp.h>

#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <arpa/inet.h>
#include <net/if.h>



#define eprintf(...) fprintf(stderr, "%s:%d:%s - ",__FILE__, __LINE__ , __func__);fprintf(stderr, __VA_ARGS__)

#define XYNET_VERSION "1.0.1"

using std::map;
using std::multimap;

namespace XYTOOLS{
    
    inline std::string mac2str(u_int8_t * mac){
        auto mac_c = (unsigned char *)mac;
        char mac_str[18];
        snprintf(mac_str, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac_c[0], mac_c[1], mac_c[2], mac_c[3], mac_c[4], mac_c[5]);
        return std::string(mac_str);
    }

    inline int cidr2int(const char * cidr, uint32_t * ip, uint32_t * mask){
        char ip_str[16];
        char mask_str[16];
        sscanf(cidr, "%[^/]/%s", ip_str, mask_str);
        struct in_addr ip_addr;
        struct in_addr mask_addr;
        inet_aton(ip_str, &ip_addr);
        int prefix = atoi(mask_str);
        *ip = ip_addr.s_addr;
        *mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
        return 0;
    }

    inline int ip2int(const char * ip, uint32_t * ip_int){
        struct in_addr ip_addr;
        auto i = inet_aton(ip, &ip_addr);
        if(i == 0){
            return -1;
        }
        *ip_int = ip_addr.s_addr;
        *ip_int = ntohl(*ip_int);
        return 0;
    }
    
    inline bool check_cidr(const char * ip, const char * cidr){
        uint32_t ip_int;
        uint32_t mask_int;
        uint32_t cidr_int;
        ip2int(ip, &ip_int);
        cidr2int(cidr, &cidr_int, &mask_int);
        auto ipAmask = htonl(ip_int & mask_int);
        return (ipAmask) == cidr_int;
    }

    inline bool ip_check_private(const char * ip){
        std::vector<std::string> private_ips = {
            "10.0.0.0/8",
            "172.16.0.0/12",
            "192.168.0.0/16",
        };
        for(auto &i : private_ips){
            if(check_cidr(ip, i.c_str())){
                return true;
            }
        }
        return false;
    }

    inline int mac2str(char * dst, u_int8_t * mac){
        auto mac_c = (unsigned char *)mac;
        return snprintf(dst, 18, "%02x:%02x:%02x:%02x:%02x:%02x", 
            mac_c[0], mac_c[1], mac_c[2], mac_c[3], mac_c[4], mac_c[5]
        );
    }
    
    inline int str2mac(const char *mac_str, u_int8_t * mac) {
        return sscanf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", 
            (unsigned int*)(mac), 
            (unsigned int*)(mac + 1), 
            (unsigned int*)(mac + 2), 
            (unsigned int*)(mac + 3), 
            (unsigned int*)(mac + 4), 
            (unsigned int*)(mac + 5)
        );
    }
}

class NetworkDevices
{
private:
    struct dev {
        u_int8_t mac[6];
        uint32_t ipv4;
        std::string name;
    };
    map<std::string, dev> devices;
public:
    NetworkDevices() = default;
    struct dev &operator[](const std::string & name){
        return devices[name];
    }
    const struct dev &operator[](const std::string & name) const {
        return devices.at(name);
    }
    map<std::string, dev>::iterator begin(){
        return devices.begin();
    }
    map<std::string, dev>::iterator end(){
        return devices.end();
    }
    //using iterator = map<std::string, dev>::iterator;
    auto size(){
        return devices.size();
    }
    auto find(const std::string & name){
        return devices.find(name);
    }
};

class _init 
{
    typedef map<std::string, pcap_if_t *> devmap;
    devmap devices;
    cmdline::parser * a;
    std::string system;
    NetworkDevices networkDevices;
    public:
    _init();
    ~_init();
    const devmap &getDevices() const { return devices; }
    cmdline::parser &getParser() { return *a; }
    const std::string &getSystem() const { return system; }
    NetworkDevices &getNetworkDevices() { return networkDevices; }
};

extern _init init;

extern "C"{

const char * xynet_version();

int send_arp
(pcap_if_t * ito, struct in_addr * ip, struct mac_addr_t * mac);

int xyarp_request_arp
(pcap_if_t * in, struct in_addr * req_ip, struct in_addr * src_ip, u_int8_t * src_mac);

int xyarp_reply_arp
(pcap_if_t * in, struct in_addr * dst_ip, struct in_addr * src_ip, u_int8_t * src_mac, u_int8_t * dst_mac);

int xyarp_scan_interface
(pcap_if_t * in);

int xyarp_broadcast_arp
(pcap_if_t * in);

int xyarp_listen_arp
(pcap_if_t * in, _init * init);

int xyarp_local_mac
(const char *eth_inf, u_int8_t * mac);

int xyarp_disguise_arp
(pcap_if_t * in, const char * victim_ip, const char * gateway_ip, _init * init, const char * victim_mac = NULL);

}