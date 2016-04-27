

cmake_minimum_required(VERSION 3.4)
project(spitter)

# not required if find_package is used
link_directories(/usr/local/lib/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -lpcap -I/usr/local/include/ -lpqxx -lpq")

set(SOURCE_FILES spitter.cpp spitter.h mac2oui/lookup.cpp mac2oui/lookup.h main.cpp config.h config.cpp spitutils.cpp spitutils.h)
add_executable(spitter ${SOURCE_FILES})

configure_file(mac2oui/oui_download.txt mac2oui/oui_download.txt COPYONLY)
configure_file(mac2oui/mac_config.txt mac2oui/mac_config.txt COPYONLY)
configure_file(config.json config.json COPYONLY)