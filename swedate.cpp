/*********************************************************
  $Header: /home/dieter/sweph/RCS/swedate.c,v 1.75 2009/04/08 07:17:29 dieter Exp $
  version 15-feb-89 16:30
  
  swe_date_conversion()
  swe_revjul()
  swe_julday()

************************************************************/
/* Copyright (C) 1997 - 2008 Astrodienst AG, Switzerland.  All rights reserved.
  
  License conditions
  ------------------

  This file is part of Swiss Ephemeris.
  
  Swiss Ephemeris is distributed with NO WARRANTY OF ANY KIND.  No author
  or distributor accepts any responsibility for the consequences of using it,
  or for whether it serves any particular purpose or works at all, unless he
  or she says so in writing.  

  Swiss Ephemeris is made available by its authors under a dual licensing
  system. The software developer, who uses any part of Swiss Ephemeris
  in his or her software, must choose between one of the two license models,
  which are
  a) GNU public license version 2 or later
  b) Swiss Ephemeris Professional License
  
  The choice must be made before the software developer distributes software
  containing parts of Swiss Ephemeris to others, and before any public
  service using the developed software is activated.

  If the developer choses the GNU GPL software license, he or she must fulfill
  the conditions of that license, which includes the obligation to place his
  or her whole software project under the GNU GPL or a compatible license.
  See http://www.gnu.org/licenses/old-licenses/gpl-2.0.html

  If the developer choses the Swiss Ephemeris Professional license,
  he must follow the instructions as found in http://www.astro.com/swisseph/ 
  and purchase the Swiss Ephemeris Professional Edition from Astrodienst
  and sign the corresponding license contract.

  The License grants you the right to use, copy, modify and redistribute
  Swiss Ephemeris, but only under certain conditions described in the License.
  Among other things, the License requires that the copyright notices and
  this notice be preserved on all copies.

  Authors of the Swiss Ephemeris: Dieter Koch and Alois Treindl

  The authors of Swiss Ephemeris have no control or influence over any of
  the derived works, i.e. over software or services created by other
  programmers which use Swiss Ephemeris functions.

  The names of the authors or of the copyright holder (Astrodienst) must not
  be used for promoting any software, product or service which uses or contains
  the Swiss Ephemeris. This copyright notice is the ONLY place where the
  names of the authors can legally appear, except in cases where they have
  given special permission in writing.

  The trademarks 'Swiss Ephemeris' and 'Swiss Ephemeris inside' may be used
  for promoting such software, products or services.
*/

/*
  swe_date_conversion():
  This function converts some date+time input {d,m,y,uttime}
  into the Julian day number tjd.
  The function checks that the input is a legal combination
  of dates; for illegal dates like 32 January 1993 it returns ERR
  but still converts the date correctly, i.e. like 1 Feb 1993.
  The function is usually used to convert user input of birth data
  into the Julian day number. Illegal dates should be notified to the user.

  Be aware that we always use astronomical year numbering for the years
  before Christ, not the historical year numbering.
  Astronomical years are done with negative numbers, historical
  years with indicators BC or BCE (before common era).
  Year 0 (astronomical)  	= 1 BC historical.
  year -1 (astronomical) 	= 2 BC
  etc.
  Many users of Astro programs do not know about this difference.

  Return: OK or ERR (for illegal date)
*********************************************************/

#include "astrolog.h"
#ifdef SWISS

# include "swephexp.h"
# include "sweph.h"
# include "swephlib.h"

static AS_BOOL init_leapseconds_done = FALSE;


int FAR PASCAL_CONV swe_date_conversion(int y,
		     int m,
		     int d,		/* day, month, year */
		     double uttime, 	/* UT in hours (decimal) */
		     char c,		/* calendar g[regorian]|j[ulian] */
		     double *tjd)
{
  int rday, rmon, ryear;
  double rut, jd;
  int gregflag = SE_JUL_CAL;
  if (c == 'g')
    gregflag = SE_GREG_CAL;
  rut = uttime;		/* hours UT */
  jd = swe_julday(y, m, d, rut, gregflag);
  swe_revjul(jd, gregflag, &ryear, &rmon, &rday, &rut);
  *tjd = jd;
  if (rmon == m && rday == d && ryear == y) {
    return OK;
  } else {
    return ERR;
  }
}	/* end date_conversion */

/*************** swe_julday ********************************************
 * This function returns the absolute Julian day number (JD)
 * for a given calendar date.
 * The arguments are a calendar date: day, month, year as integers,
 * hour as double with decimal fraction.
 * If gregflag = SE_GREG_CAL (1), Gregorian calendar is assumed,
 * if gregflag = SE_JUL_CAL (0),Julian calendar is assumed.
  
 The Julian day number is a system of numbering all days continously
 within the time range of known human history. It should be familiar
 to every astrological or astronomical programmer. The time variable
 in astronomical theories is usually expressed in Julian days or
 Julian centuries (36525 days per century) relative to some start day;
 the start day is called 'the epoch'.
 The Julian day number is a double representing the number of
 days since JD = 0.0 on 1 Jan -4712, 12:00 noon (in the Julian calendar).
 
 Midnight has always a JD with fraction .5, because traditionally
 the astronomical day started at noon. This was practical because
 then there was no change of date during a night at the telescope.
 From this comes also the fact the noon ephemerides were printed
 before midnight ephemerides were introduced early in the 20th century.
 
 NOTE: The Julian day number must not be confused with the Julian 
 calendar system.

 Be aware the we always use astronomical year numbering for the years
 before Christ, not the historical year numbering.
 Astronomical years are done with negative numbers, historical
 years with indicators BC or BCE (before common era).
 Year 0 (astronomical)  	= 1 BC
 year -1 (astronomical) 	= 2 BC
 etc.

 Original author: Marc Pottenger, Los Angeles.
 with bug fix for year < -4711   15-aug-88 by Alois Treindl
 (The parameter sequence m,d,y still indicates the US origin,
  be careful because the similar function date_conversion() uses
  other parameter sequence and also Astrodienst relative juldate.)
 
 References: Oliver Montenbruck, Grundlagen der Ephemeridenrechnung,
             Verlag Sterne und Weltraum (1987), p.49 ff
 
 related functions: swe_revjul() reverse Julian day number: compute the
  			       calendar date from a given JD
	            date_conversion() includes test for legal date values
		    and notifies errors like 32 January.
 ****************************************************************/

double FAR PASCAL_CONV swe_julday(int year, int month, int day, double hour, int gregflag) 
{
  double jd;
  double u,u0,u1,u2;
  u = year;
  if (month < 3) u -=1;
  u0 = u + 4712.0;
  u1 = month + 1.0;
  if (u1 < 4) u1 += 12.0;
  jd = floor(u0*365.25)
     + floor(30.6*u1+0.000001)
     + day + hour/24.0 - 63.5;
  if (gregflag == SE_GREG_CAL) {
    u2 = floor(fabs(u) / 100) - floor(fabs(u) / 400);
    if (u < 0.0) u2 = -u2;
    jd = jd - u2 + 2;            
    if ((u < 0.0) && (u/100 == floor(u/100)) && (u/400 != floor(u/400)))
      jd -=1;
  }
  return jd;
}

/*** swe_revjul ******************************************************
  swe_revjul() is the inverse function to swe_julday(), see the description
  there.
  Arguments are julian day number, calendar flag (0=julian, 1=gregorian)
  return values are the calendar day, month, year and the hour of
  the day with decimal fraction (0 .. 23.999999).

  Be aware the we use astronomical year numbering for the years
  before Christ, not the historical year numbering.
  Astronomical years are done with negative numbers, historical
  years with indicators BC or BCE (before common era).
  Year  0 (astronomical)  	= 1 BC historical year
  year -1 (astronomical) 	= 2 BC historical year
  year -234 (astronomical) 	= 235 BC historical year
  etc.

  Original author Mark Pottenger, Los Angeles.
  with bug fix for year < -4711 16-aug-88 Alois Treindl
*************************************************************************/
void FAR PASCAL_CONV swe_revjul (double jd, int gregflag,
	     int *jyear, int *jmon, int *jday, double *jut)
{
  double u0,u1,u2,u3,u4;
  u0 = jd + 32082.5;
  if (gregflag == SE_GREG_CAL) {
    u1 = u0 + floor (u0/36525.0) - floor (u0/146100.0) - 38.0;
    if (jd >= 1830691.5) u1 +=1;
    u0 = u0 + floor (u1/36525.0) - floor (u1/146100.0) - 38.0;
  }
  u2 = floor (u0 + 123.0);
  u3 = floor ( (u2 - 122.2) / 365.25);
  u4 = floor ( (u2 - floor (365.25 * u3) ) / 30.6001);
  *jmon = (int) (u4 - 1.0);
  if (*jmon > 12) *jmon -= 12;
  *jday = (int) (u2 - floor (365.25 * u3) - floor (30.6001 * u4));
  *jyear = (int) (u3 + floor ( (u4 - 2.0) / 12.0) - 4800);
  *jut = (jd - floor (jd + 0.5) + 0.5) * 24.0;
}

/* transform local time to UTC or UTC to local time
 *
 * input 
 *   iyear ... dsec     date and time
 *   d_timezone		timezone offset
 * output
 *   iyear_out ... dsec_out
 * 
 * For time zones east of Greenwich, d_timezone is positive.
 * For time zones west of Greenwich, d_timezone is negative.
 * 
 * For conversion from local time to utc, use +d_timezone.
 * For conversion from utc to local time, use -d_timezone.
 */
void FAR PASCAL_CONV swe_utc_time_zone(
        int32 iyear, int32 imonth, int32 iday,
        int32 ihour, int32 imin, double dsec,
        double d_timezone,
        int32 *iyear_out, int32 *imonth_out, int32 *iday_out,
        int32 *ihour_out, int32 *imin_out, double *dsec_out
        )
{
  double tjd, d;
  AS_BOOL have_leapsec = FALSE;
  double dhour;
  if (dsec >= 60.0) {
    have_leapsec = TRUE;
    dsec -= 1.0;
  }
  dhour = ((double) ihour) + ((double) imin) / 60.0 + dsec / 3600.0;
  tjd = swe_julday(iyear, imonth, iday, 0, SE_GREG_CAL);
  dhour -= d_timezone;
  if (dhour < 0.0) {
    tjd -= 1.0;
    dhour += 24.0;
  }
  if (dhour >= 24.0) {
    tjd += 1.0;
    dhour -= 24.0;
  }
  swe_revjul(tjd + 0.001, SE_GREG_CAL, iyear_out, imonth_out, iday_out, &d);
  *ihour_out = (int) dhour;
  d = (dhour - (double) *ihour_out) * 60;
  *imin_out = (int) d;
  *dsec_out = (d - (double) *imin_out) * 60;
  if (have_leapsec)
    *dsec_out += 1.0;
}

/*
 * functions for the handling of UTC
 */

/* Leap seconds were inserted at the end of the following days:*/
#define NLEAP_SECONDS 26
#define NLEAP_SECONDS_SPACE 100
static int leap_seconds[NLEAP_SECONDS_SPACE] = {
19720630,
19721231,
19731231,
19741231,
19751231,
19761231,
19771231,
19781231,
19791231,
19810630,
19820630,
19830630,
19850630,
19871231,
19891231,
19901231,
19920630,
19930630,
19940630,
19951231,
19970630,
19981231,
20051231,
20081231,
20120630,
20150630,
0  /* keep this 0 as end mark */
};
#define J1972 2441317.5
#define NLEAP_INIT 10

/* Read additional leap second dates from external file, if given.
 */
static int init_leapsec(void)
{
  FILE *fp;
  int ndat, ndat_last;
  int tabsiz = 0;
  int i;
  char s[AS_MAXCH];
  char *sp;
  if (!init_leapseconds_done) {
    init_leapseconds_done = TRUE;
    tabsiz = NLEAP_SECONDS;
    ndat_last = leap_seconds[NLEAP_SECONDS - 1];
    /* no error message if file is missing */
    if ((fp = swi_fopen(-1, "seleapsec.txt", swed.ephepath, NULL)) == NULL)
      return NLEAP_SECONDS; 
    while(fgets(s, AS_MAXCH, fp) != NULL) {
      sp = s;
      while (*sp == ' ' || *sp == '\t') sp++;
        sp++;
      if (*sp == '#' || *sp == '\n')
        continue;
      ndat = atoi(s);
      if (ndat <= ndat_last)
        continue;
      /* table space is limited. no error msg, if exceeded */
      if (tabsiz >= NLEAP_SECONDS_SPACE)
        return tabsiz;
      leap_seconds[tabsiz] = ndat;
      tabsiz++;
    }
    if (tabsiz > NLEAP_SECONDS) leap_seconds[tabsiz] = 0; /* end mark */
    fclose(fp);
    return tabsiz;
  }
  /* find table size */
  tabsiz = 0;
  for (i = 0; i < NLEAP_SECONDS_SPACE; i++) {
    if (leap_seconds[i] == 0) 
      break;
    else
      tabsiz++;
  }
  return tabsiz;
}

/*
 * Input:  Clock time UTC, year, month, day, hour, minute, second (decimal).
 *         gregflag  Calendar flag
 *         serr      error string
 * Output: An array of doubles:
 *         dret[0] = Julian day number TT (ET)
 *         dret[1] = Julian day number UT1
 *
 * Function returns OK or Error.
 *
 * - Before 1972, swe_utc_to_jd() treats its input time as UT1.
 *   Note: UTC was introduced in 1961. From 1961 - 1971, the length of the
 *   UTC second was regularly changed, so that UTC remained very close to UT1.
 * - From 1972 on, input time is treated as UTC.
 * - If delta_t - nleap - 32.184 > 1, the input time is treated as UT1.
 *   Note: Like this we avoid errors greater than 1 second in case that
 *   the leap seconds table (or the Swiss Ephemeris version) is not updated
 *   for a long time.
*/
int32 FAR PASCAL_CONV swe_utc_to_jd(int32 iyear, int32 imonth, int32 iday, int32 ihour, int32 imin, double dsec, int32 gregflag, double *dret, char *serr)
{
  double tjd_ut1, tjd_et, tjd_et_1972, dhour, d;
  int iyear2, imonth2, iday2;
  int i, j, ndat, nleap, tabsiz_nleap;
  /* 
   * error handling: invalid iyear etc. 
   */
  tjd_ut1 = swe_julday(iyear, imonth, iday, 0, gregflag);
  swe_revjul(tjd_ut1, gregflag, &iyear2, &imonth2, &iday2, &d);
  if (iyear != iyear2 || imonth != imonth2 || iday != iday2) {
    if (serr != NULL)
      sprintf(serr, "invalid date: year = %d, month = %d, day = %d", iyear, imonth, iday);
    return ERR;
  }
  if (ihour < 0 || ihour > 23 
   || imin < 0 || imin > 59 
   || dsec < 0 || dsec >= 61
   || (dsec >= 60 && (imin < 59 || ihour < 23 || tjd_ut1 < J1972))) {
    if (serr != NULL)
      sprintf(serr, "invalid time: %d:%d:%.2f", ihour, imin, dsec);
    return ERR;
  }
  dhour = (double) ihour + ((double) imin) / 60.0 + dsec / 3600.0;
  /* 
   * before 1972, we treat input date as UT1 
   */
  if (tjd_ut1 < J1972) {
    dret[1] = swe_julday(iyear, imonth, iday, dhour, gregflag);
    dret[0] = dret[1] + swe_deltat(dret[1]);
    return OK;
  }
  /* 
   * if gregflag = Julian calendar, convert to gregorian calendar 
   */
  if (gregflag == SE_JUL_CAL) {
    gregflag = SE_GREG_CAL;
    swe_revjul(tjd_ut1, gregflag, &iyear, &imonth, &iday, &d);
  }
  /* 
   * number of leap seconds since 1972: 
   */
  tabsiz_nleap = init_leapsec();
  nleap = NLEAP_INIT; /* initial difference between UTC and TAI in 1972 */
  ndat = iyear * 10000 + imonth * 100 + iday;
  for (i = 0; i < tabsiz_nleap; i++) {
    if (ndat <= leap_seconds[i])
      break;
    nleap++;
  }
  /*
   * For input dates > today:
   * If leap seconds table is not up to date, we'd better interpret the
   * input time as UT1, not as UTC. How do we find out? 
   * Check, if delta_t - nleap - 32.184 > 0.9
   */
  d = swe_deltat(tjd_ut1) * 86400.0;
  if (d - (double) nleap - 32.184 >= 1.0) {
    dret[1] = tjd_ut1 + dhour / 24.0;
    dret[0] = dret[1] + swe_deltat(dret[1]);
    return OK;
  }
  /* 
   * if input second is 60: is it a valid leap second ? 
   */
  if (dsec >= 60) {
    j = 0;
    for (i = 0; i < tabsiz_nleap; i++) {
      if (ndat == leap_seconds[i]) {
	j = 1;
	break;
      }
    }
    if (j != 1) {
      if (serr != NULL)
	sprintf(serr, "invalid time (no leap second!): %d:%d:%.2f", ihour, imin, dsec);
      return ERR;
    }
  }
  /* 
   * convert UTC to ET and UT1 
   */
  /* the number of days between input date and 1 jan 1972: */
  d = tjd_ut1 - J1972;
  /* SI time since 1972, ignoring leap seconds: */
  d += (double) ihour / 24.0 + (double) imin / 1440.0 + dsec / 86400.0; 
  /* ET (TT) */
  tjd_et_1972 = J1972 + (32.184 + NLEAP_INIT) / 86400.0;
  tjd_et = tjd_et_1972 + d + ((double) (nleap - NLEAP_INIT)) / 86400.0;
  d = swe_deltat(tjd_et);
  tjd_ut1 = tjd_et - swe_deltat(tjd_et - d);
  tjd_ut1 = tjd_et - swe_deltat(tjd_ut1);
  dret[0] = tjd_et;
  dret[1] = tjd_ut1;
  return OK;
}

/*
 * Input:  tjd_et   Julian day number, terrestrial time (ephemeris time).
 *         gregfalg Calendar flag
 * Output: UTC year, month, day, hour, minute, second (decimal).
 *
 * - Before 1 jan 1972 UTC, output UT1.
 *   Note: UTC was introduced in 1961. From 1961 - 1971, the length of the
 *   UTC second was regularly changed, so that UTC remained very close to UT1.
 * - From 1972 on, output is UTC.
 * - If delta_t - nleap - 32.184 > 1, the output is UT1.
 *   Note: Like this we avoid errors greater than 1 second in case that
 *   the leap seconds table (or the Swiss Ephemeris version) has not been
 *   updated for a long time.
 */
void FAR PASCAL_CONV swe_jdet_to_utc(double tjd_et, int32 gregflag, int32 *iyear, int32 *imonth, int32 *iday, int32 *ihour, int32 *imin, double *dsec) 
{
  int i;
  int second_60 = 0;
  int iyear2, imonth2, iday2, nleap, ndat, tabsiz_nleap;
  double d, tjd, tjd_et_1972, tjd_ut, dret[10];
  /* 
   * if tjd_et is before 1 jan 1972 UTC, return UT1
   */
  tjd_et_1972 = J1972 + (32.184 + NLEAP_INIT) / 86400.0; 
  d = swe_deltat(tjd_et);
  tjd_ut = tjd_et - swe_deltat(tjd_et - d);
  tjd_ut = tjd_et - swe_deltat(tjd_ut);
  if (tjd_et < tjd_et_1972) {
    swe_revjul(tjd_ut, gregflag, iyear, imonth, iday, &d);
    *ihour = (int32) d;
    d -= (double) *ihour;
    d *= 60;
    *imin = (int32) d;
    *dsec = (d - (double) *imin) * 60.0;
    return;
  }
  /* 
   * minimum number of leap seconds since 1972; we may be missing one leap
   * second
   */
  tabsiz_nleap = init_leapsec();
  swe_revjul(tjd_ut-1, SE_GREG_CAL, &iyear2, &imonth2, &iday2, &d);
  ndat = iyear2 * 10000 + imonth2 * 100 + iday2;
  nleap = 0; 
  for (i = 0; i < tabsiz_nleap; i++) {
    if (ndat <= leap_seconds[i])
      break;
    nleap++;
  }
  /* date of potentially missing leapsecond */
  if (nleap < tabsiz_nleap) {
    i = leap_seconds[nleap];
    iyear2 = i / 10000;
    imonth2 = (i % 10000) / 100;;
    iday2 = i % 100;
    tjd = swe_julday(iyear2, imonth2, iday2, 0, SE_GREG_CAL);
    swe_revjul(tjd+1, SE_GREG_CAL, &iyear2, &imonth2, &iday2, &d);
    swe_utc_to_jd(iyear2,imonth2,iday2, 0, 0, 0, SE_GREG_CAL, dret, NULL);
    d = tjd_et - dret[0];
    if (d >= 0) {
      nleap++;
    } else if (d < 0 && d > -1.0/86400.0) {
      second_60 = 1;
    }
  }
  /*
   * UTC, still unsure about one leap second
   */
  tjd = J1972 + (tjd_et - tjd_et_1972) - ((double) nleap + second_60) / 86400.0;
  swe_revjul(tjd, SE_GREG_CAL, iyear, imonth, iday, &d);
  *ihour = (int32) d;
  d -= (double) *ihour;
  d *= 60;
  *imin = (int32) d;
  *dsec = (d - (double) *imin) * 60.0 + second_60;
  /*
   * For input dates > today:
   * If leap seconds table is not up to date, we'd better interpret the
   * input time as UT1, not as UTC. How do we find out? 
   * Check, if delta_t - nleap - 32.184 > 0.9
   */
  d = swe_deltat(tjd_et);
  d = swe_deltat(tjd_et - d);
  if (d * 86400.0 - (double) (nleap + NLEAP_INIT) - 32.184 >= 1.0) {
    swe_revjul(tjd_et - d, SE_GREG_CAL, iyear, imonth, iday, &d);
    *ihour = (int32) d;
    d -= (double) *ihour;
    d *= 60;
    *imin = (int32) d;
    *dsec = (d - (double) *imin) * 60.0;
  }
  if (gregflag == SE_JUL_CAL) {
    tjd = swe_julday(*iyear, *imonth, *iday, 0, SE_GREG_CAL);
    swe_revjul(tjd, gregflag, iyear, imonth, iday, &d);
  }
}

/*
 * Input:  tjd_ut   Julian day number, universal time (UT1).
 *         gregfalg Calendar flag
 * Output: UTC year, month, day, hour, minute, second (decimal).
 *
 * - Before 1 jan 1972 UTC, output UT1.
 *   Note: UTC was introduced in 1961. From 1961 - 1971, the length of the
 *   UTC second was regularly changed, so that UTC remained very close to UT1.
 * - From 1972 on, output is UTC.
 * - If delta_t - nleap - 32.184 > 1, the output is UT1.
 *   Note: Like this we avoid errors greater than 1 second in case that
 *   the leap seconds table (or the Swiss Ephemeris version) has not been
 *   updated for a long time.
 */
void FAR PASCAL_CONV swe_jdut1_to_utc(double tjd_ut, int32 gregflag, int32 *iyear, int32 *imonth, int32 *iday, int32 *ihour, int32 *imin, double *dsec) 
{
  double tjd_et = tjd_ut + swe_deltat(tjd_ut);
  swe_jdet_to_utc(tjd_et, gregflag, iyear, imonth, iday, ihour, imin, dsec);
}


#ifdef ASTROLOG
/* Set up path for Swiss Ephemeris to search in for ephemeris files. */

void SwissEnsurePath()
{
  char sz[cchSzDef], serr[AS_MAXCH];
#ifdef ENVIRON
  char *env;
#endif
  static flag fPath = fFalse;

  if (!fPath) {
    /* First look for the file in the current directory. */
    sprintf(serr, ".");
#ifdef ENVIRON
    /* Next look for the file in the directory indicated by the version */
    /* specific system environment variable.                            */
    sprintf(sz, "%s%s", ENVIRONVER, szVersionCore);
    env = getenv(sz);
    if (env && *env)
      sprintf(serr + CchSz(serr), "%s%s", PATH_SEPARATOR, env);
    /* Next look in the directory in the general environment variable. */
    env = getenv(ENVIRONALL);
    if (env && *env)
      sprintf(serr + CchSz(serr), "%s%s", PATH_SEPARATOR, env);
    /* Next look in directory in the version prefix environment variable. */
    env = getenv(ENVIRONVER);
    if (env && *env)
      sprintf(serr + CchSz(serr), "%s%s", PATH_SEPARATOR, env);
#endif
    /* Finally look in a directory specified at compile time. */
    sprintf(serr + CchSz(serr), "%s%s", PATH_SEPARATOR, EPHE_DIR);
    swe_set_ephe_path(serr);
    fPath = fTrue;
  }
}


int rgObjSwiss[cUran] = {SE_VULCAN - SE_FICT_OFFSET_1,
  SE_CUPIDO   - SE_FICT_OFFSET_1, SE_HADES    - SE_FICT_OFFSET_1,
  SE_ZEUS     - SE_FICT_OFFSET_1, SE_KRONOS   - SE_FICT_OFFSET_1,
  SE_APOLLON  - SE_FICT_OFFSET_1, SE_ADMETOS  - SE_FICT_OFFSET_1,
  SE_VULKANUS - SE_FICT_OFFSET_1, SE_POSEIDON - SE_FICT_OFFSET_1};
int rgTypSwiss[cUran] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int rgPntSwiss[cUran] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

/* Given an object index and a Julian Day time, get ecliptic longitude and */
/* latitude of the object and its velocity and distance from the Earth or  */
/* Sun. This basically just calls the Swiss Ephemeris calculation function */
/* to actually do it. Because this is the one of the only routines called  */
/* from Astrolog, this routine has knowledge of and uses both Astrolog and */
/* Swiss Ephemeris definitions, and does things such as translation to     */
/* indices and formats of Swiss Ephemeris.                                 */

flag FSwissPlanet(int ind, real jd, flag fHelio,
  real *obj, real *objalt, real *dir, real *space)
{
  int iobj, iflag, nRet, nTyp, nPnt = 0, ix;
  double jde, xx[6], xnasc[6], xndsc[6], xperi[6], xaphe[6], *px;
  char serr[AS_MAXCH];
  static flag fSwissMosh = fFalse;

  /* Reset Swiss Ephemeris if changing computation method. */
  if (us.fSwissMosh != fSwissMosh)
    swe_close();
  fSwissMosh = us.fSwissMosh;
  SwissEnsurePath();

  /* Convert Astrolog object index to Swiss Ephemeris index. */
  if (ind == oSun && fHelio)
    iobj = SE_EARTH;
  else if (ind <= oPlu)
    iobj = ind-1;
  else if (ind == oChi)
    iobj = SE_CHIRON;
  else if (FBetween(ind, oCer, oVes))
    iobj = ind - oCer + SE_CERES;
  else if (ind == oNod)
    iobj = us.fTrueNode ? SE_TRUE_NODE : SE_MEAN_NODE;
  else if (ind == oLil)
    iobj = us.fTrueNode ? SE_OSCU_APOG : SE_MEAN_APOG;
  else if (FBetween(ind, uranLo, uranHi)) {
    iobj = rgObjSwiss[ind - uranLo];
    nTyp = rgTypSwiss[ind - uranLo];
    nPnt = rgPntSwiss[ind - uranLo];
    if (nTyp < 2)
      iobj += (nTyp <= 0 ? SE_FICT_OFFSET_1 : SE_AST_OFFSET);
    else {
      if (iobj <= oEar)
        iobj = SE_EARTH;
      else if (iobj <= oPlu)
        iobj--;
      else if (iobj == oChi)
        iobj = SE_CHIRON;
      else if (FBetween(iobj, oCer, oVes))
        iobj = iobj - oCer + SE_CERES;
      else if (iobj == oVul)
        iobj = SE_VULCAN;
      else if (FBetween(iobj, uranLo, uranHi))
        iobj = iobj - uranLo + SE_FICT_OFFSET_1;
      else
        return fFalse;
    }
  } else
    return fFalse;

  /* Convert Astrolog calculation settings to Swiss Ephemeris flags. */
  iflag = SEFLG_SPEED;
  iflag |= (us.fSwissMosh ? SEFLG_MOSEPH : SEFLG_SWIEPH);
  if (us.fSidereal) {
    swe_set_sid_mode(SE_SIDM_FAGAN_BRADLEY, 0.0, 0.0);
    iflag |= SEFLG_SIDEREAL;
  }
  if (fHelio && ind != oNod && ind != oLil)
    iflag |= SEFLG_HELCTR;
  if (us.fTruePos)
    iflag |= SEFLG_TRUEPOS;
  if (us.fTopoPos && !fHelio) {
    swe_set_topo(-OO, DFromR(AA), us.elvDef);
    iflag |= SEFLG_TOPOCTR;
  }

  jde = jd + swe_deltat(jd);
  if (nPnt == 0)
    nRet = swe_calc(jde, iobj, iflag, xx, serr);
  else {
    nRet = swe_nod_aps(jde, iobj, iflag, us.fTrueNode ? SE_NODBIT_OSCU :
      SE_NODBIT_MEAN, xnasc, xndsc, xperi, xaphe, serr);
    switch (nPnt) {
    case 1:  px = xnasc; break;
    case 2:  px = xndsc; break;
    case 3:  px = xperi; break;
    default: px = xaphe; break;
    }
    for (ix = 0; ix < 6; ix++)
      xx[ix] = px[ix];
  }
  if (nRet < 0) {
    if (!is.fNoEphFile) {
      is.fNoEphFile = fTrue;
      PrintWarning(serr);
    }
    return fFalse;
  }
  *obj    = xx[0] - is.rSid + us.rZodiacOffset;
  *objalt = xx[1];
  *space  = xx[2];
  *dir    = xx[3];
  return fTrue;
}


/* Compute house cusps and related variables like the Ascendant. Given a  */
/* Julian Day time, location, and house system, call Swiss Ephemeris to   */
/* compute them. This is similar to FSwissPlanet() in that it knows about */
/* and translates between Astrolog and Swiss Ephemeris defintions.        */

flag FSwissHouse(real jd, real lon, real lat, int housesystem,
  real *asc, real *mc, real *ra, real *vtx, real *ep, real *ob, real *off)
{
  double cusp[cSign+1], ascmc[10];
  int i;
  char ch;

  /* Translate Astrolog house index to Swiss Ephemeris house character. */
  /* Don't do hsWhole houses ('W') yet, until after is.rSid computed.   */
  switch (housesystem) {
  case hsPlacidus:      ch = 'P'; break;
  case hsKoch:          ch = 'K'; break;
  case hsEqual:         ch = 'E'; break;
  case hsCampanus:      ch = 'C'; break;
  case hsMeridian:      ch = 'X'; break;
  case hsRegiomontanus: ch = 'R'; break;
  case hsPorphyry:      ch = 'O'; break;
  case hsMorinus:       ch = 'M'; break;
  case hsTopocentric:   ch = 'T'; break;
  case hsAlcabitius:    ch = 'B'; break;
  case hsKrusinski:     ch = 'U'; break;
  case hsVedic:         ch = 'V'; break;
  default:              ch = 'A'; break;
  }

  jd = JulianDayFromTime(jd);
  lon = -lon;

  /* The following is largely copied from swe_houses(). */
  double armc, eps, nutlo[2];
  double tjde = jd + swe_deltat(jd);
  eps = swi_epsiln(tjde, 0) * RADTODEG;
  swi_nutation(tjde, 0, nutlo);
  for (i = 0; i < 2; i++)
    nutlo[i] *= RADTODEG;
  if (!us.fGeodetic)
    armc = swe_degnorm(swe_sidtime0(jd, eps + nutlo[1], nutlo[0]) * 15 + lon);
  else
    armc = lon;
  swe_houses_armc(armc, lat, eps + nutlo[1], ch, cusp, ascmc);

  /* Fill out return parameters for cusp array, Ascendant, etc. */
  *off = -swe_get_ayanamsa(tjde);
  is.rSid = us.rZodiacOffset + (us.fSidereal ? *off : 0.0);

  *asc = Mod(ascmc[SE_ASC] + is.rSid);
  *mc  = Mod(ascmc[SE_MC]  + is.rSid);
  *ra  = RFromD(Mod(ascmc[SE_ARMC] + is.rSid));
  *vtx = Mod(ascmc[SE_VERTEX] + is.rSid);
  *ep  = Mod(ascmc[SE_EQUASC] + is.rSid);
  *ob  = eps;
  for (i = 1; i <= cSign; i++)
    chouse[i] = Mod(cusp[i] + is.rSid);

  /* Want generic MC. Undo if house system flipped it 180 degrees. */
  if ((housesystem == hsCampanus || housesystem == hsRegiomontanus ||
    housesystem == hsTopocentric) && MinDifference(*mc, *asc) < 0.0)
    *mc = Mod(*mc + rDegHalf);

  /* Have Astrolog compute the houses if Swiss Ephemeris didn't do so. */
  if (ch == 'A')
    ComputeHouses(housesystem);
  return fTrue;
}


/* Compute fixed star locations. Given a time, call Swiss Ephemeris to    */
/* compute them. This is similar to FSwissPlanet() in that it knows about */
/* and translates between Astrolog and Swiss Ephemeris defintions.        */

void SwissComputeStars(real jd, flag fInitBright)
{
  char sz[cchSzDef], serr[AS_MAXCH];
  int i, iflag;
  double xx[6], mag;

  SwissEnsurePath();
  if (!fInitBright) {
    jd = JulianDayFromTime(jd);
    iflag = SEFLG_SPEED;
    if (us.fSidereal) {
      swe_set_sid_mode(SE_SIDM_FAGAN_BRADLEY, 0.0, 0.0);
      iflag |= SEFLG_SIDEREAL;
    }
    if (us.fTruePos)
      iflag |= SEFLG_TRUEPOS;
  }
  for (i = 1; i <= cStar; i++) {

    /* In most cases Astrolog's star name is the same as Swiss Ephemeris,  */
    /* however for a few stars we need to translate to a different string. */
    if (szStarCustom[i] == NULL || szStarCustom[i][0] == chNull) {
      if      (i == 3)  sprintf(sz, ",ze-1Ret");         /* Zeta Retic. */
      else if (i == 4)  sprintf(sz, "Pleione");          /* Pleiades    */
      else if (i == 10) sprintf(sz, "Alnilam");          /* Orion       */
      else if (i == 30) sprintf(sz, ",beCru");           /* Becrux      */
      else if (i == 36) sprintf(sz, "Rigel Kentaurus");  /* Rigel Kent. */
      else if (i == 40) sprintf(sz, "Kaus Australis");   /* Kaus Austr. */
      else
        sprintf(sz, "%s", szObjName[oNorm + i]);
    } else
      sprintf(sz, "%s", szStarCustom[i]);

    /* Compute the star location or get the star's brightness. */
    if (!fInitBright) {
      swe_fixstar(sz, jd, iflag, xx, serr);
      planet[oNorm+i] = xx[0];
      planetalt[oNorm+i] = xx[1];
      if (!us.fSidereal)
        ret[oNorm+i] = rDegMax/25765.0/rDayInYear;
      starname[i] = i;
    } else {
      swe_fixstar_mag(sz, &mag, serr);
      rStarBright[i] = mag;
    }
  }
}


/* Wrappers around Swiss ephemeris Julian Day conversion routines. */

double SwissJulDay(int month, int day, int year, real hour, int gregflag)
{
  return swe_julday(year, month, day, hour, gregflag);
}

void SwissRevJul(real jd, int gregflag,
  int *jmon, int *jday, int *jyear, double *jut)
{
  swe_revjul(jd, gregflag, jyear, jmon, jday, jut);
}
#endif /* ASTROLOG */
#endif /* SWISS */

/* swedate.cpp */
