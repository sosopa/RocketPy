struct AtmosPoint {
    double h, u, v, rho, T, P;
};

double interp(double h, vector<AtmosPoint>& table);

wind_x = interp(h).u;
wind_y = interp(h).v;
density = interp(h).rho;

wind_speed = sqrt(u*u + v*v);
wind_dir   = atan2(v, u);

Vec3 wind = {u, v, 0};

V_wind = lookup(h);

Vec3 wind = getWindFromTable(h);  // from RocketPy

Vec3 V_rel = V_body - wind;
