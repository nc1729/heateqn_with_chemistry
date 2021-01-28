#pragma once
#include <fstream>
#include <string>

namespace Log
{
	void write(std::ofstream& log_file, std::string msg);
	void write_to_console(std::string console_msg);
	void write_to_console_and_quit(std::string console_msg);
	void write_to_log_file(std::ofstream& log_file, std::string log_msg);
	void error_write(std::ofstream& log_file, std::string err_msg);
}