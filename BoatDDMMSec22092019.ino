

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
//#include <Time.h>
#include <TimeLib.h>
int intens = 1;  //display intensity 0 is okay indoors, max 15 is burn it up!
int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI ) 11 DIN ,13 CLK
int numberOfHorizontalDisplays = 8;
int numberOfVerticalDisplays = 1;
int sensorValue = 1;
int sensorValue1 = 1;
int latl = 1;
int K0 = 0;
int K1 = 1;

float maxspeed = 0;

static const int magvar = -23  ; // the magnetic variation in wellington in 2015 is East 23  (-23)

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
String Version = "19Sep 2019";
int wait200 = 200; // In milliseconds
int wait1 = 1000; // In milliseconds
String timetape = "00:00:00";
String testtape = "00:00:00";

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels
int shift = 1; // how far from the left edege to start
int l_count = 0;
int lastsec = 0 ;
int rawspeed = 0;
float smoothspeed = 0;

float goodspeed = 00.0;
int KM = 0;
int smoothcourse = 0;// 2 for 2 sec avg, 3 for 3 sec avg else no avg
int mycourse;
int oldcourse;
int oldvalue = 1;
int newvalue = 1;
int OldKnob = 5;
int oldvalue1 = 1;
int newvalue1 = 1;
int OldKnob1 = 5;
int DD;
int MM;
double Sec;
double mm;
const int offset = 12;  // New Zealand Standard Time (NZST)
//int DST = 0; // If DST is on, increase offset by 1 hour

void setup()
{
  Serial.begin(115200);
  ss.begin(GPSBaud);

  matrix.setIntensity(intens); // Use a value between 0 and 15 for brightness

  // Adjust to your own needs
  matrix.setPosition(0, 0, 0); // The first display is at <0, 0>
  matrix.setPosition(1, 1, 0); // The second display is at <1, 0>
  matrix.setPosition(2, 2, 0); // The 1hird display is at <2, 0>
  matrix.setPosition(3, 3, 0); // And the last display is at <3, 0>
  matrix.setPosition(4, 4, 0); // The first display is at <0, 0>
  matrix.setPosition(5, 5, 0); // The second display is at <1, 0>
  matrix.setPosition(6, 6, 0); // The 1hird display is at <2, 0>
  matrix.setPosition(7, 7, 0); // And the last display is at <3, 0>

  //  ...
  //  matrix.setRotation(0, 2);    // The first display is position upside down
  //  matrix.setRotation(3, 2);    // The same hold for the last display
  matrix.setRotation(0, 1);    // The first display is position up
  matrix.setRotation(1, 1);    // The first display is position up
  matrix.setRotation(2, 1);    // The first display is position up
  matrix.setRotation(3, 1);    // The first display is position up
  matrix.setRotation(4, 1);    // The first display is position up
  matrix.setRotation(5, 1);    // The first display is position up
  matrix.setRotation(6, 1);    // The first display is position up
  matrix.setRotation(7, 1);    // The first display is position up

  // writetoscreen();
  matrix.fillScreen(LOW);
  DisplayVers();
}

void loop()
{
  smartDelay(0);
    if ((gps.time.second()) != lastsec)
  {

    lastsec = gps.time.second();
    setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());
    adjustTime( offset * SECS_PER_HOUR);
    if (IsDST(day(), month(), weekday() )) adjustTime(SECS_PER_HOUR);
    //adjustTime(SECS_PER_HOUR);

    if (KnobValue() != OldKnob) {
      matrix.fillScreen(0);
    }
    if (KnobValue() == 0)
      TimeMinSec();
    if (KnobValue() == 1)
      TimeYear();
    if (KnobValue() == 2)
      LatLongM();
    if (KnobValue() == 3)
      SpeedCourseKM();
    if (KnobValue() == 4 )
      SpeedCourse7();
    if (KnobValue() == 5 )
      ShowMaxSpeed();
    if (gps.hdop.value() < 50)
      if (gps.speed.value() > maxspeed ) maxspeed = gps.speed.value();
    OldKnob = KnobValue();

    SetBrightness();
    smartDelay(300);
    showsats();
    writetoscreen();
  }
  //  showsats();
}
void ShowMaxSpeed()
{
  timetape = " Max ";
  tomatrixPos(2, timetape);

  timetape = maxspeed;
  tomatrixPos(32, timetape);
}
void LatLong()
{
  String ToPrint = "";
  //  timetape = gps.location.lat();
  //  DD_DDDDDtoDDMMSS(gps.location.lat() );
  if (latl == 1)
  { DD_DDDDDtoDDMMSS(gps.location.lat() );
    latl = 0    ;
  }

  else {
    DD_DDDDDtoDDMMSS(gps.location.lng() );
    latl = 1;
  }
  tomatrixPos(0,  String((DD)));
  tomatrixPos(18, String(abs(MM)));
  tomatrixPos(32, String(abs(Sec)));

}
void LatLongM()
{
  String ToPrint = "";
  if (latl == 1)
  { DD_DDDDDtoDDMMmm(gps.location.lat() );
    latl = 0    ;
  }

  else {
    DD_DDDDDtoDDMMmm(gps.location.lng() );
    latl = 1;
  }
  tomatrixPos(0,  String((DD)));
  tomatrixPos(18, String(abs(MM)));
  tomatrixPos(32, String(abs(round(mm / 10))));
}

void DD_DDDDDtoDDMMmm( double DD_DDDDD )
{
  DD = int(DD_DDDDD);
  MM = ((DD_DDDDD - DD) * 60);
  mm  = (((DD_DDDDD - DD) * 60) - MM) * 10000;
  //Sec =   ((DD_DDDDD - DD) * 60 - MM) * 60;
  Serial.print("  DD MM mm ");
  Serial.print((DD));
  Serial.print(abs(MM));
  Serial.print(abs(mm));
}
void DD_DDDDDtoDDMMSS( double DD_DDDDD )
{
  DD = int(DD_DDDDD);
  MM = int((DD_DDDDD - DD) * 60);
  Sec =   ((DD_DDDDD - DD) * 60 - MM) * 60;
  Serial.print("  DD MM Sec ");
  Serial.print(abs(DD));
  Serial.print(abs(MM));
  Serial.println(abs(Sec));
}

void SpeedCourse7()
{
  rawspeed = (gps.speed.value());//              + (15 * random(200) );
  if (rawspeed < 10) {
    goodspeed  = 0;
    smoothspeed = 0;
  }
  else {
    smoothspeed = (smoothspeed  + rawspeed) / 2; // average out the speed
    goodspeed = ((smoothspeed)  ) / 10;  // drop 100th
    goodspeed = goodspeed / 10;
  }
  {
    if (goodspeed <  10) timetape =  " "; else
      timetape =  "";

    timetape += ((goodspeed));
  }
  tomatrixPos(32, timetape);

  mycourse = gps.course.deg() ; // just the most recent reading plus the magvar
  if (smoothcourse == 2)  //average over two readings
  {
    mycourse = (gps.course.deg() + oldcourse) / 2;
    timetape = ".";
  }
  if (smoothcourse == 3)  //average over three readings
  {
    mycourse = (gps.course.deg() + oldcourse + oldcourse) / 3;
    timetape = ":";
  }
  timetape = tape3digits(MakeMag(mycourse, magvar));
  oldcourse = mycourse;
  tomatrixPos(6, timetape);

  tomatrixPos(25, "M");
}
int MakeMag(int thecourse, int var)
{
  int answer;
  answer = thecourse + var;
  if (answer > 360) {
    answer = answer - 360;
  }
  if (answer < 0) {
    answer = answer + 360;
  }
  return answer;
}
void SpeedCourseKM()
{
  // KMPH
  timetape = "Km";
  rawspeed = (gps.speed.kmph());// +  random(190);
  smoothspeed = (smoothspeed  + rawspeed) / 2; // average out the speed
  KM = ((smoothspeed));
  //goodspeed = goodspeed / 10;
  if (KM <  10) timetape +=  "  "; else if
  (KM < 100) timetape +=  " ";
  timetape += ((KM));

  tomatrixPos(32, timetape);
  //note that on KM we use true
  mycourse = gps.course.deg();  // just the most recent reading
  timetape = "T";
  timetape += tape3digits(mycourse);
  oldcourse = mycourse;
  tomatrixPos(0, timetape);
}
/*void tomatrix27(String thetape)
  {
  for ( int i = 0 ; i <  thetape.length() ; i++ ) {
    int letter = i ;
    int x = (24 + i * width);
    matrix.drawChar(x, 1, thetape[letter], 1, 0, 1);

    matrix.write(); // Send bitmap to display
  }
  }*/
void tomatrixPos(int pos, String thetape)
{
  for ( int i = 0 ; i <  thetape.length() ; i++ ) {
    int letter = i ;
    int x = (pos + i * width);
    matrix.drawChar(x, 1, thetape[letter], 1, 0, 1);

    matrix.write(); // Send bitmap to display
  }
}

int KnobValue ()
{ if (                     avgvalue(K0) < 200)    {
    return 0;
  }
  if (avgvalue(K0) >= 200 && avgvalue(K0) < 400)    {
    return 1;
  }
  if (avgvalue(K0) >= 400 && avgvalue(K0) < 500)    {
    return 2;
  }
  if (avgvalue(K0) >= 500 && avgvalue(K0) < 600)    {
    return 5;
  }
  if (avgvalue(K0) >= 600 && avgvalue(K0) < 800)    {
    return 3;
  }

  if (avgvalue(K0) >= 800 )                       {
    return 4;
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

void TimeMinSec()
{
  tomatrixPos(2, tape2digits(hour()));
  tomatrixPos(14, "hrs");
  tomatrixPos(34, tape2digits(minute()));
  tomatrixPos(47, tape2digits(second()));
}

void TimeYear()
{
  if (day() > 9)
    timetape = day();
  else {
    timetape = " ";
    timetape += day();
  }
  timetape += "/";
  timetape += tape2digits(month());
  timetape += "  ";
  tomatrixPos(0, timetape);

  if (IsDST(day(), month(), weekday() )) timetape = "d";
  else
    timetape = " ";
  // tomatrixPos(32, timetape);
  timetape += (year());
  //  timetape += "  ";
  tomatrixPos(32, timetape);
}

String tape3digits(int number)
{
  String tempstring = "temp";
  if (number >= 10 && number < 100) {
    tempstring = "0";
    tempstring += String(number);
    return tempstring;
  }
  if (number >= 0 && number < 10) {
    tempstring = "00";
    tempstring += String(number);
    return tempstring;
  }
  else return (String(number));
}
String tape2digits(int number) {
  String tempstring = "temp";
  if (number >= 0 && number < 10) {
    tempstring = '0';
    tempstring += String(number);
    return tempstring;
  }
  else return (String(number));
}

/*
  void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.print('0');
  }
  Serial.print(number);
  }

  void print3digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.print('00');
  }
  if (number >= 10 && number < 100) {
    Serial.print('0');
  }
  if (number > 360) {
    Serial.print(F("000"));
  }
  else Serial.print(number);
  }*/

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


void DisplayVers()
{
  matrix.fillScreen(0);
  timetape = Version;
  tomatrix(timetape);
  delay(wait1);
  //matrix.fillScreen(0);

}
void writetoscreen()
{
  Serial.println(F("Sats HDOP Latitude   Longitude   Fix  "));
  Serial.println(F("          (deg)      (deg)       Age  "));

  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  printInt(gps.hdop.value(), gps.hdop.isValid(), 5);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  printInt(gps.location.age(), gps.location.isValid(), 5);
  Serial.print(" Intens ");
  Serial.print(intens);
  Serial.print(" Max ");
  Serial.print(maxspeed);

  Serial.print(" Knob 1 ");
  Serial.print(oldvalue);
  Serial.print(" Knob 2 ");
  Serial.println(oldvalue1);

}

void tomatrix(String thetape)
{
  for ( int i = 0 ; i <  thetape.length() ; i++ ) {
    int letter = i ;
    int x = (shift + i * width);
    matrix.drawChar(x, 1, thetape[letter], 1, 0, 1);
    matrix.write(); // Send bitmap to display

  }
}

/*void tomatrix2(String thetape)
  {
  for ( int i = 0 ; i <  thetape.length() ; i++ ) {
    int letter = i ;
    int newshift = shift;
    if ((letter >= 1) && (letter  <= 4)) newshift = (newshift - 1);
    else if (letter > 4) newshift = (shift + 1);

    else newshift = shift;
    int x = (newshift + i * width);
    matrix.drawChar(x, 1, thetape[letter], 1, 0, 1);
    matrix.write(); // Send bitmap to display
  }
  }*/

int avgvalueone(int KS)
{
  sensorValue1 = analogRead(KS);   // average out the pot to stop jumping
  if ((abs(sensorValue1 - oldvalue1 )) > 9)
    newvalue1 = ((sensorValue1 / 2 + oldvalue1 / 2 ) );
  oldvalue1 = newvalue1;
  // Serial.print(oldvalue1);
  return oldvalue1;
}
void SetBrightness()
{
  intens = (avgvalueone(1) / 62);
  if (intens > 13) intens = 13;
  int intense = intens;                        // set to default intensity
  //  if (hour() > 18 || hour() < 6) intense = 0;  // 6 am till 6 pm go dull
  matrix.setIntensity(intense);                // Use a value between 0 and 15 for brightness
}
static void printFloat(float val, bool valid, int len, int prec)
{
  /*  if (!valid)
    {
      while (len-- > 1)
        Serial.print('*');
      Serial.print(' ');
    }
    else
  */  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  // if (valid)
  sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
}
static void printDateTime(TinyGPSDate & d, TinyGPSTime & t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }

  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
}
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
void showsats()
{
  for (int row = 8; row >= 1; row--) {

    matrix.drawPixel(63, (row - 1), ((gps.satellites.value()) >= (8 - (row - 1))));
  }
  matrix.write();
}
