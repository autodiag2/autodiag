import matplotlib.pyplot as plt
import numpy as np

x = np.linspace(-10, 10, 1000)
y = np.sin(x)

fig, ax = plt.subplots()
ax.plot(x, y, color='red')
for xg in np.arange(-10, 11, 1):
    ax.axvline(x=xg, color='grey', linewidth=0.5, zorder=0)
for yg in np.arange(-1, 2, 0.5):
    ax.axhline(y=yg, color='grey', linewidth=0.5, zorder=0)

def on_scroll(event):
    cur_xlim = ax.get_xlim()
    cur_ylim = ax.get_ylim()
    xdata = event.xdata
    ydata = event.ydata
    if xdata is None or ydata is None:
        return
    scale_factor = 0.9 if event.button == 'up' else 1.1
    new_width = (cur_xlim[1] - cur_xlim[0]) * scale_factor
    new_height = (cur_ylim[1] - cur_ylim[0]) * scale_factor
    relx = (xdata - cur_xlim[0]) / (cur_xlim[1] - cur_xlim[0])
    rely = (ydata - cur_ylim[0]) / (cur_ylim[1] - cur_ylim[0])
    ax.set_xlim([xdata - new_width * relx, xdata + new_width * (1 - relx)])
    ax.set_ylim([ydata - new_height * rely, ydata + new_height * (1 - rely)])
    fig.canvas.draw_idle()

fig.canvas.mpl_connect('scroll_event', on_scroll)
plt.show()
