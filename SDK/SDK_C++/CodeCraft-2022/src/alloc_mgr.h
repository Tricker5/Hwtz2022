#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <ostream>
#include <vector>
#include <algorithm>

#include "utils.h"
#include "site.h"
#include "customer.h"
#include "config.h"

using namespace std;

struct AllocMgr{
    public:
        int qos_constr;
        unordered_map<string, Site*> map_site;
        unordered_map<string, Customer*> map_customer;

        AllocMgr();
        void initQosConstraint(const string &ini_cfg);
        void initSiteMap(const string &csv_site);
        void initCustomerMap(const string &csv_qos);
        void solveDemand(const string &csv_demand);
        unordered_map<string, unordered_map<string, int>> solveOneDemand(const vector<string> &demand_vec, const vector<string> &cstm_vec);
        void resetSite();
        void outputSolution(const vector<unordered_map<string, unordered_map<string, int>>> &final_slt);
};


