/*
** Astrolog (Version 6.10) File: charts3.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2016 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** The main ephemeris databases and calculation routines are from the
** library SWISS EPHEMERIS and are programmed and copyright 1997-2008 by
** Astrodienst AG. The use of that source code is subject to the license for
** Swiss Ephemeris Free Edition, available at http://www.astro.com/swisseph.
** This copyright notice must not be changed or removed by any user of this
** program.
**
** Additional ephemeris databases and formulas are from the calculation
** routines in the program PLACALC and are programmed and Copyright (C)
** 1989,1991,1993 by Astrodienst AG and Alois Treindl (alois@azur.ch). The
** use of that source code is subject to regulations made by Astrodienst
** Zurich, and the code is not in the public domain. This copyright notice
** must not be changed or removed by any user of this program.
**
** The original planetary calculation routines used in this program have
** been copyrighted and the initial core of this program was mostly a
** conversion to C of the routines created by James Neely as listed in
** 'Manual of Computer Programming for Astrologers', by Michael Erlewine,
** available from Matrix Software.
**
** The PostScript code within the core graphics routines are programmed
** and Copyright (C) 1992-1993 by Brian D. Willoughby (brianw@sounds.wa.com).
**
** More formally: This program is free software; you can redistribute it
** and/or modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version. This program is
** distributed in the hope that it will be useful and inspiring, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details, a copy of which is in the
** LICENSE.HTM file included with Astrolog, and at http://www.gnu.org
**
** Initial programming 8/28-30/1991.
** X Window graphics initially programmed 10/23-29/1991.
** PostScript graphics initially programmed 11/29-30/1992.
** Last code change made 3/19/2016.
*/

#include "astrolog.h"


/*
******************************************************************************
** Multiple Chart Scanning Routines.
******************************************************************************
*/

/* Search through a day, and print out the times of exact aspects among the  */
/* planets during that day, as specified with the -d switch, as well as the  */
/* times when a planet changes sign or direction. To do this, we cast charts */
/* for the beginning and end of the day, or a part of a day, and do a linear */
/* equation check to see if anything exciting happens during the interval.   */
/* (This is probably the single most complicated procedure in the program.)  */

void ChartInDaySearch(flag fProg)
{
  char sz[cchSzDef];
  int source[MAXINDAY], aspect[MAXINDAY], dest[MAXINDAY],
    sign1[MAXINDAY], sign2[MAXINDAY], D1, D2, counttotal = 0, occurcount,
    division, div, fYear, yea0, yea1, yea2, i, j, k, l, s1, s2, s3;
  real time[MAXINDAY], divsiz, d1, d2, e1, e2, f1, f2, g;
  CI ciT;

  /* If parameter 'fProg' is set, look for changes in a progressed chart. */

  ciT = ciTran;
  fYear = us.fInDayMonth && (MonT == 0);
  division = (fYear || fProg) ? 1 : us.nDivision;
  divsiz = 24.0 / (real)division*60.0;

  /* If -dY in effect, then search through a range of years. */

  yea1 = fProg ? YeaT : Yea;
  yea2 = fYear ? (yea1 + us.nEphemYears - 1) : yea1;
  for (yea0 = yea1; yea0 <= yea2; yea0++) {

  /* If -dm in effect, then search through the whole month, day by day. */

  if (us.fInDayMonth) {
    D1 = 1;
    if (fYear) {
      MonT = 1; D2 = DayInYearHi(yea0);
    } else
      D2 = DayInMonth(fProg ? MonT : Mon, yea0);
  } else
    D1 = D2 = Day;

  /* Start searching the day or days in question for exciting stuff. */

  for (DayT = D1; DayT <= D2; DayT = AddDay(Mon, DayT, yea0, 1)) {
    occurcount = 0;

    /* Cast chart for beginning of day and store it for future use. */

    SetCI(ciCore, fYear ? MonT : Mon, DayT, yea0, 0.0, Dst, Zon, Lon, Lat);
    if (us.fProgress = fProg) {
      is.JDp = MdytszToJulian(MonT, DD, yea0, 0.0, Dst, Zon);
      ciCore = ciMain;
    }
    CastChart(fTrue);
    cp2 = cp0;

    /* Now divide the day into segments and search each segment in turn. */
    /* More segments is slower, but has slightly better time accuracy.   */

    for (div = 1; div <= division; div++) {

      /* Cast the chart for the ending time of the present segment. The   */
      /* beginning time chart is copied from the previous end time chart. */

      SetCI(ciCore, fYear ? MonT : Mon, DayT, yea0,
        DegToDec(24.0*(real)div/(real)division), Dst, Zon, Lon, Lat);
      if (fProg) {
        is.JDp = MdytszToJulian(MonT, DD+1, yea0, 0.0, Dst, Zon);
        ciCore = ciMain;
      }
      CastChart(fTrue);
      cp1 = cp2; cp2 = cp0;

      /* Now search through the present segment for anything exciting. */

      for (i = 0; i <= cObj; i++) if (!FIgnore(i) && (fProg || FThing(i))) {
        s1 = SFromZ(cp1.obj[i])-1;
        s2 = SFromZ(cp2.obj[i])-1;

        /* Does the current planet change into the next or previous sign? */

        if (s1 != s2 && !us.fIgnoreSign && occurcount < MAXINDAY) {
          source[occurcount] = i;
          aspect[occurcount] = aSig;
          dest[occurcount] = s2+1;
          time[occurcount] = MinDistance(cp1.obj[i],
            (real)(cp1.dir[i] >= 0.0 ? s2 : s1) * 30.0) /
            MinDistance(cp1.obj[i], cp2.obj[i])*divsiz + (real)(div-1)*divsiz;
          sign1[occurcount] = sign2[occurcount] = s1+1;
          occurcount++;
        }

        /* Does the current planet go retrograde or direct? */

        if ((cp1.dir[i] < 0.0) != (cp2.dir[i] < 0.0) && !us.fIgnoreDir &&
          occurcount < MAXINDAY) {
          source[occurcount] = i;
          aspect[occurcount] = aDir;
          dest[occurcount] = cp2.dir[i] < 0.0;
          time[occurcount] = RAbs(cp1.dir[i])/(RAbs(cp1.dir[i])+
            RAbs(cp2.dir[i]))*divsiz + (real)(div-1)*divsiz;
          sign1[occurcount] = sign2[occurcount] = s1+1;
          occurcount++;
        }

        /* Now search for anything making an aspect to the current planet. */

        for (j = i+1; j <= cObj; j++) if (!FIgnore(j) && (fProg || FThing(j)))
          for (k = 1; k <= us.nAsp; k++) if (FAcceptAspect(i, k, j)) {
            d1 = cp1.obj[i]; d2 = cp2.obj[i];
            e1 = cp1.obj[j]; e2 = cp2.obj[j];
            if (MinDistance(d1, d2) < MinDistance(e1, e2)) {
              SwapR(&d1, &e1);
              SwapR(&d2, &e2);
            }

            /* We are searching each aspect in turn. Let's subtract the  */
            /* size of the aspect from the angular difference, so we can */
            /* then treat it like a conjunction.                         */

            if (MinDistance(e1, Mod(d1-rAspAngle[k])) <
                MinDistance(e2, Mod(d2+rAspAngle[k]))) {
              e1 = Mod(e1+rAspAngle[k]);
              e2 = Mod(e2+rAspAngle[k]);
            } else {
              e1 = Mod(e1-rAspAngle[k]);
              e2 = Mod(e2-rAspAngle[k]);
            }

            /* Check to see if the aspect actually occurs during our    */
            /* segment, making sure we take into account if one or both */
            /* planets are retrograde or if they cross the Aries point. */

            f1 = e1-d1;
            if (RAbs(f1) > rDegHalf)
              f1 -= RSgn(f1)*rDegMax;
            f2 = e2-d2;
            if (RAbs(f2) > rDegHalf)
              f2 -= RSgn(f2)*rDegMax;
            if (MinDistance(Midpoint(d1, d2), Midpoint(e1, e2)) < rDegQuad &&
              RSgn(f1) != RSgn(f2) && occurcount < MAXINDAY) {
              source[occurcount] = i;
              aspect[occurcount] = k;
              dest[occurcount] = j;

              /* Horray! The aspect occurs sometime during the interval.   */
              /* Now we just have to solve an equation in two variables to */
              /* find out where the "lines" cross, i.e. the aspect's time. */

              f1 = d2-d1;
              if (RAbs(f1) > rDegHalf)
                f1 -= RSgn(f1)*rDegMax;
              f2 = e2-e1;
              if (RAbs(f2) > rDegHalf)
                f2 -= RSgn(f2)*rDegMax;
              g = (RAbs(d1-e1) > rDegHalf ?
                (d1-e1)-RSgn(d1-e1)*rDegMax : d1-e1)/(f2-f1);
              time[occurcount] = g*divsiz + (real)(div-1)*divsiz;
              sign1[occurcount] = (int)(Mod(cp1.obj[i]+
                RSgn(cp2.obj[i]-cp1.obj[i])*
                (RAbs(cp2.obj[i]-cp1.obj[i]) > rDegHalf ? -1 : 1)*
                RAbs(g)*MinDistance(cp1.obj[i], cp2.obj[i]))/30.0)+1;
              sign2[occurcount] = (int)(Mod(cp1.obj[j]+
                RSgn(cp2.obj[j]-cp1.obj[j])*
                (RAbs(cp2.obj[j]-cp1.obj[j]) > rDegHalf ? -1 : 1)*
                RAbs(g)*MinDistance(cp1.obj[j], cp2.obj[j]))/30.0)+1;
              occurcount++;
            }
          }
      }
    }

    /* After all the aspects, etc, in the day have been located, sort   */
    /* them by time at which they occur, so we can print them in order. */

    for (i = 1; i < occurcount; i++) {
      j = i-1;
      while (j >= 0 && time[j] > time[j+1]) {
        SwapN(source[j], source[j+1]);
        SwapN(aspect[j], aspect[j+1]);
        SwapN(dest[j], dest[j+1]);
        SwapR(&time[j], &time[j+1]);
        SwapN(sign1[j], sign1[j+1]); SwapN(sign2[j], sign2[j+1]);
        j--;
      }
    }

    /* Finally, loop through and display each aspect and when it occurs. */

    for (i = 0; i < occurcount; i++) {
      s1 = (int)time[i]/60;
      s2 = (int)time[i]-s1*60;
      s3 = us.fSeconds ? (int)(time[i]*60.0)-((s1*60+s2)*60) : -1;
      j = DayT;
      if (fYear || fProg) {
        l = MonT;
        while (j > (k = DayInMonth(l, yea0))) {
          j -= k;
          l++;
        }
      }
      SetCI(ciSave, fYear || fProg ? l : Mon, j, yea0,
        DegToDec(time[i] / 60.0), Dst, Zon, Lon, Lat);
      k = DayOfWeek(fYear || fProg ? l : Mon, j, yea0);
      AnsiColor(kRainbowA[k + 1]);
      sprintf(sz, "(%.3s) ", szDay[k]); PrintSz(sz);
      AnsiColor(kDefault);
      sprintf(sz, "%s %s ",
        SzDate(fYear || fProg ? l : Mon, j, yea0, fFalse),
        SzTime(s1, s2, s3)); PrintSz(sz);
      PrintAspect(source[i], sign1[i],
        (int)RSgn(cp1.dir[source[i]])+(int)RSgn(cp2.dir[source[i]]),
        aspect[i], dest[i], sign2[i],
        (int)RSgn(cp1.dir[dest[i]])+(int)RSgn(cp2.dir[dest[i]]),
        (char)(fProg ? 'e' : 'd'));
      PrintInDay(source[i], aspect[i], dest[i]);
    }
    counttotal += occurcount;
  }
  }
  if (counttotal == 0)
    PrintSz("No transit events found.\n");

  /* Recompute original chart placements as we've overwritten them. */

  ciCore = ciMain; ciTran = ciT;
  CastChart(fTrue);
}


/* Search through a month, year, or years, and print out the times of exact */
/* transits where planets in the time frame make aspect to the planets in   */
/* some other chart, as specified with the -t switch. To do this, we cast   */
/* charts for the start and end of each month, or within a month, and do an */
/* equation check for aspects to the other base chart during the interval.  */

void ChartTransitSearch(flag fProg)
{
  real planet3[objMax], house3[cSign+1], ret3[objMax], time[MAXINDAY];
  char sz[cchSzDef];
  int source[MAXINDAY], aspect[MAXINDAY], dest[MAXINDAY], sign[MAXINDAY],
    isret[MAXINDAY], M1, M2, Y1, Y2, counttotal = 0, occurcount, division,
    div, nAsp, fCusp, i, j, k, s1, s2, s3, s4;
  real divsiz, daysiz, d, e1, e2, f1, f2;
  CI ciT;

  /* Save away natal chart and initialize things. */

  ciT = ciTran;
  for (i = 1; i <= cSign; i++)
    house3[i] = chouse[i];
  for (i = 0; i <= cObj; i++) {
    planet3[i] = planet[i];
    ret3[i] = ret[i];
  }
  if (fProg)
    fCusp = fFalse;
  else {
    fCusp = fTrue;
    for (i = cuspLo; i <= cuspHi; i++)
      fCusp &= ignore2[i];
  }
  division = us.nDivision;
  if (!fProg && !fCusp)
    division = Max(division, 96);
  nAsp = is.fReturn ? aCon : us.nAsp;

  /* Hacks: Searching month number zero means to search the whole year    */
  /* instead, month by month. Searching a negative month means to search  */
  /* multiple years, with the span of the year stored in the "day" field. */

  Y1 = Y2 = YeaT;
  M1 = M2 = MonT;
  if (MonT < 1) {
    M1 = 1; M2 = 12;
    if (MonT < 0) {
      if (DayT < 1) {
        Y1 = YeaT + DayT + 1; Y2 = YeaT;
      } else {
        Y1 = YeaT; Y2 = YeaT + DayT - 1;
      }
    }
  }

  /* Start searching the year or years in question for any transits. */

  for (YeaT = Y1; YeaT <= Y2; YeaT++)

  /* Start searching the month or months in question for any transits. */

  for (MonT = M1; MonT <= M2; MonT++) {
    daysiz = (real)DayInMonth(MonT, YeaT)*24.0*60.0;
    divsiz = daysiz / (real)division;

    /* Cast chart for beginning of month and store it for future use. */

    SetCI(ciCore, MonT, 1, YeaT, 0.0, DstT, ZonT, LonT, LatT);
    if (us.fProgress = fProg) {
      is.JDp = MdytszToJulian(MM, DD, YY, 0.0, DstT, ZonT);
      ciCore = ciMain;
    }
    for (i = 0; i <= oNorm; i++)
      SwapN(ignore[i], ignore2[i]);
    CastChart(fTrue);
    for (i = 0; i <= oNorm; i++)
      SwapN(ignore[i], ignore2[i]);
    cp2 = cp0;

    /* Divide our month into segments and then search each segment in turn. */

    for (div = 1; div <= division; div++) {
      occurcount = 0;

      /* Cast the chart for the ending time of the present segment, and */
      /* copy the start time chart from the previous end time chart.    */

      d = 1.0 + (daysiz/24.0/60.0)*(real)div/(real)division;
      SetCI(ciCore, MonT, (int)d, YeaT,
        DegToDec(RFract(d)*24.0), DstT, ZonT, LonT, LatT);
      if (fProg) {
        is.JDp = MdytszToJulian(MM, DD, YY, 0.0, DstT, ZonT);
        ciCore = ciMain;
      }
      for (i = 0; i <= oNorm; i++)
        SwapN(ignore[i], ignore2[i]);
      CastChart(fTrue);
      for (i = 0; i <= oNorm; i++)
        SwapN(ignore[i], ignore2[i]);
      cp1 = cp2; cp2 = cp0;

      /* Now search through the present segment for any transits. Note that */
      /* stars can be transited, but they can't make transits themselves.   */

      for (i = 0; i <= cObj; i++) if (!FIgnore(i)) {
        for (j = 0; j <= oNorm; j++) {
          if ((is.fReturn ? i != j : FIgnore2(j)) || (fCusp && !FThing(j)))
            continue;

          /* Between each pair of planets, check if they make any aspects. */

          for (k = 1; k <= nAsp; k++) if (FAcceptAspect(i, k, j)) {
            d = planet3[i]; e1 = cp1.obj[j]; e2 = cp2.obj[j];
            if (MinDistance(e1, Mod(d-rAspAngle[k])) <
                MinDistance(e2, Mod(d+rAspAngle[k]))) {
              e1 = Mod(e1+rAspAngle[k]);
              e2 = Mod(e2+rAspAngle[k]);
            } else {
              e1 = Mod(e1-rAspAngle[k]);
              e2 = Mod(e2-rAspAngle[k]);
            }

            /* Check to see if the present aspect actually occurs during the */
            /* segment, making sure we check any Aries point crossings.      */

            f1 = e1-d;
            if (RAbs(f1) > rDegHalf)
              f1 -= RSgn(f1)*rDegMax;
            f2 = e2-d;
            if (RAbs(f2) > rDegHalf)
              f2 -= RSgn(f2)*rDegMax;
            if (MinDistance(d, Midpoint(e1, e2)) < rDegQuad &&
              RSgn(f1) != RSgn(f2) && occurcount < MAXINDAY) {

              /* Ok, we have found a transit. Now determine the time */
              /* and save this transit in our list to be printed.    */

              source[occurcount] = j;
              aspect[occurcount] = k;
              dest[occurcount] = i;
              time[occurcount] = RAbs(f1)/(RAbs(f1)+RAbs(f2))*divsiz +
                (real)(div-1)*divsiz;
              sign[occurcount] = (int)(Mod(
                MinDistance(cp1.obj[j], Mod(d-rAspAngle[k])) <
                MinDistance(cp2.obj[j], Mod(d+rAspAngle[k])) ?
                d-rAspAngle[k] : d+rAspAngle[k])/30.0)+1;
              isret[occurcount] = (int)RSgn(cp1.dir[j]) +
                (int)RSgn(cp2.dir[j]);
              occurcount++;
            }
          }
        }
      }

      /* After all transits located, sort them by time at which they occur. */

      for (i = 1; i < occurcount; i++) {
        j = i-1;
        while (j >= 0 && time[j] > time[j+1]) {
          SwapN(source[j], source[j+1]);
          SwapN(aspect[j], aspect[j+1]);
          SwapN(dest[j], dest[j+1]);
          SwapR(&time[j], &time[j+1]);
          SwapN(sign[j], sign[j+1]);
          SwapN(isret[j], isret[j+1]);
          j--;
        }
      }

      /* Now loop through list and display all the transits. */

      for (i = 0; i < occurcount; i++) {
        s1 = (uint)time[i]/24/60;
        s3 = (uint)time[i]-s1*24*60;
        s2 = s3/60;
        s3 = s3-s2*60;
        s4 = us.fSeconds ? (int)(time[i]*60.0)-(((s1*24+s2)*60+s3)*60) : -1;
        SetCI(ciSave, MonT, s1+1, YeaT, DegToDec((real)
          ((uint)time[i]-s1*24*60) / 60.0), DstT, ZonT, LonT, LatT);
        sprintf(sz, "%s %s ",
          SzDate(MonT, s1+1, YeaT, fFalse), SzTime(s2, s3, s4)); PrintSz(sz);
        PrintAspect(source[i], sign[i], isret[i], aspect[i],
          dest[i], SFromZ(planet3[dest[i]]), (int)RSgn(ret3[dest[i]]),
          (char)(fProg ? 'u' : 't'));

        /* Check for a Solar, Lunar, or any other return. */

        if (aspect[i] == aCon && source[i] == dest[i]) {
          AnsiColor(kMainA[1]);
          sprintf(sz, " (%s Return)", source[i] == oSun ? "Solar" :
            (source[i] == oMoo ? "Lunar" : szObjName[source[i]]));
          PrintSz(sz);
        }
        PrintL();
#ifdef INTERPRET
        if (us.fInterpret)
          InterpretTransit(source[i], aspect[i], dest[i]);
#endif
        AnsiColor(kDefault);
      }
      counttotal += occurcount;
    }
  }
  if (counttotal == 0)
    PrintSz("No transits found.\n");

  /* Recompute original chart placements as we've overwritten them. */

  ciCore = ciMain; ciTran = ciT;
  us.fProgress = fFalse;
  CastChart(fTrue);
}


/* Display a list of planetary rising times relative to the local horizon */
/* for the day indicated in the chart information, as specified with the  */
/* -Zd switch. For the day, the time each planet rises (transits horizon  */
/* in East half of sky), sets (transits horizon in West), reaches its     */
/* zenith point (transits meridian in South half of sky), and nadirs      */
/* transits meridian in North), is displayed.                             */

void ChartInDayHorizon(void)
{
  char sz[cchSzDef];
  int source[MAXINDAY], type[MAXINDAY], sign[MAXINDAY],
    fRet[MAXINDAY], occurcount, division, div, s1, s2, s3, i, j, fT;
  real time[MAXINDAY], rgalt1[objMax], rgalt2[objMax], azialt[MAXINDAY],
    azi1, azi2, alt1, alt2, lon, lat, mc1, mc2, xA, yA, xV, yV, d, k;
  CI ciT;

  fT = us.fSidereal; us.fSidereal = fFalse;
  lon = RFromD(Mod(Lon)); lat = RFromD(Lat);
  division = us.nDivision * 4;
  occurcount = 0;

  ciT = ciTwin; ciCore = ciMain; ciCore.tim = 0.0;
  CastChart(fTrue);
  mc2 = RFromD(planet[oMC]); k = RFromD(planetalt[oMC]);
  EclToEqu(&mc2, &k);
  cp2 = cp0;
  for (i = 1; i <= cObj; i++) {
    rgalt2[i] = planetalt[i];
  }

  /* Loop through the day, dividing it into a certain number of segments. */
  /* For each segment we get the planet positions at its endpoints.       */

  for (div = 1; div <= division; div++) {
    ciCore = ciMain; ciCore.tim = DegToDec(24.0*(real)div/(real)division);
    CastChart(fTrue);
    mc1 = mc2;
    mc2 = RFromD(planet[oMC]); k = RFromD(planetalt[oMC]);
    EclToEqu(&mc2, &k);
    cp1 = cp2; cp2 = cp0;
    for (i = 1; i <= cObj; i++) {
      rgalt1[i] = rgalt2[i]; rgalt2[i] = planetalt[i];
    }

    /* For our segment, check to see if each planet during it rises, sets, */
    /* reaches its zenith, or reaches its nadir.                           */

    for (i = 1; i <= cObj; i++) if (!ignore[i] && FThing(i)) {
      EclToHorizon(&azi1, &alt1, cp1.obj[i], rgalt1[i], lon, lat, mc1);
      EclToHorizon(&azi2, &alt2, cp2.obj[i], rgalt2[i], lon, lat, mc2);
      j = 0;

      /* Check for transits to the horizon. */
      if ((alt1 > 0.0) != (alt2 > 0.0)) {
        d = RAbs(alt1)/(RAbs(alt1)+RAbs(alt2));
        k = Mod(azi1 + d*MinDifference(azi1, azi2));
        j = 1 + 2*(MinDistance(k, rDegHalf) < rDegQuad);

      /* Check for transits to the meridian. */
      } else if (RSgn(MinDifference(azi1, rDegQuad)) !=
        RSgn(MinDifference(azi2, rDegQuad))) {
        j = 2 + 2*(MinDistance(azi1, rDegQuad) < rDegQuad);
        d = RAbs(azi1 - (j > 2 ? rDegQuad : 270.0))/MinDistance(azi1, azi2);
        k = alt1 + d*(alt2-alt1);
      }
      if (j && !ignorez[j-1] && occurcount < MAXINDAY) {
        source[occurcount] = i;
        type[occurcount] = j;
        time[occurcount] = 24.0*((real)(div-1)+d)/(real)division*60.0;
        sign[occurcount] = (int)Mod(cp1.obj[i] +
          d*MinDifference(cp1.obj[i], cp2.obj[i]))/30 + 1;
        fRet[occurcount] = (int)RSgn(cp1.dir[i]) + (int)RSgn(cp2.dir[i]);
        azialt[occurcount] = k;
        ciSave = ciMain;
        ciSave.tim = DegToDec(time[occurcount] / 60.0);
        occurcount++;
      }
    }
  }

  /* Sort each event in order of time when it happens during the day. */

  for (i = 1; i < occurcount; i++) {
    j = i-1;
    while (j >= 0 && time[j] > time[j+1]) {
      SwapN(source[j], source[j+1]);
      SwapN(type[j], type[j+1]);
      SwapR(&time[j], &time[j+1]);
      SwapN(sign[j], sign[j+1]);
      SwapN(fRet[j], fRet[j+1]);
      SwapR(&azialt[j], &azialt[j+1]);
      j--;
    }
  }

  /* Finally display the list showing each event and when it occurs. */

  for (i = 0; i < occurcount; i++) {
    ciSave = ciMain;
    ciSave.tim = DegToDec(time[i] / 60.0);
    j = DayOfWeek(Mon, Day, Yea);
    AnsiColor(kRainbowA[j + 1]);
    sprintf(sz, "(%.3s) ", szDay[j]); PrintSz(sz);
    AnsiColor(kDefault);
    s1 = (int)time[i]/60;
    s2 = (int)time[i]-s1*60;
    s3 = is.fSeconds ? (int)(time[i]*60.0)-((s1*60+s2)*60) : -1;
    sprintf(sz, "%s %s ", SzDate(Mon, Day, Yea, fFalse), SzTime(s1, s2, s3));
    PrintSz(sz);
    AnsiColor(kObjA[source[i]]);
    sprintf(sz, "%7.7s ", szObjName[source[i]]); PrintSz(sz);
    AnsiColor(kSignA(sign[i]));
    sprintf(sz, "%c%.3s%c ",
      fRet[i] > 0 ? '(' : (fRet[i] < 0 ? '[' : '<'), szSignName[sign[i]],
      fRet[i] > 0 ? ')' : (fRet[i] < 0 ? ']' : '>')); PrintSz(sz);
    AnsiColor(kElemA[type[i]-1]);
    if (type[i] == 1)
      PrintSz("rises  ");
    else if (type[i] == 2)
      PrintSz("zeniths");
    else if (type[i] == 3)
      PrintSz("sets   ");
    else
      PrintSz("nadirs ");
    AnsiColor(kDefault);
    PrintSz(" at ");
    if (FOdd(type[i])) {
      j = (int)(azialt[i]*60.0)%60;
      sprintf(sz, "%3d%c%02d'", (int)azialt[i], chDeg1, j); PrintSz(sz);
      if (is.fSeconds) {
        sprintf(sz, "%02d\"", (int)(azialt[i]*3600.0)%60); PrintSz(sz);
      }

      /* For rising and setting events, we'll also display a direction  */
      /* vector to make the 360 degree azimuth value thought of easier. */

      xA = RCosD(azialt[i]); yA = RSinD(azialt[i]);
      if (RAbs(xA) < RAbs(yA)) {
        xV = RAbs(xA / yA); yV = 1.0;
      } else {
        yV = RAbs(yA / xA); xV = 1.0;
      }
      sprintf(sz, " (%.2f%c %.2f%c)",
        yV, yA < 0.0 ? 's' : 'n', xV, xA > 0.0 ? 'e' : 'w'); PrintSz(sz);
    } else
      PrintAltitude(azialt[i]);
    PrintL();
  }
  if (occurcount == 0)
    PrintSz("No horizon events found.\n");

  /* Recompute original chart placements as we've overwritten them. */

  ciCore = ciMain; ciTwin = ciT;
  us.fSidereal = fT;
  CastChart(fTrue);
}


/* Print out an ephemeris - the positions of the planets (at the time in the */
/* current chart) each day during a specified month, as done with the -E     */
/* switch. Display the ephemeris for the whole year if -Ey is in effect.     */

void ChartEphemeris(void)
{
  char sz[cchSzDef];
  int yea, yea1, yea2, mon, mon1, mon2, daysiz, i, j, s, d, m;

  /* If -Ey is in effect, then loop through all months in the whole year. */

  if (us.nEphemYears) {
    yea1 = Yea; yea2 = yea1 + us.nEphemYears - 1; mon1 = 1; mon2 = 12;
  } else {
    yea1 = yea2 = Yea; mon1 = mon2 = Mon;
  }

  /* Loop through the year or years in question. */

  for (yea = yea1; yea <= yea2; yea++)

  /* Loop through the month or months in question, printing each ephemeris. */

  for (mon = mon1; mon <= mon2; mon++) {
    daysiz = DayInMonth(mon, yea);
    PrintSz(us.fEuroDate ? "Dy/Mo/Yr" : "Mo/Dy/Yr");
    for (j = 0; j <= cObj; j++) {
      if (!FIgnore(j)) {
        sprintf(sz, "  %s%-4.4s", is.fSeconds ? "  " : "", szObjName[j]);
        PrintSz(sz);
        PrintTab(' ', us.fParallel ? 2 + is.fSeconds : 1 + 3*is.fSeconds);
      }
    }
    PrintL();
    for (i = 1; i <= daysiz; i = AddDay(mon, i, yea, 1)) {

      /* Loop through each day in the month, casting a chart for that day. */

      SetCI(ciCore, mon, i, yea, Tim, Dst, Zon, Lon, Lat);
      CastChart(fTrue);
      PrintSz(SzDate(mon, i, yea, -1));
      PrintCh(' ');
      for (j = 0; j <= cObj; j++)
        if (!FIgnore(j)) {
          if (!us.fParallel) {
            if (is.fSeconds)
              PrintZodiac(planet[j]);
            else {
              AnsiColor(kObjA[j]);
              switch (us.nDegForm) {
              case 0:
                /* -sz: Format position in degrees/sign/minutes format. */
                s = SFromZ(planet[j]);
                d = (int)planet[j] - (s-1)*30;
                m = (int)(RFract(planet[j])*60.0);
                sprintf(sz, "%2d%s%02d", d, szSignAbbrev[s], m);
                break;
              case 1:
                /* -sh: Format position in hours/minutes. */
                s = (int)Mod(planet[j] + rRound/4.0);
                d = (int)planet[j] / 15;
                m = (int)((planet[j] - (real)d*15.0)*4.0);
                sprintf(sz, "%2dh%02dm", d, m);
                break;
              default:
                /* -sd: Format position as a simple degree. */
                sprintf(sz, "%6.2f", planet[j]);
                break;
              }
              PrintSz(sz);
            }
          } else {
            AnsiColor(kObjA[j]);
            PrintAltitude(planetalt[j]);
          }
          PrintCh((char)(ret[j] >= 0.0 ? ' ' : '.'));
        }
      PrintL();
      AnsiColor(kDefault);
    }
    if (mon < mon2 || yea < yea2)
      PrintL();
  }

  ciCore = ciMain;    /* Recast original chart. */
  CastChart(fTrue);
}

/* charts3.cpp */
