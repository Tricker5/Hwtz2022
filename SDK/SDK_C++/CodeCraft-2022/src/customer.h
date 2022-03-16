#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>

using namespace std;

struct Customer{
    public:
        string name;
        unordered_map<string, int> map_usable_site;
        int total_site_fq;
        
        Customer(string name);
};