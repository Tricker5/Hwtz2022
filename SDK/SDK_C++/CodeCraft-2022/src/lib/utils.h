#pragma once

#include <string>
#include <vector>
#include <iostream>

using namespace std;


// 用于分割字符串
vector<string> split(const string &str_line, const char &c_split);

// 一些用于sort排序的函数
bool biggerStrInt(const pair<string, int> &a, const pair<string, int> &b);
bool biggerIdxInt(const pair<size_t, int> &a, const pair<size_t, int> &b);
bool smallerStrInt(const pair<string, int> &a, const pair<string, int> &b);