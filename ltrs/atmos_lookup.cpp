#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>

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

inline double lerp(double a, double b, double t) {
    return a + t * (b - a);
}

std::vector<AtmosRow> loadAtmosTable(const std::string& filename) {
    std::vector<AtmosRow> table;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    std::string line;

    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;

        AtmosRow row;

        std::getline(ss, value, ','); row.h = std::stod(value);
        std::getline(ss, value, ','); row.u = std::stod(value);
        std::getline(ss, value, ','); row.v = std::stod(value);
        std::getline(ss, value, ','); row.rho = std::stod(value);
        std::getline(ss, value, ','); row.T = std::stod(value);
        std::getline(ss, value, ','); row.P = std::stod(value);

        table.push_back(row);
    }

    return table;
}

AtmosData getAtmosFromTable(double h, const std::vector<AtmosRow>& table) {
    AtmosData out{};

    if (table.empty()) {
        return out;  // all zeros
    }

    // Clamp below range
    if (h <= table.front().h) {
        out.wind = Vec3(table.front().u, table.front().v, 0.0);
        out.rho  = table.front().rho;
        out.T    = table.front().T;
        out.P    = table.front().P;
        return out;
    }

    // Clamp above range
    if (h >= table.back().h) {
        out.wind = Vec3(table.back().u, table.back().v, 0.0);
        out.rho  = table.back().rho;
        out.T    = table.back().T;
        out.P    = table.back().P;
        return out;
    }

    // Binary search: first element with h_i >= h
    auto it = std::lower_bound(
        table.begin(), table.end(), h,
        [](const AtmosRow& row, double value) {
            return row.h < value;
        }
    );

    // Now:
    // it       → upper point (h_i >= h)
    // it - 1   → lower point
    const AtmosRow& b = *it;
    const AtmosRow& a = *(it - 1);

    double t = (h - a.h) / (b.h - a.h);

    // Interpolate all fields
    double u   = lerp(a.u,   b.u,   t);
    double v   = lerp(a.v,   b.v,   t);
    double rho = lerp(a.rho, b.rho, t);
    double T   = lerp(a.T,   b.T,   t);
    double P   = lerp(a.P,   b.P,   t);

    out.wind = Vec3(u, v, 0.0);
    out.rho  = rho;
    out.T    = T;
    out.P    = P;

    return out;
}

void sortAtmosTable(std::vector<AtmosRow>& table) {
    std::sort(table.begin(), table.end(),
        [](const AtmosRow& a, const AtmosRow& b) {
            return a.h < b.h;
        });
}

double Cd_table(double M) {
    if (M < 0.8) return 0.3;
    if (M < 1.2) return 0.6;  // drag rise
    if (M < 3.0) return 0.25;
    return 0.2;
}

int main() {
    auto table = loadAtmosTable("data/at20260325.csv");
    sortAtmosTable(table);

    double h = 1500.0;

    AtmosData atm = getAtmosFromTable(h, table);

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

    std::cout << "U-Wind: "
              << atm.wind.x << ", V-Wind: "
              << atm.wind.y << ", Density: "
              << atm.rho << std::endl;

    return 0;
}
