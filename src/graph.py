import matplotlib.pyplot as plt
import csv
import numpy as np
import time
from matplotlib.animation import FuncAnimation

path = "./data/logs/log.csv"

def update_plot(frame):
    # Clear the existing plots
    for ax in axs:
        ax.clear()

    # Read the data from the path
    raw_data = []
    with open(path, 'r') as file:
        reader = csv.reader(file)
        for row in reader:
            raw_data.append(row)

    # Transpose the data
    data = list(map(list, zip(*raw_data)))

    # Convert string values to float, except for the first column
    for i in range(1, len(data)):
        data[i] = [float(val) if val != '' else np.nan for val in data[i][1:]]

    data[0] = [0] + [int(val) for val in data[0][1:]]

    # Plot data
    plot_data = [
        ("Max Fitness", data[0][1:], data[1]),
        ("Avg Fitness", data[0][1:], data[2]),
        ("Best Node Size", data[0][1:], data[3]),
        ("Best Connection Size", data[0][1:], data[4]),
        ("Avg node size", data[0][1:], data[5]),
        ("Avg connection size", data[0][1:], data[6])
    ]

    for i, (title, x, y) in enumerate(plot_data):
        axs[i].plot(x, y, label=title)
        axs[i].set_title(title)
        axs[i].set_xlabel('Iteration')
        axs[i].set_ylabel('Value')
        axs[i].legend()

    plt.tight_layout()

# Create a 2x3 grid of plots
fig, axs = plt.subplots(2, 3, figsize=(10, 8))
axs = axs.flatten()

# Remove extra subplots
for i in range(6, 6):
    fig.delaxes(axs[i])

update_plot(0)

# Create the animation
ani = FuncAnimation(fig, update_plot, interval=1000)  # 1000 ms = 1 second

plt.show()
