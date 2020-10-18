#include <EEPROM.h>

uint8_t attempts = 0;
int solved_state = 0;
int buzzer_pin = 2;
void setup() {
  // put your setup code here, to run once:
  EEPROM.put(0, attempts);
  EEPROM.put(1, solved_state);
  pinMode(buzzer_pin, OUTPUT);
  beep(50,0,1);
}

void loop() {
  // put your main code here, to run repeatedly:
 
}

void beep(int on_time, int off_time, int times)
{
  static bool has_run = 0;
  if (has_run == 0)
  {
    for (int i = times; i > 0; i--)
    {
      digitalWrite(buzzer_pin, HIGH);
      delay(on_time);
      digitalWrite(buzzer_pin, LOW);
      delay(off_time);
    }
    has_run = 1;
  }
}
