#include "test.h"
#include "ConfFileData.h"
#include <string>
#include <iostream>
#include <fstream>

void conf_file_test(std::string path)
{
	// read a conf file
	ConfFileData conf_file(path);

	std::cout << "Geometry = " << conf_file._geometry;
}

void log_test(std::ofstream& log_file)
{
	log_file << "Does this overwrite everything?\n";
}