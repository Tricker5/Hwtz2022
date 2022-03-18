#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <ostream>
#include <vector>
#include <algorithm>
#include <cmath>

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
        ~AllocMgr();
        void initQosConstraint(const string &ini_cfg);
        void initSiteMap(const string &csv_site);
        void initCustomerMap(const string &csv_qos);
        void solveAllDemands(const string &csv_demand);
        Demands preProDemands(const Demands &dms);
        Solutions solveDemands(const Demands &dms);
        bool solveOneCstmDm(const Demands &dms, const size_t dm_idx, Solutions &slts);
        unordered_map<string, int> reAllocCstmDm(int total_dm_bw, Customer* cstm, double load_percent);
        void overDemands(vector<Site*> &vec_is_over_site, vector<Site*> &vec_can_over_site, int &curr_dm_bw, unordered_map<string, int> &slt_per_cstm, double load_percent);
        void balanceDemands(vector<Site*> &vec_usable_site, int &curr_dm_bw, unordered_map<string, int> &slt_per_cstm);
        void resetSite();
        void allocBw(int bw, int &curr_dm_bw, Site* site, unordered_map<string, int> &slt_per_cstm);
        void outputSolutions(const vector<Solutions> &final_slt);
};


