#include <iostream>
#include<ctime>

#include "config.h"
#include "alloc_mgr.h"

using namespace std;

#ifdef TEST
void solve(){
    clock_t t_start = clock();
    AllocMgr alloc_mgr;
    alloc_mgr.initQosConstraint(ini_cfg);
    alloc_mgr.initSiteMap(csv_site_bandwidth);
    alloc_mgr.initCustomerMap(csv_qos);
    alloc_mgr.solveAllDemands(csv_demand);
    clock_t t_end = clock();
    cout << t_end - t_start << endl;
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
