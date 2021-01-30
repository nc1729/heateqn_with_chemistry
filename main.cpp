#include "ConfFileData.h"
#include "CSVFileData.h"
#include "test.h"
#include "Log.h"
#include "heateqn_solver.h"
#include "Mesh.h"
#include <fstream>

int main()
{
	// Read in file data
	ConfFileData conf_file_data("settings.conf");
	CSVFileData csv_file_data(conf_file_data._chemistry_file);

	// Set up log file and dump conf_file_data to log
	std::ofstream log_file(conf_file_data._log_filename);
	Log::write(log_file, "settings.conf and " + conf_file_data._chemistry_file + " read successfully.\n");
	Log::write_to_console("Writing read-in settings to log file: " + conf_file_data._log_filename + '\n');
	conf_file_data.log_input(log_file);
	
	// check input is OK
	conf_file_data.check_input(log_file);

	// Run simulation
	if (conf_file_data._geometry == 1)
	{
		heateqn_solver<SphereMesh>(conf_file_data, csv_file_data, log_file);
	}
	else if (conf_file_data._geometry == 2)
	{
		heateqn_solver<CylinderMesh>(conf_file_data, csv_file_data, log_file);
	}
	else if (conf_file_data._geometry == 3)
	{
		heateqn_solver<CuboidMesh>(conf_file_data, csv_file_data, log_file);
	}
	
	Log::write_to_console_and_quit("Finished!\n");
}