const double T0 = 288.15;     // K
const double P0 = 101325.0;   // Pa
const double g0 = 9.80665;    // m/s²
const double R  = 287.05;     // J/(kg·K)

#include <cmath>

struct Atmosphere {
    double T;   // Temperature (K)
    double P;   // Pressure (Pa)
    double rho; // Density (kg/m^3)
};

Atmosphere isa(double h) {
    const double T0 = 288.15;
    const double P0 = 101325.0;
    const double g0 = 9.80665;
    const double R  = 287.05;

    Atmosphere atm;

    if (h <= 11000.0) {
        // Troposphere
        double L = -0.0065;

        atm.T = T0 + L * h;
        atm.P = P0 * pow(atm.T / T0, -g0 / (R * L));
    }
    else {
        // Tropopause (11 km ~ 20 km)
        double T11 = 216.65;  // K
        double P11 = 22632.1; // Pa

        atm.T = T11;
        atm.P = P11 * exp(-g0 * (h - 11000.0) / (R * T11));
    }

    atm.rho = atm.P / (R * atm.T);

    return atm;
}

double altitude = 5000.0; // meters

Atmosphere atm = isa(altitude);

double rho = atm.rho;   // for drag
double T   = atm.T;
double P   = atm.P;

drag = 0.5 * atm.rho * V * V * Cd * A;
