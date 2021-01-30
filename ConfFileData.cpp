#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include "ConfFileData.h"
#include "Log.h"

ConfFileData::ConfFileData(std::string path)
{
	// Create input filestream
	std::ifstream conf_file(path);

	// Check whether .conf file is open
	try {
		//if (!conf_file.is_open()) error_write_to_console("Could not open settings.conf file.\n");
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what();
		std::cout << "Press ENTER to exit...\n";
		std::cin.get();
		exit(1);
	}

	// Put variables from file into collections of variables of same type
	std::string line;
	std::map<std::string, size_t> int_variables;
	std::map<std::string, double> double_variables;
	std::map<std::string, std::string> string_variables;
	std::map<std::string, bool> bool_variables;

	// Read all lines in .conf file
	while (std::getline(conf_file, line))
	{
		if (line.find('#') != std::string::npos || line.find('=') == std::string::npos)
		{
			// Ignore this line, it's a comment
		}
		else if (line.find('"') != std::string::npos)
		{
			// Line defines a string variable
			// Line is of the form var_name="var_value"
			std::int64_t equals_pos = line.find('=');
			std::string var_name = line.substr(0, equals_pos);
			std::string var_value = line.substr((std::int64_t)equals_pos + 1);

			// Strip " characters from string
			var_value = var_value.substr(1, var_value.length() - 2);

			// Add to pile of string variables
			string_variables.insert(std::make_pair(var_name, var_value));
		}
		else if (line.find('.') != std::string::npos)
		{
			// Line defines a double variable
			// Line is of the form var_name=var_value
			std::int64_t equals_pos = line.find('=');
			std::string var_name = line.substr(0, equals_pos);
			double var_value = stod(line.substr(equals_pos + 1));
			double_variables.insert(std::make_pair(var_name, var_value));
		}
		else if (line.find("true") != std::string::npos or line.find("false") != std::string::npos)
		{
			// Line defines a boolean variable
			// Line is of the form var_name=true or var_name=false
			std::int64_t equals_pos = line.find('=');
			std::string var_name = line.substr(0, equals_pos);
			if (line.find("true") != std::string::npos)
			{
				bool_variables.insert(std::make_pair(var_name, true));
			}
			else
			{
				bool_variables.insert(std::make_pair(var_name, false));
			}
		}
		else
		{
			// Default behaviour is to assume line defines integer variable
			// Line is of the form var_name=var_value
			std::int64_t equals_pos = line.find('=');
			std::string var_name = line.substr(0, equals_pos);
			std::int64_t var_value = stoi(line.substr(equals_pos + 1));
			int_variables.insert(std::make_pair(var_name, var_value));
		}
	}

	conf_file.close();

	// Now use constructed maps to generate ConfFileData members, raising an error
	// if we don't find what we need
	// int variables
	set_variable<size_t>(_geometry, "geometry", int_variables);
	set_variable<size_t>(_sphere_radial_meshsize, "sphere_radial_meshsize", int_variables);
	set_variable<size_t>(_cylinder_radial_meshsize, "cylinder_radial_meshsize", int_variables);
	set_variable<size_t>(_cylinder_height_meshsize, "cylinder_height_meshsize", int_variables);
	set_variable<size_t>(_x_meshsize, "cuboid_x_meshsize", int_variables);
	set_variable<size_t>(_y_meshsize, "cuboid_y_meshsize", int_variables);
	set_variable<size_t>(_z_meshsize, "cuboid_z_meshsize", int_variables);
	set_variable<size_t>(_heating_time, "heating_time", int_variables);
	set_variable<size_t>(_timesteps_per_second, "timesteps_per_second", int_variables);
	set_variable<size_t>(_significant_digits, "significant_digits", int_variables);

	// double variables
	set_variable<double>(_sphere_radius, "sphere_radius", double_variables);
	set_variable<double>(_cylinder_radius, "cylinder_radius", double_variables);
	set_variable<double>(_cylinder_height, "cylinder_height", double_variables);
	set_variable<double>(_x_length, "cuboid_x_length", double_variables);
	set_variable<double>(_y_length, "cuboid_y_length", double_variables);
	set_variable<double>(_z_length, "cuboid_z_length", double_variables);
	set_variable<double>(_max_temp, "max_temp", double_variables);
	set_variable<double>(_initial_temp, "initial_temp", double_variables);
	set_variable<double>(_heating_rate, "heating_rate", double_variables);
	set_variable<double>(_thermal_conductivity, "thermal_conductivity", double_variables);
	set_variable<double>(_specific_heat_capacity, "specific_heat_capacity", double_variables);
	set_variable<double>(_rock_density, "rock_density", double_variables);
	set_variable<double>(_kerogen_density, "kerogen_density", double_variables);
	set_variable<double>(_TOC, "TOC_percent", double_variables);

	// bool variables
	set_variable<bool>(_fixed_max_temperature, "fixed_max_temperature", bool_variables);
	set_variable<bool>(_fixed_thermal_conductivity, "fixed_thermal_conductivity", bool_variables);
	set_variable<bool>(_fixed_specific_heat_capacity, "fixed_specific_heat_capacity", bool_variables);
	set_variable<bool>(_cooling_phase, "cooling_phase", bool_variables);
	set_variable<bool>(_chemistry_on, "chemistry_on", bool_variables);

	// string variables
	set_variable<std::string>(_chemistry_file, "chemistry_file", string_variables);
	set_variable<std::string>(_log_level, "log_level", string_variables);
	set_variable<std::string>(_log_filename, "log_filename", string_variables);
}

void ConfFileData::check_input(std::ofstream& log_file)
{
	if (not ((_geometry == 1) or (_geometry == 2) or (_geometry == 3)))
	{
		Log::error_write(log_file, "Unrecognised geometry input.\n");
	}
	if (_sphere_radial_meshsize == 1)
	{
		Log::error_write(log_file, "Sphere meshsize must be greater than 1.\n");
	}
	if (_cylinder_radial_meshsize == 1)
	{
		Log::error_write(log_file, "Cylinder radial meshsize must be greater than 1.\n");
	}
	if (_cylinder_height_meshsize == 1)
	{
		Log::error_write(log_file, "Cylinder height meshsize must be greater than 1.\n");
	}
	if (_x_meshsize == 1)
	{
		Log::error_write(log_file, "x meshsize must be greater than 1.\n");
	}
	if (_y_meshsize == 1)
	{
		Log::error_write(log_file, "y meshsize must be greater than 1.\n");
	}
	if (_z_meshsize == 1)
	{
		Log::error_write(log_file, "z meshsize must be greater than 1.\n");
	}
	if (_sphere_radius < 0)
	{
		Log::error_write(log_file, "Sphere radius must be positive.\n");
	}
	if (_cylinder_radius < 0)
	{
		Log::error_write(log_file, "Cylinder radius must be positive.\n");
	}
	if (_cylinder_height < 0)
	{
		Log::error_write(log_file, "Cylinder height must be positive.\n");
	}
	if (_x_length < 0)
	{
		Log::error_write(log_file, "Cuboid length in x-direction must be positive.\n");
	}
	if (_y_length < 0)
	{
		Log::error_write(log_file, "Cuboid length in y-direction must be positive.\n");
	}
	if (_z_length < 0)
	{
		Log::error_write(log_file, "Cuboid length in z-direction must be positive.\n");
	}
	if (_max_temp < 0)
	{
		Log::error_write(log_file, "Max temperature must be positive.\n");
	}
	if (_thermal_conductivity < 0)
	{
		Log::error_write(log_file, "Thermal conductivity must be positive.\n");
	}
	if (_specific_heat_capacity < 0)
	{
		Log::error_write(log_file, "Specific heat capacity must be positive.\n");
	}
	if (_rock_density < 0)
	{
		Log::error_write(log_file, "Rock density must be positive.\n");
	}
	if (_kerogen_density < 0)
	{
		Log::error_write(log_file, "Kerogen density must be positive.\n");
	}
	if (_TOC < 0.0 or _TOC > 100.0)
	{
		Log::error_write(log_file, "TOC must be a real value between 0.0 and 100.0.\n");
	}
}
void ConfFileData::log_input(std::ofstream& log_file)
{
	log_file << "Contents of settings_v3.conf:\n";

	log_file << "--Integer variables--\n";
	log_file << "geometry=" << this->_geometry << '\n';
	log_file << "sphere_radial_meshsize=" << this->_sphere_radial_meshsize << '\n';
	log_file << "cylinder_radial_meshsize=" << this->_cylinder_radial_meshsize << '\n';
	log_file << "cylinder_height_meshsize=" << this->_cylinder_height_meshsize << '\n';
	log_file << "x_meshsize=" << this->_x_meshsize << '\n';
	log_file << "y_meshsize=" << this->_y_meshsize << '\n';
	log_file << "z_meshsize=" << this->_z_meshsize << '\n';
	log_file << "heating_time=" << this->_heating_rate << '\n';
	log_file << "timesteps_per_second=" << this->_timesteps_per_second << '\n';
	log_file << "significant_digits=" << this->_significant_digits << '\n';

	log_file << "--Double variables--\n";
	log_file << "cylinder_radius=" << this->_cylinder_radius << '\n';
	log_file << "cylinder_height=" << this->_cylinder_height << '\n';
	log_file << "x_length=" << this->_x_length << '\n';
	log_file << "y_length=" << this->_y_length << '\n';
	log_file << "z_length=" << this->_z_length << '\n';
	log_file << "max_temp=" << this->_max_temp << '\n';
	log_file << "initial_temp=" << this->_initial_temp << '\n';
	log_file << "heating_rate=" << this->_heating_rate << '\n';
	log_file << "thermal_conductivity=" << this->_thermal_conductivity << '\n';
	log_file << "specific_heat_capacity=" << this->_specific_heat_capacity << '\n';
	log_file << "rock_density=" << this->_rock_density << '\n';
	log_file << "kerogen_density=" << this->_kerogen_density << '\n';
	log_file << "TOC=" << this->_TOC << '\n';

	log_file << "--Bool variables--\n";
	log_file << "fixed_max_temperature=" << this->_fixed_max_temperature << '\n';
	log_file << "fixed_thermal_conductivity=" << this->_fixed_thermal_conductivity << '\n';
	log_file << "fixed_specific_heat_capacity=" << this->_fixed_specific_heat_capacity << '\n';
	log_file << "cooling_phase=" << this->_cooling_phase << '\n';
	log_file << "chemistry_on=" << this->_chemistry_on << '\n';

	log_file << "--String variables--\n";
	log_file << "chemistry_file=" << this->_chemistry_file << '\n';
	log_file << "log_level=" << this->_log_level << '\n';
	log_file << "log_filename=" << this->_log_filename << '\n';
}