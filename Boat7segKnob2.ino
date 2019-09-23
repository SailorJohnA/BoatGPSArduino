#include <NMEAGPS.h>
#include <Dusk2Dawn.h>
#include <TimeLib.h>
#include <SPI.h>
#include <bitBangedSPI.h>
#include <MAX7219.h>
#include <GPSport.h>
static NMEAGPS  gps;
static gps_fix  fix;
const float timezone = 12;
int  dst = 0;
int Heading = 360;
float Speed = 999;
float TSpeed = 999;
float SpeedLimit = .29;
int TrueCour;
int MagVar = 23; // Wellington Mag var in 2019 note negative

int DD;
int MM;
double Sec;
double mm;
int latl = 1;

//Following are knob reading variables
int sensorValue = 1;
int sensorValue1 = 1;
int K0 = 0;
int K1 = 1;
int OldKnob = 5;
int oldvalue1 = 1;
int newvalue1 = 1;
int OldKnob1 = 5;
int oldvalue = 1;
int newvalue = 1;

//Pins 8 and 9 are used by the Software serial to talk to the GPS
//Pins 11 and 13 are used for SPI. The DisplayCS const is SPI Chip Select
const int MODEPIN1 = 3;
const int MODEPIN2 = 4;
const int LEDPIN1 = 5;
const int LEDPIN2 = 6;
const int DisplayCS = 10;
const int Display2CS = 7;

const int displaylength = 8;
String message  = "ddd nnn.n";
const byte chips = 1;

//DigitLedDisplay ld = DigitLedDisplay(11, 10, 13);
MAX7219 display (chips, DisplayCS, 11, 13); // Chips / LOAD / DIN / CLK
MAX7219 display2 (chips, Display2CS, 11, 13); // Chips / LOAD / DIN / CLK

void NoGPS()  //Does not work..
{
  ShowTime(1);
  ShowSats(2);
}


static void doSomeWork()
{
  SetArduinoClock();
  SetDST();
  LEDStatus(LEDPIN1, LEDPIN2, fix.status);
  SetBrightness();
  //DoDusk();
  // switch (checkmode1()) {
  switch (checkmode2()) {
    case 0:
      ShowTime(1);
      ShowDirSpeed(2);
      break;
    case 1:
      ShowTime(1);
      ShowDirSpeedMetric(2);
      break;
    case 2:
      ShowLongM(1);
      ShowLatM(2);
      break;
    case 3:
      ShowLong(1);
      ShowLat(2);
      break;
    case 4:
      ShowTime(1);
      ShowSats(2);
      break;
    case 5:
      LatLongOneLine();
      ShowDirSpeedMetric(2);
      break;
    default:
      LatLongOneLineM();
      ShowDirSpeed(2);
      break;
  }
} // doSomeWork

int checkmode1()//this is checkmode with two switchs and static setting
// this would be replaced if a pot is used to set mode
{
  if (digitalRead(MODEPIN1) == LOW)
  { if (digitalRead(MODEPIN2) == LOW) return 1;
    else {
      return 2;
    }
  }
  else if (digitalRead(MODEPIN2) == LOW) return 3;
  else return 4;// so PIN1 goes knots (HI HI) or time (low Hi), PIN2 goes Km (Low Hi) or Sats/date (Low low)
}

int checkmode2()
{ if (                     avgvalue(K0) < 100)    {
    return 0;
  }
  if (avgvalue(K0) >= 100 && avgvalue(K0) < 250)    {
    return 1;
  }
  if (avgvalue(K0) >= 250 && avgvalue(K0) < 400)    {
    return 2;
  }
  if (avgvalue(K0) >= 400 && avgvalue(K0) < 550)    {
    return 3;
  }
  if (avgvalue(K0) >= 550 && avgvalue(K0) < 700)    {
    return 4;
  }
  if (avgvalue(K0) >= 700 && avgvalue(K0) < 850)    {
    return 5;
  }
  if (avgvalue(K0) >= 850 )                       {
    return 6;
  }
}
int avgvalue(int KS)
{
  sensorValue = analogRead(KS);   // average out the pot to stop jumping
  if ((abs(sensorValue - oldvalue )) > 9)
    newvalue = ((sensorValue / 2 + oldvalue / 2 ) );
  oldvalue = newvalue;
  // Serial.print(oldvalue);
  return oldvalue;
}



void ShowLongM(int unit) {
  DD_DDDDDtoDDMMmm(fix.longitude() );
  message = String(DD) + "." + String(abs(MM)) + "." + String(abs(round(mm / 10)));
  to7seg(message, unit);
}
void ShowLatM(int unit) {

  DD_DDDDDtoDDMMmm(fix.latitude() );
  message = String(DD) + "." + String(abs(MM)) + "." + String(abs(round(mm / 10)));
  to7seg(message, unit);
}
void ShowLong(int unit) {
  DD_DDDDDtoDDMMSS(fix.longitude() );
  message = String(DD) + String(abs(MM)) + String(abs(round(Sec)));
  to7seg(message, unit);
}
void ShowLat(int unit) {

  DD_DDDDDtoDDMMSS(fix.latitude() );
  message = String(DD) + String(abs(MM)) + String(abs(round(Sec)));
  to7seg(message, unit);
}

void LatLongOneLineM() {
  if (latl == 1)
  { ShowLatM(1);
    latl = 0    ;
  }
  else {
    ShowLongM(1);
    latl = 1;
  }
}
void LatLongOneLine() {
  if (latl == 1)
  { ShowLat(1);
    latl = 0    ;
  }
  else {
    ShowLong(1);
    latl = 1;
  }
}

void ShowSats(int unit) {
  String ssat;
  String smonth;
  String sdate;
  if (fix.satellites < 10) ssat = " " + String(fix.satellites); else ssat = String(fix.satellites);
  if (month() < 10)       smonth = "0" + String(month());       else smonth = String(month());
  if (day() < 10)         sdate = " " + String(day());         else sdate = String(day());
  // message =  ssat + String(fix.status) +  " " + sdate + "." + smonth; sat staus not needed as LED does that
  message =  ssat +  "  " + sdate + "." + smonth;
  to7seg(message, unit);
}
/*static void ShowSatsStart() {
  String ssat;
  String sminute;
  String ssecs;
  if (fix.satellites < 10) ssat = " " + String(fix.satellites); else ssat = String(fix.satellites);
  if (month() < 10)       smonth = "0" + String(month());       else smonth = String(month());
  if (day() < 10)         sdate = " " + String(day());         else sdate = String(day());
  message = String(fix.status) + " " +  ssat + sdate + "." + smonth;
  to7seg(message);
  }*/

static void ShowTime(int unit) {
  String showhour;
  String showmin;
  String showsec;
  if (hour()   < 10)  showhour = "0" + String(hour());   else showhour = String(hour());
  if (minute() < 10)  showmin  = "0" + String(minute()); else showmin =  String(minute());
  if (second() < 10)  showsec  = "0" + String(second()); else showsec =  String(second());
  message = showhour + " " + showmin + " " + showsec;
  to7seg(message, unit);
} //ShowTime

static void CalcDirSpeed() {
  float RSpeed;
  float TRSpeed;
  float LastSpeed;
  float TLastSpeed;
  TrueCour = (fix.heading_cd() / 100);
  if (TrueCour - MagVar < 0) Heading = TrueCour - MagVar + 360; else
    Heading = TrueCour - MagVar;
  //Heading = 360;
  //Heading = second() * 6;
  RSpeed = fix.speed();
  TRSpeed = fix.speed_kph();
  //TRSpeed = (fix.speed() * 100) - 1;
  if (RSpeed < 0) RSpeed = 0; //(only when in testing mode!)
  Speed =  ( LastSpeed +  RSpeed) / 2;
  //Speed = second() *2.5;   //test data only
  //TSpeed = ( TLastSpeed + TRSpeed) / 2;// Averaged
  TSpeed =  TRSpeed;                    // Not Averaged
  LastSpeed = Speed;
  TLastSpeed = TSpeed;
} //CalcDirSpeed

static void ShowDirSpeed(int unit) {
  String showmag;
  String showknot;
  CalcDirSpeed();
  //if (Speed   < 1)  showknot = " " + String(Speed);   else {
  if (Speed   < 10)  showknot = " " + String(Speed);
  else showknot = String(Speed);
  // }
  if (Speed < SpeedLimit) {
    // Not moving so remove course
    showmag = "---";
  }
  else {
    if (Heading   < 10)  showmag = "00" + String(Heading);   else {
      if (Heading   < 100)  showmag = "0" + String(Heading);
      else showmag = String(Heading);
    }
  }
  message = showmag + "  " + showknot ;
  to7seg(message, unit);
} //ShowDirSpeed

static void ShowDirSpeedMetric (int unit) {
  String showTrue;
  String showkm;
  CalcDirSpeed();
  if (TSpeed   < 1)  showkm = "  0" + String(TSpeed);   else {
    if (TSpeed   < 10)  showkm = "  " + String(TSpeed);   else {
      if (TSpeed   < 100)  showkm = " " + String(TSpeed);
      else showkm = String(TSpeed);
    }
  }
  if (TSpeed < 1) {
    // Not moving so remove course
    showTrue = "---";
  }
  else {
    if (TrueCour   < 10)  showTrue = "00" + String(TrueCour);   else {
      if (TrueCour   < 100)  showTrue = "0" + String(TrueCour);
      else showTrue = String(TrueCour);
    }
  }
  message = " " + showTrue + " " + showkm ;
  to7seg(message, unit);
} //ShowDirSpeedMetric
/*static void DoDusk()
  {
  Dusk2Dawn CurrentLoc( fix.latitude(), fix.longitude(), timezone);
  // int CurrentSunrise  = CurrentLoc.sunrise(year(),  month(),day() , IsDST(day(), month(), weekday() ));
  // int CurrentSunset   = CurrentLoc.sunset(year(),   month(),day()  , IsDST(day(), month(), weekday() ));
  int CurrentSunrise  = CurrentLoc.sunrise(2019, 2, 2 , IsDST(day(), month(), weekday() ));
  int CurrentSunset   = CurrentLoc.sunset(2019, 2, 2 , IsDST(day(), month(), weekday() ));
  char timeR[] = "00:00";
  Dusk2Dawn::min2str(timeR, CurrentSunrise);
  char timeS[] = "00:00";
  Dusk2Dawn::min2str(timeS, CurrentSunset);
  int CurrentSolarNoon = CurrentSunrise + (CurrentSunset - CurrentSunrise) / 2;
  char timeN[] = "00:00";
  Dusk2Dawn::min2str(timeN, CurrentSolarNoon);

  Serial.print( F("CurrentSunrise   :") );
  Serial.print(timeR); // 16:53
  Serial.print( F("   CurrentSunset    :") );
  Serial.print(timeS); // 16:53
  Serial.print( F("   CurrentNoon      :") );
  Serial.println(timeN); // 11:56

  // ld.clear();
  // if (hour() < 10)  ld.printDigit(0, 7);
  // if (minute() < 10)  ld.printDigit(0, 4);
  // if (second() < 10)  ld.printDigit(0, 1);
  // ld.printDigit(hour(), 6);
  // ld.printDigit(minute(), 3);
  // for (int i = 0; i < 8; i++) {
  //   ld.printDigit(char(timeS[i]), 7 - i);
  }

  } // DoDusk End  */

//------------------------------------
//  This is the main GPS parsing loop.

static void GPSloop()
{
  while (gps.available( gpsPort )) {
    fix = gps.read();
    doSomeWork();
  }
 
} // GPSloop

//--------------------------
void to7seg(String thetape, int unit)
{
  int letter;
  int DPS = 0;
  bool DP = false;
  for ( int i = 0 ; i <  displaylength ; i++ ) {
    letter = i + DPS;
    if (thetape[letter + 1] == '.') {
      DP = true;
      DPS++;
    }
    else {
      DP = false;
    }
    if (unit == 1)
      display.sendChar (i, thetape [letter], DP); else {
      if (unit == 2)
        display2.sendChar (i, thetape [letter], DP);
    }
  }
}
static void SetDST() {
  if (IsDST(day(), month(), weekday() )) dst = 1; else dst = 0;
}
static void SetArduinoClock() {
  setTime((fix.dateTime.hours + timezone + dst) , fix.dateTime.minutes, fix.dateTime.seconds, fix.dateTime.date, fix.dateTime.month, fix.dateTime.year );
}
void setup()
{
  int intense = 7;
  display.begin ();
  display.setIntensity (intense);
  display2.begin ();
  display2.setIntensity (intense);

  pinMode(MODEPIN1, INPUT_PULLUP); // Select Time (low) or speed
  pinMode(MODEPIN2, INPUT_PULLUP); // Select KM/True(low) or KNOTS/Mag
  pinMode(LEDPIN1, OUTPUT);
  pinMode(LEDPIN2, OUTPUT);
  pinMode(DisplayCS, OUTPUT);
  pinMode(Display2CS, OUTPUT);
  digitalWrite(DisplayCS, HIGH); // to stop random setup until driven
  digitalWrite(Display2CS, HIGH);// to stop random setup until driven

  gpsPort.begin( 9600 );
}

//--------------------------

void loop()
{
  GPSloop();
}
boolean IsDST(int theday, int themonth, int thedow)
{
  //  DST in NZ ens ends first Sunday in April  4
  //  DST in NZ starts last Sunday in September 9
  // time.h has sunday as 1 and sat as 7 So an extra -1 in the dow calc
  //  Daylight Saving commences on the last Sunday in September, when 2.00am becomes 3.00am. It ends on the first Sunday in April, when 3.00am becomes 2.00am.
  //  May, June, July, August are not DST
  if (themonth > 4 && themonth < 9) {
    return false;
  }
  if (themonth < 4 || themonth > 9) {
    return true;
  }
  int previousSunday = (theday - (thedow - 1));
  //In Sept, we are DST if our previous sunday was on or after the 24.
  if (themonth == 9) {
    return previousSunday >= 24;
  }
  //In April we must be before the first sunday to be dst.
  //That means the previous sunday must be before the 1st.
  return previousSunday <= 0;
}  // end of isdst function
static  void LEDStatus(int OutPin1, int OutPin2, int GPSState)
{
  if (GPSState > 1) {
    digitalWrite(LEDPIN1, HIGH);  //Green on
    digitalWrite(LEDPIN2, LOW);
  } else {
    if (GPSState == 1) {
      digitalWrite(LEDPIN1, HIGH);  //Green on and
      digitalWrite(LEDPIN2, HIGH);  //Red on
    }
    else {
      digitalWrite(LEDPIN1, LOW);
      digitalWrite(LEDPIN2, HIGH);  //Red on
    }
  }
} //End LEDStatus

void DD_DDDDDtoDDMMmm( double DD_DDDDD )
{
  DD = int(DD_DDDDD);
  MM = ((DD_DDDDD - DD) * 60);
  mm  = (((DD_DDDDD - DD) * 60) - MM) * 10000;
  //Sec =   ((DD_DDDDD - DD) * 60 - MM) * 60;
}
void DD_DDDDDtoDDMMSS( double DD_DDDDD )
{
  DD = int(DD_DDDDD);
  MM = int((DD_DDDDD - DD) * 60);
  Sec =   ((DD_DDDDD - DD) * 60 - MM) * 60;
}

void SetBrightness()
{
  int  intens = (avgvalueone(1) / 62);
  if (intens > 13) intens = 13;
  int intense = intens;                        // set to default intensity
  display.setIntensity (intense);
  display2.setIntensity (intense);
}
int avgvalueone(int KS)
{
  sensorValue1 = analogRead(KS);   // average out the pot to stop jumping
  if ((abs(sensorValue1 - oldvalue1 )) > 9)
    newvalue1 = ((sensorValue1 / 2 + oldvalue1 / 2 ) );
  oldvalue1 = newvalue1;
  // Serial.print(oldvalue1);
  return oldvalue1;
}
