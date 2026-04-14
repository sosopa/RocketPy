import numpy as np
import matplotlib.pyplot as plt

# Load your CSV file
# (skip header line starting with #)
data = np.loadtxt("data/at20260325.csv", delimiter=",", comments="#")

# Columns
h   = data[:, 0]   # altitude (m)
u   = data[:, 1]   # wind u (m/s)
v   = data[:, 2]   # wind v (m/s)
rho = data[:, 3]   # density (kg/m^3)
T   = data[:, 4]   # temperature (K)
P   = data[:, 5]   # pressure (Pa)

# Derived quantity
wind_speed = np.sqrt(u**2 + v**2)

# -------------------------------
# Plotting
# -------------------------------

fig, axs = plt.subplots(1, 3, figsize=(18, 6), sharey=True)

# 1. Wind U & V
axs[0].plot(u, h, label="u (east)")
axs[0].plot(v, h, label="v (north)")
axs[0].set_xlabel("Wind (m/s)")
axs[0].set_ylabel("Altitude (m)")
axs[0].set_title("Wind Components")
axs[0].legend()
axs[0].grid()

# 2. Wind Speed
axs[1].plot(wind_speed, h, color='black')
axs[1].set_xlabel("Wind Speed (m/s)")
axs[1].set_title("Wind Speed")
axs[1].grid()

# 3. Density
axs[2].plot(rho, h, color='blue')
axs[2].set_xlabel("Density (kg/m³)")
axs[2].set_title("Density")
axs[2].grid()

# # 4. Temperature
# axs[3].plot(T, h, color='red')
# axs[3].set_xlabel("Temperature (K)")
# axs[3].set_title("Temperature")
# axs[3].grid()

# # 5. Pressure (log scale is better)
# axs[4].plot(P, h, color='green')
# axs[4].set_xscale("log")
# axs[4].set_xlabel("Pressure (Pa)")
# axs[4].set_title("Pressure (log)")
# axs[4].grid()

plt.suptitle("Atmospheric Profile (ERA5 / 2026-03-25 20:00)", fontsize=14)
plt.tight_layout()
plt.show()
