#include <iostream>
#include <vector>
#include <cmath>
#include <random>

struct DebrisClass {
    double mass;
    double area;
    double beta;
};

std::vector<DebrisClass> generate_debris_classes(
    double length,
    double diameter,
    double total_mass,
    int n_classes = 6
) {
    const double Cd_mean = 1.4;
    double A_ref = M_PI * std::pow(diameter / 2.0, 2);

    double m_min = total_mass * 0.0001;
    double m_max = total_mass * 0.3;

    std::vector<DebrisClass> debris;
    debris.reserve(n_classes);

    for (int i = 0; i < n_classes; i++) {
        double t = static_cast<double>(i) / (n_classes - 1);

        double mass = std::pow(10.0,
            std::log10(m_min) + t * (std::log10(m_max) - std::log10(m_min))
        );

        double area_scale = 3.0 - t * (3.0 - 0.5);
        double area = A_ref * area_scale;

        double beta = mass / (Cd_mean * area);

        debris.push_back({mass, area, beta});
    }

    return debris;
}

int main() {
    double length = 5.0;     // m
    double diameter = 0.3;   // m
    double total_mass = 300; // kg

    auto debris_classes = generate_debris_classes(length, diameter, total_mass);

    // How This Fits Your 6DOF Simulation:
    for (auto& debris : debris_classes) {
        std::cout << debris.beta << " kg/m^2\n";

        Vec3 V_rel = V - wind;

        double Vmag = norm(V_rel);

        Vec3 drag = -0.5 * rho * Vmag * Vmag / debris.beta * normalize(V_rel);

        // integrate motion
    }

    return 0;
}
