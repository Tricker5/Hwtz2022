#pragma once

#include <string>

#define TEST


#ifdef TEST
const std::string data_path = "./../../../data";
const std::string csv_demand = data_path + "/demand.csv";
const std::string csv_site_bandwidth = data_path + "/site_bandwidth.csv";
const std::string csv_qos = data_path + "/qos.csv";
const std::string ini_cfg = data_path + "/config.ini";
#endif