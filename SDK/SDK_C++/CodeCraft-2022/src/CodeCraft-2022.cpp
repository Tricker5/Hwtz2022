#include <iostream>

#include "config.h"
#include "alloc_mgr.h"

using namespace std;

#ifdef TEST
#include <ctime>
void solve(){
    clock_t t_start = clock();
    AllocMgr alloc_mgr;
    alloc_mgr.initQosConstraint(ini_cfg);
    alloc_mgr.initSiteMap(csv_site_bandwidth);
    alloc_mgr.initCustomerMap(csv_qos);
    alloc_mgr.solveAllDemands(csv_demand);
    clock_t t_end = clock();
    cout << double(t_end - t_start) / CLOCKS_PER_SEC << endl;
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
