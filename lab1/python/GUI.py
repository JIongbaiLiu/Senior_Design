from threading import Thread
import serial
import time
import collections
import send_sms
import matplotlib

matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import struct
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import tkinter
from tkinter.ttk import Frame


class serialManagement:
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


# ---------------------------------------------------------------------------------------------------------------------
class Window:
    max_temp = None

    def __init__(self, fig, root, max, min_temp, phone_nums):
        self.entry = None
        self.setPoint = None
        self.master = root
        self.initWindow(fig)
        self.max_temp = max
        self.min_temp = min_temp
        self.sending_number = phone_nums[0]  # Twilio's number
        self.receiving_number = phone_nums[1]  # Your phone number

    def initWindow(self, fig):
        self.frame = Frame(self.master)
        self.master.title("Real Time Temperature")
        canvas = FigureCanvasTkAgg(fig, master=self.master)
        canvas.draw()
        canvas.get_tk_widget().grid(row=0, column=1, sticky=tkinter.E + tkinter.S)
        self.master.columnconfigure(0, weight=1)
        self.frame.grid(row=0, column=0, sticky="n")

        maxTemp_label = tkinter.Label(self.frame, text="Max Temp").grid(row=1, column=0, sticky="w")
        maxTemp_entry = tkinter.Entry(self.frame).grid(row=1, column=1, sticky=tkinter.E + tkinter.W)
        maxTemp_Button = tkinter.Button(self.frame, text="Update").grid(row=1, column=3, sticky="we")

        minTemp_label = tkinter.Label(self.frame, text="Min Temp").grid(row=2, column=0, sticky="w")
        minTemp_entry = tkinter.Entry(self.frame).grid(row=2, column=1, sticky=tkinter.E)
        minTemp_Button = tkinter.Button(self.frame, text="Update").grid(row=2, column=3, sticky="we")

        phone_label = tkinter.Label(self.frame, text="Phone #").grid(row=3, column=0, sticky="w")
        phone_entry = tkinter.Entry(self.frame).grid(row=3, column=1, sticky=tkinter.E + tkinter.W)
        phone_Button = tkinter.Button(self.frame, text="Update").grid(row=3, column=3, sticky="we")

        Button4 = tkinter.Button(self.frame, text="Turn off LEDs").grid(row=4, column=0, sticky="we")

    def update_max_temp(self):
        print("Yo")
        # TODO: get value either from entry box or send it through the callback in the button
        # self.max_temp = Entry box value

    def update_min_temp(self):
        print("Yo")
        # TODO: get value either from entry box or send it through the callback in the button
        # self.min_temp = Entry box value

    def update_phone_number(self):
        print("Yo")
        # TODO: get value either from entry box or send it through the callback in the button
        # self.receiving_number = Entry box value


# ---------------------------------------------------------------------------------------------------------------------
def animate(self, sm, lines, lineValueText, lineLabel):
    value, = struct.unpack('f', sm.rawData)
    # this is smoothing out the arduino's random data bc I'm too lazy to change the code in the arduino
    # new_value = ((5/20)*(value-30)) + 30
    # TODO: what is value when arduino is off/unplugged?
    if value is None:  # this case handles when arudino if "off"
        sm.data.appendleft(value)  # we get the latest data point and append it to our array
        lineValueText.set_text('No data available')
    elif value == -127:
        value = None
        sm.data.appendleft(value)  # we get the latest data point and append it to our array
        lineValueText.set_text('Sensor Unplugged')
    else:
        sm.data.appendleft(value)  # we get the latest data point and append it to our array
        lines.set_data(range(sm.plotMaxLength), sm.data)
        lineValueText.set_text(lineLabel + ' = ' + str(value))


# ---------------------------------------------------------------------------------------------------------------------
def main():
    # setup serial port
    portName = '/dev/cu.usbserial-1440'
    baudRate = 38400
    maxPlotLength = 101  # number of points in x-axis
    dataNumBytes = 4  # number of bytes of 1 data point
    sm = serialManagement(portName, baudRate, maxPlotLength, dataNumBytes)  # initializes all required variables
    sm.readSerialStart()  # starts background thread

    # setup texting service
    phone_nums = ['+15156196749', '+15153710142']  # Twilio's number, your number
    max_temp = 36
    min_temp = 24

    # setup plot
    pltInterval = 1000  # Period at which the plot animation update in ms
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
    lineLabel = "Temperature (\u00b0C)"
    lines = ax.plot([], [], label=lineLabel)[0]
    lineValueText = ax.text(0.50, 0.90, '', transform=ax.transAxes)
    plt.legend(loc="upper left")

    # Tkinter's GUI
    root = tkinter.Tk()
    Window(fig, root, max_temp, min_temp, phone_nums)

    # Animates the plot
    anim = animation.FuncAnimation(fig, animate, fargs=(sm, lines, lineValueText, lineLabel), interval=pltInterval)

    root.mainloop()

    sm.close()


if __name__ == '__main__':
    main()
