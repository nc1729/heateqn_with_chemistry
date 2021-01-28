#pragma once
#include <map>
#include <string>
#include <fstream>

struct ConfFileData
{
	// Domain settings
	size_t _geometry;
	
	double _sphere_radius;
	size_t _sphere_radial_meshsize;

	double _cylinder_radius;
	double _cylinder_height;
	size_t _cylinder_radial_meshsize;
	size_t _cylinder_height_meshsize;

	double _x_length;
	double _y_length;
	double _z_length;
	size_t _x_meshsize;
	size_t _y_meshsize;
	size_t _z_meshsize;

	// Time settings
	size_t _heating_time;
	size_t _timesteps_per_second;

	// Physics settings
	bool _fixed_max_temperature;
	bool _fixed_thermal_conductivity;
	bool _fixed_specific_heat_capacity;
	double _max_temp;
	double _initial_temp;
	double _heating_rate;
	bool _cooling_phase;
	double _thermal_conductivity;
	double _specific_heat_capacity;
	double _rock_density;

	// Chemistry settings
	bool _chemistry_on;
	std::string _chemistry_file;
	double _kerogen_density;
	double _TOC;

	// Output settings
	size_t _significant_digits;

	// Logging settings
	std::string _log_level;
	std::string _log_filename;

	// Constructor
	ConfFileData(std::string path);

	void check_input(std::ofstream& log_file);
	void log_input(std::ofstream& log_file);
};

template <typename T>
void set_variable(T& var, std::string varname, std::map<std::string, T> varlist)
{
	auto search = varlist.find(varname);
	if (search != varlist.end())
	{
		var = search->second;
	}
	else
	{
		//std::string err_msg = "Variable: " + std::string(var) + " not found in settings.conf.";
		//error_write_to_console(err_msg);
	}
}