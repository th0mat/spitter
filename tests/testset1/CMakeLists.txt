add_executable(runTests check1.cpp ../../mac2oui/lookup.cpp)


configure_file(../../mac2oui/oui_download.txt ../../mac2oui/oui_download.txt COPYONLY)
configure_file(../../mac2oui/mac_config.txt ../../mac2oui/mac_config.txt COPYONLY)
configure_file(10min.pcap 10min.pcap COPYONLY)