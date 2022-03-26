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
                // 每个节点记录自己可支持的用户
                this->map_site[site_name]->vec_usable_cstm_name.push_back(cstm_name);
                // 边缘节点统计自身可用时在列表中出现的频数
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
        for(const auto &dm : vec_dms.at(idx)){
            Customer* cstm = this->map_customer.at(dm.first);
            cstm->total_bw = dm.second;
            cstm->curr_bw = dm.second;
        }
        final_slt.at(idx) = this->solveDemands();
        if(final_slt.at(idx).size() < 10){
            cout << idx << endl;
        }
        this->resetSite();
    } 
    
    this->outputSolutions(final_slt);
}


/**
 * @brief 
 * 处理一条请求
 */
Solutions AllocMgr::solveDemands(){
    // 该条请求的分配方案
    Solutions slts;
    size_t idx = 0; // 计数专用

    // 记录一个site用于排序
    vector<Site*> v_site(this->map_site.size());
    // 记录当前所有site状态
    vector<SiteState> vec_site_state(this->map_site.size());
    for(auto site_pair : this->map_site){
        vec_site_state[idx] = site_pair.second->getState();
        v_site[idx] = site_pair.second;
        ++idx;
    }

    idx = 0;
    // 记录一个 customer 的序列用于排序
    vector<Customer*> vec_cstm(this->map_customer.size());
    for(auto cstm_pair : this->map_customer){
        vec_cstm[idx] = cstm_pair.second;
        ++idx;
    }

    // 以可支持的客户数排序
    sort(v_site.begin(), v_site.end(), smallerUCSize);

    
    // 遍历当前拥有超频机会的节点
    for(Site* site : v_site){
        if(site->over_times > 0){
            // 统计其可支持客户的所有需求, 并将其记录排序
            idx = 0;
            int dm_bw_count = 0;
            vector<Customer*> vec_usable_cstm(site->vec_usable_cstm_name.size());
            for(const string &c_name : site->vec_usable_cstm_name){
                Customer* cstm = this->map_customer.at(c_name);
                vec_usable_cstm[idx] = cstm;
                dm_bw_count += cstm->curr_bw;
                ++idx;
            }
            // 若大于某个阈值，那么直接超频
            if(dm_bw_count >= site->total_bw){
                site->is_over = true;
                // 以当前余量最多排序
                sort(vec_usable_cstm.begin(), vec_usable_cstm.end(), biggerCurrBwCstm);
                sort(vec_usable_cstm.begin(), vec_usable_cstm.end(), smallerUSCstm);
                for(auto cstm : vec_usable_cstm){
                    if(site->rest_bw != 0 && cstm->curr_bw != 0){
                        int bw;
                        if(site->rest_bw < cstm->curr_bw){
                            bw = site->rest_bw;
                        }else{
                            bw = cstm->curr_bw;
                        }
                        site->allocBw(bw);
                        cstm->allocBw(bw);
                        slts[cstm->name][site->name] += bw;
                    }
                }
            }
        }
    }

    // 处理每个客户的请求前，对其进行排序（当前剩余平均需求）
    sort(vec_cstm.begin(), vec_cstm.end(), biggerAvgCurrBwCstm);

    // 为了保证客户请求必然满足，回溯地去处理每个客户请求
    if(!this->solveOneCstmDm(vec_cstm, 0, slts)){
        // 要是分配失败，打印个log
        cout << "分配失败！！！" << endl;
    }
        
    return slts;
}


/**
 * @brief 
 * 回溯地去解决一整条请求，并给出相应分配方案
 */
bool AllocMgr::solveOneCstmDm(vector<Customer*> &vec_cstm, const size_t dm_idx, Solutions &slts){
    // 首先解析当前回溯的客户请求
    size_t idx; // 计数专用
    Customer* cstm = vec_cstm[dm_idx];
    string c_name = cstm->name;
    
    // 获得非超频可用节点, 并获取总裕量，及总理想裕量
    int usable_bw = 0;
    int usable_ideal_bw = 0;
    vector<Site*> vec_uno_site;
    for(const auto &s_name : cstm->vec_usable_site_name){
        Site* site = this->map_site.at(s_name);
        if(!site->is_over){
            vec_uno_site.push_back(site);
            usable_bw += site->rest_bw;
            int i_r_bw = site->ideal_bw - site->curr_bw;
            if(i_r_bw > 0){
                usable_ideal_bw += i_r_bw;
            }
        }   
    }

    // 没有充足裕量则返回上一层，进行重分配
    if(usable_bw < cstm->curr_bw){
        return false;
    }

    // 记录当前层所有site状态
    vector<SiteState> vec_site_state(this->map_site.size());
    size_t vss_idx = 0;
    for(auto site_pair : this->map_site){
        vec_site_state[vss_idx++] = site_pair.second->getState();
    }

    unordered_map<string, int> slt_per_cstm;



    // 考虑当前总理想裕量是否可以满足需求
    idx = 0;
    int update_bw = 300; // 理想负载增量
    while(usable_ideal_bw < cstm->curr_bw){
        Site* site = vec_uno_site.at(idx % vec_uno_site.size());
        int add_bw = site->total_bw - site->ideal_bw;
        add_bw = (add_bw < update_bw) ? add_bw : update_bw;
        site->ideal_bw += add_bw;
        usable_ideal_bw += add_bw;
        ++idx;
    }

    // 开始分配
    for(auto site : vec_uno_site){
        int bw = site->ideal_bw - site->curr_bw;
        bw = cstm->curr_bw > bw ? bw : cstm->curr_bw;
        if(bw != 0){
            site->allocBw(bw);
            cstm->allocBw(bw);
            slt_per_cstm[site->name] += bw;
            if(cstm->curr_bw == 0){
                break;
            }
        }
    }

    // 最后一层则直接返回
    if(dm_idx + 1 == vec_cstm.size()){
        for(auto slt_pair : slt_per_cstm){
            slts[c_name][slt_pair.first] += slt_pair.second;
        }    
        
        return true;
    }

    // 开始分配下一个客户
    while(! this->solveOneCstmDm(vec_cstm, dm_idx + 1, slts)){
        // 暂时不考虑分配出问题的情况
        return false;
    }
    // 下层分配成功，返回
    for(auto slt_pair : slt_per_cstm){
        slts[c_name][slt_pair.first] += slt_pair.second;
    }    
    return true;

}

/**
 * @brief 
 * 解决客户请求的具体细节
 */
unordered_map<string, int> AllocMgr::reAllocCstmDm(int total_dm_bw, Customer* cstm, double load_percent, bool is_new_over){
    int curr_dm_bw = total_dm_bw;
    // 首先判断当前可用节点是否有充足裕量
    vector<Site*> vec_usable_site; // 所有当前可用节点
    vector<Site*> vec_is_over_site; // 当前正在 overflow 的节点
    vector<Site*> vec_can_over_site; // 依然有 overflow 机会的节点
    for(const auto &s_name : cstm->vec_usable_site_name){
        Site* site = this->map_site.at(s_name);
        vec_usable_site.push_back(site);
        if(site->is_over){
            vec_is_over_site.push_back(site);
        }else if(is_new_over && site->over_times > 0){ // 只有在一定情况下，允许开启新的超频
            vec_can_over_site.push_back(site);
        }
    }
    unordered_map<string, int> slt_per_cstm;

    // ============== 使用超频大法！！！ ==================
    if(curr_dm_bw != 0){
        this->overDemands(vec_is_over_site, vec_can_over_site, total_dm_bw,  curr_dm_bw, slt_per_cstm, load_percent);
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
void AllocMgr::overDemands(vector<Site*> &vec_is_over_site, vector<Site*> &vec_can_over_site, const int total_dm_bw, int &curr_dm_bw, unordered_map<string, int> &slt_per_cstm, double load_percent){
    // 遍历当前正在超频的节点，暂时使用尽量直接把请求全分配给超频的策略
    for(auto site : vec_is_over_site){
        // 可用空间只能到达设定的超频状态
        int load_bw = site->rest_bw - int(site->total_bw * (1 - load_percent));
        if(load_bw > 0){
            if(load_bw < curr_dm_bw){
                this->allocBwPerCstm(load_bw, curr_dm_bw, site, slt_per_cstm);
            }else{
                this->allocBwPerCstm(curr_dm_bw, curr_dm_bw, site, slt_per_cstm);
                break;
            }
        }
    }
    
    // // 若有剩余请求，且剩余请求达到总请求的一定值，则开启其他可超频节点
    // if(curr_dm_bw != 0 && curr_dm_bw > total_dm_bw * 0.85){
    //     for(auto site : vec_can_over_site){
    //         site->openOverflow();
    //         // 可用空间只能到达设定的超频状态
    //         int load_bw = site->rest_bw - int(site->total_bw * (1 - load_percent));
    //         if(load_bw > 0){
    //             if(load_bw < curr_dm_bw){
    //                 this->allocBwPerCstm(load_bw, curr_dm_bw, site, slt_per_cstm);
    //             }else{
    //                 this->allocBwPerCstm(curr_dm_bw, curr_dm_bw, site, slt_per_cstm);
    //                 break;
    //             }
    //         }
            
    //     }
    // }
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
        this->allocBwPerCstm(curr_dm_bw, curr_dm_bw, site, slt_per_cstm);
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
                this->allocBwPerCstm(avg_bw, curr_dm_bw, site, slt_per_cstm);
            }          
            
        }
    }


    // 共同成长后，当前剩余请求有三种状态，0，小于可用队列个数，或者还剩超级多
    // 若剩超级多，继续雨露均沾，分到它比可用队列个数小
    if(curr_dm_bw >= vus_size){
        int avg_dm_bw = curr_dm_bw / vus_size;
        for(Site* site: vec_usable_site){
            this->allocBwPerCstm(avg_dm_bw, curr_dm_bw, site, slt_per_cstm);
        }
    }
    // 此时剩余请求应该只剩小于可用队列个数，将剩余的分配给前面的节点（后面的可能满了）
    for(int i = 0; i < vus_size; ++i){
        if(curr_dm_bw == 0){
            break;
        }
        this->allocBwPerCstm(1, curr_dm_bw, vec_usable_site[i], slt_per_cstm);
    }

}

/**
 * @brief 
 * 分配带宽并将其记录到解决方案
 */
void AllocMgr::allocBwPerCstm(int bw, int &curr_dm_bw, Site* site, unordered_map<string, int> &slt_per_cstm){
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