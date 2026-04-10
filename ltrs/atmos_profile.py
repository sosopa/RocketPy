import numpy as np
from rocketpy import Environment

env_era5 = Environment(
    date=(2026, 3, 25, 20),
    latitude=37.5,
    longitude=130.0,
    elevation=0,
)

filename = "data/20260325/data_stream-oper_stepType-instant.nc"

env_era5.set_atmospheric_model(
    type="Reanalysis",
    file=filename,
    dictionary="ECMWF",
)

env_era5.max_expected_height = 50000.0  # 0 ~ 50 km
height_space = env_era5.max_expected_height / 100 + 1

env_era5.plots.atmospheric_model()

heights = np.linspace(0, env_era5.max_expected_height, int(height_space))

data = []
for h in heights:
    u = env_era5.wind_velocity_x(h)
    v = env_era5.wind_velocity_y(h)
    rho = env_era5.density(h)
    T = env_era5.temperature(h)
    P = env_era5.pressure(h)

    data.append([h, u, v, rho, T, P])

np.savetxt("data/at20260325.csv", data,
           header="h,u,v,rho,T,P", delimiter=",")
