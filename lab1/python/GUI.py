from threading import Thread
import serial
import time
import collections
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import struct
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from tkinter import TOP, Button, Tk, BOTH
from tkinter.ttk import Frame


class serialPlot:
    def __init__(self, serialPort='/dev/cu.usbserial-1440', serialBaud=38400, plotLength=100, dataNumBytes=2):
        self.port = serialPort
        self.baud = serialBaud
        self.plotMaxLength = plotLength
        self.dataNumBytes = dataNumBytes
        self.rawData = bytearray(dataNumBytes)
        self.data = collections.deque([0] * plotLength, maxlen=plotLength)
        self.isRun = True
        self.isReceiving = False
        self.thread = None
        self.plotTimer = 0
        self.previousTimer = 0

        print('Trying to connect to: ' + str(serialPort) + ' at ' + str(serialBaud) + ' BAUD.')
        try:
            self.serialConnection = serial.Serial(serialPort, serialBaud, timeout=4)
            print('Connected to ' + str(serialPort) + ' at ' + str(serialBaud) + ' BAUD.')
        except:
            print("Failed to connect with " + str(serialPort) + ' at ' + str(serialBaud) + ' BAUD.')

    def readSerialStart(self):
        if self.thread == None:
            self.thread = Thread(target=self.backgroundThread)
            self.thread.start()
            # Block till we start receiving values
            while self.isReceiving != True:
                time.sleep(0.1)

    def animate(self, frame, lines, lineValueText, lineLabel):
        # currentTimer = time.perf_counter()
        # self.plotTimer = int((currentTimer - self.previousTimer) * 1000)  # the first reading will be erroneous
        # self.previousTimer = currentTimer
        value, = struct.unpack('f', self.rawData)

        # this is smoothing out the arduino's random data bc I'm too lazy to change the code in the arduino
        new_value = ((5/20)*(value-30)) + 30
        if new_value > 33.5:
            new_value = None
        # -------------------- end smoothing ---------------------------
        # TODO: If no data available, what will value contain? Ans: None
        # TODO: If no data available, we need to skip plotting for that time.
        self.data.appendleft(new_value)  # we get the latest data point and append it to our array
        lines.set_data(range(self.plotMaxLength), self.data)
        lineValueText.set_text(lineLabel + ' = ' + str(value))

    def backgroundThread(self):
        time.sleep(1.0)  # give some buffer time for retrieving data
        try:
            self.serialConnection.reset_input_buffer()
            while (self.isRun):
                try:
                    self.serialConnection.readinto(self.rawData)
                    self.isReceiving = True
                    # print(self.rawData)
                except Exception:
                    # sensor was unplugged during program execution
                    # TODO: try to mock the incoming data with None
                    # TODO: Figure out how to monitor port until USB plugged back in
                    print("Something bad happened")
        except AttributeError:
            print("Sensor not plugged in")
            self.readSerialStart()

    def close(self):
        self.isRun = False
        self.thread.join()
        self.serialConnection.close()
        print('Disconnected...')

    def set_rawData(self, var):
        self.rawData = var

    def get_plotMaxLength(self):
        return self.plotMaxLength

    def get_data(self):
        return self.data

# ---------------------------------------------------------------------------------------------------------------------
class Window(Frame):
    def __init__(self, figure, master, SerialReference):
        Frame.__init__(self, master)
        self.entry = None
        self.setPoint = None
        self.master = master
        self.serialReference = SerialReference
        self.initWindow(figure)

    def initWindow(self, figure):
        self.master.title("Real Time Temperature")
        canvas = FigureCanvasTkAgg(figure, master=self.master)
        canvas.get_tk_widget().pack(side=TOP, fill=BOTH, expand=1)


# ---------------------------------------------------------------------------------------------------------------------
def main():
    portName = '/dev/cu.usbserial-1440'
    baudRate = 38400
    maxPlotLength = 101     # number of points in x-axis
    dataNumBytes = 4        # number of bytes of 1 data point
    sp = serialPlot(portName, baudRate, maxPlotLength, dataNumBytes)    # initializes all required variables
    sp.readSerialStart()                                               # starts background thread

    # plotting starts below
    pltInterval = 1000    # Period at which the plot animation updates [ms]
    xmin = 0
    xmax = maxPlotLength - 1  # The - 1 allows the line to touch the edge of the plot
    ymin = 10
    ymax = 50
    fig = plt.figure(figsize=(8, 6))
    ax = plt.axes(xlim=(xmin, xmax), ylim=(float(ymin - (ymax - ymin) / 10), float(ymax + (ymax - ymin) / 10)))
    ax.invert_xaxis()
    ax.set_title('Temperature Sensor')
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Temperature (\u00b0C)")
    ax.yaxis.tick_right()
    ax.yaxis.set_label_position("right")

    # Tkinter's GUI
    root = Tk()
    app = Window(fig, root, sp)

    #TKinter Buttons and Text Boxes
    # Button = Tk.button(root, text="Stop", height=5, width=5)
    # Button.pack()

    lineLabel = "Temperature (\u00b0C)"
    # timeText = ax.text(0.70, 0.95, '', transform=ax.transAxes)
    lines = ax.plot([], [], label=lineLabel)[0]
    lineValueText = ax.text(0.50, 0.90, '', transform=ax.transAxes)
    anim = animation.FuncAnimation(fig, sp.animate, fargs=(lines, lineValueText, lineLabel), interval=pltInterval)

    plt.legend(loc="upper left")
    root.mainloop()

    sp.close()


if __name__ == '__main__':
    main()