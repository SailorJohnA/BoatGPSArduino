#include <Arduino.h>
#include <TM1637Display.h>
// Module connection pins (Digital Pins)
#define CLK1 2
#define DIO1 3
#define CLK2 4
#define DIO2 5
// The amount of time (in milliseconds) between tests
#define TEST_DELAY   1000

TM1637Display display1(CLK1, DIO1);
TM1637Display display2(CLK2, DIO2);
void setup()
{
}
void loop()
{
  int k;
  int j;
  display1.clear();
  display1.setBrightness(1);

  display1.showNumberDecEx(0, 0x00 , true, 3, 1);
  delay(TEST_DELAY);
  for (k = 0; k < 181; k++) {
    display1.showNumberDecEx(k, 0x00 , true, 3, 1);
    delay(TEST_DELAY / 25);
  }

  display2.clear();
  display2.setBrightness(1);
  display2.showNumberDecEx(0, 0b01000000 , true, 3, 1);
  delay(TEST_DELAY);
  for (k = 0; k < 22; k++) {
    j = (k * 10) + 1;
    display2.showNumberDecEx(j, 0b01000000 , false, 3, 1);
    delay(TEST_DELAY / 3);
  }
  /*
    display.showNumberDecEx(0, 0x00 , true);
    delay(TEST_DELAY);
    display.showNumberDecEx(777, 0xFF , true);
    delay(TEST_DELAY);
    display.showNumberDecEx(777, 0x00 , false);
    delay(TEST_DELAY);
    display.showNumberDecEx(777, 0x00 , true);
    delay(TEST_DELAY);
  */
}
