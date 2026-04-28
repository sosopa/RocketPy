import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Ellipse

# Convert lat/lon → local ENU (meters)
ref_lat = np.mean(37.0)   # all_lat or fixed 37.0
ref_lon = np.mean(127.0)   # all_lon or fixed 127.0

R = 6371000.0  # Earth radius (m)

def latlon_to_enu(lat, lon, lat0, lon0):
    lat = np.radians(lat)
    lon = np.radians(lon)
    lat0 = np.radians(lat0)
    lon0 = np.radians(lon0)

    dlat = lat - lat0
    dlon = lon - lon0

    x = R * dlon * np.cos(lat0)   # East
    y = R * dlat                  # North

    return x, y

vac = np.loadtxt("impact_vacuum.csv", delimiter=",")
isa = np.loadtxt("impact_isa.csv", delimiter=",")
era5 = np.loadtxt("impact_era5.csv", delimiter=",")

ell_vac = np.loadtxt("ellipse_vacuum.csv", delimiter=",")
ell_isa = np.loadtxt("ellipse_isa.csv", delimiter=",")
ell_era5 = np.loadtxt("ellipse_era5.csv", delimiter=",")

# Convert everything to ENU
def convert(data):
    return latlon_to_enu(data[:,0], data[:,1], ref_lat, ref_lon)

vac_x, vac_y = convert(vac)
isa_x, isa_y = convert(isa)
era5_x, era5_y = convert(era5)

ev_x, ev_y = convert(ell_vac)
ei_x, ei_y = convert(ell_isa)
ee_x, ee_y = convert(ell_era5)

# Compute global bounds
all_x = np.concatenate([vac_x, isa_x, era5_x])
all_y = np.concatenate([vac_y, isa_y, era5_y])

xmin, xmax = all_x.min(), all_x.max()
ymin, ymax = all_y.min(), all_y.max()

cx = 0.5*(xmin + xmax)
cy = 0.5*(ymin + ymax)

half = max(xmax - xmin, ymax - ymin)/2

xmin, xmax = cx - half, cx + half
ymin, ymax = cy - half, cy + half

# Plot (points + ellipse contour)
import matplotlib.pyplot as plt

fig, axes = plt.subplots(1, 3, figsize=(15,5))

cases = [
    (vac_x, vac_y, ev_x, ev_y, "Vacuum"),
    (isa_x, isa_y, ei_x, ei_y, "ISA"),
    (era5_x, era5_y, ee_x, ee_y, "ERA5"),
]

for ax, (x, y, ex, ey, title) in zip(axes, cases):
    ax.scatter(x, y, s=5)
    ax.plot(ex, ey, linewidth=2)  # ellipse contour

    ax.set_xlim(xmin, xmax)
    ax.set_ylim(ymin, ymax)
    ax.set_aspect('equal')

    ax.set_title(title)
    ax.grid(True)

plt.tight_layout()
plt.show()

# Overlay all
plt.figure(figsize=(6,6))

plt.scatter(vac_x, vac_y, s=5, label="Vacuum")
plt.scatter(isa_x, isa_y, s=5, label="ISA")
plt.scatter(era5_x, era5_y, s=5, label="ERA5")

plt.plot(ev_x, ev_y, label="Vac ellipse")
plt.plot(ei_x, ei_y, label="ISA ellipse")
plt.plot(ee_x, ee_y, label="ERA5 ellipse")

plt.xlim(xmin, xmax)
plt.ylim(ymin, ymax)
plt.gca().set_aspect('equal')

plt.legend()
plt.grid(True)
plt.show()

# Does ellipse actually cover ~95% of points?
from shapely.geometry import Point, Polygon

poly = Polygon(np.column_stack([ei_x, ei_y]))
inside = sum(poly.contains(Point(px, py)) for px, py in zip(isa_x, isa_y))

print("Coverage:", inside / len(isa_x))
