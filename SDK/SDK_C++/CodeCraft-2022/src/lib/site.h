#pragma once

#include <string>

using namespace std;

struct Site{
    public:
        string name;
        int total_bw;
        int rest_bw;
        int usable_fq; // 自身被可用频数
        
        Site(string name, int total_bw);
        bool allocBw(int bw);
};
