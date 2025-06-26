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
        self.mpl_connect('button_press_event', self.on_press)
        self.mpl_connect('motion_notify_event', self.on_motion)
        self.mpl_connect('button_release_event', self.on_release)
        self._press_pos = None
        self._press_xlim = None
        self._press_ylim = None

    def on_press(self, event):
        if event.button == 1 and event.inaxes:
            self._press_pos = (event.xdata, event.ydata)
            self._press_xlim = self.ax.get_xlim()
            self._press_ylim = self.ax.get_ylim()

    def on_motion(self, event):
        if self._press_pos is None or not event.inaxes:
            return
        dx = self._press_pos[0] - event.xdata
        dy = self._press_pos[1] - event.ydata
        x0, x1 = self._press_xlim
        y0, y1 = self._press_ylim
        self.ax.set_xlim(x0 + dx, x1 + dx)
        self.ax.set_ylim(y0 + dy, y1 + dy)
        self.draw_idle()

    def on_release(self, event):
        self._press_pos = None
        self._press_xlim = None
        self._press_ylim = None

    def set_grey_lines(self):
        [l.remove() for l in self.ax.get_lines()[1:]]  # Keep plot line, remove extra
        [l.remove() for l in self.ax.lines if l.get_linestyle() == '--']  # If you use dashed for grid, adjust as needed
        for xg in self.ax.get_xticks():
            self.ax.axvline(x=xg, color='grey', linewidth=0.5, zorder=0)
        for yg in self.ax.get_yticks():
            self.ax.axhline(y=yg, color='grey', linewidth=0.5, zorder=0)

    def plot(self):
        x = np.linspace(-100, 100, 1000)
        y = np.sin(x)
        self.ax.clear()
        self.ax.set_title("Sine Wave")
        self.ax.set_xlabel("X Axis")
        self.ax.set_ylabel("Y Axis")
        self.ax.plot(x, y, color='red')
        self.set_grey_lines()
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
        self.set_grey_lines()
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
