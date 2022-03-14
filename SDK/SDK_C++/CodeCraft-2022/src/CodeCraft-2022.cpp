#include <iostream>


#include "config.h"
#include "alloc_mgr.h"

int main(){
    
    AllocMgr alloc_mgr;
    alloc_mgr.initQosConstraint(ini_cfg);
    std::cout << alloc_mgr.qos << std::endl;
    alloc_mgr.initSiteMap(csv_site_bandwidth);
    std::cout << alloc_mgr.map_site.size() << std::endl;
	return 0;
}
