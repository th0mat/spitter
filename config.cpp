//
// Created by Thomas Natter on 3/14/16.
//

#include "config.h"


#include <cstdlib>
#include <iostream>




Config::Config(){

    location = "home";
    device = "en0";
    bpf = "";
    currentSessionId = -1;
    periodLength = 2;
    maxPkts = 0;
    auto pwd = std::getenv("DBPWD");
    if (!pwd) {
        std::cout << "set env variable for postgres pwd";
        exit(1);
    }
    dbConnect = "dbname=test1 host=localhost user=postgres password='" +
            std::string(pwd) +
            "' hostaddr=127.0.0.1 port=5432";


}


Config& Config::get(){
    static Config config;
    return config;
}
