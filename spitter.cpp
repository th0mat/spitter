//
// Created by Thomas Natter on 3/20/16.
//

#include <iostream>
#include <vector>
#include <pcap/pcap.h>
#include "spitter.h"
#include "config.h"
#include "spitutils.h"



//SpitterConfig config{"en0", "", 5, 0, 100};

bool crc32(const pcap_pkthdr *header, const u_char *packet);
short getRadioTapLength(const u_char *packet);
uint32_t crc32buf(char *buf, size_t len);
void addToSummary(const Packet& pkt);
void checkPeriod();


void (*pktHandler)(const Packet&);
void (*summaryHandler)(const Summary&);
std::unique_ptr<Summary> current(new Summary{time(nullptr), Config::get().periodLength});


void configHandlers(void (*p)(const Packet&), void (*s)(const Summary&)){
    pktHandler = p;
    summaryHandler = s;
}

namespace rt {
    const int MIN_MAC_LENGTH{14};  // CTS and ACK
    const int MIN_RADIOTAP_LENGTH{15};
    const int MAX_RADIOTAP_LENGTH{50};
}




Summary::Summary(long start, int duration) : periodStart{start}, periodLength{duration} {
    location = Config::get().location;
}

StaData::StaData() : packets{0}, bytes{0} {};

/* loop callback function - set in pcap_loop() */
void rawHandler(u_char *args, const pcap_pkthdr *header, const u_char *packet) {
    bool crc = crc32(header, packet);
    long timeStampMicroSecs = header->ts.tv_sec*1000000 + header->ts.tv_usec;
    int lengthInclRadioTap = header->len;

    const MacHeader* mac = reinterpret_cast<const MacHeader*>(packet +
                                             getRadioTapLength(packet));
    const RadioTapHeader* radio = reinterpret_cast<const RadioTapHeader*>(packet);
    Packet pkt{crc, timeStampMicroSecs, lengthInclRadioTap, *radio, *mac};

    checkPeriod();
    addToSummary(pkt);
    pktHandler(pkt);
}


int startSpitting() {
    pcap_t *handle;                        // session handle
    char errbuf[PCAP_ERRBUF_SIZE];         // buff for error string
    struct bpf_program fp;                 // compiled filter
    //bpf_u_int32 mask;					   // netmask mask - not set
    bpf_u_int32 net = 0;                   // our IP - needed only as arg
    /* pcap_create allows setting params before activation - pacap_open_golive
    combines those two steps -> setting monitor mode not possible */
    //SpitterConfig config;
    handle = pcap_create(Config::get().device.c_str(), errbuf);
    if (handle == NULL) {
        printf("pcap_create failed: %s\n", errbuf);
        return (2);
    }

    /* set monitor monde */
    if (pcap_set_rfmon(handle, 1) == 0) {
    }
    pcap_set_snaplen(handle, -1);          // -1: full pkt
    pcap_set_timeout(handle, 500);         // millisec
    pcap_activate(handle);
    /* check for link layer */
    if (pcap_datalink(handle) != 127) {
        printf("device %s doesn't exist or provide RadioTap headers - try 'en0'\n", Config::get().device.c_str());
        return (2);
    }
    /* compile and apply BPF */
    if (pcap_compile(handle, &fp, Config::get().bpf.c_str(), 0, net) == -1) {
        printf("failed to parse filter %s: %s\n", Config::get().bpf.c_str(), pcap_geterr(handle));
        return (2);
    }
    if (pcap_setfilter(handle, &fp) == -1) {
        printf("failed to install filter %s: %s\n", Config::get().bpf.c_str(), pcap_geterr(handle));
        return (2);
    }
    // Todo: move to sessions or config
    dbLogSession();
    pcap_loop(handle, Config::get().maxPkts, rawHandler, nullptr);        // -1: no pkt number limit
    pcap_close(handle);
    return (0);
}

bool crc32(const pcap_pkthdr *header, const u_char *packet) {
    if (header->caplen < 39) return false;        // min 14 for pkt and 25 for radiotap
    u_short radio = getRadioTapLength(packet);
    if (radio > rt::MAX_RADIOTAP_LENGTH || radio < rt::MIN_RADIOTAP_LENGTH) return false;
    if ((header->caplen - radio) < rt::MIN_MAC_LENGTH) return false;
    u_char *tmp = const_cast<u_char *>(packet);  // crc32buf expects non-const
    char *tmp2 = reinterpret_cast<char *>(tmp);  // can't go from u_char* directly to *int
    // and need unsigned int (= return val
    // of crc32buf)
    int *present = reinterpret_cast<int *>(tmp2 + header->caplen - 4);
    int calculated = crc32buf(tmp2 + radio, header->caplen - radio - 4);
    return calculated == *present;
}

short getRadioTapLength(const u_char *packet) {
    const RadioTapHeader *rth = reinterpret_cast<const RadioTapHeader *>(packet);
    const u_short result = rth->length;
    return result;
}

void checkPeriod() {
    static time_t tmp = current->periodStart;
    time_t nowTime = time(nullptr);
    if ((nowTime - current->periodStart) > Config::get().periodLength) {
        summaryHandler(*current);
        // Todo: period jump defense
        tmp = tmp + Config::get().periodLength;
        current = std::unique_ptr<Summary>(new Summary{tmp, Config::get().periodLength});
    }}


namespace PktTypes {
    // if type == 00
    //      subtype  0: addr2   // Assoc Request
    //      subtype  1: addr1   // Assoc Resp
    //      subtype  2: addr2   // Reassoc Request
    //      subtype  3: addr1   // Reassoc Response
    //      subtype  4: addr2   // Probe Request
    //      subtype  5: addr1   // Probe Response
    //      subtype  6: ??
    //      subtype  7: ??
    //      subtype  8: addr1   // Beacon
    //      subtype  9: addr1   // ATIM
    //      subtype 10: addr2   // Disassociation
    //      subtype 11: addr2   // Authentication
    //      subtype 12: addr2   // Deauthentication
    //      subtype 13: addr2   // Action??
    //      subtype 14: -       // reserved
    //      subtype 15: -       // reserved  // obsered in the wild with passed crc // at starbucks
    const std::vector<int> t0{2, 1, 2, 1, 2, 1, 0, 0, 1, 1, 2, 2, 2, 2, 0, 0};

    // if type == 01
    // length of CTS and ACK is 14, all others 20 bytes
    //      subtype  0: -         // reserved
    //      subtype  1: -         // reserved
    //      subtype  2: -         // reserved
    //      subtype  3: -         // reserved
    //      subtype  4: -         // reserved
    //      subtype  5: -         // reserved
    //      subtype  6: -         // reserved
    //      subtype  7: -         // reserved
    //      subtype  8: addr1/2   // Block ACK request
    //      subtype  9: addr1/2   // Block ACK
    //      subtype 10: addr1/2   // PS-Poll
    //      subtype 11: addr1/2   // RTS
    //      subtype 12: addr1/2   // CTS
    //      subtype 13: addr1/2   // ACK
    //      subtype 14: addr1/2   // CF End
    //      subttpe 15: addr1/2   // CF End + ACK
    // const std::vector<int> t1{0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 2, 1, 1, 0, 0};

    // if type == 02
    // toFromDs table
    //      toFromDs 0:  addr0
    //      toFromDs 1:  addr1
    //      toFromDs 2:  addr2
    //      toFromDs 3:  addr0
    const std::vector<int> t2{0, 2, 1, 0};
};


long addressToLong(const u_char *p) {
    u_char tmp[8]{};
    // leave tmp[6] and tmp[7] = 0;
    for (int i = 0; i < 6; i++) {
        tmp[5 - i] = p[i];
    }
    return *(reinterpret_cast<long*>(&tmp));
}

long getStaAddr(const Packet& pkt) {
    int no = 0;
    if (pkt.macHeader.type == 0) { no = PktTypes::t0[pkt.macHeader.subtype]; }
    // if (pkt.macHeader.type == 1) { no = PktTypes::t1[pkt.macHeader.subtype]; } // excluded already
    if (pkt.macHeader.type == 2) { no = PktTypes::t2[pkt.macHeader.toFromDs]; }
    // Todo: undefined return
    if (no == 0) {
        std::cout << "StaAddr fail: " << pkt.macHeader.type << "/" << pkt.macHeader.subtype << std::endl;
                return 666;

        };   // for "undefined";
    if (no == 1) return addressToLong(pkt.macHeader.addr1);
    if (no == 2) return addressToLong(pkt.macHeader.addr2);
    std::cout << "StaAddr fail: " << pkt.macHeader.type << "/" << pkt.macHeader.subtype << std::endl;
    return 666;
}



void addToSummary(const Packet& pkt) {
    // if corrupted, add to corrupted
    if (!pkt.crc) {
        current->corrupted.bytes += pkt.lengthInclRadioTap;
        current->corrupted.packets += 1;
        return;
    }
    current->valid.bytes += pkt.lengthInclRadioTap;
    current->valid.packets += 1;
    if (pkt.macHeader.type == 1) return; // exclude control frames from STA identification
    if (pkt.macHeader.type == 2 && pkt.macHeader.toFromDs == 0) return; // exclude data frames not to/from STA
    if (pkt.macHeader.type == 2 && pkt.macHeader.toFromDs == 3) return; // exclude data frames not to/from STA
    long sta = getStaAddr(pkt);
    auto ptr = current->stations.find(sta);
    if (ptr != current->stations.end()){
        ptr->second.packets += 1;
        ptr->second.bytes += pkt.lengthInclRadioTap;
        return;
    }
    StaData data;
    data.bytes = pkt.lengthInclRadioTap;
    data.packets = 1;
    current->stations[sta] = data;
    return;
};



/* Below: copyright (C) 1986 Gary S. Brown:  "You may use this program, or
   code or tables extracted from it, as desired without restriction." */

#define UPDC32(octet, crc) (crc_32_tab[((crc)\
     ^ ((uint8_t)octet)) & 0xff] ^ ((crc) >> 8))

static uint32_t crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
        0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
        0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
        0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
        0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
        0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
        0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
        0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
        0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
        0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
        0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
        0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
        0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
        0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
        0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
        0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
        0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t crc32buf(char *buf, size_t len) {
    uint32_t oldcrc32;
    oldcrc32 = 0xFFFFFFFF;
    for ( ; len; --len, ++buf) {
        oldcrc32 = UPDC32(*buf, oldcrc32);
    }
    return ~oldcrc32;
}
