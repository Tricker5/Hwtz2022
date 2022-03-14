#include "utils.h"

std::vector<std::string> split(std::string str_line, char c_split){
    std::vector<std::string> str_vec;
    std::string str_temp;
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