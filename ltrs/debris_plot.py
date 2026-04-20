import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

df = pd.read_csv("impact.csv")

plt.scatter(df["lon"], df["lat"], s=5)

ax = plt.gca()
ax.ticklabel_format(style='plain', axis='both')
ax.xaxis.set_major_formatter(ticker.ScalarFormatter())
ax.yaxis.set_major_formatter(ticker.ScalarFormatter())
ax.xaxis.get_major_formatter().set_useOffset(False)
ax.yaxis.get_major_formatter().set_useOffset(False)

plt.xlabel("Longitude")
plt.ylabel("Latitude")
plt.axis("equal")
plt.show()
