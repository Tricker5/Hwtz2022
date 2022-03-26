#include "customer.h"


Customer::Customer(string name){
    this->name = name;
    this->total_site_fq = 0;
    this->total_bw = 0;
    this->curr_bw = 0;
}

bool Customer::allocBw(int bw){
    this->curr_bw -= bw;
    return true;
}

bool biggerAvgCurrBwCstm(const Customer* a, const Customer* b){
    int avg_bw_a = a->curr_bw / a->vec_usable_site_name.size();
    int avg_bw_b = b->curr_bw / b->vec_usable_site_name.size();
    return avg_bw_a > avg_bw_b;
}

bool biggerCurrBwCstm(const Customer* a, const Customer* b){
    return a->curr_bw > b->curr_bw;
}

bool smallerUSCstm(const Customer*a, const Customer* b){
    return a->vec_usable_site_name.size() < b->vec_usable_site_name.size();
}