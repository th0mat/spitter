//
// Created by Thomas Natter on 5/1/16.
//

#include "gtest/gtest.h"
#include "../../mac2oui/lookup.h"
#include "../../macchina/spitter.h"



/// mac resolution tests

TEST(lookups, mac_to_oui_long){
    EXPECT_EQ(resolveMac(8888888888), "NatureWo_AE38");
}

TEST(lookups, mac_to_oui_hex_known){
    EXPECT_EQ(resolveMac("4c:a5:6d:2e:03:d5"), "NEMA_Sam");
}

TEST(lookups, mac_to_oui_hex_in_range){
    EXPECT_EQ(resolveMac("00:50:C2:37:00:25"), "EuropeTe_0025");
}

TEST(lookups, broadcast){
    EXPECT_TRUE(resolveMac("FFFFFFFFFFFF") == "Broadcast");
    EXPECT_TRUE(resolveMac("ffffffffffff") == "Broadcast");
    EXPECT_TRUE(resolveMac("ff:ff:ff:ff:ff:ff") == "Broadcast");
}




// read pcap file
MacHeader* getTestMac() {
    std::ifstream ifs("10min.pcap", std::ifstream::binary);
    char tmp[1000];
    char* buffer = &tmp[0];
    ifs.read(buffer, 1000); // read the first 1KB
    ifs.close();
    buffer = buffer + 24; // jump over the pcap file header
    buffer = buffer + 16; // jump over the pcap packet header
    buffer = buffer + 25; // jump over the radioTap header
    return reinterpret_cast<MacHeader*>(buffer);
}

/// check endianness in test file

char* getFile() {
    std::ifstream ifs("10min.pcap", std::ifstream::binary);
    char tmp[1000];
    char* buffer = &tmp[0];
    ifs.read(buffer, 1000); // read the first 1KB
    ifs.close();
    return buffer;
}

TEST(file_hdr, endianness){
    auto ptr = getFile();
    u_int* magic = reinterpret_cast<u_int*>(ptr);
    long rescue = *magic; // to avoid overrun
    char buffer[20];
    sprintf(buffer, "%lx", rescue);
    EXPECT_EQ("a1b2c3d4", std::string(buffer));
}


/// packet disection tests

TEST(pkt_header, protocol){
    auto ptr = getTestMac();
    EXPECT_EQ(0, ptr->protocol);
}

TEST(pkt_header, type){
    auto ptr = getTestMac();
    EXPECT_EQ(0, ptr->type);
}

TEST(pkt_header, sub_type){
    auto ptr = getTestMac();

    EXPECT_EQ(5, ptr->subtype);
}

TEST(pkt_header, toFromDs){
    auto ptr = getTestMac();
    EXPECT_EQ(0, ptr->toFromDs);
}

TEST(pkt_header, duration){
    auto ptr = getTestMac();
    u_short duration = ptr->duration;
    short dur = duration;
    EXPECT_EQ(314, dur);
}

// addr1 is special case -> resolves to chromecast

TEST(pkt_header, addr1_known){
    auto ptr = getTestMac();
    std::stringstream hexmac;
    for (int i = 0; i < 6; i++) hexmac << std::hex <<  (int) ptr->addr1[i];
    EXPECT_EQ("INF_ChrCst", resolveMac(hexmac.str()));
}
TEST(pkt_header, addr2_oui){
    auto ptr = getTestMac();
    std::stringstream hexmac;
    for (int i = 0; i < 6; i++) hexmac << std::hex <<  (int) ptr->addr2[i];
    EXPECT_EQ("Humax_6AA0", resolveMac(hexmac.str()));
}

// Todo: understand why I cannot lookup &mac->addr3[0]
TEST(pkt_header, addr3_oui){
    MacHeader* mac = getTestMac();
    u_char tmp[6]{};
    for (int j = 0; j < 6 ; ++j) {
        tmp[j] = mac->addr3[j];
    }
    EXPECT_EQ("Humax_6AA0", resolveMac(tmp));
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}