#pragma once

#include <string>
#include <vector>
#include <iostream>

using namespace std;


// 用于分割字符串
vector<string> split(const string &str_line, const char &c_split);
// 对 dm_pair_vec 进行降序排序
bool biggerStrIntPair(const pair<string, int> &a, const pair<string, int> &b);