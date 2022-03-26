#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace std;

struct Customer{
    public:
        string name;
        vector<string> vec_usable_site_name;
        int total_site_fq;
        int total_bw;
        int curr_bw;
        
        Customer(string name);
        bool allocBw(int bw);
        
};

// 以当前平均请求排序
bool biggerAvgCurrBwCstm(const Customer* a, const Customer* b);
// 以当前剩余请求排序
bool biggerCurrBwCstm(const Customer*a, const Customer* b);
// 以可用节点数量排序
bool smallerUSCstm(const Customer*a, const Customer* b);