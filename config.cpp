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
    location = pt.get<std::string>("config.location");
    device = pt.get<std::string>("config.device");
    bpf = pt.get<std::string>("config.bpf");
    periodLength = pt.get<int>("config.periodLength");
    maxPkts = pt.get<int>("config.maxPkts");
    dbConnect = postgresConnectString(pt);
    currentSessionId = -1;
}


std::string postgresConnectString(const boost::property_tree::ptree& pt){
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
    static Config config;
    return config;
}

