#include <iostream>
#include <vector>
#include <cmath>

std::vector<double> generate_beta_set_random(
    double length,
    double diameter,
    double total_mass,
    int n_classes = 6
) {
    auto beta = generate_beta_set(length, diameter, total_mass, n_classes);

    // Random generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dist(0.0, 0.3);  // log-normal variation

    for (auto& b : beta) {
        double factor = std::exp(dist(gen));
        b *= factor;
    }

    return beta;
}

std::vector<double> generate_beta_set(
    double length,       // [m]
    double diameter,     // [m]
    double total_mass,   // [kg]
    int n_classes = 6
) {
    const double Cd_mean = 1.4;

    // Reference area (circular cross-section)
    double A_ref = M_PI * std::pow(diameter / 2.0, 2);

    // Mass range (log scale)
    double m_min = total_mass * 0.0001;  // 0.01%
    double m_max = total_mass * 0.3;     // 30%

    std::vector<double> beta_set;
    beta_set.reserve(n_classes);

    for (int i = 0; i < n_classes; i++) {
        // ---- Log-space interpolation ----
        double t = static_cast<double>(i) / (n_classes - 1);

        double mass = std::pow(10.0,
            std::log10(m_min) + t * (std::log10(m_max) - std::log10(m_min))
        );

        // ---- Area scaling (heuristic) ----
        double area_scale = 3.0 - t * (3.0 - 0.5);  // from 3.0 → 0.5
        double area = A_ref * area_scale;

        // ---- Beta calculation ----
        double beta = mass / (Cd_mean * area);

        beta_set.push_back(beta);
    }

    return beta_set;
}

int main() {
    double length = 5.0;     // m
    double diameter = 0.3;   // m
    double total_mass = 300; // kg

    auto beta_set = generate_beta_set(length, diameter, total_mass);

    std::cout << "Beta values:\n";
    for (double b : beta_set) {
        std::cout << b << " kg/m^2\n";
    }

    return 0;
}
