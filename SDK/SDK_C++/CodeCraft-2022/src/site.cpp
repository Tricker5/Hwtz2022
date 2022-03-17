#include "site.h"


Site::Site(string name, int total_bw){
    this->name = name;
    this->total_bw = total_bw;
    this->rest_bw = total_bw;
    this->usable_fq = 0;
    this->over_times = 0;
    this->is_over = false;
}

bool Site::allocBw(int bw){
    if(bw > this->rest_bw){
        return false;
    }
    this->rest_bw -= bw;
    return true;
}


bool biggerRestBw(const Site* a, const Site* b){
    return a->rest_bw > b->rest_bw;
}