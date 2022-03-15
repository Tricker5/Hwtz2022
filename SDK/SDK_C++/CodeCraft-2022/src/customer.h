#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>

struct Customer{
    public:
        std::string name;
        std::unordered_map<std::string, int> map_usable_site;
        int total_site_fq;
        
        Customer(std::string name);
};