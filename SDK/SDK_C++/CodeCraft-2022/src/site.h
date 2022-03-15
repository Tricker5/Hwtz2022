#pragma once

#include <string>

struct Site{
    public:
        std::string name;
        int total_bw;
        int rest_bw;
        int usable_fq; // 自身被可用频数
        
        Site(std::string name, int total_bw);
        bool allocBw(int bw);
};
