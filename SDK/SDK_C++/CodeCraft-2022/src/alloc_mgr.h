#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <ostream>
#include <vector>

#include "utils.h"
#include "site.h"
#include "customer.h"

struct AllocMgr{
    public:
        int qos;
        std::unordered_map<std::string, Site*> map_site;
        std::unordered_map<std::string, Customer*> map_customer;

        AllocMgr();
        void initQosConstraint(const std::string& ini_cfg);
        void initSiteMap(const std::string&  csv_site);
        void initCustomerMap(const std::string& csv_qos);
};
