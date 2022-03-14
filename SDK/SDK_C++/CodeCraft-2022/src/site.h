#pragma once

#include <string>

struct Site{
    public:
        std::string name;
        int total_bw;
        
        Site(std::string name, int total_bw);
};
