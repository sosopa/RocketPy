#include "atmos_debris.h"

Vec3 operator*(double s, const Vec3& v) { return Vec3(v.x * s, v.y * s, v.z * s); }

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

void sortAtmosTable(std::vector<AtmosRow>& table) {
    std::sort(table.begin(), table.end(),
        [](const AtmosRow& a, const AtmosRow& b) {
            return a.h < b.h;
        });
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

AtmosData isa(double h) {
    AtmosData atm;

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

AtmosData getAtmos(AtmosMode mode, double h, const std::vector<AtmosRow>& table) {
    if (mode == AtmosMode::ERA5)
        return getAtmosFromTable(h, table);

    if (mode == AtmosMode::ISA)
        return isa(h);

    if (mode == AtmosMode::VACUUM)
        return {{0.0, 0.0, 0.0}, 0.0, 0.0, 0.0};

    // fallback
    return {{0.0, 0.0, 0.0}, 0.0, 0.0, 0.0};
}

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
    d.beta = std::clamp(d.beta, 0.1, 500.0);

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
        a_drag = (V_rel / V) * (-0.5 * atm.rho * V * V / d.beta);
    }

    // Gravity (simple spherical Earth)
    double r = d.pos.norm();
    Vec3 g = -MU / (r*r) * (d.pos / r);

    // Coriolis
    Vec3 omega(0, 0, OMEGA_EARTH);
    Vec3 a_cor = -2.0 * cross(omega, d.vel);

    return g + a_drag + a_cor;
}

bool propagateDebris(Debris& d, const std::vector<AtmosRow>& table, double dt) {
    int max_steps = 200000;  // ~2000 s if dt=0.01
    int steps = 0;

    while (true) {
        double r = d.pos.norm();
        double h = r - EARTH_RADIUS;

        if (h <= 0) return true;  // impact

        // safety: escape to space
        if (r > EARTH_RADIUS + 200000) {
            std::cout << "!> Escape to space\n";
            return false;
        }
        
        // safety: NaN check
        if (!std::isfinite(r)) {
            std::cout << "!> NaN detected, aborting particle\n";
            return false;
        }

        if (++steps > max_steps) {
            std::cout << "!> Max steps reached\n";
            return false;
        }

        h = std::clamp(h, table.front().h, table.back().h);
        AtmosData atm = getAtmos(atmos_mode, h, table);

        // RK4 integration
        Vec3 k1_v = computeAcceleration(d, atm);
        Vec3 k1_x = d.vel;

        Debris d2 = d;
        d2.vel = d2.vel + 0.5 * dt * k1_v;
        d2.pos = d2.pos + 0.5 * dt * k1_x;

        Vec3 k2_v = computeAcceleration(d2, atm);
        Vec3 k2_x = d2.vel;

        Debris d3 = d;
        d3.vel = d3.vel + 0.5 * dt * k2_v;
        d3.pos = d3.pos + 0.5 * dt * k2_x;

        Vec3 k3_v = computeAcceleration(d3, atm);
        Vec3 k3_x = d3.vel;

        Debris d4 = d;
        d4.vel = d4.vel + dt * k3_v;
        d4.pos = d4.pos + dt * k3_x;

        Vec3 k4_v = computeAcceleration(d4, atm);
        Vec3 k4_x = d4.vel;

        d.vel = d.vel + dt / 6.0 * (k1_v + 2*k2_v + 2*k3_v + k4_v);
        d.pos = d.pos + dt / 6.0 * (k1_x + 2*k2_x + 2*k3_x + k4_x);

        if (steps % 10000 == 0) {
            std::cout << "step=" << steps 
                    << " h=" << h 
                    << " V=" << d.vel.norm()
                    << std::endl;
        }
    }
}

ImpactPoint toLatLon(const Vec3& pos) {
    ImpactPoint p;

    double r = pos.norm();
    p.lat = asin(pos.z / r);
    p.lon = atan2(pos.y, pos.x);

    return p;
}

std::vector<ImpactPoint> runMonteCarlo(const Vec3& pos0, const Vec3& vel0, const std::vector<AtmosRow>& table, int epoch, int N) {
    std::vector<ImpactPoint> impacts;

    for (int i = 0; i < N; ++i) {
        std::cout << "Simulating debris " << i + 1 + N * epoch << std::endl;

        Debris d = sampleDebris(pos0, vel0);

        if (propagateDebris(d, table, 0.01)) {  // dt = 10 ms
            impacts.push_back(toLatLon(d.pos));
        }
    }

    return impacts;
}

MatrixXd computeCovariance(const std::vector<XY>& pts, Vector2d& mean) {
    int N = pts.size();

    mean.setZero();

    for (const auto& p : pts) {
        mean(0) += p.x;
        mean(1) += p.y;
    }
    mean /= N;

    MatrixXd cov = MatrixXd::Zero(2,2);

    for (const auto& p : pts) {
        Vector2d d;
        d << p.x - mean(0), p.y - mean(1);
        cov += d * d.transpose();
    }

    cov /= N;  // or (N-1) if you want unbiased

    return cov;
}

void computeEllipse(const Matrix2d& cov, const Vector2d& mean, double& major_axis, double& minor_axis, double& angle) {
    SelfAdjointEigenSolver<Matrix2d> solver(cov);

    Vector2d eigenvalues = solver.eigenvalues();
    Matrix2d eigenvectors = solver.eigenvectors();

    // χ² value for 95% confidence (2 DOF)
    double chi2 = 5.991;

    // Axes lengths
    major_axis = std::sqrt(chi2 * eigenvalues(1)); // larger eigenvalue
    minor_axis = std::sqrt(chi2 * eigenvalues(0));

    // Orientation (angle from x-axis)
    Vector2d major_vec = eigenvectors.col(1);
    angle = std::atan2(major_vec(1), major_vec(0));
}

std::vector<XY> generateEllipsePoints(const Vector2d& mean, double a, double b, double angle, int n = 100) {
    std::vector<XY> pts;

    for (int i = 0; i < n; ++i) {
        double t = 2.0 * M_PI * i / n;

        double x = a * cos(t);
        double y = b * sin(t);

        // rotate
        double xr = cos(angle)*x - sin(angle)*y;
        double yr = sin(angle)*x + cos(angle)*y;

        pts.push_back({ mean(0) + xr, mean(1) + yr });
    }

    return pts;
}

ImpactPoint xyToLatLon(const XY& p, const ImpactPoint& ref) {
    ImpactPoint out;

    out.lat = ref.lat + p.y / EARTH_RADIUS;
    out.lon = ref.lon + p.x / (EARTH_RADIUS * cos(ref.lat));

    return out;
}

std::vector<State> generateTrajectory(double dt = 0.1) {
    std::vector<State> traj;

    // Initial conditions (ENU)
    Vec3 pos = {0.0, 0.0, 10000.0};   // meters
    Vec3 vel = {250.0, 50.0, -20.0};  // m/s

    const double g = 9.81;

    double t = 0.0;

    while (pos.z > 0.0)
    {
        traj.push_back({t, pos, vel});

        // Update position
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
        pos.z += vel.z * dt - 0.5 * g * dt * dt;

        // Update velocity
        vel.z -= g * dt;

        t += dt;
    }

    return traj;
}

Vec3 enuToECEF(const Vec3& enu, double lat, double lon) {
    double sinLat = sin(lat);
    double cosLat = cos(lat);
    double sinLon = sin(lon);
    double cosLon = cos(lon);

    Vec3 ecef;

    ecef.x = -sinLon * enu.x
             - sinLat * cosLon * enu.y
             + cosLat * cosLon * enu.z;

    ecef.y =  cosLon * enu.x
             - sinLat * sinLon * enu.y
             + cosLat * sinLon * enu.z;

    ecef.z =  cosLat * enu.y
             + sinLat * enu.z;

    return ecef;
}

void ellipse_main(std::vector<ImpactPoint> v_ip) {
    // 1. Project to XY
    std::vector<XY> pts;
    for (auto& p : v_ip) {
        pts.push_back(project(p, v_ip[0])); // use first or mean
    }

    // 2. Compute stats
    Vector2d mean;
    Matrix2d cov = computeCovariance(pts, mean);

    // 3. Ellipse
    double a, b, theta;
    computeEllipse(cov, mean, a, b, theta);

    // 4. Generate ellipse
    auto ellipse_xy = generateEllipsePoints(mean, a, b, theta);

    // 5. Convert back to lat/lon for plotting
    std::vector<ImpactPoint> ellipse_ll;
    for (auto& p : ellipse_xy) {
        ellipse_ll.push_back(xyToLatLon(p, v_ip[0])); // use first or mean
    }

    saveCSV(ellipse_ll, "ellipse.csv");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"Usage: %s mode(0: VACUUM, 1: ISA, 2: ERA5)\n", argv[0]);
        return 1;
    }
    atmos_mode = AtmosMode(atoi(argv[1]));

    std::cout << "RNG seed = " << seed << std::endl;

    auto table = loadAtmosTable("data/at20260325.csv");
    sortAtmosTable(table);

    std::vector<ImpactPoint> v_ip;
    std::vector<ImpactPoint> v_ip_sum;
    
    // Reference point
    double lat = 37.0 * M_PI / 180.0;
    double lon = 127.0 * M_PI / 180.0;
    double alt = 10000.0;
    Vec3 ref_ecef = geodeticToECEF(lat, lon, alt);

    // ECEF trajectory from telemetry
    auto trajectory = generateTrajectory(10);   // dt sec
    int epoch = 0;
    for (auto& tj : trajectory) {
        // For each trajectory point
        Vec3 enu = tj.pos;
        Vec3 ecef_offset = enuToECEF(enu, lat, lon);

        Vec3 ecef_pos = {
            ref_ecef.x + ecef_offset.x,
            ref_ecef.y + ecef_offset.y,
            ref_ecef.z + ecef_offset.z
        };
        
        v_ip = runMonteCarlo(ecef_pos, tj.vel, table, epoch, 10);
        v_ip_sum.reserve(v_ip_sum.size() + v_ip.size());
        v_ip_sum.insert(v_ip_sum.end(), v_ip.begin(), v_ip.end());
        epoch++;
    };

    saveCSV(v_ip_sum, "impact.csv");

    ellipse_main(v_ip_sum);

    return 0;
}
