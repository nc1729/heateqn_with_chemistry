# heateqn_with_chemistry
Solver for the heat equation for a particle in an environment with steadily rising temperature, with chemical kinetics modelled inside the particle.

## Overview
This is an implementation of a finite difference method solver for the heat equation on various different geometries (sphere, cylinder, cuboid.) The domain is a particle in an environment where the temperature steadily rises (inside a pyrolysis oven.) Chemistry inside the particle is also modelled with an ensemble of species, each undergoing a first-order reaction with user-defined activation enthalpies, heat generated per mole of reactant, and other quantities.
Using the settings.conf file, the user may specify the duration of heating and an optional cooling phase (where the oven remains at a fixed temperature after heating and the model continues until the particle reaches equilibrium. The user may also allow physical properties of the particle, such as thermal conductivity/heat capacity to vary with temperature.

## How to use
After pulling the repository, run 'make' to create an executable in ./build/release. 'make debug' will create an executable in ./build/debug. Move the executable to a fresh folder with the settings.conf and sample_chem.csv files, and then run the executable. Output files will be generated that will contain temperature and chemistry data for all points on the mesh for each second the model is run.

## Tips for running
- Ensure `timesteps_per_second` is set high enough; about 50,000 seems to be fairly stable, but the denser your mesh, the higher this value has to be and the greater the computational cost (in the 1D case, the required number of timesteps for convergence goes as the number of gridpoints squared.)
- Edit settings.conf in the text editor of your choice. Make sure the instructions in that file are followed carefully, otherwise the relevant variables in the model may not be set correctly.
