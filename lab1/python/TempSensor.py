import matplotlib
import SerialManagement as SM
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import struct
import tkinter
from GUI import Window
from sendSMS import TextSMS


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
            # Comment this out when testing!!!!
            # TextSMS.send_message(message, Window.sending_number, Window.receiving_number)
            print(message)
            Window.sent_max_sms = True
            Window.need_to_reset_max_sms = True
    elif value < Window.min_temp:
        if not Window.sent_min_sms:  # if we haven't sent the text yet
            message = f'Temperature value has fallen under {Window.min_temp}'
            # Comment this out when testing!!!
            # TextSMS.send_message(message, Window.sending_number, Window.receiving_number)
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
# and hands off program execution to tkinter
def main():
    # setup serial port
    port_name = '/dev/cu.usbserial-1440'  # this is specific to os and which usb port it's plugged into
    baud_rate = 9600  # make sure this matches the rate specified in arduino code
    max_plot_length = 301  # number of points in x-axis
    data_num_bytes = 4  # number of bytes of 1 data point
    sm = SM.SerialManagement(port_name, baud_rate, max_plot_length, data_num_bytes)
    sm.start_serial_thread()  # starts background thread

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

    # Tkinter's GUI.py
    root = tkinter.Tk()
    Window(fig, root, sm)

    # Animates the plot
    anim = animation.FuncAnimation(fig, animate, fargs=(sm, lines, line_value_text, line_label), interval=plt_interval)

    root.mainloop()

    sm.close()


if __name__ == '__main__':
    main()
