#pragma once

#include <string>

using namespace std;

struct Site{
    public:
        string name;
        int total_bw;
        int rest_bw;
        int usable_fq; // 自身被可用频数
        int over_times; // 可 overflow 次数
        bool is_over;
        
        Site(string name, int total_bw);
        bool allocBw(int bw);
};

bool biggerRestBw(const Site* a, const Site* b);