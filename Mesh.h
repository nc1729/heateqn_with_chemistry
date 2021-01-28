#pragma once
#include <vector>
#include <string>
#include "ConfFileData.h"

class Mesh
{
protected:
	// Mesh variables
	size_t _mesh_size;
	std::vector<double> _mesh_data;
	std::string _mesh_name;

	// File variables
	std::vector<std::string> _filenames;
	bool _files_ready = false;

public:
	Mesh(size_t const mesh_size, std::string const& mesh_name);
	Mesh(Mesh const& mesh);

	double& operator[](size_t const index);
	const double& operator[](size_t const index) const;

	size_t size() const;
	void fill(double const value);
	double virtual laplacian(size_t const index) const;
	bool nearly_equal(Mesh& other);
	bool is_nan() const;
};

class SphereMesh : public Mesh
{
protected:
	// Mesh variables
	double _radius;
	size_t _radial_meshsize;
	double _dr;

	// At centre array
	std::vector<bool> _at_centre;

	// On boundary array
	std::vector<bool> _on_boundary;

	// operator() for internal use
	double& operator()(size_t const i);

	// easier semantics for accessing at_centre array
	bool is_at_centre(size_t const i) const;

public:
	SphereMesh(ConfFileData& conf_file_data, std::string const& mesh_name);
	SphereMesh(SphereMesh const& mesh);
	double laplacian(size_t const index) const;
	void setup_files();
	void write_files(size_t const second_count, size_t const sig_figs);
	// easier semantics for accessing on_boundary array
	bool is_on_boundary(size_t const n) const;
};

class CylinderMesh : public Mesh
{
protected:
	// Mesh variables
	double _radius;
	double _height;
	size_t _radial_meshsize;
	size_t _height_meshsize;
	double _dr;
	double _dz;

	// At centre array
	std::vector<bool> _at_centre;

	// On boundary array
	std::vector<bool> _on_boundary;

	// operator() for internal use
	double& operator()(size_t const i, size_t const j);

	// easier semantics for accessing at_centre array
	bool is_at_centre(size_t const i, size_t const j) const;

public:
	CylinderMesh(ConfFileData& conf_file_data, std::string const& mesh_name);
	CylinderMesh(CylinderMesh const& mesh);
	double laplacian(size_t const index) const;
	void setup_files();
	void write_files(size_t const second_count, size_t const sig_figs);
	// easier semantics for accessing on_boundary array
	bool is_on_boundary(size_t const n) const;
};

class CuboidMesh : public Mesh
{
protected:
	// Mesh variables
	double _x_length;
	double _y_length;
	double _z_length;
	size_t _x_meshsize;
	size_t _y_meshsize;
	size_t _z_meshsize;
	double _dx;
	double _dy;
	double _dz;

	// on boundary array
	std::vector<bool> _on_boundary;

	// operator() for internal use
	double& operator()(size_t const i, size_t const j, size_t const k);


public:
	CuboidMesh(ConfFileData& conf_file_data, std::string const& mesh_name);
	CuboidMesh(CuboidMesh const& mesh);
	double laplacian(size_t const index) const;
	void setup_files();
	void write_files(size_t const second_count, size_t const sig_figs);
	// easier semantics for accessing on_boundary array
	bool is_on_boundary(size_t const n) const;
};