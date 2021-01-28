#include <cmath>
#include <vector>
#include <iostream>
#include "ChemSpecies.h"

// Constructor
ChemSpecies::ChemSpecies(double A, double Ea, double proportion, double dH, double molar_mass)
{
	m_A = A;
	m_Ea = Ea;
	m_proportion = proportion;
	m_dH = dH;
	m_molar_mass = molar_mass;
}

/*
alpha, in joules per kg, is the amount of energy produced by converting
1 kg of this kerogen.
dH < 0 implies exothermic, dH > 0 is endothermic.
*/
double ChemSpecies::alpha()
{
	return m_dH * (m_proportion/100) / m_molar_mass;
}

double ChemSpecies::k(double temp)
{
	return m_A * exp(-m_Ea / (R * temp));
}

double ChemSpecies::proportion()
{
	return m_proportion;
}

void ChemSpecies::print()
{
	std::cout << "Chemical species" << '\n';
	std::cout << "A = " << m_A << '\n';
	std::cout << "Ea = " << m_Ea << '\n';
	std::cout << "proportion = " << m_proportion << '\n';
	std::cout << "dH = " << m_dH << '\n';
	std::cout << "molar mass = " << m_molar_mass << '\n';
}