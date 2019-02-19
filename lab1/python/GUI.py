from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from tkinter.ttk import Frame
import tkinter
from SerialManagement import SerialManagement


# Sets up GUI.py
class Window:
    max_temp = 0.0
    sent_max_sms = False
    need_to_reset_max_sms = False

    min_temp = 0.0
    sent_min_sms = False
    need_to_reset_min_sms = False

    sending_number = ''  # Twilio's number
    receiving_number = ''  # Your phone number

    def __init__(self, fig, root, sm):
        self.entry = None
        self.setPoint = None
        self.master = root
        self.frame = tkinter.Frame(self.master, bg="white")
        self.sm = sm
        self.leds_on = False

        # set up window
        self.master.title("Real Time Temperature")
        canvas = FigureCanvasTkAgg(fig, master=self.master)
        canvas.draw()
        canvas.get_tk_widget().grid(row=0, column=1, sticky=tkinter.E + tkinter.S + tkinter.N + tkinter.W)
        self.master.columnconfigure(0, weight=1)
        self.frame.grid(row=0, column=0, sticky=tkinter.W + tkinter.E + tkinter.N + tkinter.S)

        # Max Temp elements setup
        tkinter.Label(self.frame, text="Max Temp").grid(row=1, column=0, sticky="w")
        max_temp_label_val = tkinter.StringVar(self.frame, value=Window.max_temp)
        self.max_temp_entry = tkinter.Entry(self.frame, textvariable=max_temp_label_val)
        self.max_temp_entry.grid(row=1, column=1, sticky=tkinter.E + tkinter.W)
        max_temp_button = tkinter.Button(self.frame, text="Update", command=self.update_max_temp)
        max_temp_button.grid(row=1, column=3, sticky="we")

        # Min Temp elements setup
        tkinter.Label(self.frame, text="Min Temp").grid(row=2, column=0, sticky="w")
        min_temp_label_val = tkinter.StringVar(self.frame, value=Window.min_temp)
        self.min_temp_entry = tkinter.Entry(self.frame, textvariable=min_temp_label_val)
        self.min_temp_entry.grid(row=2, column=1, sticky=tkinter.E)
        tkinter.Button(self.frame, text="Update", command=self.update_min_temp).grid(row=2, column=3, sticky="we")

        # Phone Number elements setup
        tkinter.Label(self.frame, text="Phone #").grid(row=3, column=0, sticky="w")
        phone_label_val = tkinter.StringVar(self.frame, value=Window.receiving_number)
        self.phone_entry = tkinter.Entry(self.frame, textvariable=phone_label_val)
        self.phone_entry.grid(row=3, column=1, sticky=tkinter.E + tkinter.W)
        tkinter.Button(self.frame, text="Update", command=self.update_phone_number).grid(row=3, column=3, sticky="we")

        # toggle led button
        tkinter.Button(self.frame, text="Toggle LEDs", command=self.change_leds).grid(row=4, column=0, sticky="we")

        self.frame.rowconfigure(0)
        self.frame.rowconfigure(1, weight=1)
        self.frame.rowconfigure(2, weight=1)
        self.frame.rowconfigure(3, weight=1)
        self.frame.rowconfigure(4, weight=1)
        self.frame.columnconfigure(0, weight=1)
        self.frame.columnconfigure(1, weight=1)
        self.frame.columnconfigure(2, weight=1)
        self.frame.columnconfigure(3, weight=1)

        self.master.rowconfigure(0, weight=1)
        self.master.columnconfigure(0, weight=1)
        self.master.columnconfigure(1, weight=3)

    def update_max_temp(self):
        print(f'Before update max: Max = {Window.max_temp}')  # cwb debug
        Window.max_temp = float(self.max_temp_entry.get())
        print(f'After update max: Max = {Window.max_temp}')  # cwb debug

    def update_min_temp(self):
        print(f'Before update min: Min = {Window.min_temp}')  # cwb debug
        Window.min_temp = float(self.min_temp_entry.get())
        print(f'After update min: Min = {Window.min_temp}')  # cwb debug

    def update_phone_number(self):
        print(f'Before update phone #: {Window.receiving_number}')  # cwb debug
        Window.receiving_number = self.phone_entry.get()
        print(f'After update phone #: {Window.receiving_number}')  # cwb debug

    def change_leds(self):
        if self.leds_on:
            self.sm.toggle_led(SerialManagement.LED_OFF)
        else:
            self.sm.toggle_led(SerialManagement.LED_ON)
        self.leds_on = not self.leds_on
