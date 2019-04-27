class SetPoint {
  public:
    enum DayType {Weekday, Weekend};
    void set_values(DayType dt, int hr, int mn, int t, bool am) {
      day_type = dt;
      h = hr;
      m = mn;
      temp = t; 
      am_on = am;
      is_set = true;
    }
    DayType get_dayType() { return day_type; }
    int get_hour() { return h; }
    int get_min()  { return m; }
    int get_temp() { return temp; }
    void clear_data() { is_set = false; }
    bool is_am_on() { return am_on; }
    String get_dayType_asString() {
      // This doesn't work
      if (day_type = Weekday) { return "Weekday"; }
      return "Weekend";
    }

  private:
    DayType day_type;
    int h;
    int m;
    bool am_on;
    int temp;
    bool is_set;
};
