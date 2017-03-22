/*
** Astrolog (Version 6.20) File: matrix.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2017 by
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
** 1989,1991,1993 by Astrodienst AG and Alois Treindl (alois@astro.ch). The
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
** Last code change made 3/19/2017.
*/

#include "astrolog.h"


/*
******************************************************************************
** Assorted Calculations.
******************************************************************************
*/

/* Given a month, day, and year, convert it into a single Julian day value, */
/* i.e. the number of days passed since a fixed reference date.             */

long MdyToJulian(int mon, int day, int yea)
{
#ifdef MATRIX
  if (!us.fEphemFiles) {
    long im, j;

    im = 12*((long)yea+4800)+(long)mon-3;
    j = (2*(im%12) + 7 + 365*im)/12;
    j += (long)day + im/48 - 32083;
    if (j > 2299171)                   /* Take care of dates in */
      j += im/4800 - im/1200 + 38;     /* Gregorian calendar.   */
    return j;
  }
#endif
#ifdef EPHEM
  int fGreg = fTrue;
  double jd;

  if (yea < yeaJ2G || (yea == yeaJ2G &&
    (mon < monJ2G || (mon == monJ2G && day < 15))))
    fGreg = fFalse;
  jd =
#ifdef EPHEM2
    !us.fPlacalcPla ?
#endif
#ifdef SWISS
    SwissJulDay(mon, day, yea, 12.0, fGreg) + rRound
#endif
#ifdef EPHEM2
    :
#endif
#ifdef PLACALC
    julday(mon, day, yea, 12.0, fGreg) + rRound
#endif
    ;
  return (long)RFloor(jd);
#else
  return 0;         /* Shouldn't ever be reached. */
#endif /* EPHEM */
}


/* Like above but return a fractional Julian time given the extra info. */

real MdytszToJulian(int mon, int day, int yea, real tim, real dst, real zon)
{
  return (real)MdyToJulian(mon, day, yea) + (tim + zon - dst) / 24.0;
}


/* Take a Julian day value, and convert it back into the corresponding */
/* month, day, and year.                                               */

void JulianToMdy(real JD, int *mon, int *day, int *yea)
{
#ifdef MATRIX
  if (!us.fEphemFiles) {
    long L, N, IT, JT, K, IK;

    L  = (long)RFloor(JD+rRound)+68569L;
    N  = Dvd(4L*L, 146097L);
    L  -= Dvd(146097L*N + 3L, 4L);
    IT = Dvd(4000L*(L+1L), 1461001L);
    L  -= Dvd(1461L*IT, 4L) - 31L;
    JT = Dvd(80L*L, 2447L);
    K  = L-Dvd(2447L*JT, 80L);
    L  = Dvd(JT, 11L);
    JT += 2L - 12L*L;
    IK = 100L*(N-49L) + IT + L;
    *mon = (int)JT; *day = (int)K; *yea = (int)IK;
  }
#endif
#ifdef EPHEM
  double tim;

#ifdef EPHEM2
  if (!us.fPlacalcPla)
#endif
#ifdef SWISS
    SwissRevJul(JD, JD >= 2299171.0 /* Oct 15, 1582 */, mon, day, yea, &tim);
#endif
#ifdef EPHEM2
  else
#endif
#ifdef PLACALC
    revjul(JD, JD >= 2299171.0 /* Oct 15, 1582 */, mon, day, yea, &tim);
#endif
#endif /* EPHEM */
}


/* This is a subprocedure of CastChart(). Once we have the chart parameters */
/* calculate a few important things related to the date, i.e. the Greenwich */
/* time, the Julian day and fractional part of the day, the offset to the   */
/* sidereal, and a couple of other things.                                  */

real ProcessInput(flag fDate)
{
  TT = RSgn(TT)*RFloor(RAbs(TT))+RFract(RAbs(TT)) + (ZZ - SS);
  AA = Min(AA, rDegQuad-rSmall);    /* Make sure the chart isn't being cast */
  AA = Max(AA, -(rDegQuad-rSmall)); /* on the precise North or South Pole.  */
  AA = RFromD(AA);

  /* if parameter 'fDate' isn't set, then we can assume that the true time */
  /* has already been determined (as in a -rm switch time midpoint chart). */

  if (fDate) {
    is.JD = (real)MdyToJulian(MM, DD, YY);
    is.T = (is.JD + TT/24.0);
    if (us.fProgress && !us.fSolarArc) {
      /* Determine actual time that a progressed chart is to be cast for. */
      is.T += ((is.JDp - is.T) / us.rProgDay);
    }
    is.T = (is.T - 2415020.5) / 36525.0;
  }

#ifdef MATRIX
  if (!(us.fEphemFiles && !us.fPlacalcPla)) {
    real Ln, Off;

    /* Compute angle that the ecliptic is inclined to the Celestial Equator */
    is.OB = RFromD(23.452294-0.0130125*is.T);

    Ln = Mod((933060-6962911*is.T+7.5*is.T*is.T)/3600.0); /* Mean lunar node */
    Off = (259205536.0*is.T+2013816.0)/3600.0;            /* Mean Sun        */
    Off = 17.23*RSin(RFromD(Ln)) + 1.27*RSin(RFromD(Off)) -
      (5025.64+1.11*is.T)*is.T;
    Off = (Off-84038.27)/3600.0;
    is.rSid = (us.fSidereal ? Off : 0.0) + us.rZodiacOffset;
    return Off;
  }
#endif
  return 0.0;
}


/* Convert polar to rectangular coordinates. */

void PolToRec(real a, real r, real *x, real *y)
{
  if (a == 0.0)
    a = rSmall;
  *x = r*RCos(a);
  *y = r*RSin(a);
}


/* Convert rectangular to polar coordinates. */

void RecToPol(real x, real y, real *a, real *r)
{
  if (y == 0.0)
    y = rSmall;
  *r = RSqr(x*x + y*y);
  *a = Angle(x, y);
}


#ifdef MATRIX
/* Convert rectangular to spherical coordinates. */

real RecToSph(real B, real L, real O)
{
  real R, Q, G, X, Y, A;

  A = B; R = 1.0;
  PolToRec(A, R, &X, &Y);
  Q = Y; R = X; A = L;
  PolToRec(A, R, &X, &Y);
  G = X; X = Y; Y = Q;
  RecToPol(X, Y, &A, &R);
  A += O;
  PolToRec(A, R, &X, &Y);
  Q = RAsin(Y);
  Y = X; X = G;
  RecToPol(X, Y, &A, &R);
  if (A < 0.0)
    A += 2*rPi;
  G = A;
  return G;  /* We only ever care about and return one of the coordinates. */
}
#endif


/* Do a coordinate transformation: Given a longitude and latitude value,    */
/* return the new longitude and latitude values that the same location      */
/* would have, were the equator tilted by a specified number of degrees.    */
/* In other words, do a pole shift! This is used to convert among ecliptic, */
/* equatorial, and local coordinates, each of which have zero declination   */
/* in different planes. In other words, take into account the Earth's axis. */

void CoorXform(real *azi, real *alt, real tilt)
{
  real x, y, a1, l1;
  real sinalt, cosalt, sinazi, sintilt, costilt;

  sinalt = RSin(*alt); cosalt = RCos(*alt); sinazi = RSin(*azi);
  sintilt = RSin(tilt); costilt = RCos(tilt);

  x = cosalt * sinazi * costilt;
  y = sinalt * sintilt;
  x -= y;
  a1 = cosalt;
  y = cosalt * RCos(*azi);
  l1 = Angle(y, x);
  a1 = a1 * sinazi * sintilt + sinalt * costilt;
  a1 = RAsin(a1);
  *azi = l1; *alt = a1;
}


#ifdef MATRIX
/* This is another subprocedure of CastChart(). Calculate a few variables */
/* corresponding to the chart parameters that are used later on. The      */
/* astrological vertex (object number 20) is also calculated here.        */

void ComputeVariables(real *vtx)
{
  real B, L, G, tim = TT;

  if (us.fProgress && !us.fSolarArc) {
    tim = JulianDayFromTime(is.T) + 0.5;
    tim = RFract(tim)*24.0;
  }
  is.RA = RFromD(Mod((6.6460656 + 2400.0513*is.T + 2.58E-5*is.T*is.T +
    tim)*15.0 - OO));
  L = is.RA + rPi; B = rPiHalf - RAbs(AA);
  if (AA < 0.0)
    B = -B;
  G = RecToSph(B, L, -is.OB);
  *vtx = Mod(is.rSid + DFromR(G+rPiHalf));    /* Vertex */
}


/*
******************************************************************************
** House Cusp Calculations.
******************************************************************************
*/

/* The following three functions calculate the Midheaven, Ascendant, and  */
/* East Point of the chart in question, based on time and location. The   */
/* first two are also used in some of the house cusp calculation routines */
/* as a quick way to get the 10th and 1st cusps. The East Point object is */
/* technically defined as the Ascendant's position at zero latitude.      */

real CuspMidheaven(void)
{
  real MC;

  MC = RAtn(RTan(is.RA)/RCos(is.OB));
  if (MC < 0.0)
    MC += rPi;
  if (is.RA > rPi)
    MC += rPi;
  return Mod(DFromR(MC)+is.rSid);
}

real CuspAscendant(void)
{
  real Asc;

  Asc = Angle(-RSin(is.RA)*RCos(is.OB)-RTan(AA)*RSin(is.OB), RCos(is.RA));
  return Mod(DFromR(Asc)+is.rSid);
}

real CuspEastPoint(void)
{
  real EP;

  EP = Angle(-RSin(is.RA)*RCos(is.OB), RCos(is.RA));
  return Mod(DFromR(EP)+is.rSid);
}


/* These are various different algorithms for calculating the house cusps: */

real CuspPlacidus(real deg, real FF, flag fNeg)
{
  real LO, R1, XS, X;
  int i;

  R1 = is.RA+RFromD(deg);
  X = fNeg ? 1.0 : -1.0;
  /* Looping 10 times is arbitrary, but it's what other programs do. */
  for (i = 1; i <= 10; i++) {

    /* This formula works except at 0 latitude (AA == 0.0). */

    XS = X*RSin(R1)*RTan(is.OB)*RTan(AA == 0.0 ? 0.0001 : AA);
    XS = RAcos(XS);
    if (XS < 0.0)
      XS += rPi;
    R1 = is.RA + (fNeg ? rPi-(XS/FF) : (XS/FF));
  }
  LO = RAtn(RTan(R1)/RCos(is.OB));
  if (LO < 0.0)
    LO += rPi;
  if (RSin(R1) < 0.0)
    LO += rPi;
  return DFromR(LO);
}

void HousePlacidus(void)
{
  int i;

  chouse[1] = Mod(is.Asc-is.rSid);
  chouse[4] = Mod(is.MC+rDegHalf-is.rSid);
  chouse[5] = CuspPlacidus(30.0, 3.0, fFalse) + rDegHalf;
  chouse[6] = CuspPlacidus(60.0, 1.5, fFalse) + rDegHalf;
  chouse[2] = CuspPlacidus(120.0, 1.5, fTrue);
  chouse[3] = CuspPlacidus(150.0, 3.0, fTrue);
  for (i = 1; i <= cSign; i++) {
    if (i <= 6)
      chouse[i] = Mod(chouse[i]+is.rSid);
    else
      chouse[i] = Mod(chouse[i-6]+rDegHalf);
  }
}

void HouseKoch(void)
{
  real A1, A2, A3, KN, D, X;
  int i;

  A1 = RSin(is.RA)*RTan(AA)*RTan(is.OB);
  A1 = RAsin(A1);
  for (i = 1; i <= cSign; i++) {
    D = Mod(60.0+30.0*(real)i);
    A2 = D/rDegQuad-1.0; KN = 1.0;
    if (D >= rDegHalf) {
      KN = -1.0;
      A2 = D/rDegQuad-3.0;
    }
    A3 = RFromD(Mod(DFromR(is.RA)+D+A2*DFromR(A1)));
    X = Angle(RCos(A3)*RCos(is.OB)-KN*RTan(AA)*RSin(is.OB), RSin(A3));
    chouse[i] = Mod(DFromR(X)+is.rSid);
  }
}

void HouseEqual(void)
{
  int i;

  for (i = 1; i <= cSign; i++)
    chouse[i] = Mod(is.Asc + ZFromS(i));
}

void HouseCampanus(void)
{
  real KO, DN, X;
  int i;

  for (i = 1; i <= cSign; i++) {
    KO = RFromD(60.000001+30.0*(real)i);
    DN = RAtn(RTan(KO)*RCos(AA));
    if (DN < 0.0)
      DN += rPi;
    if (RSin(KO) < 0.0)
      DN += rPi;
    X = Angle(RCos(is.RA+DN)*RCos(is.OB)-RSin(DN)*RTan(AA)*RSin(is.OB),
      RSin(is.RA+DN));
    chouse[i] = Mod(DFromR(X)+is.rSid);
  }
}

void HouseMeridian(void)
{
  real D, X;
  int i;

  for (i = 1; i <= cSign; i++) {
    D = RFromD(60.0+30.0*(real)i);
    X = Angle(RCos(is.RA+D)*RCos(is.OB), RSin(is.RA+D));
    chouse[i] = Mod(DFromR(X)+is.rSid);
  }
}

void HouseRegiomontanus(void)
{
  real D, X;
  int i;

  for (i = 1; i <= cSign; i++) {
    D = RFromD(60.0+30.0*(real)i);
    X = Angle(RCos(is.RA+D)*RCos(is.OB)-RSin(D)*RTan(AA)*RSin(is.OB),
      RSin(is.RA+D));
    chouse[i] = Mod(DFromR(X)+is.rSid);
  }
}

void HousePorphyry(void)
{
  real X, Y;
  int i;

  X = is.Asc-is.MC;
  if (X < 0.0)
    X += rDegMax;
  Y = X/3.0;
  for (i = 1; i <= 2; i++)
    chouse[i+4] = Mod(rDegHalf+is.MC+i*Y);
  X = Mod(rDegHalf+is.MC)-is.Asc;
  if (X < 0.0)
    X += rDegMax;
  chouse[1]=is.Asc;
  Y = X/3.0;
  for (i = 1; i <= 3; i++)
    chouse[i+1] = Mod(is.Asc+i*Y);
  for (i = 1; i <= 6; i++)
    chouse[i+6] = Mod(chouse[i]+rDegHalf);
}

void HouseMorinus(void)
{
  real D, X;
  int i;

  for (i = 1; i <= cSign; i++) {
    D = RFromD(60.0+30.0*(real)i);
    X = Angle(RCos(is.RA+D), RSin(is.RA+D)*RCos(is.OB));
    chouse[i] = Mod(DFromR(X)+is.rSid);
  }
}

real CuspTopocentric(real deg)
{
  real OA, X, LO;

  OA = ModRad(is.RA+RFromD(deg));
  X = RAtn(RTan(AA)/RCos(OA));
  LO = RAtn(RCos(X)*RTan(OA)/RCos(X+is.OB));
  if (LO < 0.0)
    LO += rPi;
  if (RSin(OA) < 0.0)
    LO += rPi;
  return LO;
}

void HouseTopocentric(void)
{
  real TL, P1, P2, LT;
  int i;

  chouse[4] = ModRad(RFromD(is.MC+rDegHalf-is.rSid));
  TL = RTan(AA); P1 = RAtn(TL/3.0); P2 = RAtn(TL/1.5); LT = AA;
  AA = P1; chouse[5] = CuspTopocentric(30.0) + rPi;
  AA = P2; chouse[6] = CuspTopocentric(60.0) + rPi;
  AA = LT; chouse[1] = CuspTopocentric(90.0);
  AA = P2; chouse[2] = CuspTopocentric(120.0);
  AA = P1; chouse[3] = CuspTopocentric(150.0);
  AA = LT;
  for (i = 1; i <= 6; i++) {
    chouse[i] = Mod(DFromR(chouse[i])+is.rSid);
    chouse[i+6] = Mod(chouse[i]+rDegHalf);
  }
}


/*
******************************************************************************
** Planetary Position Calculations.
******************************************************************************
*/

/* Given three values, return them combined as the coefficients of a */
/* quadratic equation as a function of the chart time.               */

real ReadThree(real r0, real r1, real r2)
{
  return RFromD(r0 + r1*is.T + r2*is.T*is.T);
}


/* Another coordinate transformation. This is used by the ComputePlanets() */
/* procedure to rotate rectangular coordinates by a certain amount.        */

void RecToSph2(real AP, real AN, real _IN, real *X, real *Y, real *G)
{
  real R, D, A;

  RecToPol(*X, *Y, &A, &R); A += AP; PolToRec(A, R, X, Y);
  D = *X; *X = *Y; *Y = 0.0; RecToPol(*X, *Y, &A, &R);
  A += _IN; PolToRec(A, R, X, Y);
  *G = *Y; *Y = *X; *X = D; RecToPol(*X, *Y, &A, &R); A += AN;
  if (A < 0.0)
    A += 2.0*rPi;
  PolToRec(A, R, X, Y);
}


/* Calculate some harmonic delta error correction factors to add onto the */
/* coordinates of Jupiter through Pluto, for better accuracy.             */

void ErrorCorrect(int ind, real *x, real *y, real *z)
{
  real U, V, W, A, S0, T0[4], *pr;
  int IK, IJ, irError;

  irError = cErrorCount[ind-oJup];
  pr = (real *)&rErrorData[iErrorOffset[ind-oJup]];
  for (IK = 1; IK <= 3; IK++) {
    if (ind == oJup && IK == 3) {
      T0[3] = 0.0;
      break;
    }
    if (IK == 3)
      irError--;
    S0 = ReadThree(pr[0], pr[1], pr[2]); pr += 3;
    A = 0.0;
    for (IJ = 1; IJ <= irError; IJ++) {
      U = *pr++; V = *pr++; W = *pr++;
      A += RFromD(U)*RCos((V*is.T+W)*rPi/rDegHalf);
    }
    T0[IK] = DFromR(S0+A);
  }
  *x += T0[2]; *y += T0[1]; *z += T0[3];
}


/* This is probably the heart of the whole program of Astrolog. Calculate  */
/* the position of each body that orbits the Sun. A heliocentric chart is  */
/* most natural; extra calculation is needed to have other central bodies. */

void ComputePlanets(void)
{
  real helioret[oNorm+1], heliox[oNorm+1], helioy[oNorm+1], helioz[oNorm+1];
  real aber = 0.0, AU, E, EA, E1, M, XW, YW, AP, AN, _IN, X, Y, G, XS, YS, ZS;
  int ind = oSun, i;
  OE *poe;

  while (ind <= (us.fUranian ? oNorm : cPlanet)) {
    if (ignore[ind] && ind > oSun)
      goto LNextPlanet;
    poe = &rgoe[IoeFromObj(ind)];

    EA = M = ModRad(ReadThree(poe->ma0, poe->ma1, poe->ma2));
    E = DFromR(ReadThree(poe->ec0, poe->ec1, poe->ec2));
    for (i = 1; i <= 5; i++)
      EA = M+E*RSin(EA);            /* Solve Kepler's equation */
    AU = poe->sma;                  /* Semi-major axis         */
    E1 = 0.01720209/(pow(AU, 1.5)*
      (1.0-E*RCos(EA)));                     /* Begin velocity coordinates */
    XW = -AU*E1*RSin(EA);                    /* Perifocal coordinates      */
    YW = AU*E1*pow(1.0-E*E,0.5)*RCos(EA);
    AP = ReadThree(poe->ap0, poe->ap1, poe->ap2);
    AN = ReadThree(poe->an0, poe->an1, poe->an2);
    _IN = ReadThree(poe->in0, poe->in1, poe->in2); /* Calculate inclination  */
    X = XW; Y = YW;
    RecToSph2(AP, AN, _IN, &X, &Y, &G);  /* Rotate velocity coords */
    heliox[ind] = X; helioy[ind] = Y;
    helioz[ind] = G;                     /* Helio ecliptic rectangular */
    X = AU*(RCos(EA)-E);                 /* Perifocal coordinates for        */
    Y = AU*RSin(EA)*pow(1.0-E*E,0.5);    /* rectangular position coordinates */
    RecToSph2(AP, AN, _IN, &X, &Y, &G);  /* Rotate for rectangular */
    XS = X; YS = Y; ZS = G;              /* position coordinates   */
    if (FBetween(ind, oJup, oPlu))
      ErrorCorrect(ind, &XS, &YS, &ZS);
    ret[ind] = DFromR((XS*helioy[ind]-YS*heliox[ind]) /
      (XS*XS+YS*YS));  /* Helio daily motion */
    spacex[ind] = XS; spacey[ind] = YS; spacez[ind] = ZS;
    ProcessPlanet(ind, 0.0);
LNextPlanet:
    ind += (ind == oSun ? 2 : (ind != cPlanet ? 1 : uranLo+1-cPlanet));
  }

  spacex[oEar] = spacex[oSun];
  spacey[oEar] = spacey[oSun];
  spacez[oEar] = spacez[oSun];
  planet[oEar] = planet[oSun]; planetalt[oEar] = planetalt[oSun];
  ret[oEar] = ret[oSun];
  heliox[oEar] = heliox[oSun]; helioy[oEar] = helioy[oSun];
  helioret[oEar] = helioret[oSun] = RFromD(1.0);
  spacex[oSun] = spacey[oSun] = spacez[oSun] =
    planet[oSun] = planetalt[oSun] = heliox[oSun] = helioy[oSun] = 0.0;
  if (us.objCenter == oSun) {
    if (us.fVelocity)
      for (i = 0; i <= oNorm; i++)    /* Use relative velocity */
        ret[i] = 1.0;                 /* if -v0 is in effect.  */
    return;
  }

  /* A second loop is needed for geocentric charts or central bodies other */
  /* than the Sun. For example, we can't find the position of Mercury in   */
  /* relation to Pluto until we know the position of Pluto in relation to  */
  /* the Sun, and since Mercury is calculated first, another pass needed.  */

  ind = us.objCenter;
  for (i = 0; i <= oNorm; i++) {
    helioret[i] = ret[i];
    if (i != oMoo && i != ind) {
      spacex[i] -= spacex[ind];
      spacey[i] -= spacey[ind];
      spacez[i] -= spacez[ind];
    }
  }
  for (i = oEar; i <= (us.fUranian ? oNorm : cPlanet);
    i += (i == oSun ? 2 : (i != cPlanet ? 1 : uranLo+1-cPlanet))) {
    if ((ignore[i] && i > oSun) || i == ind)
      continue;
    XS = spacex[i]; YS = spacey[i]; ZS = spacez[i];
    ret[i] = DFromR((XS*(helioy[i]-helioy[ind])-YS*(heliox[i]-heliox[ind])) /
      (XS*XS + YS*YS));
    if (ind == oEar && !us.fTruePos)
      aber = 0.0057756 * RSqr(XS*XS+YS*YS+ZS*ZS) * ret[i];  /* Aberration */
    ProcessPlanet(i, aber);
    if (us.fVelocity)                 /* Use relative velocity */
      ret[i] = ret[i]/helioret[i];    /* if -v0 is in effect   */
  }
  spacex[ind] = spacey[ind] = spacez[ind] = 0.0;
}


/*
******************************************************************************
** Lunar Position Calculations
******************************************************************************
*/

/* Calculate the position and declination of the Moon, and the Moon's North  */
/* Node. This has to be done separately from the other planets, because they */
/* all orbit the Sun, while the Moon orbits the Earth.                       */

void ComputeLunar(real *moonlo, real *moonla, real *nodelo, real *nodela)
{
  real LL, G, N, G1, D, L, ML, L1, MB, T1, Y, M = 3600.0, T2;

  T2 = is.T*is.T;
  LL = 973563.0+1732564379.0*is.T-4.0*T2; /* Compute mean lunar longitude    */
  G = 1012395.0+6189.0*is.T;              /* Sun's mean longitude of perigee */
  N = 933060.0-6962911.0*is.T+7.5*T2;     /* Compute mean lunar node         */
  G1 = 1203586.0+14648523.0*is.T-37.0*T2; /* Mean longitude of lunar perigee */
  D = 1262655.0+1602961611.0*is.T-5.0*T2; /* Mean elongation of Moo from Sun */
  L = (LL-G1)/M; L1 = ((LL-D)-G)/M;       /* Some auxiliary angles           */
  T1 = (LL-N)/M; D = D/M; Y = 2.0*D;

  /* Compute Moon's perturbations. */

  ML = 22639.6*RSinD(L) - 4586.4*RSinD(L-Y) + 2369.9*RSinD(Y) +
    769.0*RSinD(2.0*L) - 669.0*RSinD(L1) - 411.6*RSinD(2.0*T1) -
    212.0*RSinD(2.0*L-Y) - 206.0*RSinD(L+L1-Y);
  ML += 192.0*RSinD(L+Y) - 165.0*RSinD(L1-Y) + 148.0*RSinD(L-L1) -
    125.0*RSinD(D) - 110.0*RSinD(L+L1) - 55.0*RSinD(2.0*T1-Y) -
    45.0*RSinD(L+2.0*T1) + 40.0*RSinD(L-2.0*T1);

  *moonlo = G = Mod((LL+ML)/M+is.rSid);    /* Lunar longitude */

  /* Compute lunar latitude. */

  MB = 18461.5*RSinD(T1) + 1010.0*RSinD(L+T1) - 999.0*RSinD(T1-L) -
    624.0*RSinD(T1-Y) + 199.0*RSinD(T1+Y-L) - 167.0*RSinD(L+T1-Y);
  MB += 117.0*RSinD(T1+Y) + 62.0*RSinD(2.0*L+T1) -
    33.0*RSinD(T1-Y-L) - 32.0*RSinD(T1-2.0*L) - 30.0*RSinD(L1+T1-Y);
  *moonla = MB =
    RSgn(MB)*((RAbs(MB)/M)/rDegMax-RFloor((RAbs(MB)/M)/rDegMax))*rDegMax;

  /* Compute position of the North Lunar Node, either True or Mean. */

  if (us.fTrueNode)
    N = N+5392.0*RSinD(2.0*T1-Y)-541.0*RSinD(L1)-442.0*RSinD(Y)+
      423.0*RSinD(2.0*T1)-291.0*RSinD(2.0*L-2.0*T1);
  *nodelo = Mod(N/M+is.rSid);
  *nodela = 0.0;
}
#endif /* MATRIX */

/* matrix.cpp */
