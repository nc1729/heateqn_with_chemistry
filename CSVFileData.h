#pragma once
#include <vector>
#include "ChemSpecies.h"
#include <string>

class CSVFileData
{
public:
	std::vector<ChemSpecies> _chem_array;
	CSVFileData(std::string path);
	size_t number_of_species();
};
