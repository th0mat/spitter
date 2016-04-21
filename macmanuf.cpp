// MacLookup v0.1

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <map>
#include <iomanip>
#include "macmanuf.h"


// for creating ranges in lookup tables
// input s=lower limit in hex, m=manuf name
OuiRange::OuiRange(std::string s, std::string m) {
    const std::string hexWithSlash = "0123456789ABCDEFabcdef/";
    int mask;
    strncpy(manufDesc, m.c_str(), sizeof(manufDesc));
    std::string cleaned;
    for (char c : s) {
        if (hexWithSlash.find(c) != std::string::npos) cleaned += c;
    }
    if (cleaned.find('/') == std::string::npos) {
        if (cleaned.length() == 6) { cleaned += "/24"; }
        if (cleaned.length() == 12) { cleaned += "/48"; }
    }
    if (cleaned.length() < 15) {
        int pad = 15 - cleaned.length();
        for (int i = 0; i < pad; i++) {
            cleaned.insert(cleaned.length() - 3, "0");
        }
    }
    lowerLimit = stoll(cleaned.substr(0, 12), 0, 16);
    mask = stoi(cleaned.substr(13, 2), 0, 10);
    upperLimit = lowerLimit + std::pow(2, 48 - mask);
}

// for lookup by string
OuiRange::OuiRange(std::string s) : OuiRange(s, "lookup") { };


// for lookup by long
OuiRange::OuiRange(u_int64_t m) {
    lowerLimit = m;
    upperLimit = m + 1; // needed for sort by upper limit (lower_bound)
}

bool comp(const OuiRange& lhs, const OuiRange& rhs) {
    return lhs.upperLimit < rhs.upperLimit;
}

void MacLookup::populateAllOui() {
    const std::string hexOnly = "0123456789ABCDEFabcdef";
    std::ifstream ifs{"./manufconfig/macs_all.txt"};
    if (!ifs.is_open()) {
        std::cout << "*** txt file with oui not found";
        std::exit(-2);
    }
    std::stringstream ss;
    std::string line, mac, name;
    while (getline(ifs, line)) {
        if (hexOnly.find(line[0]) == std::string::npos) { continue; }
        ss.str(line);
        ss >> mac >> name;
        ss.clear();
        if (mac == "FC:FF:AA") break;   // exclude known addrs or subranges -> often overlapping
        if (name == "IeeeRegi") continue;   // overlapping and meaningless
        allOuiRanges.push_back(OuiRange(mac, name));
    }
    std::sort(allOuiRanges.begin(), allOuiRanges.end(), comp);
    ifs.close();
}

MacLookup::MacLookup() {
    populateAllOui();
    populateTaggedMacs();
}


void MacLookup::populateTaggedMacs() {
    const std::string hexOnly = "0123456789ABCDEFabcdef";
    std::ifstream ifs{"./manufconfig/macs_known.txt"};
    if (!ifs.is_open()) std::cout << "*** txt file with known macs not found";
    std::stringstream ss;
    std::string line, mac, name;
    u_int64_t key = 0;
    std::string cleaned;
    while (getline(ifs, line)) {
        cleaned = "";
        if (hexOnly.find(line[0]) == std::string::npos) { continue; }
        ss.str(line);
        ss >> mac >> name;
        ss.clear();
        for (char c : mac) {
            if (hexOnly.find(c) != std::string::npos) cleaned += c;
        }
        if (cleaned.length() != 12) continue;
        key = std::stoll(cleaned, 0, 16);
        taggedMacs[key] = name;
    }
}

std::string MacLookup::resolve(const OuiRange& mac) const {
    std::string result;
    // look up tagged macs
    auto itr = taggedMacs.find(mac.lowerLimit);
    if (itr != taggedMacs.end()) {
        return itr->second;
    }
    // look up oui table
    auto oui = std::lower_bound(allOuiRanges.begin(), allOuiRanges.end(), mac, comp);
    char hexmac[20];
    sprintf(hexmac, "%012llX", mac.lowerLimit);
    if (mac.lowerLimit < oui->upperLimit) {
        std::string tail = hexmac;
        return std::string(oui->manufDesc) + "_" + tail.substr(tail.length() - 4, 4);
    } else {
        return '[' + std::string(hexmac) + ']';
    }
}

std::string MacLookup::resolve(const std::string& mac) const {
    static std::string cacheValue;
    static std::string cacheKey{"nam"};
    if (mac == cacheKey) {
        return cacheValue;
    }
    cacheKey = mac;
    if (!validate(mac)) {
        cacheValue = "invalid";
        return cacheValue;
    }
    cacheValue = resolve(OuiRange(mac));
    return cacheValue;
}


std::string MacLookup::resolve(const u_int64_t& mac64) const {
    static u_int64_t cacheKey;
    static std::string cacheValue;
    if (mac64 == cacheKey){
        return cacheValue;
    }
    cacheKey = mac64;
    if (!validate(mac64)) {
        cacheValue = "invalid";
        return cacheValue;
    }
    cacheValue = resolve(OuiRange(mac64));
    return cacheValue;
}


std::string MacLookup::resolve(u_char* macPtr) const {
    static u_char cacheKey[8];
    static std::string cacheValue;
    u_char tmp[8]{};
    // leave tmp[6] and tmp[7] = 0;
    for (int i = 0; i < 6; i++) {
        tmp[5 - i] = macPtr[i];
    }
    if (std::equal(std::begin(tmp), std::end(tmp), std::begin(cacheKey))) {
        return cacheValue;

    }
    u_int64_t* int64 = reinterpret_cast<u_int64_t*>(&tmp);
    if (!validate(*int64)) { cacheValue = "invalid"; }
    std::memcpy(cacheKey, tmp, 8);
    cacheValue = resolve(OuiRange(*int64));
    return cacheValue;
}


bool MacLookup::validate(const std::string& macStr) const {
    const std::string hexOnly = "0123456789ABCDEFabcdef";
    std::string tmp = "";
    for (char c : macStr) {
        if (hexOnly.find(c) != std::string::npos) tmp += c;
    }
    return tmp.length() == 12;
}

bool MacLookup::validate(const u_int64_t& t) const {
    return t < std::pow(2, 48);
}

MacLookup& MacLookup::getMacLookup() {
    static MacLookup instance;
    return instance;
}

std::string resolveMac(const u_int64_t mac64){
    static MacLookup& macLookup = MacLookup::getMacLookup();
    return macLookup.resolve(mac64);
}

std::string resolveMac(const std::string& macStr){
    static MacLookup& macLookup = MacLookup::getMacLookup();
    return macLookup.resolve(macStr);
}

std::string resolveMac(u_char* charPtr){
    static MacLookup& macLookup = MacLookup::getMacLookup();
    return macLookup.resolve(charPtr);
}
