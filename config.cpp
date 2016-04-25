//
// Created by Thomas Natter on 3/14/16.
//

#include "config.h"


#include <boost/property_tree/json_parser.hpp>


boost::property_tree::ptree pt;

Config::Config() {
    const std::string configFileName = "config.json";
    boost::property_tree::json_parser::read_json(configFileName, pt);

    location = pt.get<std::string>("config.location");
    device = pt.get<std::string>("config.device");
    bpf = pt.get<std::string>("config.bpf");
    currentSessionId = -1;
    periodLength = pt.get<int>("config.periodLength");
    maxPkts = pt.get<int>("config.maxPkts");
    dbConnect = pt.get<std::string>("db.dbConnect");
    auto pwd = std::getenv("DBPWD");
    if (!pwd) {
        std::cout << "set env variable for postgres pwd";
        exit(1);
    }
    dbConnect.replace(dbConnect.find("xxxxxxxx"), 8, pwd);
}


Config& Config::get() {
    static Config config;
    return config;
}

