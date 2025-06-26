import sys
import numpy as np
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure

class PlotCanvas(FigureCanvas):
    def __init__(self, parent=None):
        fig = Figure()
        self.ax = fig.add_subplot(111)
        super().__init__(fig)
        self.setParent(parent)
        self.plot()
        self.mpl_connect('scroll_event', self.on_scroll)

    def plot(self):
        x = np.linspace(-10, 10, 1000)
        y = np.sin(x)
        self.ax.clear()
        self.ax.plot(x, y, color='red')
        for xg in np.arange(-10, 11, 1):
            self.ax.axvline(x=xg, color='grey', linewidth=0.5, zorder=0)
        for yg in np.arange(-1, 2, 0.5):
            self.ax.axhline(y=yg, color='grey', linewidth=0.5, zorder=0)
        self.draw()

    def on_scroll(self, event):
        cur_xlim = self.ax.get_xlim()
        cur_ylim = self.ax.get_ylim()
        xdata, ydata = event.xdata, event.ydata
        if xdata is None or ydata is None:
            return
        scale_factor = 0.9 if event.button == 'up' else 1.1
        new_width = (cur_xlim[1] - cur_xlim[0]) * scale_factor
        new_height = (cur_ylim[1] - cur_ylim[0]) * scale_factor
        relx = (xdata - cur_xlim[0]) / (cur_xlim[1] - cur_xlim[0])
        rely = (ydata - cur_ylim[0]) / (cur_ylim[1] - cur_ylim[0])
        self.ax.set_xlim([xdata - new_width * relx, xdata + new_width * (1 - relx)])
        self.ax.set_ylim([ydata - new_height * rely, ydata + new_height * (1 - rely)])
        self.draw_idle()

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        widget = QWidget()
        layout = QVBoxLayout(widget)
        self.plot_canvas = PlotCanvas(self)
        layout.addWidget(self.plot_canvas)
        self.setCentralWidget(widget)

app = QApplication(sys.argv)
w = MainWindow()
w.show()
app.exec_()
