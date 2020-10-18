
//REVERSE GEOCACHE
//LUCAS SALAS


//include libraries
#include <math.h>
#include <LiquidCrystal_I2C.h> //for lcd display funcitonality
#include <TinyGPS++.h> //for gps functionality
#include <NeoSWSerial.h> //because tx and rx are occpied so augment serial on other pins for gps
#include <EEPROM.h> //to save attempts to memory
#include <Servo.h>
#include <stdio.h>


// Home: 40.671564, -111.951259
// LAC: 40.672318, -111.945058
float trg_lat = 40.672318;
float trg_lng = -111.945058;

//pins
int TXPin = 4;
int RXPin = 3;
int servo1_pin = 11;
int servo2_pin = 10;
uint8_t btn_pin = 12;
int buzzer_pin = 2;

int servo_close = 0;
int servo_open = 50;
bool button_pressed_yet = 0;
uint8_t btn_counter = 0;
uint8_t attempts;
uint8_t attempts_remaining;
uint8_t attempts_allowed = 15;
bool solved_msg_flag = 0;
int dist_to_target;
long bearing_deg;
String cardinal_dir;
int solved_state; //0 is unsolved, 1 is solved, 2 is failed

//INITIALIZE
LiquidCrystal_I2C lcd(0x27, 20, 4); //establish lcd
TinyGPSPlus gps; //create instance called gps
Servo servo;
NeoSWSerial ss(TXPin, RXPin);  // The software-serial connection to the GPS device

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200); //don't forget to change serial monitor baud
  ss.begin(9600); //important, must be at 9600 due to gps hardware
  //  ss.print("GPS TEST");
  pinMode(btn_pin, INPUT_PULLUP);
  pinMode(buzzer_pin, OUTPUT);
  servo.attach(servo1_pin);
  servo.write(servo_close);
  //Setup LCD Display
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("     Hello.");
  delay(5000);
  lcd.clear();
}

void loop()
{
  // put your main code here, to run repeatedly:
  while (ss.available() > 0)
    gps.encode(ss.read());

  solved_state = EEPROM.get(1, solved_state);



  if (solved_state == 0)
  {
    servo.write(servo_close);


    if (gps.location.isValid())
    {
      // clear LCD 1st time through only
      static bool has_run = 0;
      if (has_run == 0)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("      GPS");
        lcd.setCursor(0, 1);
        lcd.print("    ACQUIRED");
        beep(15,0,1);
        smartDelay(3000);
        has_run = 1;
      }

      //      static bool has_run1 = 0;
      //      if (has_run1 == 0)
      //      {
      //        EEPROM.get(0, attempts);
      //        attempts_remaining = attempts_allowed - attempts;
      //        lcd.clear();
      //        lcd.setCursor(0, 0);
      //        lcd.print("ATTEMPTS LEFT:");
      //        lcd.setCursor(0, 1);
      //        lcd.print("      ");
      //        lcd.print(attempts_remaining);
      //        smartDelay(3000);
      //        has_run1 = 1;
      //      }

      lcd.clear();
      lcd.setCursor(0, 0);

      uint8_t btn_state = digitalRead(btn_pin);

      //blink "press button" on second line as long as button hasn't been pressed
      if (button_pressed_yet == false)
      {
        static bool press_button_flag = 0;
        if (press_button_flag)
        {
          lcd.print("                ");
          press_button_flag = 0;
          smartDelay(650);
        }
        else
        {
          lcd.print("Hold Button...");
          press_button_flag = 1;
          smartDelay(650);
        }
      }

      //display info if button is pressed
      if (btn_state ==  0)
      {
        EEPROM.get(0, attempts); //check number of attempts, stored in EEPROM. Manually reset using rest file.
        attempts++; //increase number of attempts
        EEPROM.put(0, attempts);
        attempts_remaining = attempts_allowed - attempts;


        //open servo at location
        calcDirections();
        if (dist_to_target < 30)
        {
          puzzleSolved();
          solved_state = 1;
          EEPROM.put(1, solved_state);
        }
        if (solved_state == 0)
        {
          displayInfo();
          smartDelay(10000);
        }
        if (attempts_remaining == 0 && solved_state != 1) //check to see if user has used all attempts
        {
          solved_state = 2;
          EEPROM.put(1, solved_state);
          lcd.clear();
          lcd.backlight();
          lcd.setCursor(0, 0);
          lcd.print("Womp womp...");
          lcd.setCursor(0, 1);
          lcd.print("No more attempts");
          smartDelay(4000);
        }
        button_pressed_yet = true;
      }


    }
    else   // invalid gps - blink "Acquiring GPS..."
    {
      lcd.setCursor(0, 0);
      lcd.print("Please wait:");
      lcd.setCursor(0, 1);
      static bool acq_disp_flag = 0;
      if (acq_disp_flag)
      {
        lcd.print("                ");
        acq_disp_flag = 0;
        smartDelay(650);
      }
      else
      {
        lcd.print("Acquiring GPS...");
        acq_disp_flag = 1;
        smartDelay(650);
      }



    }
  }
  else if ( solved_state == 1)
  {
    static bool has_run = 0;
    if (has_run == 0)
    {
      servo.write(servo_open);
      lcd.clear();
      lcd.backlight();
      lcd.setCursor(0, 0);
      lcd.print(" Puzzle already");
      lcd.setCursor(0, 1);
      lcd.print("     solved");
      smartDelay(7000);
      lcd.clear();
      lcd.noBacklight();

      has_run = 1;
    }

    uint8_t btn_state = digitalRead(btn_pin);
    if (btn_state == 0)
    {
      lcd.backlight();
      lcd.setCursor(0, 0);
      lcd.print(" Puzzle already");
      lcd.setCursor(0, 1);
      lcd.print("     solved");
      smartDelay(7000);
      lcd.clear();
      lcd.noBacklight();
    }
  }
  else if (solved_state == 2)
  {
    servo.write(servo_close);
    static bool has_run = 0;
    if (has_run == 0)
    {
      lcd.clear();
      lcd.backlight();
      lcd.setCursor(0, 0);
      lcd.print("Puzzle Failed");
      lcd.setCursor(0, 1);
      lcd.print("Return to Lucas");
      smartDelay(7000);
      lcd.clear();
      lcd.noBacklight();
      has_run = 1;
    }

    uint8_t btn_state = digitalRead(btn_pin);
    if (btn_state == 0)
    {
      lcd.backlight();
      lcd.setCursor(0, 0);
      lcd.print("Puzzle Failed");
      lcd.setCursor(0, 1);
      lcd.print("Return to Lucas");
      smartDelay(7000);
      lcd.clear();
      lcd.noBacklight();
    }
  }
}



////////////////  Functions  ////////////////////


static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void calcDirections()
{
  float crrnt_lat = gps.location.lat();
  float crrnt_lng = gps.location.lng();

  dist_to_target = gps.distanceBetween(crrnt_lat, crrnt_lng, trg_lat, trg_lng) * 3.281; //converts to feet at the end
  //Find x and y values to plug into atan() to find bearing to target
  float x_val = cos(trg_lat) * sin(trg_lng - crrnt_lng);
  float y_val = cos(crrnt_lat) * sin(trg_lat) - sin(crrnt_lat) * cos(trg_lat) * cos((trg_lng - crrnt_lng));
  //  Serial.print("Target Long: "); Serial.println(trg_lng, 6);
  //  Serial.print("Current Long: "); Serial.println(crrnt_lng, 6);
  //  Serial.print("x: "); Serial.println(sin(trg_lng - crrnt_lng), 6);
  //  Serial.print("y: "); Serial.println(y_val, 6);
  //  Serial.print("Radians: "); Serial.println(atan2(x_val, y_val) );

  bearing_deg = round(atan2(x_val, y_val) * 180 / 3.14159265); //calculate bearing in degrees. 0 is north, -90 is east, 180 is south, 90 is west.

  //  Serial.print("Bearing: "); Serial.println(bearing_deg);

  if (bearing_deg >= -5 && bearing_deg <= 5)
    cardinal_dir = "NORTH";
  else if (bearing_deg > 5 && bearing_deg < 85)
    cardinal_dir = "NORTHWEST";
  else if (bearing_deg >= 85 && bearing_deg <= 95)
    cardinal_dir = "WEST";
  else if (bearing_deg > 95 && bearing_deg < 175)
    cardinal_dir = "SOUTHWEST";
  else if ( bearing_deg >= 175 || bearing_deg <= -175)
    cardinal_dir = "SOUTH";
  else if (bearing_deg < -5 && bearing_deg > -85)
    cardinal_dir = "NORTHEAST";
  else if (bearing_deg <= -85 && bearing_deg >= -95)
    cardinal_dir = "EAST";
  else if (bearing_deg < -95 && bearing_deg > -175)
    cardinal_dir = "SOUTHEAST";


}


void displayInfo()
{

  static bool has_run1 = 0;
  if (has_run1 == 0);
  {
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0, 0);

    if (dist_to_target >= 1500) //conver distance to miles if greater than 1500 feet
    {
      float dist_mi = dist_to_target / 5280.0;
      lcd.print("Distance (mi):");
      lcd.setCursor(0, 1);
      lcd.print(dist_mi, 2);
    }
    else
    {
      lcd.print("Distance in feet:");
      lcd.setCursor(0, 1);
      lcd.print(dist_to_target);
    }

    smartDelay(6000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Direction to:");
    lcd.setCursor(0, 1);
    lcd.print(cardinal_dir);
    smartDelay(6000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ATTEMPTS LEFT:");
    lcd.setCursor(0, 1);
    lcd.print("      ");
    lcd.print(attempts_remaining);
    smartDelay(6000);
    lcd.clear();
    lcd.noBacklight();
    has_run1 = 1;
  }
}

void puzzleSolved()
{
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("CONGRATULATIONS");
  lcd.setCursor(0, 1);
  lcd.print("      !!!!");
  beep(50,50,5);
  smartDelay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("You have reached");
  lcd.setCursor(0, 1);
  lcd.print("your destination");
  smartDelay(3000);
  servo.write(servo_open);
  smartDelay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Unplug me after");
  lcd.setCursor(0, 1);
  lcd.print("claiming prize..");
  smartDelay(3000);
  lcd.clear();
  lcd.noBacklight();
  smartDelay(10000);


}

void beep(int on_time, int off_time, int times)
{
  static bool has_run = 0;
  if (has_run == 0)
  {
    for (int i = times; i > 0; i--)
    {
      digitalWrite(buzzer_pin, HIGH);
      smartDelay(on_time);
      digitalWrite(buzzer_pin, LOW);
      smartDelay(off_time);
    }
    has_run = 1;
  }
}
