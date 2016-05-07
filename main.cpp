//
// Created by Thomas Natter on 3/20/16.
//
// Todo: hopper implementation

#include "macchina/spitter.h"
#include "macchina/spitutils.h"
#include "config.h"
#include <csignal>

// output configuration

void packetHandler(const Packet& pkt) {
    if (Config::get().outScrPkts) screenPrintPacket(pkt);
    if (Config::get().outTxtPkts && pkt.crc) txtLogPacket(pkt);
    if (Config::get().outPgPkts) dbLogPacket(pkt);
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


// signal handling
// Todo: clean-up functions
void signalHandler( int signum ) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    exit(signum);
}


// program start
int main(int argc, char* argv[]) {
    std::signal(SIGINT, signalHandler);
    configHandlers(packetHandler, summaryHandler);
    startSpitting();
}

