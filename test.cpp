//
// Created by Thomas Natter on 3/20/16.
//

#include "spitter.h"
#include "spitutils.h"
#include "config.h"


void packetHandler(const Packet& pkt) {
    if (Config::get().outScrPkts && pkt.crc) screenPrintPacket(pkt);
    if (Config::get().outTxtPkts && pkt.crc) txtLogPacket(pkt);
    if (Config::get().outPgPkts && pkt.crc) dbLogPacket(pkt);
};

void summaryHandler(const Summary& summary) {
    if (Config::get().outScrPeriodHdr) screenPrintPeriodHeader(summary);
    if (Config::get().outScrPeriodDetails) screenPrintPeriodDetails(summary);
    if (Config::get().outTxtPeriods) {
        txtLogPeriodHeader(summary);
        txtLogPeriodDetails(summary);
    }
    if (Config::get().outPgPeriods) dbLogPeriod(summary);
};

int main(int argc, char* argv[]) {
    configHandlers(packetHandler, summaryHandler);
    startSpitting();

}

