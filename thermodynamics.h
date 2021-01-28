#pragma once

// Sekiguchi-Waples model for thermal conductivity (eqn (3.10) in Hantschel & Kauerauf)
double waples_thermal_conductivity(double thermal_cond_at_20, double temp);
// Waples model for heat capacity (eqn (3.21) in Hantschel & Kauerauf)
double waples_heat_capacity(double heat_cap_at_20, double temp);