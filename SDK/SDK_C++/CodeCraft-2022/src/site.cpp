#include "site.h"


Site::Site(string name, int total_bw){
    this->name = name;
    this->total_bw = total_bw;
    this->rest_bw = total_bw;
    this->ideal_bw = 0;
    this->curr_bw = 0;
    this->usable_fq = 0;
    this->over_times = 0;
    this->is_over = false;
}

bool Site::allocBw(int bw){
    // 暂时忽略检查
    // if(bw > this->rest_bw){
    //     return false;
    // }
    this->rest_bw -= bw;
    this->curr_bw += bw;
    return true;
}

void Site::openOverflow(){
    this->is_over = true;
}

void Site::closeOverflow(){
    if(this->is_over){
        this->is_over = false;
        --this->over_times;
    }
}

void Site::reset(){
    this->rest_bw = this->total_bw;
    this->curr_bw = 0;
    this->closeOverflow();
}

SiteState Site::getState(){
    SiteState s_state;
    s_state.name = this->name;
    s_state.rest_bw = this->rest_bw;
    s_state.curr_bw = this->curr_bw;
    s_state.ideal_bw = this->ideal_bw;
    s_state.over_times = this->over_times;
    s_state.is_over = this->is_over;
    return s_state;
}

void Site::setState(SiteState s_state){
    this->rest_bw = s_state.rest_bw;
    this->curr_bw = s_state.curr_bw;
    this->ideal_bw = s_state.ideal_bw;
    this->over_times = s_state.over_times;
    this->is_over = s_state.is_over;
}

bool biggerRestBw(const Site* a, const Site* b){
    return a->rest_bw > b->rest_bw;
}

bool biggerUCSite(const Site* a, const Site* b){
    return a->vec_usable_cstm_name.size() > b->vec_usable_cstm_name.size();
}

bool smallerUCSite(const Site* a, const Site* b){
    return a->vec_usable_cstm_name.size() < b->vec_usable_cstm_name.size();
}
