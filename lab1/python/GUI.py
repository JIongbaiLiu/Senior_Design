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


# Connects serial port and starts thread to monitor it. As it gets data it puts it in raw_data for animate to use
class SerialManagement:
    def __init__(self, serial_port='/dev/cu.usbserial-1440', serial_baud=9600, plot_length=100, data_num_bytes=2):
        self.port = serial_port
        self.baud = serial_baud
        self.plot_max_length = plot_length
        self.data_num_bytes = data_num_bytes
        self.raw_data = bytearray(data_num_bytes)
        self.data = collections.deque([None] * plot_length, maxlen=plot_length)
        self.is_run = True
        self.is_receiving = False
        self.thread = None
        self.plot_timer = 0
        self.previous_timer = 0

        print('Trying to connect to: ' + str(serial_port) + ' at ' + str(serial_baud) + ' BAUD.')
        try:
            self.serialConnection = serial.Serial(serial_port, serial_baud, timeout=4)
            print('Connected to ' + str(serial_port) + ' at ' + str(serial_baud) + ' BAUD.')
        except:
            print("Failed to connect with " + str(serial_port) + ' at ' + str(serial_baud) + ' BAUD.')

    def readSerialStart(self):
        if self.thread == None:
            self.thread = Thread(target=self.backgroundThread)
            self.thread.start()
            # Block till we start receiving values
            while self.is_receiving != True:
                time.sleep(0.1)

    def backgroundThread(self):
        time.sleep(1.0)  # give some buffer time for retrieving data
        try:
            self.serialConnection.reset_input_buffer()
            while self.is_run:
                try:
                    self.serialConnection.readinto(self.raw_data)
                    self.is_receiving = True
                    # print(self.rawData)
                except serial.serialutil.SerialException:
                    # sensor was unplugged during program execution or the program can't read the data
                    print("Device unplugged")
                    break
        except AttributeError:
            print("Sensor not plugged in")
            self.readSerialStart()

    def close(self):
        self.is_run = False
        self.thread.join()
        self.serialConnection.close()
        print('Serial Communication Disconnected')


# Sets up GUI
class Window:
    max_temp = 0.0
    sent_max_sms = False
    need_to_reset_max_sms = False

    min_temp = 0.0
    sent_min_sms = False
    need_to_reset_min_sms = False

    sending_number = 0  # Twilio's number
    receiving_number = 0  # Your phone number

    def __init__(self, fig, root):
        self.entry = None
        self.setPoint = None
        self.master = root
        self.frame = Frame(self.master)

        self.master.title("Real Time Temperature")
        canvas = FigureCanvasTkAgg(fig, master=self.master)
        canvas.draw()
        canvas.get_tk_widget().grid(row=0, column=1, sticky=tkinter.E + tkinter.S)
        self.master.columnconfigure(0, weight=1)
        self.frame.grid(row=0, column=0, sticky="n")

        # Max Temp elements setup
        tkinter.Label(self.frame, text="Max Temp").grid(row=1, column=0, sticky="w")
        max_temp_label_val = tkinter.StringVar(self.frame, value=Window.max_temp)
        self.max_temp_entry = tkinter.Entry(self.frame, textvariable=max_temp_label_val)
        self.max_temp_entry.grid(row=1, column=1, sticky=tkinter.E + tkinter.W)
        max_temp_button = tkinter.Button(self.frame, text="Update", command=self.update_max_temp)
        max_temp_button.grid(row=1, column=3, sticky="we")

        # Min Temp elements setup
        tkinter.Label(self.frame, text="Min Temp").grid(row=2, column=0, sticky="w")
        min_temp_label_val = tkinter.StringVar(self.frame, value=self.max_temp)
        self.min_temp_entry = tkinter.Entry(self.frame, textvariable=min_temp_label_val)
        self.min_temp_entry.grid(row=2, column=1, sticky=tkinter.E)
        tkinter.Button(self.frame, text="Update", command=self.update_min_temp).grid(row=2, column=3,
                                                                                                       sticky="we")

        # Phone Number elements setup
        tkinter.Label(self.frame, text="Phone #").grid(row=3, column=0, sticky="w")
        phone_label_val = tkinter.StringVar(self.frame, value=self.receiving_number)
        self.phone_entry = tkinter.Entry(self.frame, textvariable=phone_label_val)
        self.phone_entry.grid(row=3, column=1, sticky=tkinter.E + tkinter.W)
        tkinter.Button(self.frame, text="Update", command=self.update_phone_number).grid(row=3, column=3,
                                                                                                        sticky="we")

        led_button = tkinter.Button(self.frame, text="Turn off LEDs").grid(row=4, column=0, sticky="we")

    def update_max_temp(self):
        Window.max_temp = float(self.max_temp_entry.get())

    def update_min_temp(self):
        self.min_temp = self.min_temp_entry.get()

    def update_phone_number(self):
        self.receiving_number = self.phone_entry.get()


# This method animates the graph
def animate(self, sm, lines, line_value_text, line_label):
    value, = struct.unpack('f', sm.raw_data)
    line_label_text = str(value)
    # TODO: what is value when arduino is off/unplugged?
    if value is None:  # this case handles when arduino if "off"
        sm.data.appendleft(value)  # TODO: I don't think this is needed anymore
        line_label_text = 'No data available'
    elif value == -127:
        value = None
        sm.data.appendleft(value)  # TODO: I don't think this is needed anymore
        line_label_text = 'Sensor Unplugged'
    elif value > Window.max_temp:
        if not Window.sent_max_sms:  # if we haven't sent the text yet
            message = f'Temperature value has exceeded {Window.max_temp}'
            # send_sms.TextSMS.send_message(message, Window.sending_number, Window.receiving_number)
            print(message)
            Window.sent_max_sms = True
            Window.need_to_reset_max_sms = True
    elif value < Window.min_temp:
        if not Window.sent_min_sms:  # if we haven't sent the text yet
            message = f'Temperature value has fallen under {Window.min_temp}'
            # send_sms.TextSMS.send_message(message, Window.sending_number, Window.receiving_number)
            print(message)
            Window.sent_min_sms = True
            Window.need_to_reset_min_sms = True
    else:
        if Window.need_to_reset_max_sms:
            Window.sent_max_sms = False
            Window.need_to_reset_max_sms = False
            print("Reset Max SMS")
        if Window.need_to_reset_min_sms:
            Window.sent_min_sms = False
            Window.need_to_reset_min_sms = False
            print("Reset Min SMS")

    sm.data.appendleft(value)  # we get the latest data point and append it to our array
    lines.set_data(range(sm.plot_max_length), sm.data)
    line_value_text.set_text(line_label + ' = ' + line_label_text)


# Makes calls to set up serial port & tkinter. Then it starts animation of plot
#  and hands off program execution to tkinter
def main():
    # setup serial port
    port_name = '/dev/cu.usbserial-1440'  # this is specific to os and which usb port it's plugged into
    baud_rate = 9600  # make sure this matches the rate specified in arduino code
    max_plot_length = 101  # number of points in x-axis
    data_num_bytes = 4  # number of bytes of 1 data point
    sm = SerialManagement(port_name, baud_rate, max_plot_length, data_num_bytes)
    sm.readSerialStart()  # starts background thread

    # setup texting service
    phone_nums = ['+15156196749', '+15153710142']  # [Twilio's number, your number]
    Window.sending_number = phone_nums[0]
    Window.receiving_number = phone_nums[1]
    Window.max_temp = 28.0
    Window.min_temp = 24.0

    # setup plot
    plt_interval = 1000  # Period at which the plot animation update in ms
    xmin = 0
    xmax = max_plot_length - 1  # The - 1 allows the line to touch the edge of the plot
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
    line_label = "Temperature (\u00b0C)"
    lines = ax.plot([], [], label=line_label)[0]
    line_value_text = ax.text(0.50, 0.90, '', transform=ax.transAxes)
    plt.legend(loc="upper left")

    # Tkinter's GUI
    root = tkinter.Tk()
    Window(fig, root)

    # Animates the plot
    anim = animation.FuncAnimation(fig, animate, fargs=(sm, lines, line_value_text, line_label), interval=plt_interval)

    root.mainloop()

    sm.close()


if __name__ == '__main__':
    main()
