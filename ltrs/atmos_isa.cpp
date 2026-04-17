#include <cmath>

const double T0 = 288.15;     // K
const double P0 = 101325.0;   // Pa
const double g0 = 9.80665;    // m/s²
const double R  = 287.05;     // J/(kg·K)

struct Vec3 {
    double x, y, z;

    // Constructors
    Vec3() : x(0.0), y(0.0), z(0.0) {}
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    // norm (magnitude)
    double norm() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    // normalize (unit vector)
    Vec3 normalize() const {
        double n = norm();
        if (n < 1e-8) return {0,0,0};
        return {x/n, y/n, z/n};
    }

    // operators
    Vec3 operator+(const Vec3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    Vec3 operator-(const Vec3& v) const { return {x-v.x, y-v.y, z-v.z}; }
    Vec3 operator*(double s) const { return {x*s, y*s, z*s}; }
};

struct Atmosphere {
    double T;   // Temperature (K)
    double P;   // Pressure (Pa)
    double rho; // Density (kg/m^3)
};

Atmosphere isa(double h) {
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

double Cd_table(double M) {
    if (M < 0.8) return 0.3;
    if (M < 1.2) return 0.6;  // drag rise
    if (M < 3.0) return 0.25;
    return 0.2;
}

double altitude = 5000.0; // meters

Atmosphere atm = isa(altitude);

double rho = atm.rho;   // for drag
double T   = atm.T;
double P   = atm.P;

Vec3 V_rel = V_body - V_wind;
double V = V_rel.norm();

// Speed of sound
double a = sqrt(1.4 * 287.05 * atm.T);

// Mach number
double M = V / a;

// Drag coefficient
double Cd = Cd_table(M);

// Drag force
Vec3 drag = -0.5 * atm.rho * V * V * Cd * A * V_rel.normalize();
