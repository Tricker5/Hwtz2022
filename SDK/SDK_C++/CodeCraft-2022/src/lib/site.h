#pragma once

#include <string>
#include <vector>

using namespace std;

struct SiteState{
    string name;
    int rest_bw;
    int curr_bw;
    int ideal_bw;
    int over_times;
    bool is_over;
};

struct Site{
    public:
        string name;
        int total_bw;
        int rest_bw;
        int ideal_bw;  // 理想负载
        int curr_bw;   // 当前负载
        int over_times; // 可超频次数
        bool is_over;
        int usable_fq; // 自身被可用频数
        vector<string> vec_usable_cstm_name;
        
        Site(string name, int total_bw);
        bool allocBw(int bw);
        void openOverflow();
        void closeOverflow();
        SiteState getState();
        void setState(SiteState s_state);
        void reset();
};




bool biggerRestBw(const Site* a, const Site* b);
bool biggerUCSite(const Site* a, const Site* b);
bool smallerUCSite(const Site* a, const Site* b);