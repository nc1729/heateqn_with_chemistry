#include "ConfFileData.h"
#include "CSVFileData.h"
#include "ChemSpecies.h"
#include "Mesh.h"
#include "Log.h"
#include "thermodynamics.h"
#include <fstream>
#include <chrono>

template <typename M>
void heateqn_solver(ConfFileData& cf, CSVFileData& csv_file_data, std::ofstream& log_file)
{
	/* 
	Set up temperature, thermodynamical and chemistry meshes
	*/
	// temperature mesh
	M temp(cf, "output_temp");
	temp.fill(cf._initial_temp + 273.15);

	// thermodynamical meshes
	// thermal conductivity
	M thermal_conductivity(cf, "thermal_conductivity");
	if (not cf._fixed_thermal_conductivity)
	{
		for (size_t i = 0; i < thermal_conductivity.size(); i++)
		{
			thermal_conductivity[i] = waples_thermal_conductivity(cf._thermal_conductivity, temp[i]);
		}
	}
	else
	{
		thermal_conductivity.fill(cf._thermal_conductivity);
	}

	// specific heat capacity
	M heat_capacity(cf, "specific_heat_capacity");
	if (not cf._fixed_specific_heat_capacity)
	{
		for (size_t i = 0; i < heat_capacity.size(); i++)
		{
			heat_capacity[i] = waples_heat_capacity(cf._specific_heat_capacity, temp[i]);
		}
	}
	else
	{
		heat_capacity.fill(cf._specific_heat_capacity);
	}

	M thermal_diffusivity(cf, "thermal_diffusivity");
	if (not (cf._fixed_thermal_conductivity or cf._fixed_specific_heat_capacity))
	{
		// calculate thermal diffusivity
		for (size_t i = 0; i < thermal_diffusivity.size(); i++)
		{
			thermal_diffusivity[i] = thermal_conductivity[i] / (cf._rock_density * heat_capacity[i]);
		}
	}
	else
	{
		thermal_diffusivity.fill(
			cf._thermal_conductivity / (cf._rock_density * cf._specific_heat_capacity));
	}

	// chemistry meshes
	std::vector<ChemSpecies> chem_species_array = csv_file_data._chem_array;
	std::vector<M> chem_meshes;
	if (cf._chemistry_on)
	{
		for (size_t species = 0; species < csv_file_data.number_of_species(); species++)
		{
			chem_meshes.push_back(M(cf, "output_chem" + std::to_string(species + 1)));
			chem_meshes[species].fill(1.0);
		}
	}

	/*
	Set up files for logging of output and write initial state to them
	*/

	Log::write(log_file, "Mesh initiailisations successful.\n");

	temp.setup_files();
	temp.write_files(0, cf._significant_digits);

	if (not cf._fixed_specific_heat_capacity)
	{
		heat_capacity.setup_files();
		heat_capacity.write_files(0, cf._significant_digits);
	}

	if (not cf._fixed_thermal_conductivity)
	{
		thermal_conductivity.setup_files();
		thermal_conductivity.write_files(0, cf._significant_digits);
	}

	if (cf._chemistry_on)
	{
		for (size_t species = 0; species < csv_file_data.number_of_species(); species++)
		{
			chem_meshes[species].setup_files();
			chem_meshes[species].write_files(0, cf._significant_digits);
		}
	}
	Log::write(log_file, "Mesh files created and written to successfully.\n");

	/*
	Set up for heating time loop
	*/
	auto clock_start = std::chrono::steady_clock::now();

	// Calculate time_meshsize
	size_t time_meshsize = cf._heating_time * cf._timesteps_per_second;
	if (cf._fixed_max_temperature)
	{
		size_t timesteps_to_reach_max = 
			static_cast<size_t>(((cf._max_temp - cf._initial_temp) / (cf._heating_rate / 60)) * cf._timesteps_per_second) + 1;
		if (timesteps_to_reach_max < time_meshsize)
		{
			time_meshsize = timesteps_to_reach_max;
		}
	}
	// Calculate timestep duration
	double dt = 1.0 / cf._timesteps_per_second;

	// Make new spare arrays for calculations
	M new_temp = temp;
	M new_heat_capacity = heat_capacity;
	M new_thermal_conductivity = thermal_conductivity;
	M new_thermal_diffusivity = thermal_diffusivity;
	std::vector<M> new_chem_meshes;
	if (cf._chemistry_on)
	{
		for (size_t species = 0; species < csv_file_data.number_of_species(); species++)
		{
			new_chem_meshes.push_back(chem_meshes[species]);
		}
	}

	// useful variables
	double chem_heat = 0.0;
	size_t mesh_size = temp.size();
	const size_t write_frequency = cf._timesteps_per_second;
	const size_t number_of_species = csv_file_data.number_of_species();

	/*
	Heating loop
	*/
	Log::write(log_file, "Beginning heating loop.\n");
	for (size_t timestep = 1; timestep < time_meshsize + 1; timestep++)
	{
		// loop over points in mesh
#pragma omp parallel for
		for (size_t index = 0; index < mesh_size; index++)
		{
			if (not temp.is_on_boundary(index))
			{
				// calculate heat from chemistry at this point
				chem_heat = 0.0;
				if (cf._chemistry_on)
				{
					for (size_t species = 0; species < number_of_species; species++)
					{
						chem_heat -=
							(chem_species_array[species].alpha() * (cf._TOC / 100) * cf._kerogen_density)
							* chem_species_array[species].k(temp[index]) * chem_meshes[species][index];
					}
				}

				// use heat equation to calculate new_temp at interior points
				new_temp[index] = temp[index] + thermal_diffusivity[index] * dt * temp.laplacian(index) + dt * chem_heat;

				// update thermodynamics arrays with new temps
				if (not cf._fixed_specific_heat_capacity)
				{
					new_heat_capacity[index] = waples_heat_capacity(cf._specific_heat_capacity, temp[index]);
				}
				if (not cf._fixed_thermal_conductivity)
				{
					new_thermal_conductivity[index] = waples_thermal_conductivity(cf._thermal_conductivity, temp[index]);
				}
				if (not cf._fixed_thermal_conductivity or not cf._fixed_specific_heat_capacity)
				{
					new_thermal_diffusivity[index] = new_thermal_conductivity[index] / (new_heat_capacity[index] * cf._rock_density);
				}

				// update chem arrays at interior points
				if (cf._chemistry_on)
				{
					for (size_t species = 0; species < number_of_species; species++)
					{
						new_chem_meshes[species][index] = chem_meshes[species][index] -
							dt * chem_species_array[species].k(temp[index]) * chem_meshes[species][index];
					}
				}
			}
			else
			{
				// on boundary, apply boundary condition
				new_temp[index] = temp[index] + (cf._heating_rate / 60) * dt;

				// update thermodynamic arrays on boundary
				if (not cf._fixed_specific_heat_capacity)
				{
					new_heat_capacity[index] = waples_heat_capacity(cf._specific_heat_capacity, temp[index]);
				}
				if (not cf._fixed_thermal_conductivity)
				{
					new_thermal_conductivity[index] = waples_thermal_conductivity(cf._thermal_conductivity, temp[index]);
				}
				if (not cf._fixed_thermal_conductivity or not cf._fixed_specific_heat_capacity)
				{
					new_thermal_diffusivity[index] = new_thermal_conductivity[index] / (new_heat_capacity[index] * cf._rock_density);
				}

				// update chemistry on boundary
				if (cf._chemistry_on)
				{
					for (size_t species = 0; species < number_of_species; species++)
					{
						new_chem_meshes[species][index] = chem_meshes[species][index] -
							dt * chem_species_array[species].k(temp[index]) * chem_meshes[species][index];
					}
				}
			}
		}

		// copy arrays for next timestep
		temp = new_temp;
		if (not cf._fixed_specific_heat_capacity)
		{
			heat_capacity = new_heat_capacity;
		}
		if (not cf._fixed_thermal_conductivity)
		{
			thermal_conductivity = new_thermal_conductivity;
		}
		if (not cf._fixed_thermal_conductivity or not cf._fixed_specific_heat_capacity)
		{
			thermal_diffusivity = new_thermal_diffusivity;
		}
		if (cf._chemistry_on)
		{
			for (size_t species = 0; species < number_of_species; species++)
			{
				chem_meshes[species] = new_chem_meshes[species];
			}
		}

		// if we've hit a round number of timesteps, write arrays to output files
		if (timestep % write_frequency == 0)
		{
			size_t current_model_time_secs = timestep / cf._timesteps_per_second;

			// check for divergence and raise error if we have diverged
			if (temp.is_nan())
			{
				Log::error_write(log_file, "Temperature field has diverged. Try increasing timesteps_per_second.\n");
			}

			// write to output files
			temp.write_files(current_model_time_secs, cf._significant_digits);
			if (not cf._fixed_specific_heat_capacity)
			{
				heat_capacity.write_files(current_model_time_secs, cf._significant_digits);
			}

			if (not cf._fixed_thermal_conductivity)
			{
				thermal_conductivity.write_files(current_model_time_secs, cf._significant_digits);
			}

			if (cf._chemistry_on)
			{
				for (size_t species = 0; species < number_of_species; species++)
				{
					chem_meshes[species].write_files(current_model_time_secs, cf._significant_digits);
				}
			}

			// write progress update to stdout
			auto clock_tick = std::chrono::steady_clock::now();
			std::chrono::duration<double> elapsed_seconds = clock_tick - clock_start;
			Log::write_to_console(std::to_string(current_model_time_secs)
				+ " seconds elapsed in simulation. "
				+ "Time taken so far: "
				+ std::to_string(elapsed_seconds.count())
				+ " seconds.\n");
		}
	} 

	/*
	Begin cooling loop
	*/
	if (cf._cooling_phase)
	{
		Log::write(log_file, "Beginning cooling loop.\n");
		bool equilibrium_reached = false;

		for (size_t timestep = time_meshsize + 1; not equilibrium_reached; timestep++)
		{
			// loop over all points in meshes
#pragma omp parallel for
			for (size_t index = 0; index < mesh_size; index++)
			{
				if (not temp.is_on_boundary(index))
				{
					chem_heat = 0.0;
					if (cf._chemistry_on)
					{
						for (size_t species = 0; species < number_of_species; species++)
						{
							chem_heat -=
								(chem_species_array[species].alpha() * (cf._TOC / 100) * cf._kerogen_density)
								* chem_species_array[species].k(temp[index]) * chem_meshes[species][index];
						}
					}

					// use heat equation to calculate new_temp at interior points
					new_temp[index] = temp[index] + thermal_diffusivity[index] * dt * temp.laplacian(index) + dt * chem_heat;

					// update thermodynamics arrays with new temps
					if (not cf._fixed_specific_heat_capacity)
					{
						new_heat_capacity[index] = waples_heat_capacity(cf._specific_heat_capacity, temp[index]);
					}
					if (not cf._fixed_thermal_conductivity)
					{
						new_thermal_conductivity[index] = waples_thermal_conductivity(cf._thermal_conductivity, temp[index]);
					}
					if (not cf._fixed_thermal_conductivity or not cf._fixed_specific_heat_capacity)
					{
						new_thermal_diffusivity[index] = new_thermal_conductivity[index] / (new_heat_capacity[index] * cf._rock_density);
					}

					// update chem arrays at interior points
					if (cf._chemistry_on)
					{
						for (size_t species = 0; species < number_of_species; species++)
						{
							new_chem_meshes[species][index] = chem_meshes[species][index] -
								dt * chem_species_array[species].k(temp[index]) * chem_meshes[species][index];
						}
					}
				}
				else
				{
					// on the boundary, apply boundary conditions
					new_temp[index] = temp[index];

					// update thermodynamic arrays on boundary
					if (not cf._fixed_specific_heat_capacity)
					{
						new_heat_capacity[index] = waples_heat_capacity(cf._specific_heat_capacity, temp[index]);
					}
					if (not cf._fixed_thermal_conductivity)
					{
						new_thermal_conductivity[index] = waples_thermal_conductivity(cf._thermal_conductivity, temp[index]);
					}
					if (not cf._fixed_thermal_conductivity or not cf._fixed_specific_heat_capacity)
					{
						new_thermal_diffusivity[index] = new_thermal_conductivity[index] / (new_heat_capacity[index] * cf._rock_density);
					}

					// update chemistry on boundary
					if (cf._chemistry_on)
					{
						for (size_t species = 0; species < number_of_species; species++)
						{
							new_chem_meshes[species][index] = chem_meshes[species][index] -
								dt * chem_species_array[species].k(temp[index]) * chem_meshes[species][index];
						}
					}
				}
			}

			// copy arrays for next timestep
			temp = new_temp;
			if (not cf._fixed_specific_heat_capacity)
			{
				heat_capacity = new_heat_capacity;
			}
			if (not cf._fixed_thermal_conductivity)
			{
				thermal_conductivity = new_thermal_conductivity;
			}
			if (not cf._fixed_thermal_conductivity or not cf._fixed_specific_heat_capacity)
			{
				thermal_diffusivity = new_thermal_diffusivity;
			}
			if (cf._chemistry_on)
			{
				for (size_t species = 0; species < csv_file_data.number_of_species(); species++)
				{
					chem_meshes[species] = new_chem_meshes[species];
				}
			}

			// if we've hit a round number of timesteps, check for equilibrium and write arrays to output files
			if (timestep % write_frequency == 0)
			{
				equilibrium_reached = temp.nearly_equal(new_temp);

				size_t current_model_time_secs = timestep / cf._timesteps_per_second;

				// check for divergence and raise error if we have diverged
				if (temp.is_nan())
				{
					Log::error_write(log_file, "Temperature field has diverged. Try increasing timesteps_per_second.\n");
				}

				// write to output files
				temp.write_files(current_model_time_secs, cf._significant_digits);
				if (not cf._fixed_specific_heat_capacity)
				{
					heat_capacity.write_files(current_model_time_secs, cf._significant_digits);
				}

				if (not cf._fixed_thermal_conductivity)
				{
					thermal_conductivity.write_files(current_model_time_secs, cf._significant_digits);
				}

				if (cf._chemistry_on)
				{
					for (size_t species = 0; species < csv_file_data.number_of_species(); species++)
					{
						chem_meshes[species].write_files(current_model_time_secs, cf._significant_digits);
					}
				}

				// write progress update to stdout
				auto clock_tick = std::chrono::steady_clock::now();
				std::chrono::duration<double> elapsed_seconds = clock_tick - clock_start;
				Log::write_to_console(std::to_string(current_model_time_secs)
					+ " seconds elapsed in simulation. "
					+ "Time taken so far: "
					+ std::to_string(elapsed_seconds.count())
					+ " seconds.\n");				
			}
			
		}
	}

	// simulation completed
	auto clock_tick = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = clock_tick - clock_start;
	Log::write(log_file, "Simulation completed. Time elapsed: " + std::to_string(elapsed_seconds.count()) + " seconds.\n");
}

// to keep the linker happy, need to instantiate concrete versions of the template function
template void heateqn_solver<SphereMesh>(ConfFileData&, CSVFileData&, std::ofstream&);
template void heateqn_solver<CylinderMesh>(ConfFileData&, CSVFileData&, std::ofstream&);
template void heateqn_solver<CuboidMesh>(ConfFileData&, CSVFileData&, std::ofstream&);
