from threading import Thread
import serial
import time
import collections


# Connects serial port and starts thread to monitor it. As it gets data it puts it in raw_data for animate to use
class SerialManagement:
    LED_OFF = 0
    LED_ON = 1

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
        self.is_on = True

        print('Trying to connect to: ' + str(serial_port) + ' at ' + str(serial_baud) + ' BAUD.')
        try:
            self.serialConnection = serial.Serial(serial_port, serial_baud)
            print('Connected to ' + str(serial_port) + ' at ' + str(serial_baud) + ' BAUD.')
        except serial.SerialException:
            print("Failed to connect with " + str(serial_port) + ' at ' + str(serial_baud) + ' BAUD.')

    # starts background thread to monitor serial port
    def start_serial_thread(self):
        if self.thread is None:
            self.thread = Thread(target=self.read_serial_port)
            self.thread.start()
            # Block till we start receiving values
            while not self.is_receiving:
                time.sleep(0.1)

    # background thread to monitor serial port
    def read_serial_port(self):
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
            self.start_serial_thread()

    def change_led(self, command):
        print("Made it to change_led")  # cwb debug
        self.serialConnection.reset_output_buffer()
        self.serialConnection.write(command)

    def close(self):
        self.is_run = False
        self.thread.join()
        self.serialConnection.close()
        print('Serial Communication Disconnected')
