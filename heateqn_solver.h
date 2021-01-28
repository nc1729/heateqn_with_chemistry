#pragma once
#include "ConfFileData.h"
#include "CSVFileData.h"
template <typename T>
void heateqn_solver(ConfFileData& conf_file_data, CSVFileData& csv_file_data, std::ofstream& log_file);