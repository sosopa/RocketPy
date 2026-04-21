#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <random>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <cmath>

const double MU = 3.986004418e14;       // [m^3/s^2] Earth gravitational parameter
const double OMEGA_EARTH = 7.2921159e-5; // [rad/s]
const double EARTH_RADIUS = 6371000.0;  // [m]

static std::mt19937 rng(std::random_device{}());

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

Vec3 operator*(double s, const Vec3& v) { return Vec3(v.x * s, v.y * s, v.z * s); }

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

double randu() {
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

double uniform(double a, double b) {
    std::uniform_real_distribution<double> dist(a, b);
    return dist(rng);
}

double normal(double mean, double stddev) {
    std::normal_distribution<double> dist(mean, stddev);
    return dist(rng);
}

double lognormal(double mu, double sigma) {
    std::lognormal_distribution<double> dist(mu, sigma);
    return dist(rng);
}

Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    );
}

Vec3 randomDirection() {
    double u = uniform(-1.0, 1.0);     // cos(theta)
    double theta = uniform(0.0, 2*M_PI);

    double s = std::sqrt(1 - u*u);

    return Vec3(
        s * std::cos(theta),
        s * std::sin(theta),
        u
    );
}

Vec3 geodeticToECEF(double lat, double lon, double h) {
    double R = EARTH_RADIUS + h;

    double clat = cos(lat);
    double slat = sin(lat);
    double clon = cos(lon);
    double slon = sin(lon);

    return Vec3(
        R * clat * clon,
        R * clat * slon,
        R * slat
    );
}

void saveCSV(const std::vector<ImpactPoint>& pts, const std::string& filename) {
    std::ofstream file(filename);
    file << "lat,lon\n";

    for (const auto& p : pts) {
        file << p.lat * 180.0 / M_PI << "," 
             << p.lon * 180.0 / M_PI << "\n";
    }
}

Vec3 latlonToUnit(const ImpactPoint& p) {
    double lat = p.lat;
    double lon = p.lon;

    return Vec3(
        cos(lat)*cos(lon),
        cos(lat)*sin(lon),
        sin(lat)
    );
}

struct XY {
    double x, y;
};

XY project(const ImpactPoint& p, const ImpactPoint& ref) {
    double dlat = p.lat - ref.lat;
    double dlon = p.lon - ref.lon;

    double R = EARTH_RADIUS;

    double x = R * dlon * cos(ref.lat);
    double y = R * dlat;

    return {x, y};
}
