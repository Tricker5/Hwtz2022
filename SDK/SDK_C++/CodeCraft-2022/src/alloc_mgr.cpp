#include "alloc_mgr.h"

AllocMgr::AllocMgr(){
}


/**
 * @brief 初始化 qos 限制参数
 * 
 */
void AllocMgr::initQosConstraint(const std::string& ini_cfg){
    std::ifstream if_cfg(ini_cfg);
    std::string str_line;
    getline(if_cfg, str_line); // 跳过第一行说明
    getline(if_cfg, str_line); // 第二行得到 qos_constraint
    std::vector<std::string> str_vec = split(str_line, '=');
    this->qos = stoi(str_vec[1]);
}

/**
 * @brief 初始化 site 表
 * 
 */
void AllocMgr::initSiteMap(const std::string& csv_site){
    std::ifstream if_site(csv_site);
    std::string str_line;
    getline(if_site, str_line); // 跳过第一行说明部分
    while(getline(if_site, str_line)){
        std::vector<std::string> str_vec = split(str_line, ',');
        Site* site = new Site(str_vec[0], stoi(str_vec[1]));
        this->map_site.emplace(str_vec[0], site);
    }
    if_site.close();
}


/**
 * @brief 初始化 customer 表
 * 
 */
void AllocMgr::initCustomerMap(const std::string& csv_qos){
    std::ifstream if_customer(csv_qos);
    std::string str_line;
    getline(if_customer, str_line);
    // 先处理中 qos.csv 中的第一行，含有 customer 的名字信息

}