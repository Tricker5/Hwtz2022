#pragma once

#include <string>

// #define TEST

using namespace std;


#ifdef TEST
const string data_path = "./../../../data";
const string output_path = "./../../../output";
#else
const string data_path = "/data";
const string output_path = "/output";
#endif
const string csv_demand = data_path + "/demand.csv";
const string csv_site_bandwidth = data_path + "/site_bandwidth.csv";
const string csv_qos = data_path + "/qos.csv";
const string ini_cfg = data_path + "/config.ini";