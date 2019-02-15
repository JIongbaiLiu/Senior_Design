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
# TODO: refactor all this with the current GUI that's being set up in main
class Window(Frame):
    def __init__(self, fig, root, max_temp, min_temp, phone_nums):
        Frame.__init__(self, root)
        self.entry = None
        self.setPoint = None
        self.master = root
        self.initWindow(fig)
        self.max_temp = max_temp
        self.min_temp = min_temp
        self.frame = Frame(root)
        self.receiving_number = phone_nums[0]
        self.sending_number = phone_nums[1]


    def initWindow(self, fig):
        self.master.title("Real Time Temperature")
        canvas = FigureCanvasTkAgg(fig, master=self.master)  # A tk.DrawingArea.
        canvas.draw()
        canvas.get_tk_widget().grid(row=0, column=1)
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

    # def update_max:
    #     # self.max_temp = maxTemp_entry or something like this
    #
    # def update_min:
    #     #stuff
    #
    # def update_phone_number:
    #     #

# ---------------------------------------------------------------------------------------------------------------------
def animate(self, sp, lines, lineValueText, lineLabel):
    value, = struct.unpack('f', sp.rawData)
    # this is smoothing out the arduino's random data bc I'm too lazy to change the code in the arduino
    # new_value = ((5/20)*(value-30)) + 30
    if value > 34:
        value = None
        sp.data.appendleft(value)  # we get the latest data point and append it to our array
        lineValueText.set_text('Sensor Unplugged')
    else:
        sp.data.appendleft(value)  # we get the latest data point and append it to our array
        lines.set_data(range(sp.plotMaxLength), sp.data)
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
    phone_nums = ['+15156196749', '+15153710142']
    # sending_number = '+15156196749'  # Twilio's number
    # receiving_number = '+15153710142'  # Your phone number
    max_temp = 36
    min_temp = 24
    # texting_service = send_sms.TextSMS()

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

    # Tkinter's GUI
    root = tkinter.Tk()

    app = Window(fig, root, max_temp, min_temp, phone_nums)

    # canvas = FigureCanvasTkAgg(fig, master=root)  # A tk.DrawingArea.
    # canvas.draw()
    # # canvas.get_tk_widget().pack(side=tkinter.RIGHT, fill=tkinter.BOTH, expand=1)
    # canvas.get_tk_widget().grid(row=0, column=1)
    # root.columnconfigure(0, weight=1)
    # # root.title("Real Time Temperature")
    #
    # frame = Frame(root)
    # frame.grid(row=0, column=0, sticky="n")

    # maxTemp_label = tkinter.Label(frame, text="Max Temp").grid(row=1, column=0, sticky="w")
    # maxTemp_entry = tkinter.Entry(frame).grid(row=1, column=1, sticky=tkinter.E + tkinter.W)
    # maxTemp_Button = tkinter.Button(frame, text="Update", command=update_max).grid(row=1, column=3, sticky="we")
    #
    # minTemp_label = tkinter.Label(frame, text="Min Temp").grid(row=2, column=0, sticky="w")
    # minTemp_entry = tkinter.Entry(frame).grid(row=2, column=1, sticky=tkinter.E)
    # minTemp_Button = tkinter.Button(frame, text="Update").grid(row=2, column=3, sticky="we")
    #
    # phone_label = tkinter.Label(frame, text="Phone #").grid(row=3, column=0, sticky="w")
    # phone_entry = tkinter.Entry(frame).grid(row=3, column=1, sticky=tkinter.E + tkinter.W)
    # phone_Button = tkinter.Button(frame, text="Update").grid(row=3, column=3, sticky="we")
    #
    # Button4 = tkinter.Button(frame, text="Turn off LEDs").grid(row=4, column=0, sticky="we")

    lineLabel = "Temperature (\u00b0C)"
    lines = ax.plot([], [], label=lineLabel)[0]
    lineValueText = ax.text(0.50, 0.90, '', transform=ax.transAxes)
    anim = animation.FuncAnimation(fig, animate, fargs=(sm, lines, lineValueText, lineLabel),interval=pltInterval)

    plt.legend(loc="upper left")
    root.mainloop()

    sm.close()


if __name__ == '__main__':
    main()
