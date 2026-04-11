#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>

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
    Vec3 wind;   // (u, v, w)
    double rho;
    double T;
    double P;

    AtmosData() : wind(0.0, 0.0, 0.0), rho(0.0), T(0.0), P(0.0) {}
    AtmosData(Vec3 wind_, double rho_, double T_, double P_) : wind(wind_), rho(rho_), T(T_), P(P_) {}
};

double lerp(double a, double b, double t) {
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
    AtmosData result;

    if (table.empty()) {
        return AtmosData(Vec3(0., 0., 0.), 0., 0., 0.);
    }

    // Clamp below range
    if (h <= table.front().h) {
        return AtmosData(Vec3(table.front().u, table.front().v, 0.0), table.front().rho, table.front().T, table.front().P);
    }

    // Clamp above range
    if (h >= table.back().h) {
        return AtmosData(Vec3(table.back().u, table.back().v, 0.0), table.back().rho, table.back().T, table.back().P);
    }

    // Find interval
    for (size_t i = 0; i < table.size() - 1; ++i) {
        const auto& a = table[i];
        const auto& b = table[i + 1];

        if (h >= a.h && h <= b.h) {
            double t = (h - a.h) / (b.h - a.h);

            double u = lerp(a.u, b.u, t);
            double v = lerp(a.v, b.v, t);
            double rho = lerp(a.rho, b.rho, t);
            double T = lerp(a.T, b.T, t);
            double P = lerp(a.P, b.P, t);

            result.wind = Vec3(u, v, 0.0);
            result.rho  = rho;
            result.T    = T;
            result.P    = P;

            return result;
        }
    }

    return AtmosData(Vec3(0, 0, 0), 0, 0, 0); // fallback
}

int main() {
    auto table = loadAtmosTable("data/at20260325.csv");

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
