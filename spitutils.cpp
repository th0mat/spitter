//
// Created by Thomas Natter on 3/23/16.
//

#include <fstream>      // std::ofstream
#include <sstream>
#include <iomanip>
#include "spitutils.h"
#include "macmanuf.h"
#include "config.h"
#include <pqxx/pqxx>

using std::chrono::microseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
typedef std::chrono::time_point<system_clock> t_point;



std::string longToHex(const long& mac64) {
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(12) << std::hex << mac64;
    return stream.str();

}

void screenPrintPeriodDetails(const Summary& summary) {
    char timeStamp[100];
    auto t = time(nullptr);
    std::strftime(timeStamp, sizeof(timeStamp), "%Y%m%d %H:%M.%S", std::localtime(&t));
    for (auto ptr = summary.stations.begin(); ptr != summary.stations.end(); ptr++) {
        printf("%s ----  %-20s  KB/s: %8.2f\n",
               timeStamp,
               resolveMac(ptr->first).c_str(),
               ptr->second.bytes / 1024.0 / summary.periodLength
        );
    }
    std::cout << "\n";
}

void screenPrintPeriodHeader(const Summary& summary) {
    char timeStamp[100];
    auto t = time(nullptr);
    std::strftime(timeStamp, sizeof(timeStamp), "%Y%m%d %H:%M.%S", std::localtime(&t));
    printf("%s  %3d secs | %3d sta | val/s %8.2f pkts %8.2f kb | corr/s  %8.2f pkts %8.2f kb | %s\n",
           timeStamp,
           summary.periodLength,
           (int) summary.stations.size(),
           summary.valid.packets * 1.0 / summary.periodLength,
           summary.valid.bytes / 1024.0 / summary.periodLength,
           summary.corrupted.packets * 1.0 / summary.periodLength,
           summary.corrupted.bytes / 1024.0 / summary.periodLength,
           summary.location.c_str()
    );
}

void screenPrintPacket(const Packet& pkt) {
    static int runningNo = 0;
    char tmp[50];
    char* timeStamp = timeStampFromPkt(pkt, tmp);
    int macPktLength = pkt.lengthInclRadioTap - pkt.radioTapHeader.length;
    std::string addr1 = resolveMac(const_cast<u_char*>(pkt.macHeader.addr1));
    std::string addr2{"n/a"};
    if (macPktLength >= 20) { addr2 = resolveMac(const_cast<u_char*>(pkt.macHeader.addr2)); }
    std::string addr3{"n/a"};
    if (macPktLength >= 26) { addr3 = resolveMac(const_cast<u_char*>(pkt.macHeader.addr3)); }
    printf("[%8d] %s | %5d bytes | %1d prot | %1d / %2d | %3d tfDs | %16s | %16s | %16s | \n",
           runningNo,
           timeStamp,
           macPktLength,
           pkt.macHeader.protocol,
           pkt.macHeader.type,
           pkt.macHeader.subtype,
           pkt.macHeader.toFromDs,
           addr1.c_str(),
           addr2.c_str(),
           addr3.c_str()
    );
    runningNo++;
}

void txtLogPeriodDetails(const Summary& summary) {
    std::ofstream ofs{"./manufconfig/period_details_log.txt", std::ofstream::app};
    char timeStamp[100];
    char buffer[100];
    auto t = time(nullptr);
    std::strftime(timeStamp, sizeof(timeStamp), "%Y%m%d %H:%M.%S", std::localtime(&t));
    for (auto ptr = summary.stations.begin(); ptr != summary.stations.end(); ptr++) {
        sprintf(buffer, "%s %4d | %-12s | %s   %-15s | %9.3f \n",
                timeStamp,
                summary.periodLength,
                summary.location.c_str(),
                longToHex(ptr->first).c_str(),
                resolveMac(ptr->first).c_str(),
                ptr->second.bytes / 1024.0 / summary.periodLength
        );
        ofs << buffer;
    }
    ofs.close();
}

void txtLogPeriodHeader(const Summary& summary) {
    std::ofstream ofs ("./period_headers_log.txt", std::ofstream::out | std::ofstream::app);
    if (!ofs.is_open()) {
        std::cout << "*** could not open log_file";
        std::exit(-2);
    }
    char buffer[200];
    char timeStamp[100];
    auto t = time(nullptr);
    std::strftime(timeStamp, sizeof(timeStamp), "%Y%m%d %H:%M.%S", std::localtime(&t));
    sprintf(buffer, "%s  %3d secs | %3d sta | valid: %8.2f pkt/s %8.2f kb/s | corr:  %8.2f pkt/s %8.2f kb/s | %s\n",
            timeStamp,
            summary.periodLength,
            (int) summary.stations.size(),
            summary.valid.packets * 1.0 / summary.periodLength,
            summary.valid.bytes / 1024.0 / summary.periodLength,
            summary.corrupted.packets * 1.0 / summary.periodLength,
            summary.corrupted.bytes / 1024.0 / summary.periodLength,
            summary.location.c_str()
    );
    ofs << buffer;
    ofs.close();
}


void txtLogPacket(const Packet& pkt) {
    std::ofstream ofs{"./manufconfig/packets_log.txt", std::ofstream::app};
    char buffer[100];
    static int runningNo = 0;
    char timeStamp[100];
    auto t = time(nullptr);
    int macPktLength = pkt.lengthInclRadioTap - pkt.radioTapHeader.length;
    std::string addr1 = resolveMac(const_cast<u_char*>(pkt.macHeader.addr1));
    std::string addr2{"n/a"};
    if (macPktLength >= 20) { addr2 = resolveMac(const_cast<u_char*>(pkt.macHeader.addr2)); }
    std::string addr3{"n/a"};
    if (macPktLength >= 26) { addr3 = resolveMac(const_cast<u_char*>(pkt.macHeader.addr3)); }
    std::strftime(timeStamp, sizeof(timeStamp), "%Y%m%d %H:%M.%S", std::localtime(&t));
    sprintf(buffer, "[%8d] %s | %5d bytes | %1d prot | %1d / %2d | %3d tfDs | %16s | %16s | %16s | \n",
           runningNo,
           timeStamp,
           macPktLength,
           pkt.macHeader.protocol,
           pkt.macHeader.type,
           pkt.macHeader.subtype,
           pkt.macHeader.toFromDs,
           addr1.c_str(),
           addr2.c_str(),
           addr3.c_str()
    );
    ofs << "whatever  "; //buffer;
    ofs.close();
    runningNo++;
}

void dbLogSession() {
    pqxx::connection conn(Config::get().dbConnect.c_str());
    pqxx::work work(conn);
    conn.prepare("session", "INSERT INTO sessions (period, name, location, start_time) VALUES "
            "($1, 'default', 'home', localtimestamp) RETURNING session_id;");
    pqxx::result r = work.prepared("session")
                    (Config::get().periodLength)
            .exec();
    pqxx::from_string(r[0][0], Config::get().currentSessionId);
    work.commit();
}

void dbLogPeriod(const Summary& summary) {
    long periodId = -1;
    pqxx::connection conn(Config::get().dbConnect.c_str());
    pqxx::work work(conn);
    conn.prepare("periods",
                 "INSERT INTO periods (session_id, start_time, no_pkts_valid, no_pkts_corr, bytes_valid, bytes_corr, no_stations) "
                         "VALUES ($1, to_timestamp($2), $3, $4, $5, $6, $7) RETURNING period_id;");

    pqxx::result r = work.prepared("periods")
                    (Config::get().currentSessionId)
                    (summary.periodStart)
                    (summary.valid.packets)
                    (summary.corrupted.packets)
                    (summary.valid.bytes)
                    (summary.corrupted.bytes)
                    (summary.stations.size())
            .exec();
    pqxx::from_string(r[0][0], periodId);

    conn.prepare("summaries", "INSERT INTO summaries (period_id, mac_int, mac_res, no_pkts, no_bytes) "
            "VALUES ($1, $2, $3, $4, $5);");

    for (auto ptr = summary.stations.begin(); ptr != summary.stations.end(); ptr++) {
        work.prepared("summaries")
                        (periodId)
                        (ptr->first)
                        (resolveMac(ptr->first))
                        (ptr->second.packets)
                        (ptr->second.bytes)
                .exec();
    }

    work.commit();
}

char* timeStampFromPkt(const Packet& pkt, char* timeStamp){
    t_point pkt_t_point = t_point();
    pkt_t_point = pkt_t_point + microseconds(pkt.timeStampMicroSecs);
    time_t tt = system_clock::to_time_t(pkt_t_point);
    tm* ptm = localtime(&tt);
    strftime(timeStamp, 50, "%H:%M:%S", ptm);
    int micros = pkt.timeStampMicroSecs % 1000000;
    std::string microStr = std::to_string(micros);
    const int lengthMicro = microStr.length();
    for (int i = 0; i < 6 - lengthMicro; ++i) {
        microStr = '0' + microStr;
    }
    strcat(timeStamp, ".");
    strcat(timeStamp, microStr.c_str());
    return timeStamp;

}