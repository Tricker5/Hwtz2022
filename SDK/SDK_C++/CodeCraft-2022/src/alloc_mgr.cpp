#include "alloc_mgr.h"

AllocMgr::AllocMgr(){
}


/**
 * @brief 初始化 qos 限制参数
 * 
 */
void AllocMgr::initQosConstraint(const std::string &ini_cfg){
    std::ifstream if_cfg(ini_cfg);
    std::string str_line;
    getline(if_cfg, str_line); // 跳过第一行说明
    getline(if_cfg, str_line); // 第二行得到 qos_constraint
    std::vector<std::string> str_vec = split(str_line, '=');
    this->qos_constr = stoi(str_vec[1]);
    if_cfg.close();
}

/**
 * @brief 初始化 site 表
 * 
 */
void AllocMgr::initSiteMap(const std::string &csv_site){
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
void AllocMgr::initCustomerMap(const std::string &csv_qos){
    std::ifstream if_qos(csv_qos);
    std::string str_line;
    getline(if_qos, str_line);
    // 先处理中 qos.csv 中的第一行，含有 customer 的名字信息
    std::vector<std::string> c_n_vec = split(str_line, ',');
    for(auto i = 1; i < c_n_vec.size(); ++i){
        Customer* cstm = new Customer(c_n_vec[i]);
        this->map_customer.emplace(c_n_vec[i], cstm);
    }
    // 接着处理qos限制
    while(getline(if_qos, str_line)){
        std::vector<std::string> qos_vec = split(str_line, ',');
        std::string site_name = qos_vec[0];
        for(auto i = 1; i < qos_vec.size(); ++i){
            std::string cstm_name = c_n_vec[i];
            int qos = stoi(qos_vec[i]);
            if(qos <= this->qos_constr){
                // 当满足 qos 限制时，每个客户记录自己可用的边缘节点
                this->map_customer[cstm_name]->map_usable_site.emplace(site_name, 0);
                // 边缘节点统计自身可用的“频数”
                this->map_site[site_name]->usable_fq += 1;
            }
        }
    }
    // 统计每个客户可用节点的频数
    for(auto cstm_pair : this->map_customer){
        Customer* cstm = cstm_pair.second;
        for(auto &site_fq_pair : cstm->map_usable_site){
            int fq = this->map_site.at(site_fq_pair.first)->usable_fq;
            // 每个可用节点的频数加到客户的信息中，便于分配权重
            site_fq_pair.second += fq;
            cstm->total_site_fq += fq;
        }
    }
    if_qos.close();
}

/**
 * @brief 
 * 处理所有请求
 */
void AllocMgr::solveDemand(const std::string &csv_demand){
    std::ifstream if_demand(csv_demand);
    std::string str_line;
    // 先处理第一行 customer 名字的信息
    getline(if_demand, str_line);
    std::vector<std::string> cstm_vec = split(str_line, ',');
    std::vector<std::unordered_map<std::string, std::unordered_map<std::string, int>>> final_slt;
    while(getline(if_demand, str_line)){
        std::vector<std::string> demand_vec = split(str_line, ','); 
        std::unordered_map<std::string, std::unordered_map<std::string, int>> slt_per_dm = this->solveOneDemand(demand_vec, cstm_vec);
        this->resetSite();
        final_slt.push_back(slt_per_dm);   
    }
    if_demand.close();
    this->outputSolution(final_slt);
}

/**
 * @brief 
 * 处理一条请求
 */
std::unordered_map<std::string, std::unordered_map<std::string, int>> AllocMgr::solveOneDemand(const std::vector<std::string> &demand_vec, const std::vector<std::string> &cstm_vec){
    std::string m_time = demand_vec[0];
    std::vector<std::pair<std::string, int>> dm_pair_vec;
    for(auto i = 1; i < demand_vec.size(); ++i){
        std::pair<std::string, int> dm_pair = {cstm_vec[i], stoi(demand_vec[i])};
        dm_pair_vec.push_back(dm_pair);
    }

    // 首先对客户需求排序，优先对大需求进行分配
    std::sort(dm_pair_vec.begin(), dm_pair_vec.end(), cmpDemandPairVec);
    
    // 最后请求的分配方案
    std::unordered_map<std::string, std::unordered_map<std::string, int>> slt_per_dm;
    // 对于处理一条请求中的每个客户请求
    for(auto dm_pair : dm_pair_vec){
        std::string c_name = dm_pair.first;
        Customer* cstm = this->map_customer[c_name];
        int total_dm_bw = dm_pair.second;
        int rest_dm_bw = total_dm_bw;
        std::unordered_map<std::string, int> slt_per_cstm;
        for(auto site_fq_pair : cstm->map_usable_site){
            std::string s_name = site_fq_pair.first;
            int fq = site_fq_pair.second;
            int alloc_bw = total_dm_bw * fq / cstm->total_site_fq;
            // 判断是否能成功分配，若不能，进行后续处理
            if(!this->map_site.at(s_name)->allocBw(alloc_bw)){
                // 暂时认为必然能全分配进去
                std::cout << "this time cannot alloc: " << std::endl;
            }
            rest_dm_bw -= alloc_bw;
            slt_per_cstm.emplace(s_name, alloc_bw);
        }
        // 上述分配最后一次分配没有完全分配完成，随机取出一个来分
        auto site_fq_pair = *(cstm->map_usable_site.begin());
        std::string s_name = site_fq_pair.first;
        int alloc_bw = rest_dm_bw;
        // 判断是否能成功分配，若不能，进行后续处理
        if(!this->map_site.at(s_name)->allocBw(alloc_bw)){
            // 暂时认为必然能全分配进去
            std::cout << "this time cannot alloc: " << std::endl;
        }
        rest_dm_bw -= alloc_bw;
        slt_per_cstm.at(s_name) += alloc_bw;
        slt_per_dm.emplace(c_name, slt_per_cstm);
    }
    
    return slt_per_dm;
    
}

/**
 * @brief 当处理完一次需求后对带宽进行重置
 * 
 */
void AllocMgr::resetSite(){
    for(auto site_pair : this->map_site){
        site_pair.second->rest_bw = site_pair.second->total_bw;
    }
}


/**
 * @brief 输出解决方案
 * 
 */
void AllocMgr::outputSolution(const std::vector<std::unordered_map<std::string, std::unordered_map<std::string, int>>> &final_slt){
    std::ofstream of_slt(output_path + "/solution.txt");
    std::string output_str;
    for(auto slt_per_time : final_slt){
        for(auto slt_per_cstm : slt_per_time){
            std::string c_name = slt_per_cstm.first;
            output_str += c_name;
            output_str += ":";
            for(auto slt : slt_per_cstm.second){
                output_str += "<";
                output_str += slt.first;
                output_str += ",";
                output_str += std::to_string(slt.second);
                output_str += ">";
                output_str += ",";
            }
            // 去除最后一个逗号
            if(slt_per_cstm.second.bucket_count() != 0){
                output_str.assign(output_str.begin(), output_str.end() - 1);
            }
            output_str += "\n";
        }
    }
    // 去除最后一个换行符
    output_str.assign(output_str.begin(), output_str.end() - 1);
    // std::cout << output_str;
    of_slt << output_str;
    of_slt.close();
}
