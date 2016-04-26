//
// Created by Thomas Natter on 3/3/16.
//

#ifndef PCAP_CONFIG_H
#define PCAP_CONFIG_H

#include <iostream>
#include <pqxx/pqxx>

class Config{

private:
    Config();

public:
    std::string location;
    std::string device;
    std::string bpf;
    long currentSessionId; // not from config file
    int periodLength;
    long maxPkts;
    std::string dbConnect;
    static Config& get();


};


#endif //PCAP_CONFIG_H
