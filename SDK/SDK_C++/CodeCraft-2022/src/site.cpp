#include "site.h"


Site::Site(std::string name, int total_bw){
    this->name = name;
    this->total_bw = total_bw;
    this->rest_bw = total_bw;
    this->usable_fq = 0;
}

bool Site::allocBw(int bw){
    if(bw > this->rest_bw){
        return false;
    }
    this->rest_bw -= bw;
    return true;
}