#include <NMEAGPS.h>
#include <Dusk2Dawn.h>
#include <TimeLib.h>

#include "DigitLedDisplay.h"

DigitLedDisplay ld = DigitLedDisplay(11, 10, 13);
//======================================================================
//  Program: NMEA.ino
//
//  Description:  This program uses the fix-oriented methods available() and
//    read() to handle complete fix structures.
//
//    When the last character of the LAST_SENTENCE_IN_INTERVAL (see NMEAGPS_cfg.h)
//    is decoded, a completed fix structure becomes available and is returned
//    from read().  The new fix is saved the 'fix' structure, and can be used
//    anywhere, at any time.
//
//    If no messages are enabled in NMEAGPS_cfg.h, or
//    no 'gps_fix' members are enabled in GPSfix_cfg.h, no information will be
//    parsed, copied or printed.
//
//  Prerequisites:
//     1) Your GPS device has been correctly powered.
//          Be careful when connecting 3.3V devices.
//     2) Your GPS device is correctly connected to an Arduino serial port.
//          See GPSport.h for the default connections.
//     3) You know the default baud rate of your GPS device.
//          If 9600 does not work, use NMEAdiagnostic.ino to
//          scan for the correct baud rate.
//     4) LAST_SENTENCE_IN_INTERVAL is defined to be the sentence that is
//          sent *last* in each update interval (usually once per second).
//          The default is NMEAGPS::NMEA_RMC (see NMEAGPS_cfg.h).  Other
//          programs may need to use the sentence identified by NMEAorder.ino.
//     5) NMEAGPS_RECOGNIZE_ALL is defined in NMEAGPS_cfg.h
//
//  'Serial' is for debug output to the Serial Monitor window.
//
//  License:
//    Copyright (C) 2014-2017, SlashDevin
//
//    This file is part of NeoGPS
//
//    NeoGPS is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NeoGPS is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with NeoGPS.  If not, see <http://www.gnu.org/licenses/>.
//
//======================================================================

//-------------------------------------------------------------------------
//  The GPSport.h include file tries to choose a default serial port
//  for the GPS device.  If you know which serial port you want to use,
//  edit the GPSport.h file.

#include <GPSport.h>

//------------------------------------------------------------
// For the NeoGPS example programs, "Streamers" is common set
//   of printing and formatting routines for GPS data, in a
//   Comma-Separated Values text format (aka CSV).  The CSV
//   data will be printed to the "debug output device".
// If you don't need these formatters, simply delete this section.

#include <Streamers.h>

//------------------------------------------------------------
// This object parses received characters
//   into the gps.fix() data structure

static NMEAGPS  gps;

//------------------------------------------------------------
//  Define a set of GPS fix information.  It will
//  hold on to the various pieces as they are received from
//  an RMC sentence.  It can be used anywhere in your sketch.

static gps_fix  fix;
const int timezone = 12;
int  dst = 0;

//----------------------------------------------------------------
//  This function gets called about once per second, during the GPS
//  quiet time.  It's the best place to do anything that might take
//  a while: print a bunch of things, write to SD, send an SMS, etc.
//
//  By doing the "hard" work during the quiet time, the CPU can get back to
//  reading the GPS chars as they come in, so that no chars are lost.

static void doSomeWork()
{
  // Print all the things!

  trace_all( DEBUG_PORT, gps, fix );
  /*DEBUG_PORT.print( F("Kts   :") );
    DEBUG_PORT.println ((fix.speed_mkn() / 1000));
    DEBUG_PORT.print( F("Heading    :") );
    DEBUG_PORT.println (fix.heading_cd() / 100);
    DEBUG_PORT.print( F("satellites    :") );
    DEBUG_PORT.println (fix.satellites);
    DEBUG_PORT.print( F("fix.status    :") );
    DEBUG_PORT.println (fix.status);*/
  setTime((fix.dateTime.hours + timezone + dst) , fix.dateTime.minutes, fix.dateTime.seconds, fix.dateTime.date, fix.dateTime.month, fix.dateTime.year );
  if (IsDST(day(), month(), weekday() )) dst = 1; else dst = 0;
  DoDusk();
} // doSomeWork

static void DoDusk()
{
  ld.clear();
  if (hour() < 10)  ld.printDigit(0, 7);
  if (minute() < 10)  ld.printDigit(0, 4);
  if (second() < 10)  ld.printDigit(0, 1);
  ld.printDigit(hour(), 6);
  ld.printDigit(minute(), 3);
  ld.printDigit(second(), 0);
  Serial.print( F("Lat    :") );
  Serial.println(fix.latitude()); // 16:53
  Serial.print( F("Long   :") );
  Serial.println(fix.longitude()); // 16:53
  Dusk2Dawn CurrentLoc( fix.latitude(), fix.longitude(), 12);
  Serial.print( F("Day   :") );
  Serial.print(day()); // 16:53
  Serial.print( F("  Month   :") );
  Serial.print(month()); // 16:53
  Serial.print( F(" weeekday   :") );
  Serial.print(weekday()); // 16:53
  Serial.print( F("  Is DST?    :") );
  Serial.println(IsDST(day(), month(), weekday() )); // 16:53
  int CurrentSunrise  = CurrentLoc.sunrise(year(), month(), day() , IsDST(day(), month(), weekday() ));
  int CurrentSunset   = CurrentLoc.sunset( year(), month(), day() , IsDST(day(), month(), weekday() ));
  char timeR[] = "00:00";
  Dusk2Dawn::min2str(timeR, CurrentSunrise);
  Serial.print( F("CurrentSunrise   :") );
  Serial.print(timeR); // 16:53
  char timeS[] = "00:00";
  Dusk2Dawn::min2str(timeS, CurrentSunset);
  Serial.print( F("   CurrentSunset    :") );
  Serial.print(timeS); // 16:53
  int CurrentSolarNoon = CurrentSunrise + (CurrentSunset - CurrentSunrise) / 2;
  char timeN[] = "00:00";
  Dusk2Dawn::min2str(timeN, CurrentSolarNoon);
  Serial.print( F("   CurrentNoon      :") );
  Serial.println(timeN); // 11:56


  /*Serial.print( F("Time till next noon     :") );
    Serial.println(time - timeN); // 11:56
  */
} // DoDusk End

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

void setup()
{
  /* Set the brightness min:1, max:15 */
  ld.setBright(10);
  /* Set the digit count */
  ld.setDigitLimit(8);
  ld.clear();
  ld.printDigit(12345678);
  delay(1000);
  ld.clear();
  ld.printDigit(360, 5);
  ld.printDigit(123, 0);
  delay(1000);
  DEBUG_PORT.begin(9600);
  while (!DEBUG_PORT)
    ;

  DEBUG_PORT.print( F("NMEA.INO: started\n") );
  DEBUG_PORT.print( F("  fix object size = ") );
  DEBUG_PORT.println( sizeof(gps.fix()) );
  DEBUG_PORT.print( F("  gps object size = ") );
  DEBUG_PORT.println( sizeof(gps) );
  DEBUG_PORT.println( F("Looking for GPS device on " GPS_PORT_NAME) );

#ifndef NMEAGPS_RECOGNIZE_ALL
#error You must define NMEAGPS_RECOGNIZE_ALL in NMEAGPS_cfg.h!
#endif

#ifdef NMEAGPS_INTERRUPT_PROCESSING
#error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
#endif

#if !defined( NMEAGPS_PARSE_GGA ) & !defined( NMEAGPS_PARSE_GLL ) & \
      !defined( NMEAGPS_PARSE_GSA ) & !defined( NMEAGPS_PARSE_GSV ) & \
      !defined( NMEAGPS_PARSE_RMC ) & !defined( NMEAGPS_PARSE_VTG ) & \
      !defined( NMEAGPS_PARSE_ZDA ) & !defined( NMEAGPS_PARSE_GST )

  DEBUG_PORT.println( F("\nWARNING: No NMEA sentences are enabled: no fix data will be displayed.") );

#else
  if (gps.merging == NMEAGPS::NO_MERGING) {
    DEBUG_PORT.print  ( F("\nWARNING: displaying data from ") );
    DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
    DEBUG_PORT.print  ( F(" sentences ONLY, and only if ") );
    DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
    DEBUG_PORT.println( F(" is enabled.\n"
                          "  Other sentences may be parsed, but their data will not be displayed.") );
  }
#endif

  DEBUG_PORT.print  ( F("\nGPS quiet time is assumed to begin after a ") );
  DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
  DEBUG_PORT.println( F(" sentence is received.\n"
                        "  You should confirm this with NMEAorder.ino\n") );

  trace_header( DEBUG_PORT );
  DEBUG_PORT.flush();

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
