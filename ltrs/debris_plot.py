import pandas as pd
import numpy as np

# 1. Load CSV (with header)
traj = pd.read_csv("trajectory.csv")

vac = pd.read_csv("impact_vacuum.csv")
isa = pd.read_csv("impact_isa.csv")
era5 = pd.read_csv("impact_era5.csv")

ell_vac = pd.read_csv("ellipse_vacuum.csv")
ell_isa = pd.read_csv("ellipse_isa.csv")
ell_era5 = pd.read_csv("ellipse_era5.csv")

# 2. Extract columns
traj_lat,traj_lon = traj["lat"], traj["lon"]

vac_lat, vac_lon = vac["lat"], vac["lon"]
isa_lat, isa_lon = isa["lat"], isa["lon"]
era5_lat, era5_lon = era5["lat"], era5["lon"]

ev_lat, ev_lon = ell_vac["lat"], ell_vac["lon"]
ei_lat, ei_lon = ell_isa["lat"], ell_isa["lon"]
ee_lat, ee_lon = ell_era5["lat"], ell_era5["lon"]

# 3. Compute global bounds (same scale)
all_lat = np.concatenate([
    vac_lat, isa_lat, era5_lat,
    ev_lat, ei_lat, ee_lat
])

all_lon = np.concatenate([
    vac_lon, isa_lon, era5_lon,
    ev_lon, ei_lon, ee_lon
])

lat_min, lat_max = all_lat.min(), all_lat.max()
lon_min, lon_max = all_lon.min(), all_lon.max()

# Make square-ish view (important for fair comparison)
lat_center = 0.5 * (lat_min + lat_max)
lon_center = 0.5 * (lon_min + lon_max)

half_range = max(lat_max - lat_min, lon_max - lon_min) / 2

lat_min = lat_center - half_range
lat_max = lat_center + half_range
lon_min = lon_center - half_range
lon_max = lon_center + half_range

# 4. Plot 3 panels (geographical view)
import matplotlib.pyplot as plt

fig, axes = plt.subplots(1, 3, figsize=(15,5))

cases = [
    (vac_lon, vac_lat, ev_lon, ev_lat, "Vacuum"),
    (isa_lon, isa_lat, ei_lon, ei_lat, "ISA"),
    (era5_lon, era5_lat, ee_lon, ee_lat, "ERA5"),
]

for ax, (lon, lat, elon, elat, title) in zip(axes, cases):
    ax.scatter(lon, lat, s=5)
    ax.plot(elon, elat, linewidth=2)

    ax.set_xlim(lon_min, lon_max)
    ax.set_ylim(lat_min, lat_max)

    ax.set_xlabel("Longitude")
    ax.set_ylabel("Latitude")
    ax.set_title(title)
    ax.grid(True)

plt.tight_layout()
plt.show()

# 5. Overlay plot (very useful for comparison)
plt.figure(figsize=(8,8))

# trajectory
plt.plot(traj_lon, traj_lat, linewidth=2, label="Trajectory")
plt.scatter(traj_lon, traj_lat, c=traj["alt"], s=2)

# impacts
plt.scatter(vac_lon, vac_lat, s=5, label="Vacuum")
plt.scatter(isa_lon, isa_lat, s=5, label="ISA")
plt.scatter(era5_lon, era5_lat, s=5, label="ERA5")

# ellipses
plt.plot(ev_lon, ev_lat, label="Vac ellipse")
plt.plot(ei_lon, ei_lat, label="ISA ellipse")
plt.plot(ee_lon, ee_lat, label="ERA5 ellipse")

plt.xlim(lon_min, lon_max)
plt.ylim(lat_min, lat_max)

plt.xlabel("Longitude")
plt.ylabel("Latitude")
plt.legend()
plt.grid(True)

plt.show()

# Does ellipse actually cover ~95% of points?
from shapely.geometry import Point, Polygon

poly_vac = Polygon(np.column_stack([ev_lon, ev_lat]))
inside_vac = sum(poly_vac.contains(Point(px, py)) for px, py in zip(vac_lon, vac_lat))
print("Vacuum Coverage:", inside_vac / len(vac_lon))

poly_isa = Polygon(np.column_stack([ei_lon, ei_lat]))
inside_isa = sum(poly_isa.contains(Point(px, py)) for px, py in zip(isa_lon, isa_lat))
print("ISA Coverage:", inside_isa / len(isa_lon))

poly_era5 = Polygon(np.column_stack([ee_lon, ee_lat]))
inside_era5 = sum(poly_era5.contains(Point(px, py)) for px, py in zip(era5_lon, era5_lat))
print("ERA5 Coverage:", inside_era5 / len(era5_lon))
