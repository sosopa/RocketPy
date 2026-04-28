#pragma once
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <random>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../eigen-5.0.0/Eigen/Dense"
using namespace Eigen;

enum class AtmosMode {
    VACUUM,
    ISA,
    ERA5
};
AtmosMode atmos_mode = AtmosMode::VACUUM;

const double T0 = 288.15;     // K
const double P0 = 101325.0;   // Pa
const double g0 = 9.80665;    // m/s²
const double R  = 287.05;     // J/(kg·K)

const double MU = 3.986004418e14;       // [m^3/s^2] Earth gravitational parameter
const double OMEGA_EARTH = 7.2921159e-5; // [rad/s]
const double EARTH_RADIUS = 6371000.0;  // [m]

unsigned int seed = std::random_device{}();
std::mt19937 rng(seed);

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
    Vec3 operator+(const Vec3& v) const { return Vec3{x+v.x, y+v.y, z+v.z}; }
    Vec3 operator-(const Vec3& v) const { return Vec3{x-v.x, y-v.y, z-v.z}; }
    Vec3 operator*(double s) const { return Vec3(x*s, y*s, z*s);}
    Vec3 operator/(double s) const { return Vec3(x/s, y/s, z/s); }
};

struct State {
    double t;
    Vec3 pos; // ENU
    Vec3 vel;
};

struct AtmosRow {
    double h;
    double u;
    double v;
    double rho;
    double T;
    double P;
};

struct AtmosData {
    Vec3 wind;
    double rho;
    double T;
    double P;
};

struct Debris {
    double m;   // mass, LogNormal(μ=0, σ=1.0) → ~0.3–5 kg
    double A;   // area, correlated with m
    double Cd;  // drag coefficient, ~1.0–1.3
    double beta;    // m/(Cd·A), 50–500 kg/m²

    Vec3 pos;
    Vec3 vel;
};

struct ImpactPoint {
    double lat;
    double lon;
};

std::vector<AtmosRow> loadAtmosTable(const std::string& filename);
void sortAtmosTable(std::vector<AtmosRow>& table);
std::vector<ImpactPoint> runMonteCarlo(const Vec3& pos0, const Vec3& vel0, const std::vector<AtmosRow>& table, int N);
void saveCSV(const std::vector<ImpactPoint>& pts, const std::string& filename);
void ellipse_main(std::vector<ImpactPoint> v_ip);
