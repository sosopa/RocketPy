#include "atmos_debris.hpp"

Debris sampleDebris(const Vec3& pos0, const Vec3& vel0) {
    Debris d;

    double r = randu();

    double k;

    if (r < 0.3) { // dense
        d.m = lognormal(0.0, 1.0);
        k = uniform(0.01, 0.03);
        d.Cd = uniform(1.0, 1.3);
    }
    else if (r < 0.7) { // panel
        d.m = lognormal(1.5, 0.7);
        k = uniform(0.05, 0.15);
        d.Cd = uniform(1.2, 1.8);
    }
    else { // light
        d.m = lognormal(-2.0, 1.0);
        k = uniform(0.2, 0.6);
        d.Cd = uniform(1.5, 2.5);
    }

    d.A = k * pow(d.m, 2.0/3.0);
    d.beta = d.m / (d.Cd * d.A);

    // Initial state = telemetry + perturbation
    d.pos = pos0;

    Vec3 dv = randomDirection() * normal(0.0, 10.0); // ~10 m/s spread
    d.vel = vel0 + dv;

    return d;
}

Vec3 computeAcceleration(const Debris& d, const AtmosData& atm) {

    Vec3 V_rel = d.vel - atm.wind;
    double V = V_rel.norm();

    Vec3 a_drag(0,0,0);
    if (V > 1e-6) {
        a_drag = -0.5 * atm.rho * V * V / d.beta * (V_rel / V);
    }

    // Gravity (simple spherical Earth)
    double r = d.pos.norm();
    Vec3 g = -MU / (r*r) * (d.pos / r);

    // Coriolis
    Vec3 omega(0, 0, OMEGA_EARTH);
    Vec3 a_cor = -2.0 * cross(omega, d.vel);

    return g + a_drag + a_cor;
}

void propagateDebris(Debris& d, const std::vector<AtmosData>& table, double dt) {

    while (true) {

        double h = d.pos.norm() - EARTH_RADIUS;
        if (h <= 0) break; // impact

        AtmosData atm = getAtmosFromTable(h, table);

        // RK4 integration
        Vec3 k1_v = computeAcceleration(d, atm);
        Vec3 k1_x = d.vel;

        Debris d2 = d;
        d2.vel += 0.5 * dt * k1_v;
        d2.pos += 0.5 * dt * k1_x;

        Vec3 k2_v = computeAcceleration(d2, atm);
        Vec3 k2_x = d2.vel;

        Debris d3 = d;
        d3.vel += 0.5 * dt * k2_v;
        d3.pos += 0.5 * dt * k2_x;

        Vec3 k3_v = computeAcceleration(d3, atm);
        Vec3 k3_x = d3.vel;

        Debris d4 = d;
        d4.vel += dt * k3_v;
        d4.pos += dt * k3_x;

        Vec3 k4_v = computeAcceleration(d4, atm);
        Vec3 k4_x = d4.vel;

        d.vel += dt / 6.0 * (k1_v + 2*k2_v + 2*k3_v + k4_v);
        d.pos += dt / 6.0 * (k1_x + 2*k2_x + 2*k3_x + k4_x);
    }
}

ImpactPoint toLatLon(const Vec3& pos) {
    ImpactPoint p;

    double r = pos.norm();
    p.lat = asin(pos.z / r);
    p.lon = atan2(pos.y, pos.x);

    return p;
}

std::vector<ImpactPoint> runMonteCarlo(
    const Vec3& pos0,
    const Vec3& vel0,
    const std::vector<AtmosData>& table,
    int N)
{
    std::vector<ImpactPoint> impacts;

    for (int i = 0; i < N; ++i) {

        Debris d = sampleDebris(pos0, vel0);

        propagateDebris(d, table, 0.01); // dt = 10 ms

        impacts.push_back(toLatLon(d.pos));
    }

    return impacts;
}

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

    // double h = 1500.0;

    // AtmosData atm = getAtmosFromTable(h, table);

    // Vec3 V_rel = V_body - V_wind;
    // double V = V_rel.norm();

    // // Speed of sound
    // double a = sqrt(1.4 * 287.05 * atm.T);

    // // Mach number
    // double M = V / a;

    // // Drag coefficient
    // double Cd = Cd_table(M);

    // // Drag force
    // Vec3 drag = -0.5 * atm.rho * V * V * Cd * A * V_rel.normalize();

    // std::cout << "U-Wind: "
    //           << atm.wind.x << ", V-Wind: "
    //           << atm.wind.y << ", Density: "
    //           << atm.rho << std::endl;

    return 0;
}
