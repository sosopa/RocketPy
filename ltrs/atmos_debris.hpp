#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>

using namespace std;

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
    double m;
    double A;
    double Cd;
    double beta;

    Vec3 pos;
    Vec3 vel;
};

struct ImpactPoint {
    double lat;
    double lon;
};
