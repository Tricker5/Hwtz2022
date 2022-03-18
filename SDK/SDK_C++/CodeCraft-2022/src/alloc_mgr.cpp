#include "alloc_mgr.h"

AllocMgr::AllocMgr(){
}

AllocMgr::~AllocMgr(){
    for(auto& item : this->map_site){
        delete(item.second);
    }
    for(auto& item : this->map_customer){
        delete(item.second);
    }
}


/**
 * @brief 初始化 qos 限制参数
 * 
 */
void AllocMgr::initQosConstraint(const string &ini_cfg){
    ifstream if_cfg(ini_cfg);
    string str_line;
    getline(if_cfg, str_line); // 跳过第一行说明
    getline(if_cfg, str_line); // 第二行得到 qos_constraint
    vector<string> str_vec = split(str_line, '=');
    this->qos_constr = stoi(str_vec[1]);
    if_cfg.close();
}

/**
 * @brief 初始化 site 表
 * 
 */
void AllocMgr::initSiteMap(const string &csv_site){
    ifstream if_site(csv_site);
    string str_line;
    getline(if_site, str_line); // 跳过第一行说明部分
    while(getline(if_site, str_line)){
        vector<string> str_vec = split(str_line, ',');
        Site* site = new Site(str_vec[0], stoi(str_vec[1]));
        this->map_site.emplace(str_vec[0], site);
    }
    if_site.close();
}


/**
 * @brief 初始化 customer 表
 * 
 */
void AllocMgr::initCustomerMap(const string &csv_qos){
    ifstream if_qos(csv_qos);
    string str_line;
    getline(if_qos, str_line);
    // 先处理中 qos.csv 中的第一行，含有 customer 的名字信息
    vector<string> c_n_vec = split(str_line, ',');
    for(size_t i = 1; i < c_n_vec.size(); ++i){
        Customer* cstm = new Customer(c_n_vec[i]);
        this->map_customer.emplace(c_n_vec[i], cstm);
    }
    // 接着处理qos限制
    while(getline(if_qos, str_line)){
        vector<string> qos_vec = split(str_line, ',');
        string site_name = qos_vec[0];
        for(size_t i = 1; i < qos_vec.size(); ++i){
            string cstm_name = c_n_vec[i];
            int qos = stoi(qos_vec[i]);
            // 必须小于时延
            if(qos < this->qos_constr){
                // 当满足 qos 限制时，每个客户记录自己可用的边缘节点
                this->map_customer[cstm_name]->vec_usable_site_name.push_back(site_name);
                // 边缘节点统计自身可用的“频数”
                this->map_site[site_name]->usable_fq += 1;
            }
        }
    }
    if_qos.close();
}

/**
 * @brief 
 * 处理所有请求
 */
void AllocMgr::solveAllDemands(const string &csv_demand){
    ifstream if_demand(csv_demand);
    string str_line;
    // 先处理第一行 customer 名字的信息
    getline(if_demand, str_line);
    vector<string> cstm_vec = split(str_line, ',');
    size_t dm_idx = 0;
    // Trick!!! 将所有的请求都存入这个 vector
    vector<Demands> vec_dms;
    // 使用 pair 绑定请求之和以及请求在 vector 中原本的位置，用于排序
    vector<pair<size_t, int>> vec_idx_dm_sum;

    // 读取所有请求
    while(getline(if_demand, str_line)){
        vector<string> str_dms_vec = split(str_line, ',');
        // 将每条请求与对应的客户名绑定
        Demands dms(cstm_vec.size() - 1);
        int dm_sum = 0; // 计算请求之和
        for(size_t i = 1; i < str_dms_vec.size(); ++i){
            int dm_bw = stoi(str_dms_vec[i]);
            dms.at(i - 1).first = cstm_vec[i];
            dms.at(i - 1).second = dm_bw;
            dm_sum += dm_bw;
        }
        // 存入 idx 和请求之和
        vec_dms.push_back(dms);
        vec_idx_dm_sum.push_back({dm_idx++, dm_sum});  
    }
    if_demand.close();

    // 根据请求之和开始排序
    sort(vec_idx_dm_sum.begin(), vec_idx_dm_sum.end(), biggerIdxInt);

    size_t size_all_dms = vec_dms.size();

    // 已知所有的请求个数时，初始化每个 site 的 overflow 次数
    int over_times = int(size_all_dms - ceil(size_all_dms * pos_percent));
    for(auto &site_pair : this->map_site){
       site_pair.second->over_times = over_times;
    }

    vector<Solutions> final_slt(size_all_dms);
    // 开始处理所有时刻的请求
    for(auto dm_sum : vec_idx_dm_sum){
        size_t idx = dm_sum.first;
        Demands dms = this->preProDemands(vec_dms.at(idx));
        final_slt.at(idx) = this->solveDemands(dms);
        this->resetSite();
    } 
    
    this->outputSolutions(final_slt);
}

/**
 * @brief 
 * 预处理请求
 */
Demands AllocMgr::preProDemands(const Demands &dms){
    // 首先对客户需求排序
    size_t dms_size = dms.size();
    vector<pair<size_t, int>> dm_vec_for_sort(dms_size); // 将请求和原本请求的 idx 绑定成 pair 便于排序
    Demands sorted_dms(dms_size);

    for(size_t i = 0; i < dms_size; ++i){
        int dm_bw = dms[i].second;
        // 取平均需求
        pair<size_t, int> dm_pair = {i, dm_bw / this->map_customer.at(dms[i].first)->vec_usable_site_name.size()};
        // // 取总需求
        // pair<size_t, int> dm_pair = {i, dm_bw};
        dm_vec_for_sort[i] = dm_pair;
    }

    // 从大到小排序
    sort(dm_vec_for_sort.begin(), dm_vec_for_sort.end(), biggerIdxInt);
    for(size_t i = 0; i < dms_size; ++i){
        sorted_dms[i] = dms[dm_vec_for_sort[i].first];
    }


    // 对可超频的节点数进行排序
    for(size_t i = 0; i < dms_size; ++i){
        int count = 0;
        for(const auto &s_name: this->map_customer.at(dms[i].first)->vec_usable_site_name){
            if(this->map_site.at(s_name)->over_times != 0){
                ++count;
            }
        }
        // 获得可超频节点数
        dm_vec_for_sort[i] = {i, count};
    }

    // 从小到大排序
    sort(dm_vec_for_sort.begin(), dm_vec_for_sort.end(), smallerIdxInt);
    for(size_t i = 0; i < dms_size; ++i){
        sorted_dms[i] = dms[dm_vec_for_sort[i].first];
    }


    return sorted_dms;

}

/**
 * @brief 
 * 处理一条请求
 */
Solutions AllocMgr::solveDemands(const Demands &dms){
    // 该条请求的分配方案
    Solutions slts;
    // 为了保证客户请求必然满足，回溯地去处理每个客户请求
    if(!dms.empty()){
        if(!this->solveOneCstmDm(dms, 0, slts)){
            // 要是分配失败，打印个log
            cout << "分配失败！！！" << endl;
        }
    }
        
    return slts;
}


bool AllocMgr::solveOneCstmDm(const Demands &dms, const size_t dm_idx, Solutions &slts){
    // 首先解析当前回溯的客户请求
    pair<string, int> dm = dms[dm_idx];
    string c_name = dm.first;
    Customer* cstm = this->map_customer.at(c_name);
    int total_dm_bw = dm.second;
    double load_percent = 1; // 默认超频作业时满载额度

    // 首先判断当前可用节点是否有充足裕量
    int usable_bw = 0;
    for(const auto &s_name : cstm->vec_usable_site_name){
        usable_bw += this->map_site.at(s_name)->rest_bw;
    }
    // 没有充足裕量则返回上一层，进行重分配
    if(usable_bw < total_dm_bw){
        return false;
    }

    // 记录当前层所有site状态
    vector<SiteState> vec_site_state(this->map_site.size());
    size_t vss_idx = 0;
    for(auto site_pair : this->map_site){
        vec_site_state[vss_idx++] = site_pair.second->getState();
    }

    unordered_map<string, int> slt_per_cstm;

    slt_per_cstm = this->reAllocCstmDm(total_dm_bw, cstm, load_percent);

    // 最后一层则直接返回
    if(dm_idx + 1 == dms.size()){    
        slts.emplace(c_name, slt_per_cstm);
        return true;
    }

    // 开始分配下一个客户
    while(! this->solveOneCstmDm(dms, dm_idx + 1, slts)){
        // cout << "================= ReAllocating dm_idx = " << dm_idx << " !!! ===============" << endl;
        load_percent -= 0.05;
        if(load_percent < 0.5){  // 本层最多迭代至超频负荷到达0.5
            return false;
        }
        // 还原一下 map_site 的状态
        for(auto s_state : vec_site_state){
            this->map_site.at(s_state.name)->setState(s_state);
        }
        // 重分配
        slt_per_cstm = this->reAllocCstmDm(total_dm_bw, cstm, load_percent);
    }
    // 下层分配成功，返回
    slts.emplace(c_name, slt_per_cstm);
    return true;

}

unordered_map<string, int> AllocMgr::reAllocCstmDm(int total_dm_bw, Customer* cstm, double load_percent){
    int curr_dm_bw = total_dm_bw;
    // 首先判断当前可用节点是否有充足裕量, 并使用 vector 存储便于排序
    vector<Site*> vec_usable_site; // 所有当前可用节点
    vector<Site*> vec_is_over_site; // 当前正在 overflow 的节点
    vector<Site*> vec_can_over_site; // 依然有 overflow 机会的节点
    for(const auto &s_name : cstm->vec_usable_site_name){
        Site* site = this->map_site.at(s_name);
        vec_usable_site.push_back(site);
        if(site->is_over){
            vec_is_over_site.push_back(site);
        }else if(site->over_times > 0){
            vec_can_over_site.push_back(site);
        }
    }
    unordered_map<string, int> slt_per_cstm;
    // ============== 使用超频大法！！！ ==================
    if(curr_dm_bw != 0){
        this->overDemands(vec_is_over_site, vec_can_over_site, curr_dm_bw, slt_per_cstm, load_percent);
    }

    // ============== 使用“均衡”大法！！！ ==================
    if(curr_dm_bw != 0){
        this->balanceDemands(vec_usable_site, curr_dm_bw, slt_per_cstm);
    }

    return slt_per_cstm;
}


/**
 * @brief 
 * 对每个节点进行超频
 */
void AllocMgr::overDemands(vector<Site*> &vec_is_over_site, vector<Site*> &vec_can_over_site, int &curr_dm_bw, unordered_map<string, int> &slt_per_cstm, double load_percent){
    // 遍历当前正在超频的节点，暂时使用尽量直接把请求全分配给超频的策略
    for(auto site : vec_is_over_site){
        // 可用空间只能到达设定的超频状态
        int load_bw = site->rest_bw - int(site->total_bw * (1 - load_percent));
        if(load_bw > 0){
            if(load_bw < curr_dm_bw){
                this->allocBw(load_bw, curr_dm_bw, site, slt_per_cstm);
            }else{
                this->allocBw(curr_dm_bw, curr_dm_bw, site, slt_per_cstm);
                break;
            }
        }
    }
    
    // 若有剩余请求，则开启其他可超频节点
    if(curr_dm_bw != 0){
        for(auto site : vec_can_over_site){
            site->openOverflow();
            // 可用空间只能到达设定的超频状态
            int load_bw = site->rest_bw - int(site->total_bw * (1 - load_percent));
            if(load_bw > 0){
                if(load_bw < curr_dm_bw){
                    this->allocBw(load_bw, curr_dm_bw, site, slt_per_cstm);
                }else{
                    this->allocBw(curr_dm_bw, curr_dm_bw, site, slt_per_cstm);
                    break;
                }
            }
            
        }
    }
}


/**
 * @brief 
 * 对请求的带宽进行均衡
 */
void AllocMgr::balanceDemands(vector<Site*> &vec_usable_site, int &curr_dm_bw, unordered_map<string, int> &slt_per_cstm){
    int vus_size = vec_usable_site.size();
    // 首先对可用节点的裕量进行从大到小排序
    sort(vec_usable_site.begin(), vec_usable_site.end(), biggerRestBw);

    // 对裕量进行均衡, 可用节点只有1个则将所有请求放在改节点上
    if(vus_size == 1){
        Site* site = vec_usable_site[0];
        site->allocBw(curr_dm_bw);
        curr_dm_bw = 0;
        slt_per_cstm[site->name] += curr_dm_bw;
    }else{
        int target_s_idx = 1;
        vector<Site*> balance_s_vec = {vec_usable_site[0]}; // 一起向目标均衡点靠近的成长队列
        Site* target_s = vec_usable_site[1]; // 目标均衡节点

        // 当目前的剩余请求足以用来均衡所有的成长节点时，执行共同成长
        while(curr_dm_bw >= int(balance_s_vec.size())){
            // 判断当前离目标的距离
            int dis = balance_s_vec[0]->rest_bw - target_s->rest_bw;
            // 若已逼近目标
            if(dis == 0){
                // 判断是否有下一个逼近目标, 若有，则将当前目标点加入成长队列，更新目标，否则终止共同成长
                if(target_s_idx + 1 < vus_size){
                    balance_s_vec.push_back(target_s);
                    target_s = vec_usable_site[++target_s_idx];
                    continue;
                }else{
                    break;
                }
            }
            int avg_bw;
            if(curr_dm_bw < int(dis * balance_s_vec.size())){
                // 小于距离之和，则留出部分余数
                avg_bw = curr_dm_bw / balance_s_vec.size();
            }else{
                // 否则直接填满距离
                avg_bw = dis;
            }
            // 共同成长，向目标逼近
            for(Site* site : balance_s_vec){
                site->allocBw(avg_bw);
                curr_dm_bw -= avg_bw;
                slt_per_cstm[site->name] += avg_bw;
            }          
            
        }
    }


    // 共同成长后，当前剩余请求有三种状态，0，小于可用队列个数，或者还剩超级多
    // 若剩超级多，继续雨露均沾，分到它比可用队列个数小
    if(curr_dm_bw >= vus_size){
        int avg_dm_bw = curr_dm_bw / vus_size;
        for(Site* site: vec_usable_site){
            this->allocBw(avg_dm_bw, curr_dm_bw, site, slt_per_cstm);
        }
    }
    // 此时剩余请求应该只剩小于可用队列个数，将剩余的分配给前面的节点（后面的可能满了）
    for(int i = 0; i < vus_size; ++i){
        if(curr_dm_bw == 0){
            break;
        }
        this->allocBw(1, curr_dm_bw, vec_usable_site[i], slt_per_cstm);
    }

}

/**
 * @brief 
 * 分配带宽并将其记录到解决方案
 */
void AllocMgr::allocBw(int bw, int &curr_dm_bw, Site* site, unordered_map<string, int> &slt_per_cstm){
    site->allocBw(bw);
    curr_dm_bw -= bw;
    slt_per_cstm[site->name] += bw;
}

/**
 * @brief 当处理完一次需求后对带宽进行重置，并重置超频状态
 * 
 */
void AllocMgr::resetSite(){
    for(auto site_pair : this->map_site){
        site_pair.second->reset();
    }
}


/**
 * @brief 输出解决方案
 * 
 */
void AllocMgr::outputSolutions(const vector<Solutions> &final_slt){
    ofstream of_slt(output_path + "/solution.txt");
    string output_str;
    for(auto slt_per_time : final_slt){
        for(auto slt_per_cstm : slt_per_time){
            string c_name = slt_per_cstm.first;
            output_str += c_name;
            output_str += ":";
            for(auto slt_s : slt_per_cstm.second){
                output_str += "<";
                output_str += slt_s.first;
                output_str += ",";
                output_str += to_string(slt_s.second);
                output_str += ">";
                output_str += ",";
            }
            // 去除最后一个逗号
            if(slt_per_cstm.second.size() != 0){
                output_str.assign(output_str.begin(), output_str.end() - 1);
            }
            output_str += "\n";
        }
    }
    // 去除最后一个换行符
    output_str.assign(output_str.begin(), output_str.end() - 1);
    // cout << output_str;
    of_slt << output_str;
    of_slt.close();
}
