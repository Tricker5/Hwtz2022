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

typedef unordered_map<string, unordered_map<string, int>> Solutions;
typedef vector<pair<string, int>> Demands;

struct AllocMgr{
    public:
        int qos_constr;
        unordered_map<string, Site*> map_site;
        unordered_map<string, Customer*> map_customer;

        AllocMgr();
        void initQosConstraint(const string &ini_cfg);
        void initSiteMap(const string &csv_site);
        void initCustomerMap(const string &csv_qos);
        void solveAllDemands(const string &csv_demand);
        Demands preProDemands(const Demands &dms);
        Solutions solveDemands(const Demands &dms);
        bool solveOneCstmDm(const Demands &dms, const size_t dm_pair_idx, unordered_map<string, Site*> map_site_state, Solutions &slt_per_dm);
        void resetSite();
        void outputSolutions(const vector<Solutions> &final_slt);
};


