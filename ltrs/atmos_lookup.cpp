#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>

struct Vec3 {
    double x;
    double y;
    double z;

    // Constructors
    Vec3() : x(0.0), y(0.0), z(0.0) {}
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    // Operator overloading
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(double scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    // Optional helpers
    double norm() const {
        return std::sqrt(x*x + y*y + z*z);
    }
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

int main() {
    auto table = loadAtmosTable("data/at20260325.csv");
    sortAtmosTable(table);

    double h = 1500.0;

    AtmosData atmos = getAtmosFromTable(h, table);

    // Vec3 V_rel = V_body - atmos.wind;

    // double speed = V_rel.norm();
    // double drag = 0.5 * atmos.rho * speed * speed * Cd * A;

    std::cout << "U-Wind: "
              << atmos.wind.x << ", V-Wind: "
              << atmos.wind.y << ", Density: "
              << atmos.rho << std::endl;

    return 0;
}
