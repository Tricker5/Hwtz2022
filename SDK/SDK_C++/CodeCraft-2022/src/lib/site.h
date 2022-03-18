#pragma once

#include <string>

using namespace std;

struct Site{
    public:
        string name;
        int total_bw;
        int rest_bw;
        int usable_fq; // 自身被可用频数
        int over_times; // 可 超频 次数
        bool is_over;
        
        Site(string name, int total_bw);
        bool allocBw(int bw);
        void openOverflow();
        void closeOverflow();
        void reset();
};

bool biggerRestBw(const Site* a, const Site* b);