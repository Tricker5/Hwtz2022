#include <iostream>


#include "config.h"
#include "alloc_mgr.h"

using namespace std;

#ifdef TEST
void solveDemand(){
    AllocMgr alloc_mgr;
    alloc_mgr.initQosConstraint(ini_cfg);
    cout << alloc_mgr.qos_constr << endl;
    alloc_mgr.initSiteMap(csv_site_bandwidth);
    cout << alloc_mgr.map_site.size() << endl;
    alloc_mgr.initCustomerMap(csv_qos);
    cout << alloc_mgr.map_customer.size() << endl;
    cout << alloc_mgr.map_customer.at("A")->map_usable_site.size() << endl;
    cout << alloc_mgr.map_site.at("A")->usable_fq << endl;
    alloc_mgr.solveDemand(csv_demand);
    cout << alloc_mgr.map_customer.at("A")->total_site_fq << endl;

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
