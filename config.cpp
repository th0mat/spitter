//
// Created by Thomas Natter on 3/14/16.
//

#include "config.h"

#include <boost/property_tree/json_parser.hpp>

std::string postgresConnectString(const boost::property_tree::ptree&);


Config::Config() {
    const std::string configFileName = "config.json";
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(configFileName, pt);
    // session
    location = pt.get<std::string>("session.location");
    title = pt.get<std::string>("session.title");
    currentSessionId = -1;
    // sniffer
    device = pt.get<std::string>("sniffer.device");
    bpf = pt.get<std::string>("sniffer.bpf");
    periodLength = pt.get<int>("sniffer.periodLength");
    maxPkts = pt.get<int>("sniffer.maxPkts");
    // output
    outScrPkts = pt.get<bool>("output.screen.packets");
    outScrPeriodHdr = pt.get<bool>("output.screen.period_header");
    outScrPeriodDetails = pt.get<bool>("output.screen.period_details");
    outScrPeriodJSON = pt.get<bool>("output.screen.period_JSON");
    outTxtPkts = pt.get<bool>("output.log_file.packets");
    outTxtPeriods = pt.get<bool>("output.log_file.packets");
    outTxtDir = pt.get<std::string>("output.log_file.dir");
    outPgPkts = pt.get<bool>("output.postgres.packets");
    outPgPeriods = pt.get<bool>("output.postgres.periods");
    // db
    if (outPgPkts || outPgPeriods) dbConnect = postgresConnectString(pt);
    // hopper
    hop = pt.get<bool>("hopper.hop");
    hopsPerSec = pt.get<int>("hopper.hops_per_sec");
    std::stringstream chStream(pt.get<std::string>("hopper.channels"));
    int n;
    while(chStream >> n){
       channels.push_back(n);
    }
    // read from file
    readPcapFile = pt.get<bool>("file.read_pcap_file");
    fileName = pt.get<std::string>("file.file_name");
}


std::string postgresConnectString(const boost::property_tree::ptree& pt) {
    std::string pgc{"dbname="};
    pgc += pt.get<std::string>("db.dbname");
    pgc += " host=";
    pgc += pt.get<std::string>("db.host");
    pgc += " user=";
    pgc += pt.get<std::string>("db.user");
    pgc += " password='";
    auto pwd = std::getenv("DBPWD");
    if (!pwd) {
        std::cout << "set env variable for postgres pwd";
        exit(1);
    }
    pgc += pwd;
    pgc += "'";
    pgc += " hostaddr=";
    pgc += pt.get<std::string>("db.hostaddr");
    pgc += " port=";
    pgc += pt.get<std::string>("db.port");
    return pgc;
}


Config& Config::get() {
    static Config* config = new Config();
    return *config;
}

