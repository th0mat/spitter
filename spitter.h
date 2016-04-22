//
// Created by Thomas Natter on 3/19/16.
//

#ifndef SPITTER_SPITTER_H
#define SPITTER_SPITTER_H


#include <iostream>
#include <vector>
#include <pcap/pcap.h>
#include <map>



void rawHandler(u_char *args, const pcap_pkthdr *header, const u_char *packet);


struct StaData {
    int packets;
    int bytes;
    StaData();
};

struct Summary {

    long periodStart;
    int periodLength;
    Summary(long, int, std::chrono::time_point<std::chrono::system_clock>);
    StaData corrupted;
    StaData valid;  // data and management frames
    std::string location;
    std::chrono::time_point<std::chrono::system_clock> periodEnd;
    //~Summary() {std::cout << "\nobj destroyed: " << this->periodStart << std::endl;}
    std::map<long, StaData> stations;
};


struct RadioTapHeader {
    u_char version;    // set to 0
    u_char pad;
    u_short length;    // entire length
    u_int present;     // fields present
};

struct MacHeader {
    unsigned protocol:2;
    unsigned type:2;
    unsigned subtype:4;
    unsigned toFromDs:2;
    unsigned frags:1;
    unsigned retry:1;
    unsigned pwrMgt:1;
    unsigned moreData:1;
    unsigned wep:1;
    unsigned order:1;
    u_short duration;
    u_char addr1[6];
    u_char addr2[6];
    u_char addr3[6];
    u_char sc[2];
};

// Todo: add timestamp from pcap header
struct Packet {
    bool crc;
    long timeStampMicroSecs;  // in micorsec unix time
    int lengthInclRadioTap;
    RadioTapHeader radioTapHeader;
    MacHeader macHeader;
};


void configHandlers(void (*pktHandler)(const Packet&), void (*summaryHandler)(const Summary&));
int startSpitting();



#endif //SPITTER_SPITTER_H
