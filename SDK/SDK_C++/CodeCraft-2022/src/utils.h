#pragma once

#include <string>
#include <vector>
#include <iostream>


// 用于分割字符串
std::vector<std::string> split(const std::string &str_line, const char &c_split);
// 对 dm_pair_vec 进行降序排序
bool cmpDemandPairVec(const std::pair<std::string, int> &a, const std::pair<std::string, int> &b);