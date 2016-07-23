//
// Created by Thomas Natter on 3/23/16.
//

#ifndef SPITTER_SPITUTILS_H
#define SPITTER_SPITUTILS_H

#include <iostream>
#include "spitter.h"


std::string longToHex(const long&);

void txtLogPeriodDetails(const Summary&);
void txtLogPeriodHeader(const Summary&);
void txtLogPacket(const Packet&);

void screenPrintPeriodDetails(const Summary&);
void screenPrintPeriodJSON(const Summary&);
void screenPrintPeriodHeader(const Summary&);
void screenPrintPacket(const Packet&);

void dbLogSession();
void dbLogPeriod(const Summary&);
void dbLogPacket(const Packet&);

void errorLogPacket(const Packet&);

char* timeStampFromPkt(const Packet&, char*);

#endif //SPITTER_SPITUTILS_H
