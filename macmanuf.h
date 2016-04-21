// MacLookup v0.1

#ifndef MACLOOKUP_MACLOOKUP_H_H
#define MACLOOKUP_MACLOOKUP_H_H

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <map>
#include <iomanip>


class OuiRange {

private:
    u_int64_t lowerLimit = 0;
    u_int64_t upperLimit = 0;
    char manufDesc[20];

    friend class MacLookup;
    friend bool comp(const OuiRange& lhs, const OuiRange& rhs);

    OuiRange(std::string, std::string);
    OuiRange(std::string);
    OuiRange(u_int64_t);

public:
};


class MacLookup {

private:
    std::map<u_int64_t, std::string> taggedMacs;
    std::vector<OuiRange> allOuiRanges;
    bool validate(const std::string&) const;
    bool validate(const u_int64_t&) const;
    void populateAllOui();
    void populateTaggedMacs();
    friend OuiRange::OuiRange(std::string, std::string);
    MacLookup();
    std::string resolve(const OuiRange&) const;

public:
    std::string resolve(const u_int64_t&) const;
    std::string resolve(const std::string&) const;
    std::string resolve(u_char*) const;
    static MacLookup& getMacLookup();
};

std::string resolveMac(const u_int64_t);
std::string resolveMac(const std::string&);
std::string resolveMac(u_char*);


#endif //MACLOOKUP_MACLOOKUP_H_H
