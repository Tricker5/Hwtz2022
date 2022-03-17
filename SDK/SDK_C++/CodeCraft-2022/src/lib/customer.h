#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace std;

struct Customer{
    public:
        string name;
        vector<pair<string, int>> vec_usable_site_fq;
        int total_site_fq;
        
        Customer(string name);
};