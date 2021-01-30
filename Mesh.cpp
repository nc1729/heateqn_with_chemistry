#include "Mesh.h"
#include "ConfFileData.h"
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <cmath>
#include <iomanip> // for std::setprecision

Mesh::Mesh(size_t const mesh_size, std::string const& mesh_name)
{
	_mesh_size = mesh_size;
	_mesh_data.resize(_mesh_size, 0.0);
	_mesh_name = mesh_name;
}
Mesh::Mesh(Mesh const& mesh)
{
	_mesh_size = mesh._mesh_size;
	_mesh_data = mesh._mesh_data;
	_mesh_name = mesh._mesh_name;

	_filenames = mesh._filenames;
	_files_ready = mesh._files_ready;
}

double& Mesh::operator[](size_t const index)
{
	return _mesh_data[index];
}
const double& Mesh::operator[](size_t const index) const
{
	return _mesh_data[index];
}

size_t Mesh::size() const
{
	return _mesh_size;
}
void Mesh::fill(double const value)
{
	std::fill(_mesh_data.begin(), _mesh_data.end(), value);
}
double Mesh::laplacian(size_t const index) const
{
	if (index)
	{
		// gcc complains if index isn't used
	}
	return 0.0;
}
bool Mesh::nearly_equal(Mesh& other)
{
	if (_mesh_size != other.size())
	{
		return false;
	}

	double sum_of_square_diffs = 0.0;
	for (size_t n = 0; n < _mesh_size; n++)
	{
		sum_of_square_diffs += ((*this)[n] - other[n]) * ((*this)[n] - other[n]);
	}
	return (sum_of_square_diffs < std::pow(10, -12));
}
bool Mesh::is_nan() const
{
	// check all points in mesh for NaN
	for (auto& point : _mesh_data)
	{
		if (std::isnan(point)) return true;
	}
	return false;
}

/*
SphereMesh
*/

double& SphereMesh::operator()(size_t const i)
{
	return _mesh_data[i];
}
bool SphereMesh::is_at_centre(size_t const i) const
{
	return _at_centre[i];
}
bool SphereMesh::is_on_boundary(size_t const i) const
{
	return _on_boundary[i];
}

SphereMesh::SphereMesh(ConfFileData& conf_file_data, std::string const& mesh_name)
	: Mesh(conf_file_data._sphere_radial_meshsize, mesh_name)
{
	_radius = conf_file_data._sphere_radius;
	_radial_meshsize = conf_file_data._sphere_radial_meshsize;
	_dr = _radius / (1000000 * (_radial_meshsize - 1));

	_at_centre.resize(_mesh_size, false);
	_on_boundary.resize(_mesh_size, false);

	// determine centre points
	_at_centre[0] = true;

	// determine boundary points
	_on_boundary[_radial_meshsize - 1] = true;
}
SphereMesh::SphereMesh(SphereMesh const& mesh)
	: Mesh(mesh)
{
	_radius = mesh._radius;
	_radial_meshsize = mesh._radial_meshsize;
	_dr = mesh._dr;

	_at_centre = mesh._at_centre;
	_on_boundary = mesh._on_boundary;
}

double SphereMesh::laplacian(size_t const index) const
{
	if (is_on_boundary(index))
	{
		throw std::runtime_error("Laplace operator: invalid index " + std::to_string(index) + '\n');
	}
	else
	{
		if (is_at_centre(index))
		{
			return 6 * (_mesh_data[index + 1] - _mesh_data[index]) / (_dr * _dr);
		}
		else
		{
			return (_mesh_data[index + 1] - 2 * _mesh_data[index] + _mesh_data[index - 1]) / (_dr * _dr)
				+ 2 * (_mesh_data[index + 1] - _mesh_data[index]) / (index * _dr * _dr);
		}
	}
}

void SphereMesh::setup_files()
{
	// create filename
	std::string filename = _mesh_name + ".csv";
	_filenames.push_back(filename);

	// open filename
	std::ofstream output_file(filename);

	// write top row
	output_file << ",Distance from centre (microns)";
	for (size_t i = 0; i < _radial_meshsize - 1; i++)
	{
		output_file << ',';
	}
	output_file << '\n';

	// write second row (containing values of r in microns)
	output_file << "Time (s),";
	for (size_t i = 0; i < _radial_meshsize - 1; i++)
	{
		output_file << i * _dr * 1000000 << ',';
	}
	output_file << (_radial_meshsize - 1) * _dr * 1000000 << '\n';

	_files_ready = true;
}

void SphereMesh::write_files(size_t const second_count, size_t sig_figs)
{
	if (_files_ready)
	{
		// loop over files
		for (auto& filename : _filenames)
		{
			// open file to append
			std::ofstream output_file(filename, std::ofstream::app);
			output_file << std::setprecision(sig_figs);

			// write to file
			output_file << second_count << ',';
			for (size_t i = 0; i < _radial_meshsize - 1; i++)
			{
				output_file << (*this)(i) << ',';
			}
			output_file << (*this)(_radial_meshsize - 1) << '\n';
		}
	}
	else
	{
		throw std::runtime_error("Files not set up.\n");
	}
}

/*
CylinderMesh
*/

double& CylinderMesh::operator()(size_t const i, size_t const j)
{
	return _mesh_data[i * _height_meshsize + j];
}
bool CylinderMesh::is_at_centre(size_t const i, size_t const j) const
{
	return _at_centre[i * _height_meshsize + j];
}
bool CylinderMesh::is_on_boundary(size_t const n) const
{
	return _on_boundary[n];
}

CylinderMesh::CylinderMesh(ConfFileData& conf_file_data, std::string const& mesh_name) 
	: Mesh(conf_file_data._cylinder_radial_meshsize * conf_file_data._cylinder_height_meshsize, mesh_name)
{
	_radius = conf_file_data._cylinder_radius;
	_height = conf_file_data._cylinder_height;
	_radial_meshsize = conf_file_data._cylinder_radial_meshsize;
	_height_meshsize = conf_file_data._cylinder_height_meshsize;
	_dr = _radius / (1000000 * (_radial_meshsize - 1));
	_dz = _height / (1000000 * (_height_meshsize - 1));

	_at_centre.resize(_mesh_size, false);
	_on_boundary.resize(_mesh_size, false);

	// define centre points
	for (size_t j = 0; j < _height_meshsize; j++)
	{
		_at_centre[j] = true;
	}

	// define boundary points
	for (size_t i = 0; i < _radial_meshsize; i++)
	{
		// bottom of cylinder
		_on_boundary[i * _height_meshsize] = true;
		// top of cylinder
		_on_boundary[i * _height_meshsize + _height_meshsize - 1] = true;
	}
	for (size_t j = 0; j < _height_meshsize; j++)
	{
		// curved edge of cylinder
		_on_boundary[(_radial_meshsize - 1) * _height_meshsize + j] = true;
	}
}
CylinderMesh::CylinderMesh(CylinderMesh const& mesh)
	: Mesh(mesh)
{
	_radius = mesh._radius;
	_height = mesh._height;
	_radial_meshsize = mesh._radial_meshsize;
	_height_meshsize = mesh._height_meshsize;
	_dr = mesh._dr;
	_dz = mesh._dz;

	_at_centre = mesh._at_centre;
	_on_boundary = mesh._on_boundary;
	

}

double CylinderMesh::laplacian(size_t const index) const
{
	size_t i = index / _height_meshsize;
	size_t j = index % _height_meshsize;

	if (is_on_boundary(index))
	{
		throw std::runtime_error("Incorrect index entered for mesh.\n");
	}
	else
	{
		// Useful variables
		size_t cell_position = i * _height_meshsize + j;
		size_t radius_plus = (i + 1) * _height_meshsize + j;
		size_t height_plus = i * _height_meshsize + j + 1;
		size_t height_minus = i * _height_meshsize + j - 1;

		if (is_at_centre(i, j))
		{
			// polar contribution (different at r = 0)
			double laplace_polar_part =
				4 * (_mesh_data[radius_plus] - _mesh_data[cell_position]) / (_dr * _dr);

			// axial contribution
			double laplace_axial_part =
				(_mesh_data[height_plus] - 2 * _mesh_data[cell_position] + _mesh_data[height_minus]) / (_dz * _dz);

			return laplace_polar_part + laplace_axial_part;
		}
		else
		{
			// Extra useful variable
			size_t radius_minus = (i - 1) * _height_meshsize + j;

			// polar contribution
			double laplace_polar_part =
				(_mesh_data[radius_plus] - 2 * _mesh_data[cell_position] + _mesh_data[radius_minus]) / (_dr * _dr)
				+ (_mesh_data[radius_plus] - _mesh_data[cell_position]) / (i * _dr * _dr);

			// axial contribution
			double laplace_axial_part =
				(_mesh_data[height_plus] - 2 * _mesh_data[cell_position] + _mesh_data[height_minus]) / (_dz * _dz);

			return laplace_polar_part + laplace_axial_part;
		}
	}
}

void CylinderMesh::setup_files()
{
	for (size_t j = 0; j < _height_meshsize; j++)
	{
		// create filename
		double const z = j * _dz;
		std::string filename = _mesh_name + "_z=" + std::to_string(z) + "m.csv";
		_filenames.push_back(filename);

		// open file
		std::ofstream output_file(filename);

		// write top row
		output_file << ",Distance from centre (microns)";
		for (size_t i = 0; i < _radial_meshsize - 1; i++)
		{
			output_file << ',';
		}
		output_file << '\n';

		// write second row (containing values of r, in microns)
		output_file << "Time (s),";
		for (size_t i = 0; i < _radial_meshsize - 1; i++)
		{
			output_file << i * _dr * 1000000 << ',';
		}

		output_file << (_radial_meshsize - 1) * _dr * 1000000 << '\n';
	}

	// files set up and ready
	_files_ready = true;
}
void CylinderMesh::write_files(size_t const second_count, size_t const sig_figs)
{
	if (_files_ready)
	{
		// loop over files
		size_t index = 0;
		for (auto& filename : _filenames)
		{
			// open file to append
			std::ofstream output_file(filename, std::ofstream::app);
			output_file << std::setprecision(sig_figs);

			// write to file
			output_file << second_count << ',';
			for (size_t i = 0; i < _radial_meshsize - 1; i++)
			{
				output_file << (*this)(i, index % _height_meshsize) << ',';
			}
			output_file << (*this)(_radial_meshsize - 1, index % _height_meshsize) << '\n';
			index++;
		}
	}
	else
	{
		throw std::runtime_error("Files not set up.\n");
	}
}

/*
CuboidMesh
*/
double& CuboidMesh::operator()(size_t const i, size_t const j, size_t const k)
{
	return _mesh_data[i * _y_meshsize * _z_meshsize + j * _z_meshsize + k];
}
bool CuboidMesh::is_on_boundary(size_t const n) const
{
	return _on_boundary[n];
}

CuboidMesh::CuboidMesh(ConfFileData& conf_file_data, std::string const& mesh_name)
	: Mesh(conf_file_data._x_meshsize * conf_file_data._y_meshsize * conf_file_data._z_meshsize, mesh_name)
{
	_x_length = conf_file_data._x_length;
	_y_length = conf_file_data._y_length;
	_z_length = conf_file_data._z_length;
	_x_meshsize = conf_file_data._x_meshsize;
	_y_meshsize = conf_file_data._y_meshsize;
	_z_meshsize = conf_file_data._z_meshsize;

	// dx, dy, dz are in metres rather than microns
	_dx = _x_length / (1000000 * (_x_meshsize - 1));
	_dy = _y_length / (1000000 * (_y_meshsize - 1));
	_dz = _z_length / (1000000 * (_z_meshsize - 1));

	// mark boundary points
	_on_boundary.resize(_mesh_size, false);
	for (size_t i = 0; i < _x_meshsize; i++)
	{
		for (size_t j = 0; j < _y_meshsize; j++)
		{
			for (size_t k = 0; k < _z_meshsize; k++)
			{
				if (i == 0 or j == 0 or k == 0 or i == _x_meshsize - 1 or j == _y_meshsize - 1 or k == _z_meshsize - 1)
				{
					_on_boundary[i * _y_meshsize * _z_meshsize + j * _z_meshsize + k] = true;
				}
			}
		}
	}
}
CuboidMesh::CuboidMesh(CuboidMesh const& mesh)
	: Mesh(mesh)
{
	_x_length = mesh._x_length;
	_y_length = mesh._y_length;
	_z_length = mesh._z_length;
	_x_meshsize = mesh._x_meshsize;
	_y_meshsize = mesh._y_meshsize;
	_z_meshsize = mesh._z_meshsize;

	_dx = mesh._dx;
	_dy = mesh._dy;
	_dz = mesh._dz;

	_on_boundary = mesh._on_boundary;
}

double CuboidMesh::laplacian(size_t const index) const
{
	size_t i = (index / _z_meshsize) / _y_meshsize;
	size_t j = (index / _z_meshsize) % _y_meshsize;
	size_t k = index % _z_meshsize;

	if (is_on_boundary(index))
	{
		throw std::runtime_error("Laplace operator not defined on boundary.\n");
	}
	else
	{
		// Useful variables
		size_t cell_position = i * _y_meshsize * _z_meshsize + j * _z_meshsize + k;
		size_t x_plus = (i + 1) * _y_meshsize * _z_meshsize + j * _z_meshsize + k;
		size_t x_minus = (i - 1) * _y_meshsize * _z_meshsize + j * _z_meshsize + k;
		size_t y_plus = i * _y_meshsize * _z_meshsize + (j + 1) * _z_meshsize + k;
		size_t y_minus = i * _y_meshsize * _z_meshsize + (j - 1) * _z_meshsize + k;
		size_t z_plus = i * _y_meshsize * _z_meshsize + j * _z_meshsize + k + 1;
		size_t z_minus = i * _y_meshsize * _z_meshsize + j * _z_meshsize + k - 1;

		double laplace_x_part =
			(_mesh_data[x_plus] - 2 * _mesh_data[cell_position] + _mesh_data[x_minus]) / (_dx * _dx);
		double laplace_y_part =
			(_mesh_data[y_plus] - 2 * _mesh_data[cell_position] + _mesh_data[y_minus]) / (_dy * _dy);
		double laplace_z_part =
			(_mesh_data[z_plus] - 2 * _mesh_data[cell_position] + _mesh_data[z_minus]) / (_dz * _dz);

		return laplace_x_part + laplace_y_part + laplace_z_part;
	}
}
void CuboidMesh::setup_files()
{
	for (size_t i = 0; i < _x_meshsize; i++)
	{
		for (size_t j = 0; j < _y_meshsize; j++)
		{
			// create filename
			double const x = i * _dx;
			double const y = j * _dy;
			std::string filename = _mesh_name + "_x=" + std::to_string(x)
				+ "m,y=" + std::to_string(y) + "m.csv";
			_filenames.push_back(filename);

			// open file
			std::ofstream output_file(filename);

			// write top row
			output_file << ",Distance along z-axis (microns)";
			for (size_t k = 0; k < _z_meshsize - 1; k++)
			{
				output_file << ',';
			}
			output_file << '\n';

			// write second row (containing values of z, in microns)
			output_file << "Time (s),";
			for (size_t k = 0; k < _z_meshsize - 1; k++)
			{
				output_file << k * _dz * 1000000 << ',';
			}
			output_file << (_z_meshsize - 1) * _dz * 1000000 << '\n';
		}
	}
	_files_ready = true;
}
void CuboidMesh::write_files(size_t const second_count, size_t const sig_figs)
{
	if (_files_ready)
	{
		size_t index = 0;
		for (auto& filename : _filenames)
		{
			size_t i = index / _y_meshsize;
			size_t j = index % _y_meshsize;

			// open file to append
			std::ofstream output_file(filename, std::ofstream::app);
			output_file << std::setprecision(sig_figs);

			// write to file
			output_file << second_count << ',';
			for (size_t k = 0; k < _z_meshsize; k++)
			{
				output_file << (*this)(i, j, k) << ',';
			}
			output_file << (*this)(i, j, _z_meshsize - 1) << '\n';
			index++;
		}
		
	}
	else
	{
		throw std::runtime_error("Files not set up.\n");
	}
}