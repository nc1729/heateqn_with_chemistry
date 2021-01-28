double waples_thermal_conductivity(double thermal_cond_at_20, double temp)
{
	return 358 * (1.0227 * thermal_cond_at_20 - 1.882) * (1 / temp - 0.00068) + 1.84;
}
double waples_heat_capacity(double heat_cap_at_20, double temp)
{
	return heat_cap_at_20 * (0.953 + 2.29e-03 * (temp - 273.15)
		- 2.835e-06 * (temp - 273.15) * (temp - 273.15) + 1.191e-09 * (temp - 273.15) * (temp - 273.15) * (temp - 273.15));
}