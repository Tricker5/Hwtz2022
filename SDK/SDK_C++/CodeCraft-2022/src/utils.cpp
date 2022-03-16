#include "utils.h"

vector<string> split(const string &str_line, const char &c_split){
    vector<string> str_vec;
    string str_temp;
    for(auto c : str_line){
        if(c != c_split && c != '\r' && c!= '\n'){
            str_temp += c;
        }else{
            if(!str_temp.empty()){
                str_vec.push_back(str_temp);
                str_temp = "";
            }
        }
    }
    // 若文件没有换行符，则处理最后一个temp
    if(!str_temp.empty()){
        str_vec.push_back(str_temp);
        str_temp = "";
    }
    return str_vec;
}

bool cmpDemandPairVec(const pair<string, int> &a, const pair<string, int> &b){
    return a.second > b.second;
}