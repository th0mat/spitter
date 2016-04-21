//
// Created by Thomas Natter on 3/20/16.
//

#include "spitter.h"
#include "spitutils.h"




void packetHandler(const Packet& pkt) {
    if (pkt.crc) screenPrintPacket(pkt);
    //if (pkt.crc) txtLogPacket(pkt);
};


void summaryHandler(const Summary& summary) {
    screenPrintPeriodHeader(summary);
    //txtLogPeriodHeader(summary);
    //wtf();

    //txtLogPeriodDetails(summary);
};



int main(int argc, char* argv[]) {
    configHandlers(packetHandler, summaryHandler);
    startSpitting();

}

