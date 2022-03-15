#include <iostream>


#include "config.h"
#include "alloc_mgr.h"

#ifdef TEST
void solveDemand(){
    AllocMgr alloc_mgr;
    alloc_mgr.initQosConstraint(ini_cfg);
    std::cout << alloc_mgr.qos_constr << std::endl;
    alloc_mgr.initSiteMap(csv_site_bandwidth);
    std::cout << alloc_mgr.map_site.size() << std::endl;
    alloc_mgr.initCustomerMap(csv_qos);
    std::cout << alloc_mgr.map_customer.size() << std::endl;
    std::cout << alloc_mgr.map_customer.at("A")->map_usable_site.size() << std::endl;
    std::cout << alloc_mgr.map_site.at("A")->usable_fq << std::endl;
    alloc_mgr.solveDemand(csv_demand);
    std::cout << alloc_mgr.map_customer.at("A")->total_site_fq << std::endl;

}
#else
void solveDemand(){
    AllocMgr alloc_mgr;
    alloc_mgr.initQosConstraint(ini_cfg);
    alloc_mgr.initSiteMap(csv_site_bandwidth);
    alloc_mgr.initCustomerMap(csv_qos);
    alloc_mgr.solveDemand(csv_demand);
}
#endif



int main(){
    solveDemand();
	return 0;
}
