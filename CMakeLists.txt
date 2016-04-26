

cmake_minimum_required(VERSION 3.4)
project(spitter)

# not required if find_package is used
link_directories(/usr/local/lib/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -lpcap -I/usr/local/include/ -lpqxx -lpq")

set(SOURCE_FILES spitter.cpp spitter.h oui_lookup/lookup.cpp oui_lookup/lookup.h test.cpp config.h config.cpp spitutils.cpp spitutils.h)
add_executable(spitter ${SOURCE_FILES})

configure_file(oui_lookup/oui_download.txt oui_lookup/oui_download.txt COPYONLY)
configure_file(oui_lookup/mac_config.txt oui_lookup/mac_config.txt COPYONLY)
configure_file(config.json config.json COPYONLY)