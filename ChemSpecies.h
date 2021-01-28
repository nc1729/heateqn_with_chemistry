#pragma once
#include <cmath>

class ChemSpecies
{
private:
	double m_A;
	double m_Ea;
	double m_proportion;
	double m_dH;
	double m_molar_mass;
	double m_amount_remaining = 1.0;
	double R = 8.314472;

public:
	ChemSpecies(double A, double Ea, double proportion, double dH, double molar_mass);
	double alpha();
	double k(double temp);
	double proportion();
	void print();
};