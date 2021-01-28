#include "ChemSpecies.h"
#include "CSVFileData.h"
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

CSVFileData::CSVFileData(std::string path)
{
	// Create input filestream
	std::ifstream csv_file(path);

	// Check whether file is open
	if (!csv_file.is_open()) throw std::runtime_error("Could not open csv file.");

	// line is a place to put lines read by program
	// species_list is the output vector of ChemSpecies objects.
	std::string line;

	// Read the first line and ignore it (just labels)
	std::getline(csv_file, line);
	line.clear();

	double file_A{ 0.0 };
	double file_Ea{ 0.0 };
	double file_proportion{ 0.0 };
	double file_dH{ 0.0 };
	double file_molar_mass{ 0.0 };
	std::string str_double;

	// Now read all other lines
	while (std::getline(csv_file, line))
	{
		// Make the input line a stream of its own
		std::stringstream linestream(line);

		std::getline(linestream, str_double, ',');
		file_A = stod(str_double);

		std::getline(linestream, str_double, ',');
		file_Ea = stod(str_double);

		std::getline(linestream, str_double, ',');
		file_proportion = stod(str_double);

		std::getline(linestream, str_double, ',');
		file_dH = stod(str_double);

		std::getline(linestream, str_double, ',');
		file_molar_mass = stod(str_double);

		this->_chem_array.push_back(ChemSpecies(file_A, file_Ea * 4184, file_proportion,
			file_dH, file_molar_mass));
	}

	// Check proportions sum to 100.0 (or, at least, are close!)
	double proportion_sum = 0.0;
	for (auto& species : _chem_array)
	{
		proportion_sum += species.proportion();
	}
	if (abs(proportion_sum - 100.0) > pow(2, -32))
	{
		//error_write_to_console("Chemical proportions don't sum to 100% - cancelling.\n");
	}

	csv_file.close();
}

size_t CSVFileData::number_of_species()
{
	return _chem_array.size();
}