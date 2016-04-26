//
// Created by Thomas Natter on 3/20/16.
//

#include "spitter.h"
#include "spitutils.h"




void packetHandler(const Packet& pkt) {
    //if (pkt.lengthInclRadioTap < 45) screenPrintPacket(pkt);  // only valid packets
    //if (pkt.crc) screenPrintPacket(pkt);   // all packets incl corrupted
    //if (pkt.crc) txtLogPacket(pkt);
    if (pkt.crc) dbLogPacket(pkt);
};

void summaryHandler(const Summary& summary) {
    //screenPrintPeriodHeader(summary);
    screenPrintPeriodDetails(summary);
    //txtLogPeriodHeader(summary);
    //txtLogPeriodDetails(summary);
    dbLogPeriod(summary);
};

int main(int argc, char* argv[]) {
    configHandlers(packetHandler, summaryHandler);
    startSpitting();

}

