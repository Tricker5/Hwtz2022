#include <iostream>


#include "config.h"
#include "alloc_mgr.h"

using namespace std;

#ifdef TEST
void solve(){
    AllocMgr alloc_mgr;
    alloc_mgr.initQosConstraint(ini_cfg);
    alloc_mgr.initSiteMap(csv_site_bandwidth);
    alloc_mgr.initCustomerMap(csv_qos);
    alloc_mgr.solveAllDemands(csv_demand);
}
#else
void solve(){
    AllocMgr alloc_mgr;
    alloc_mgr.initQosConstraint(ini_cfg);
    alloc_mgr.initSiteMap(csv_site_bandwidth);
    alloc_mgr.initCustomerMap(csv_qos);
    alloc_mgr.solveAllDemands(csv_demand);
}
#endif



int main(){
    solve();
	return 0;
}
