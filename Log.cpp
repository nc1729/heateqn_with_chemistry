#include <fstream>
#include <iostream>
#include <string>
#include "Log.h"


void Log::write(std::ofstream& log_file, std::string msg)
{
	log_file << msg;
	std::cout << msg;
}

void Log::write_to_console(std::string console_msg)
{
	std::cout << console_msg;
}
void Log::write_to_console_and_quit(std::string console_msg)
{
	std::cout << console_msg;
	std::cout << "Press ENTER to exit.\n";
	std::cin.get();
	exit(0);
}

void Log::write_to_log_file(std::ofstream& log_file, std::string log_msg)
{
	log_file << log_msg;
}

void Log::error_write(std::ofstream& log_file, std::string err_msg)
{
	log_file << "[ERROR]: " << err_msg;
	std::cout << "[ERROR]: " << err_msg;
	std::cout << "Press ENTER to exit.\n";
	std::cin.get();
	exit(1);
}