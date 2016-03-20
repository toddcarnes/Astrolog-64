/*
** Astrolog (Version 6.10) File: calc.cpp
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
** House Cusp Calculations.
******************************************************************************
*/

/* This is a subprocedure of ComputeInHouses(). Given a zodiac position,  */
/* return which of the twelve houses it falls in. Remember that a special */
/* check has to be done for the house that spans 0 degrees Aries.         */

int HousePlaceIn(real rDeg)
{
  int i = 0;

  rDeg = Mod(rDeg + rSmall);
  do {
    i++;
  } while (!(i >= cSign ||
    (rDeg >= chouse[i] && rDeg < chouse[Mod12(i+1)]) ||
    (chouse[i] > chouse[Mod12(i+1)] &&
    (rDeg >= chouse[i] || rDeg < chouse[Mod12(i+1)]))));
  return i;
}


/* For each object in the chart, determine what house it belongs in. */

void ComputeInHouses(void)
{
  int i;

  for (i = 0; i <= cObj; i++)
    inhouse[i] = HousePlaceIn(planet[i]);
}


/* This house system is just like the Equal system except that we start */
/* our 12 equal segments from the Midheaven instead of the Ascendant.   */

void HouseEqualMidheaven(void)
{
  int i;

  for (i = 1; i <= cSign; i++)
    chouse[i] = Mod(is.MC-270.0+30.0*(real)(i-1));
}


/* Compute the cusp positions using the Alcabitius house system. */

void HouseAlcabitius(void)
{
  real rDecl, rSda, rSna, r, rLon;
  int i;

  rDecl = RAsin(RSin(is.OB) * RSinD(is.Asc));
  r = -RTan(AA) * RTan(rDecl);
  rSda = DFromR(RAcos(r));
  rSna = rDegHalf - rSda;
  chouse[sLib] = DFromR(is.RA) - rSna;
  chouse[sSco] = DFromR(is.RA) - rSna*2.0/3.0;
  chouse[sSag] = DFromR(is.RA) - rSna/3.0;
  chouse[sCap] = DFromR(is.RA);
  chouse[sAqu] = DFromR(is.RA) + rSda/3.0;
  chouse[sPis] = DFromR(is.RA) + rSda*2.0/3.0;
  for (i = sLib; i <= sPis; i++) {
    r = RFromD(Mod(chouse[i]));
    /* The transformation below is also done in CuspMidheaven(). */
    rLon = RAtn(RTan(r)/RCos(is.OB));
    if (rLon < 0.0)
      rLon += rPi;
    if (r > rPi)
      rLon += rPi;
    chouse[i] = Mod(DFromR(rLon)+is.rSid);
  }
  for (i = sAri; i <= sVir; i++)
    chouse[i] = Mod(chouse[i+6]+rDegHalf);
}


/* This is a newer house system similar in philosophy to Porphyry houses,  */
/* and therefore (at least in the past) has also been called Neo-Porphyry. */
/* Instead of just trisecting the difference in each quadrant, we do a     */
/* smooth sinusoidal distribution of the difference around all the cusps.  */
/* Note that middle houses become 0 sized if a quadrant is <= 30 degrees.  */

void HousePullenSinusoidalDelta(void)
{
  real rQuad, rDelta;
  int iHouse;

  /* Solve equations: x+n + x + x+n = q, x+3n + x+4n + x+3n = 180-q. */
  rQuad = MinDistance(is.MC, is.Asc);
  rDelta = (rQuad - rDegQuad)/4.0;
  chouse[sLib] = Mod(is.Asc+rDegHalf); chouse[sCap] = is.MC;
  if (rQuad >= 30.0) {
    chouse[sAqu] = Mod(chouse[sCap] + 30.0 + rDelta);
    chouse[sPis] = Mod(chouse[sAqu] + 30.0 + rDelta*2.0);
  } else
    chouse[sAqu] = chouse[sPis] = Midpoint(chouse[sCap], is.Asc);
  if (rQuad <= 150.0) {
    chouse[sSag] = Mod(chouse[sCap] - 30.0 + rDelta);
    chouse[sSco] = Mod(chouse[sSag] - 30.0 + rDelta*2.0);
  } else
    chouse[sSag] = chouse[sSco] = Midpoint(chouse[sCap], chouse[sLib]);
  for (iHouse = sAri; iHouse < sLib; iHouse++)
    chouse[iHouse] = Mod(chouse[iHouse+6] + rDegHalf);
}


/* This is a new house system very similar to Sinusoidal Delta. Instead of */
/* adding a sine wave offset, multiply a sine wave ratio.                  */

void HousePullenSinusoidalRatio(void)
{
  real qSmall, rRatio, rRatio3, rRatio4, xHouse, rLo, rHi;
  int iHouse, dir;

  /* Start by determining the quadrant sizes. */
  qSmall = MinDistance(is.MC, is.Asc);
  dir = qSmall <= rDegQuad ? 1 : -1;
  if (dir < 0)
    qSmall = rDegHalf - qSmall;

#if TRUE
  /* Solve equations: rx + x + rx = q, xr^3 + xr^4 + xr^3 = 180-q. Solve */
  /* quartic for r, then compute x given 1st equation: x = q / (2r + 1). */
  if (qSmall > 0.0) {
    rLo = (2.0*pow(qSmall*qSmall - 270.0*qSmall + 16200.0, 1.0/3.0)) /
      pow(qSmall, 2.0/3.0);
    rHi = RSqr(rLo + 1.0);
    rRatio = 0.5*rHi +
      0.5*RSqr(-6.0*(qSmall-120.0)/(qSmall*rHi) - rLo + 2.0) - 0.5;
  } else
    rRatio = 0.0;
  rRatio3 = rRatio * rRatio * rRatio; rRatio4 = rRatio3 * rRatio;
  xHouse = qSmall / (2.0 * rRatio + 1.0);

#else
  /* Can also solve equations empirically. Given candidate for r, compute x */
  /* given 1st equation: x = q / (2r + 1), then compare both against 2nd:   */
  /* 2xr^3 + xr^4 = 180-q, to see whether current r is too large or small.  */
  /* Before binary searching, first keep doubling rHi until too large.      */

  real qLarge = rDegHalf - qSmall;
  flag fBinarySearch = fFalse;

  rLo = rRatio = 1.0;
  loop {
    rRatio = fBinarySearch ? (rLo + rHi) / 2.0 : rRatio * 2.0;
    rRatio3 = rRatio * rRatio * rRatio; rRatio4 = rRatio3 * rRatio;
    xHouse = qSmall / (2.0 * rRatio + 1.0);
    if ((fBinarySearch && (rRatio <= rLo || rRatio >= rHi)) || xHouse <= 0.0)
      break;
    if (2.0 * xHouse * rRatio3 + xHouse * rRatio4 >= qLarge) {
      rHi = rRatio;
      fBinarySearch = fTrue;
    } else if (fBinarySearch)
      rLo = rRatio;
  }
#endif

  /* xHouse and rRatio have been calculated. Fill in the house cusps. */
  if (dir < 0)
    neg(xHouse);
  chouse[sAri] = is.Asc; chouse[sCap] = is.MC;
  chouse[sLib] = Mod(is.Asc+rDegHalf); 
  chouse[sCap + dir]   = Mod(chouse[sCap]                + xHouse * rRatio);
  chouse[sCap + dir*2] = Mod(chouse[Mod12(sCap + dir*3)] - xHouse * rRatio);
  chouse[sCap - dir]   = Mod(chouse[sCap]                - xHouse * rRatio3);
  chouse[sCap - dir*2] = Mod(chouse[Mod12(sCap - dir*3)] + xHouse * rRatio3);
  for (iHouse = sTau; iHouse < sLib; iHouse++)
    chouse[iHouse] = Mod(chouse[iHouse+6] + rDegHalf);
}


/* The "Whole" house system is like the Equal system with 30 degree houses, */
/* where the 1st house starts at zero degrees of the sign of the Ascendant. */

void HouseWhole(void)
{
  int i;

  for (i = 1; i <= cSign; i++)
    chouse[i] = Mod((real)((SFromZ(is.Asc)-1)*30) + ZFromS(i));
}


/* The "Vedic" house system is like the Equal system except each house      */
/* starts 15 degrees earlier. The Asc falls in the middle of the 1st house. */

void HouseVedic(void)
{
  int i;

  for (i = 1; i <= cSign; i++)
    chouse[i] = Mod(is.Asc - 15.0 + ZFromS(i));
}


/* In "null" houses, the cusps are fixed to start at their corresponding */
/* sign, i.e. the 1st house is always at 0 degrees Aries, etc.           */

void HouseNull()
{
  int i;

  for (i = 1; i <= cSign; i++)
    chouse[i] = Mod(ZFromS(i));
}


/* Calculate the house cusp positions, using the specified algorithm. */

void ComputeHouses(int housesystem)
{
  char sz[cchSzDef];

  /* Don't allow polar latitudes if system not defined in polar zones. */
  if (((housesystem == hsPlacidus || housesystem == hsKoch) &&
    RAbs(AA) >= RFromD(rDegQuad-rAxis))) {
    sprintf(sz,
      "The %s system of houses is not defined at extreme latitudes.",
      szSystem[housesystem]);
    PrintWarning(sz);
    housesystem = hsPorphyry;
  }

  /* Flip the Ascendant or MC if it falls in the wrong half of the zodiac. */
  if (MinDifference(is.MC, is.Asc) < 0.0) {
    if (us.fPolarAsc)
      is.MC = Mod(is.MC + rDegHalf);
    else
      is.Asc = Mod(is.Asc + rDegHalf);
  }

  switch (housesystem) {
#ifdef MATRIX
  case hsPlacidus:      HousePlacidus();              break;
  case hsKoch:          HouseKoch();                  break;
  case hsEqual:         HouseEqual();                 break;
  case hsCampanus:      HouseCampanus();              break;
  case hsMeridian:      HouseMeridian();              break;
  case hsRegiomontanus: HouseRegiomontanus();         break;
  case hsPorphyry:      HousePorphyry();              break;
  case hsMorinus:       HouseMorinus();               break;
  case hsTopocentric:   HouseTopocentric();           break;
#endif
  case hsAlcabitius:    HouseAlcabitius();            break;
  case hsEqualMC:       HouseEqualMidheaven();        break;
  case hsSinewaveRatio: HousePullenSinusoidalRatio(); break;
  case hsSinewaveDelta: HousePullenSinusoidalDelta(); break;
  case hsWhole:         HouseWhole();                 break;
  case hsVedic:         HouseVedic();                 break;
  default:              HouseNull();                  break;
  }
}


/*
******************************************************************************
** Star Position Calculations.
******************************************************************************
*/

/* This is used by the chart calculation routine to calculate the positions */
/* of the fixed stars. Since the stars don't move in the sky over time,     */
/* getting their positions is mostly just reading info from an array and    */
/* converting it to the correct reference frame. However, we have to add    */
/* in the correct precession for the tropical zodiac, and sort the final    */
/* index list based on what order the stars are supposed to be printed in.  */

void ComputeStars(real SD)
{
  int i, j;
  real x, y, z;

  /* Read in star positions. */

  for (i = 1; i <= cStar; i++) {
    x = rStarData[i*6-6]; y = rStarData[i*6-5]; z = rStarData[i*6-4];
    planet[oNorm+i] = RFromD(x*rDegMax/24.0+y*15.0/60.0+z*0.25/60.0);
    x = rStarData[i*6-3]; y = rStarData[i*6-2]; z = rStarData[i*6-1];
    if (x < 0.0) {
      neg(y); neg(z);
    }
    planetalt[oNorm+i] = RFromD(x+y/60.0+z/60.0/60.0);
    /* Convert to ecliptic zodiac coordinates. */
    EquToEcl(&planet[oNorm+i], &planetalt[oNorm+i]);
    planet[oNorm+i] = Mod(DFromR(planet[oNorm+i])+rEpoch2000+SD);
    planetalt[oNorm+i] = DFromR(planetalt[oNorm+i]);
    ret[oNorm+i] = RFromD(rDegMax/26000.0/365.25);
    starname[i] = i;
  }

  /* Sort the index list if -Uz, -Ul, -Un, or -Ub switch in effect. */

  if (us.nStar > 1) for (i = 2; i <= cStar; i++) {
    j = i-1;

    /* Compare star names for -Un switch. */

    if (us.nStar == 'n') while (j > 0 && NCompareSz(
      szObjName[oNorm+starname[j]], szObjName[oNorm+starname[j+1]]) > 0) {
      SwapN(starname[j], starname[j+1]);
      j--;

    /* Compare star brightnesses for -Ub switch. */

    } else if (us.nStar == 'b') while (j > 0 &&
      rStarBright[starname[j]] > rStarBright[starname[j+1]]) {
      SwapN(starname[j], starname[j+1]);
      j--;

    /* Compare star zodiac locations for -Uz switch. */

    } else if (us.nStar == 'z') while (j > 0 &&
      planet[oNorm+starname[j]] > planet[oNorm+starname[j+1]]) {
      SwapN(starname[j], starname[j+1]);
      j--;

    /* Compare star declinations for -Ul switch. */

    } else if (us.nStar == 'l') while (j > 0 &&
      planetalt[oNorm+starname[j]] < planetalt[oNorm+starname[j+1]]) {
      SwapN(starname[j], starname[j+1]);
      j--;
    }
  }
}


/*
******************************************************************************
** Chart Calculation.
******************************************************************************
*/

/* Given a zodiac degree, transform it into its Decan sign, where each    */
/* sign is trisected into the three signs of its element. For example,    */
/* 1 Aries -> 3 Aries, 10 Leo -> 0 Sagittarius, 25 Sagittarius -> 15 Leo. */

real Decan(real deg)
{
  int sign;
  real unit;

  sign = SFromZ(deg);
  unit = deg - ZFromS(sign);
  sign = Mod12(sign + 4*((int)RFloor(unit/10.0)));
  unit = (unit - RFloor(unit/10.0)*10.0)*3.0;
  return ZFromS(sign)+unit;
}


/* Given a zodiac degree, transform it into its Navamsa position, where   */
/* each sign is divided into ninths, which determines the number of signs */
/* after a base element sign to use. Degrees within signs are unaffected. */

real Navamsa(real deg)
{
  int sign, sign2;
  real unit;

  sign = SFromZ(deg);
  unit = deg - ZFromS(sign);
  sign2 = Mod12(((sign-1 & 3)^(2*FOdd(sign-1)))*3+(int)(unit*0.3)+1);
  return ZFromS(sign2)+unit;
}


/* Transform spherical to rectangular coordinates in x, y, z. */

void SphToRec(real r, real azi, real alt, real *rx, real *ry, real *rz)
{
  real rT;

  *rz = r *RSinD(alt);
  rT  = r *RCosD(alt);
  *rx = rT*RCosD(azi);
  *ry = rT*RSinD(azi);
}


/* Another subprocedure of the ComputeEphem() routine. Convert the final   */
/* rectangular coordinates of a planet to zodiac position and declination. */

void ProcessPlanet(int ind, real aber)
{
  real ang, rad;

  RecToPol(spacex[ind], spacey[ind], &ang, &rad);
  planet[ind] = Mod(DFromR(ang) - aber + is.rSid);
  RecToPol(rad, spacez[ind], &ang, &rad);
  if (us.objCenter == oSun && ind == oSun)
    ang = 0.0;
  ang = DFromR(ang);
  while (ang > rDegQuad)    /* Ensure declination is from -90..+90 degrees. */
    ang -= rDegHalf;
  while (ang < -rDegQuad)
    ang += rDegHalf;
  planetalt[ind] = ang;
}


#ifdef EPHEM
/* Compute the positions of the planets at a certain time using the Swiss  */
/* Ephemeris accurate formulas. This will supersede the Matrix routine     */
/* values and is only called when the -b switch is in effect. Not all      */
/* objects or modes are available using this, but some additional values   */
/* such as Moon and Node velocities not available without -b are. (This is */
/* the one place in Astrolog which calls the Swiss Ephemeris functions.)   */

void ComputeEphem(real t)
{
  int i;
  real r1, r2, r3, r4;

  /* We can compute the positions of Sun through Pluto, Chiron, the four  */
  /* asteroids, Lilith, and the (true or mean) North Node using ephemeris */
  /* files. Earth and the other objects must be computed elsewhere.       */

  for (i = oSun; i <= uranHi; i++) {
    if ((ignore[i] &&
      i != us.objCenter && i > oMoo && (i != oNod || ignore[oSou])) ||
      FBetween(i, oFor, cuspHi) ||
      (us.fPlacalcPla && i >= oFor) ||
      (us.fPlacalcAst && FBetween(i, oCer, oVes)))
      continue;
    if (
#ifdef EPHEM2
      !us.fPlacalcPla ?
#endif
#ifdef SWISS
      FSwissPlanet(i, JulianDayFromTime(t), us.objCenter != oEar,
        &r1, &r2, &r3, &r4)
#endif
#ifdef EPHEM2
      :
#endif
#ifdef PLACALC
      FPlacalcPlanet(i, JulianDayFromTime(t), us.objCenter != oEar,
        &r1, &r2, &r3, &r4)
#endif
      ) {

      /* Note that this can't compute charts with central planets other */
      /* than the Sun or Earth or relative velocities in current state. */

      planet[i]    = Mod(r1 + is.rSid);
      planetalt[i] = r2;
      ret[i]       = RFromD(r3);
    } else
      r4 = 0.0;

    /* Compute x,y,z coordinates from azimuth, altitude, and distance. */

    SphToRec(r4, planet[i], planetalt[i],
      &spacex[i], &spacey[i], &spacez[i]);
  }

  /* If heliocentric, move Earth position to object slot zero. */

  if (!ignore[oSou]) {
    spacex[oSou] = -spacex[oNod];
    spacey[oSou] = -spacey[oNod];
    spacez[oSou] = -spacez[oNod];
    ret[oSou] = ret[oNod];
  }
  if (us.objCenter == oEar)
    return;
  planet[oEar] = planet[oSun];
  planetalt[oEar] = planetalt[oSun];
  ret[oEar] = ret[oSun];
  spacex[oEar] = spacex[oSun];
  spacey[oEar] = spacey[oSun];
  spacez[oEar] = spacez[oSun];
  planet[oSun] = planetalt[oSun] = ret[oSun] =
    spacex[oSun] = spacey[oSun] = spacez[oSun] = 0.0;
  for (i = oNod; i <= oLil; i++) if (!ignore[i]) {
    spacex[i] += spacex[oEar];
    spacey[i] += spacey[oEar];
    spacez[i] += spacez[oEar];
  }
  if (us.objCenter == oSun)
    return;

  /* If other planet centered, shift all positions by central planet. */

  for (i = 0; i <= oNorm; i++) if (!ignore[i] && i != us.objCenter) {
    spacex[i] -= spacex[us.objCenter];
    spacey[i] -= spacey[us.objCenter];
    spacez[i] -= spacez[us.objCenter];
    ProcessPlanet(i, 0.0);
  }
  planet[us.objCenter] = planetalt[us.objCenter] = ret[us.objCenter] =
    spacex[us.objCenter] = spacey[us.objCenter] = spacez[us.objCenter] = 0.0;
}
#endif


/* This is probably the main routine in all of Astrolog. It generates a   */
/* chart, calculating the positions of all the celestial bodies and house */
/* cusps, based on the current chart information, and saves them for use  */
/* by any of the display routines.                                        */

real CastChart(flag fDate)
{
  CI ci;
  real housetemp[cSign+1], Off, vtx, ep, j;
  int i, k;

  /* Hack: Time zone +/-24 means to have the time of day be in Local Mean */
  /* Time (LMT). This is done by making the time zone value reflect the   */
  /* logical offset from GMT as indicated by the chart's longitude value. */

  if (RAbs(ZZ) == 24.0)
    ZZ = DegToDec(DecToDeg(OO)/15.0);
  ci = ciCore;

  if (MM == -1) {

    /* Hack: If month is negative, then know chart was read in through a */
    /* -o0 position file, so planet positions are already in the arrays. */

    is.MC = planet[oMC]; is.Asc = planet[oAsc];
  } else {
    ClearB((lpbyte)&cp0, sizeof(CP));     /* On ecliptic unless we say so.  */
    ClearB((lpbyte)spacex, (oNorm+1)*sizeof(real));
    ClearB((lpbyte)spacey, (oNorm+1)*sizeof(real));
    ClearB((lpbyte)spacez, (oNorm+1)*sizeof(real));
    for (i = 0; i <= cObj; i++)
      ret[i] = 1.0;                       /* Direct until we say otherwise. */
    Off = ProcessInput(fDate);
#ifdef SWISS
    if (us.fEphemFiles && !us.fPlacalcPla) {
      FSwissHouse(is.T, OO, DFromR(AA), us.nHouseSystem,
        &is.Asc, &is.MC, &is.RA, &vtx, &ep, &is.OB, &Off);
    } else
#endif
    {
#ifdef MATRIX
      ComputeVariables(&vtx);
      if (us.fGeodetic)               /* Check for -G geodetic chart. */
        is.RA = RFromD(Mod(-OO));
      is.MC  = CuspMidheaven();       /* Calculate Ascendant & Midheaven. */
      is.Asc = CuspAscendant();
      ep = CuspEastPoint();
      ComputeHouses(us.nHouseSystem);    /* Go calculate house cusps. */
#endif
    }

#ifdef MATRIX
    /* Go calculate planet, Moon, and North Node positions. */

    if (us.fMatrixPla &&
      (!us.fEphemFiles || (us.fPlacalcPla && us.fUranian))) {
      ComputePlanets();
      if (!ignore[oMoo] || !ignore[oNod] || !ignore[oSou] || !ignore[oFor]) {
        ComputeLunar(&planet[oMoo], &planetalt[oMoo],
          &planet[oNod], &planetalt[oNod]);
        ret[oNod] = -1.0;
      }
    }
#endif

#ifdef EPHEM
    /* Compute more accurate ephemeris positions for certain objects. */

    if (us.fEphemFiles)
      ComputeEphem(is.T);
#endif

    /* Certain objects are positioned directly opposite to other objects. */

    i = us.objCenter == oEar ? oSun : oEar;
    planet[us.objCenter] = Mod(planet[i]+rDegHalf);
    ret[us.objCenter] = ret[i];
    planet[oSou] = Mod(planet[oNod]+rDegHalf);
    if (!us.fEphemFiles) {
      ret[oSou] = ret[oNod] = RFromD(-0.053);
      ret[oMoo] = RFromD(12.2);
    }

    /* Calculate position of Part of Fortune. */

    j = planet[oMoo]-planet[oSun];
    if (us.nArabicNight < 0 ||
      (us.nArabicNight == 0 && HousePlaceIn(planet[oSun]) < sLib))
      neg(j);
    j = RAbs(j) < rDegQuad ? j : j - RSgn(j)*rDegMax;
    planet[oFor] = Mod(j+is.Asc);

    /* Fill in "planet" positions corresponding to house cusps. */

    planet[oVtx] = vtx; planet[oEP] = ep;
    for (i = 1; i <= cSign; i++)
      planet[cuspLo + i - 1] = chouse[i];
    if (!us.fHouseAngle) {
      planet[oAsc] = is.Asc; planet[oMC] = is.MC;
      planet[oDes] = Mod(is.Asc + rDegHalf);
      planet[oNad] = Mod(is.MC + rDegHalf);
    }
    for (i = oFor; i <= cuspHi; i++)
      ret[i] = RFromD(rDegMax);
  }

  /* Go calculate star positions if -U switch in effect. */

  if (us.nStar)
    ComputeStars((us.fSidereal ? 0.0 : -Off) + us.rZodiacOffset);

  /* Transform ecliptic to equatorial coordinates if -sr in effect. */

  if (us.fEquator)
    for (i = 0; i <= cObj; i++) if (!ignore[i]) {
      planet[i]    = RFromD(Tropical(planet[i]));
      planetalt[i] = RFromD(planetalt[i]);
      EclToEqu(&planet[i], &planetalt[i]);
      planet[i]    = DFromR(planet[i]);
      planetalt[i] = DFromR(planetalt[i]);
    }

  /* Now, we may have to modify the base positions we calculated above */
  /* based on what type of chart we are generating.                    */

  if (us.fProgress && us.fSolarArc) {  /* Are we doing -p0 solar arc chart? */
    j = (is.JDp - JulianDayFromTime(is.T) - 0.5) / us.rProgDay;
    for (i = 0; i <= cObj; i++)
      planet[i] = Mod(planet[i] + j);
    for (i = 1; i <= cSign; i++)
      chouse[i] = Mod(chouse[i] + j);
  }
  if (us.rHarmonic != 1.0)         /* Are we doing a -x harmonic chart?     */
    for (i = 0; i <= cObj; i++)
      planet[i] = Mod(planet[i] * us.rHarmonic);
  if (us.objOnAsc) {
    if (us.objOnAsc > 0)           /* Is -1 put on Ascendant in effect?     */
      j = planet[us.objOnAsc-1]-is.Asc;
    else                           /* Or -2 put object on Midheaven switch? */
      j = planet[-us.objOnAsc-1]-is.MC;
    for (i = 1; i <= cSign; i++)   /* If so, rotate the houses accordingly. */
      chouse[i] = Mod(chouse[i]+j);
  }

  /* Check to see if we are -F forcing any objects to be particular values. */

  for (i = 0; i <= cObj; i++)
    if (force[i] != 0.0) {
      planet[i] = force[i]-rDegMax;
      planetalt[i] = ret[i] = 0.0;
    }

  ComputeInHouses();        /* Figure out what house everything falls in. */

  /* If -f domal chart switch in effect, switch planet and house positions. */

  if (us.fFlip) {
    for (i = 0; i <= cObj; i++) {
      k = inhouse[i];
      inhouse[i] = SFromZ(planet[i]);
      planet[i] = ZFromS(k)+MinDistance(chouse[k], planet[i]) /
        MinDistance(chouse[k], chouse[Mod12(k+1)])*30.0;
    }
    for (i = 1; i <= cSign; i++) {
      k = HousePlaceIn(ZFromS(i));
      housetemp[i] = ZFromS(k)+MinDistance(chouse[k], ZFromS(i)) /
        MinDistance(chouse[k], chouse[Mod12(k+1)])*30.0;
    }
    for (i = 1; i <= cSign; i++)
      chouse[i] = housetemp[i];
  }

  /* If -3 decan chart switch in effect, edit planet positions accordingly. */

  if (us.fDecan) {
    for (i = 0; i <= cObj; i++)
      planet[i] = Decan(planet[i]);
    ComputeInHouses();
  }

  /* If -9 navamsa chart switch in effect, edit positions accordingly. */

  if (us.fNavamsa) {
    for (i = 0; i <= cObj; i++)
      planet[i] = Navamsa(planet[i]);
    ComputeInHouses();
  }

  ciCore = ci;
  return is.T;
}


/* Calculate the position of each planet with respect to the Gauquelin      */
/* sectors. This is used by the sector charts. Fill out the planet position */
/* array where one degree means 1/10 the way across one of the 36 sectors.  */

void CastSectors()
{
  int source[MAXINDAY], type[MAXINDAY], occurcount, division, div,
    i, j, s1, s2, ihouse, fT;
  real time[MAXINDAY], rgalt1[objMax], rgalt2[objMax],
    azi1, azi2, alt1, alt2, lon, lat, mc1, mc2, d, k;

  /* If the -l0 approximate sectors flag is set, we can quickly get rough   */
  /* positions by having each position be the location of the planet as     */
  /* mapped into Placidus houses. The -f flip houses flag does this for us. */

  if (us.fSectorApprox) {
    ihouse = us.nHouseSystem; us.nHouseSystem = 0;
    inv(us.fFlip);
    CastChart(fTrue);
    inv(us.fFlip);
    us.nHouseSystem = ihouse;
    return;
  }

  /* If not approximating sectors, then they need to be computed the formal */
  /* way: based on a planet's nearest rising and setting times. The code    */
  /* below is similar to ChartInDayHorizon() accessed by the -Zd switch.    */

  fT = us.fSidereal; us.fSidereal = fFalse;
  lon = RFromD(Mod(Lon)); lat = RFromD(Lat);
  division = us.nDivision * 4;
  occurcount = 0;

  /* Start scanning from 18 hours before to 18 hours after the time of the */
  /* chart in question, to find the closest rising and setting times.      */

  ciCore = ciMain; ciCore.tim -= 18.0;
  if (ciCore.tim < 0.0) {
    ciCore.tim += 24.0;
    ciCore.day--;
  }
  CastChart(fTrue);
  mc2 = RFromD(planet[oMC]); k = RFromD(planetalt[oMC]);
  EclToEqu(&mc2, &k);
  cp2 = cp0;
  for (i = 0; i <= cObj; i++)
    rgalt2[i] = planetalt[i];

  /* Loop through 36 hours, dividing it into a certain number of segments. */
  /* For each segment we get the planet positions at its endpoints.        */

  for (div = 1; div <= division; div++) {
    ciCore = ciMain;
    ciCore.tim = DecToDeg(ciCore.tim) - 18.0 + 36.0*(real)div/(real)division;
    if (ciCore.tim < 0.0) {
      ciCore.tim += 24.0;
      ciCore.day--;
    } else if (ciCore.tim >= 24.0) {
      ciCore.tim -= 24.0;
      ciCore.day++;
    }
    ciCore.tim = DegToDec(ciCore.tim);
    CastChart(fTrue);
    mc1 = mc2;
    mc2 = RFromD(planet[oMC]); k = RFromD(planetalt[oMC]);
    EclToEqu(&mc2, &k);
    cp1 = cp2; cp2 = cp0;
    for (i = 1; i <= cObj; i++) {
      rgalt1[i] = rgalt2[i]; rgalt2[i] = planetalt[i];
    }

    /* During our segment, check to see if each planet rises or sets. */

    for (i = 0; i <= cObj; i++) if (!FIgnore(i) && FThing(i)) {
      EclToHorizon(&azi1, &alt1, cp1.obj[i], rgalt1[i], lon, lat, mc1);
      EclToHorizon(&azi2, &alt2, cp2.obj[i], rgalt2[i], lon, lat, mc2);
      j = 0;
      if ((alt1 > 0.0) != (alt2 > 0.0)) {
        d = RAbs(alt1)/(RAbs(alt1)+RAbs(alt2));
        k = Mod(azi1 + d*MinDifference(azi1, azi2));
        j = 1 + (MinDistance(k, rDegHalf) < rDegQuad);
      }
      if (j && occurcount < MAXINDAY) {
        source[occurcount] = i;
        type[occurcount] = j;
        time[occurcount] = 36.0*((real)(div-1)+d)/(real)division*60.0;
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
      j--;
    }
  }

  /* Now fill out the planet array with the appropriate sector location. */

  for (i = 0; i <= cObj; i++) if (!ignore[i] && FThing(i)) {
    planet[i] = 0.0;
    /* Search for the first rising or setting event of our planet. */
    for (s2 = 0; s2 < occurcount && source[s2] != i; s2++)
      ;
    if (s2 == occurcount) {
LFail:
      /* If we failed to find a rising/setting bracket around our time, */
      /* automatically restrict that planet so it doesn't show up.      */
      ignore[i] = fTrue;
      continue;
    }
LRetry:
    /* One rising or setting event was found. Now search for the next one. */
    s1 = s2;
    for (s2 = s1 + 1; s2 < occurcount && source[s2] != i; s2++)
      ;
    if (s2 == occurcount)
      goto LFail;
    /* Reject the two events if either (1) they're both the same, i.e. both */
    /* rising or both setting, or (2) they don't bracket the chart's time.  */
    if (type[s2] == type[s1] || time[s1] > 18.0*60.0 || time[s2] < 18.0*60.0)
      goto LRetry;
    /* Cool, we've found our rising/setting bracket. The sector position is */
    /* the proportion the chart time is between the two event times.        */
    planet[i] = (18.0*60.0 - time[s1])/(time[s2] - time[s1])*rDegHalf;
    if (type[s1] == 2)
      planet[i] += rDegHalf;
    planet[i] = Mod(rDegMax - planet[i]);
  }

  /* Restore original chart info as we've overwritten it. */

  ciCore = ciMain;
  us.fSidereal = fT;
}


/*
******************************************************************************
** Aspect Calculations.
******************************************************************************
*/

/* Set up the aspect/midpoint grid. Allocate memory for this array, if not */
/* already done. Allocation is only done once, first time this is called.  */

flag FEnsureGrid(void)
{
  if (grid != NULL)
    return fTrue;
  grid = (GridInfo *)PAllocate(sizeof(GridInfo), "grid");
  return grid != NULL;
}


/* Indicate whether some aspect between two objects should be shown. */

flag FAcceptAspect(int obj1, int asp, int obj2)
{
  int fSupp;

  if (FIgnoreA(asp))    /* If the aspect restricted, reject immediately. */
    return fFalse;
  if (!us.fSmartCusp)
    return fTrue;

  /* Allow only conjunctions to the minor house cusps. */
  if ((FMinor(obj1) || FMinor(obj2)) && asp > aCon)
    return fFalse;

  /* Is this a weak aspect supplemental to a stronger one present? */
  fSupp = (asp == aOpp && !FIgnoreA(aCon)) ||
    (asp == aSex && !FIgnoreA(aTri)) || (asp == aSSx && !FIgnoreA(aInc)) ||
    (asp == aSes && !FIgnoreA(aSSq));

  /* Prevent any simultaneous aspects to opposing angle cusps,     */
  /* e.g. if conjunct one, don't be opposite the other; if trine   */
  /* one, don't sextile the other; don't square both at once, etc. */
  if (!ignore[oMC] && !ignore[oNad] &&
    (((obj1 == oMC || obj2 == oMC) && fSupp) ||
    ((obj1 == oNad || obj2 == oNad) && (fSupp || asp == aSqu))))
    return fFalse;
  if (!ignore[oAsc] && !ignore[oDes] &&
    (((obj1 == oAsc || obj2 == oAsc) && fSupp) ||
    ((obj1 == oDes || obj2 == oDes) && (fSupp || asp == aSqu))))
    return fFalse;

  /* Prevent any simultaneous aspects to the North and South Node. */
  if (!ignore[oNod] && !ignore[oSou] &&
    (((obj1 == oNod || obj2 == oNod) && fSupp) ||
    ((obj1 == oSou || obj2 == oSou) && (fSupp || asp == aSqu))))
    return fFalse;

  /* Prevent any simultaneous aspects to the Sun and Earth. */
  if (!ignore[oEar] && !ignore[oSun] &&
    (((obj1 == oSun || obj2 == oSun) && fSupp) ||
    ((obj1 == oEar || obj2 == oEar) && (fSupp || asp == aSqu))))
    return fFalse;

  return fTrue;
}


/* This is a subprocedure of FCreateGrid() and FCreateGridRelation().   */
/* Given two planets, determine what aspect, if any, is present between */
/* them, and save the aspect name and orb in the specified grid cell.   */

void GetAspect(real *planet1, real *planet2, real *ret1, real *ret2,
  int i, int j)
{
  int k;
  real l, m;

  grid->v[i][j] = grid->n[i][j] = 0;
  l = MinDistance(planet2[i], planet1[j]);
  for (k = us.nAsp; k >= 1; k--) {
    if (!FAcceptAspect(i, k, j))
      continue;
    m = l-rAspAngle[k];
    if (RAbs(m) < GetOrb(i, j, k)) {
      grid->n[i][j] = k;

      /* If -ga switch in effect, then change the sign of the orb to    */
      /* correspond to whether the aspect is applying or separating.    */
      /* To do this, we check the velocity vectors to see if the        */
      /* planets are moving toward, away, or are overtaking each other. */

      if (us.fAppSep)
        m = RSgn2(ret1[j]-ret2[i])*
          RSgn2(MinDifference(planet2[i], planet1[j]))*RSgn2(m)*RAbs(m);
      grid->v[i][j] = (int)(m*3600.0);
    }
  }
}


/* Very similar to GetAspect(), this determines if there is a parallel or */
/* contraparallel aspect between the given two planets, and stores the    */
/* result as above. The settings and orbs for conjunction are used for    */
/* parallel and those for opposition are used for contraparallel.         */

void GetParallel(real *planet1, real *planet2,
  real *planetalt1, real *planetalt2, int i, int j)
{
  int k;
  real l, alt1, alt2;

  l = RFromD(planet1[j]); alt1 = RFromD(planetalt1[j]);
  EclToEqu(&l, &alt1); alt1 = DFromR(alt1);
  l = RFromD(planet2[i]); alt2 = RFromD(planetalt2[i]);
  EclToEqu(&l, &alt2); alt2 = DFromR(alt2);
  grid->v[i][j] = grid->n[i][j] = 0;
  for (k = Min(us.nAsp, aOpp); k >= 1; k--) {
    if (!FAcceptAspect(i, k, j))
      continue;
    l = RAbs(k == aCon ? alt1 - alt2 : RAbs(alt1) - RAbs(alt2));
    if (l < GetOrb(i, j, k)) {
      grid->n[i][j] = k;
      grid->v[i][j] = (int)(l*3600.0);
    }
  }
}


/* Fill in the aspect grid based on the aspects taking place among the */
/* planets in the present chart. Also fill in the midpoint grid.       */

flag FCreateGrid(flag fFlip)
{
  int i, j, k;
  real l;

  if (!FEnsureGrid())
    return fFalse;
  for (j = 0; j <= cObj; j++) if (!FIgnore(j))
    for (i = 0; i <= cObj; i++) if (!FIgnore(i))

      /* The parameter 'flip' determines what half of the grid is filled in */
      /* with the aspects and what half is filled in with the midpoints.    */

      if (fFlip ? i > j : i < j) {
        if (us.fParallel)
          GetParallel(planet, planet, planetalt, planetalt, i, j);
        else
          GetAspect(planet, planet, ret, ret, i, j);
      } else if (fFlip ? i < j : i > j) {
        l = Mod(Midpoint(planet[i], planet[j])); k = (int)l;  /* Calculate */
        grid->n[i][j] = k/30+1;                               /* midpoint. */
        grid->v[i][j] = (int)(l*3600.0) % (30*3600);
      } else {
        grid->n[i][j] = SFromZ(planet[j]);
        grid->v[i][j] = (int)(planet[j]*60.0) % (30*60);
      }
  return fTrue;
}


/* This is similar to the previous function; however, this time fill in the */
/* grid based on the aspects (or midpoints if 'fMidpoint' set) taking place */
/* among the planets in two different charts, as in the -g -r0 combination. */

flag FCreateGridRelation(flag fMidpoint)
{
  int i, j, k;
  real l;

  if (!FEnsureGrid())
    return fFalse;
  for (j = 0; j <= cObj; j++) if (!FIgnore(j) || !FIgnore2(j))
    for (i = 0; i <= cObj; i++) if (!FIgnore(i) || !FIgnore2(i))
      if (!fMidpoint) {
        if (us.fParallel)
          GetParallel(cp1.obj, cp2.obj, cp1.alt, cp2.alt, i, j);
        else
          GetAspect(cp1.obj, cp2.obj, cp1.dir, cp2.dir, i, j);
      } else {
        l = Mod(Midpoint(cp2.obj[i], cp1.obj[j])); k = (int)l; /* Calculate */
        grid->n[i][j] = k/30+1;                                /* midpoint. */
        grid->v[i][j] = (int)(l*3600.0) % (30*3600);
      }
  return fTrue;
}


/*
******************************************************************************
** Other Calculations.
******************************************************************************
*/

/* Fill out tables based on the number of unrestricted planets in signs by  */
/* element, signs by mode, as well as other values such as the number of    */
/* objects in yang vs. yin signs, in various house hemispheres (north/south */
/* and east/west), and the number in first six signs vs. second six signs.  */
/* This is used by the -v chart listing and the sidebar in graphics charts. */

void CreateElemTable(ET *pet)
{
  int i, s;

  ClearB((lpbyte)pet, (int)sizeof(ET));
  for (i = 0; i <= cObj; i++) if (!FIgnore(i)) {
    pet->coSum++;
    s = SFromZ(planet[i]);
    pet->coSign[s-1]++;
    pet->coElemMode[(s-1)&3][(s-1)%3]++;
    pet->coElem[(s-1)&3]++; pet->coMode[(s-1)%3]++;
    pet->coYang += FOdd(s);
    pet->coLearn += (s < sLib);
    if (!FCusp(i)) {
      pet->coHemi++;
      s = inhouse[i];
      pet->coHouse[s-1]++;
      pet->coModeH[(s-1)%3]++;
      pet->coMC += (s >= sLib);
      pet->coAsc += (s < sCan || s >= sCap);
    }
  }
  pet->coYin   = pet->coSum  - pet->coYang;
  pet->coShare = pet->coSum  - pet->coLearn;
  pet->coDes   = pet->coHemi - pet->coAsc;
  pet->coIC    = pet->coHemi - pet->coMC;
}

/* calc.cpp */
