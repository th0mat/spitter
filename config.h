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
    // session
    std::string location;
    std::string title;

    // sniffer
    std::string device;
    std::string bpf;
    long currentSessionId; // not from config file
    int periodLength;
    long maxPkts;

    // db
    std::string dbConnect;

    // output
    bool outScrPkts;
    bool outScrPeriodHdr;
    bool outScrPeriodDetails;
    bool outScrPeriodJSON;
    bool outTxtPkts;
    bool outTxtPeriods;
    std::string outTxtDir;
    bool outPgPkts;
    bool outPgPeriods;

    // hopper
    bool hop;
    int hopsPerSec;
    std::vector<int> channels;

    // read from file
    bool  readPcapFile;
    std::string fileName;

    static Config& get();


};


#endif //PCAP_CONFIG_H
