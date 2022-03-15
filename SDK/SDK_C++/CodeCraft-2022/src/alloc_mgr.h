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

struct AllocMgr{
    public:
        int qos_constr;
        std::unordered_map<std::string, Site*> map_site;
        std::unordered_map<std::string, Customer*> map_customer;

        AllocMgr();
        void initQosConstraint(const std::string &ini_cfg);
        void initSiteMap(const std::string &csv_site);
        void initCustomerMap(const std::string &csv_qos);
        void solveDemand(const std::string &csv_demand);
        std::unordered_map<std::string, std::unordered_map<std::string, int>> solveOneDemand(const std::vector<std::string> &demand_vec, const std::vector<std::string> &cstm_vec);
        void resetSite();
        void outputSolution(const std::vector<std::unordered_map<std::string, std::unordered_map<std::string, int>>> &final_slt);
};


