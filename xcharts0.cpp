/*
** Astrolog (Version 6.30) File: xcharts0.cpp
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
** Last code change made 10/22/2017.
*/

#include "astrolog.h"


#ifdef GRAPH
/*
******************************************************************************
** Subchart Graphics Routines.
******************************************************************************
*/

/* Given a string, draw it on the screen using the given color. The       */
/* position of the text is based the saved positions of where we drew the */
/* text the last time the routine was called, being either directly below */
/* in the same column or in the same row just to the right. This is used  */
/* by the sidebar drawing routine to print a list of text on the chart.   */

int DrawPrint(CONST char *sz, int m, int n)
{
  static int x0, x, y;

  if (sz == NULL) {    /* Null string means just initialize position. */
    x0 = x = m; y = n;
    return y;
  }
  if (y >= gs.yWin-1)  /* Don't draw if we've scrolled off the chart bottom. */
    return y;
  DrawColor(m);
  DrawSz(sz, x, y, dtLeft | dtBottom | dtScale2);

  /* If the second parameter is TRUE, we stay on the same line, otherwise */
  /* when FALSE we go to the next line at the original column setting.    */

  if (n)
    x += CchSz(sz) * xFont * gi.nScaleText * gi.nScaleT;
  else {
    x = x0;
    n = y;
    y += yFont * gi.nScaleText * gi.nScaleT;
  }
  return y;
}


/* Draw a zodiac position on the screen, rounding and formatting as needed. */

int DrawZodiac(real deg, int n)
{
  if (us.fRound) {
    if (us.nDegForm == 0)
      deg = Mod(deg + (is.fSeconds ? rRound/60.0/60.0 : rRound/60.0));
    else if (us.nDegForm == 1)
      deg = Mod(deg + (is.fSeconds ? rRound/4.0/60.0 : rRound/4.0));
  }
  return DrawPrint(SzZodiac(deg), kSignB(SFromZ(deg)), n);
}


/* Print chart info in the sidebar for a single chart. Called from   */
/* DrawSidebar() 1-4 times depending on the relationship chart mode. */

void DrawInfo(CI *pci, char *szHeader, flag fAll)
{
  char sz[cchSzDef];

  if (szHeader != NULL)
    DrawPrint(szHeader, gi.kiOn, fFalse);
  if (*pci->nam)
    DrawPrint(pci->nam, gi.kiLite, fFalse);
  if (FNoTimeOrSpace(ciMain))
    DrawPrint("No time or space", gi.kiLite, fFalse);
  else if (us.nRel == rcComposite)
    DrawPrint("Composite chart", gi.kiLite, fFalse);
  else {

    /* Standard case: Print date, time, location, and maybe more. */
    sprintf(sz, "%.3s %s", szDay[DayOfWeek(pci->mon, pci->day, pci->yea)],
      SzDate(pci->mon, pci->day, pci->yea, fTrue));
    DrawPrint(sz, gi.kiLite, fFalse);
    DrawPrint(SzTim(pci->tim), gi.kiLite, fTrue);
    sprintf(sz, " (%cT %s GMT)", ChDst(pci->dst), SzZone(pci->zon));
    DrawPrint(sz, gi.kiLite, fFalse);
    if (*pci->loc)
      DrawPrint(pci->loc, gi.kiLite, fFalse);
    DrawPrint(SzLocation(pci->lon, pci->lat), gi.kiLite, fFalse);
    if (fAll) {
      sprintf(sz, "%s%s houses", us.fHouse3D && gi.nMode != gSphere ?
        "3D " : "", szSystem[us.nHouseSystem]);
      DrawPrint(sz, gi.kiLite, fFalse);
      sprintf(sz, "%s, %s", us.fSidereal ? "Sidereal" : "Tropical",
        us.objCenter == oSun ? "Heliocentric" :
        (us.objCenter == oEar ? "Geocentric" : szObjDisp[us.objCenter]));
      DrawPrint(sz, gi.kiLite, fFalse);
      sprintf(sz, "Julian Day: %13.5f", JulianDayFromTime(is.T));
      DrawPrint(sz, gi.kiLite, fFalse);
    }
  }
}


/* Print text showing the chart information and house and planet positions */
/* of a chart in a "sidebar" to the right of the chart in question. This   */
/* is always done for the -v and -w graphic wheel charts unless the -v0    */
/* switch flag is also set, in which case none of the stuff here is done.  */

void DrawSidebar()
{
  char sz[cchSzDef];
  ET et;
  int i, j, k, l, y, a, s;
  real r;

  /* Decorate the chart a little. */

  if (gs.nDecaType == 1) {

    /* If decoration value 1, draw spider web lines in each corner. */

    DrawColor(gi.kiGray);
    j = gs.nDecaLine + 1;
    k = gs.yWin * gs.nDecaSize / 100;
    l = gs.xWin * gs.nDecaSize / 100;
    for (y = 0; y <= 1; y++)
      for (i = 0; i <= 1; i++)
        for (a = 1; a < j; a++)
          DrawLine(i*(gs.xWin-1), y ? (gs.yWin-1-a*k/j) : a*k/j,
            i ? gs.xWin-1-l+a*l/j : l-a*l/j, y*(gs.yWin-1));
  } else if (gs.nDecaType == 2) {

    /* If decoration value 2, draw a moire pattern in each corner. */

    k = gs.yWin * gs.nDecaSize / 200;
    l = gs.xWin * gs.nDecaSize / 200;
    for (y = 0; y <= 1; y++)
      for (i = 0; i <= 1; i++)
        for (s = 0; s <= 1; s++)
          for (a = 1; a < (s ? l : k)*2; a++) {
            DrawColor(FOdd(a) ? gi.kiGray : gi.kiOff);
            DrawLine(i ? gs.xWin-1-l : l, y ? gs.yWin-1-k : k,
              s ? (i ? gs.xWin-1-a : a) : i*(gs.xWin-1),
              s ? y*(gs.yWin-1) : (y ? gs.yWin-1-a : a));
          }
  }

  if (!gs.fText || us.fVelocity)    /* Don't draw sidebar if */
    return;                         /* -v0 flag is set.      */
  a = us.fAnsiChar;
  us.fAnsiChar = (!gs.fFont || (!gs.fMeta && !gs.fPS)) << 1;
  DrawColor(gi.kiLite);
  if (gs.fBorder)
    DrawLine(gs.xWin-1, 0, gs.xWin-1, gs.yWin-1);
  gs.xWin += xSideT;
  DrawPrint(NULL, gs.xWin-xSideT+xFontT-gi.nScaleT, yFontT*7/5);

  /* Print chart header and setting information. */

  sprintf(sz, "%s %s", szAppName, szVersionCore);
  DrawPrint(sz, gi.kiOn, fFalse);
  if (us.nRel == rcComposite) {
    DrawInfo(&ciMain, "Chart #1", fFalse);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfo(&ciTwin, "Chart #2", fFalse);
  } else if (us.nRel == rcSynastry && !FEqCI(ciMain, ciTwin)) {
    DrawInfo(&ciMain, "Chart #1: Houses", fFalse);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfo(&ciTwin, "Chart #2: Planets", fFalse);
  } else if (us.nRel == rcDual && !FEqCI(ciMain, ciTwin)) {
    DrawInfo(&ciMain, "#1: Inner Wheel, Houses", fFalse);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfo(&ciTwin, "#2: Outer Wheel, Planets", fFalse);
  } else if (us.nRel == rcTriWheel &&
    !(FEqCI(ciMain, ciTwin) && FEqCI(ciTwin, ciThre))) {
    DrawInfo(&ciMain, "#1: Outer Wheel, Houses", fFalse);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfo(&ciTwin, "#2: Middle Wheel", fFalse);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfo(&ciThre, "#3: Inner Wheel, Planets", fFalse);
  } else if (us.nRel == rcQuadWheel && !(FEqCI(ciMain, ciTwin) &&
    FEqCI(ciTwin, ciThre) && FEqCI(ciThre, ciFour))) {
    DrawInfo(&ciMain, "#1: Outer Wheel, Houses", fFalse);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfo(&ciTwin, "#2: Outer Middle Wheel", fFalse);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfo(&ciThre, "#3: Inner Middle Wheel", fFalse);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfo(&ciFour, "#4: Inner Wheel, Planets", fFalse);
  } else if (us.nRel == rcTransit || us.nRel == rcProgress) {
    DrawInfo(&ciMain, "#1: Inner Wheel: Natal", fFalse);
    DrawPrint("", gi.kiLite, fFalse);
    if (us.nRel == rcTransit)
      DrawInfo(&ciTwin, "#2: Outer Wheel: Transit", fFalse);
    else
      DrawInfo(&ciTwin, "#2: Outer Wheel: Progress", fFalse);
  } else
    DrawInfo(&ciMain, NULL, fTrue);

  /* Print house cusp positions. */

  DrawPrint("", gi.kiLite, fFalse);
  for (i = 1; i <= cSign; i++) {
    sprintf(sz, "%2d%s house: ", i, szSuffix[i]);
    y = DrawPrint(sz, kSignB(i), fTrue);
    if (!is.fSeconds && (gs.nScale == 100 ||
      !gs.fFont || !gi.fFile || gs.fBitmap) && y < gs.yWin-1) {
      s = gi.nScale;
      gi.nScale = gi.nScaleText * gi.nScaleT;
      DrawSign(SFromZ(chouse[i]),
        gs.xWin-12*gi.nScale, y-(yFont/2-1)*gi.nScale);
      gi.nScale = s;
    }
    DrawZodiac(chouse[i], fFalse);
  }

  /* Print planet positions. */

  DrawPrint("", gi.kiLite, fFalse);
  for (i = 0; i <= oNorm; i++) if (FProper2(i) && !FCusp(i)) {
    sprintf(sz, is.fSeconds ? "%-3.3s: " : "%-4.4s: ", szObjDisp[i]);
    DrawPrint(sz, kObjB[i], fTrue);
    y = DrawZodiac(planet[i], fTrue);
    if (!is.fSeconds && i < starLo && gi.nMode != gSector && (gs.nScale ==
      100 || !gs.fFont || !gi.fFile || gs.fBitmap) && y < gs.yWin-1) {
      s = gi.nScale;
      gi.nScale = gi.nScaleText * gi.nScaleT;
      DrawObject(-i-1, gs.xWin-12*gi.nScale, y-(yFont/2-1)*gi.nScale);
      gi.nScale = s;
    }
    sprintf(sz, "%c ", ret[i] < 0.0 ? chRet : ' ');
    DrawPrint(sz, gi.kiOn, fTrue);
    if (gi.nMode != gSector || !is.fSeconds) {
      if (FThing(i)) {
        is.fSeconds = fFalse;
        DrawPrint(SzAltitude(planetalt[i]), gi.kiLite, fTrue);
        is.fSeconds = us.fSeconds;
      } else
        DrawPrint("       ", gi.kiOn, fTrue);
    }
    if (gi.nMode == gSector) {
      r = GFromO(cp1.obj[i]); s = (int)r + 1;
      if (!is.fSeconds)
        sprintf(sz, " %2d", s);
      else
        sprintf(sz, "%6.3f%c", r + 1.0, pluszone[s] ? '+' : '-');
      DrawPrint(sz, pluszone[s] ? kRainbowB[1] : kMainB[5], fFalse);
    } else
      DrawPrint("", gi.kiOn, fFalse);
  }

  /* Print star positions. */

  for (i = starLo; i <= starHi; i++) if (FProper(i)) {
    s = oNorm+starname[i-oNorm];
    sprintf(sz, is.fSeconds ? "%3.3s: " : "%4.4s: ", szObjDisp[s]);
    DrawPrint(sz, kObjB[s], fTrue);
    DrawZodiac(planet[s], fTrue);
    DrawPrint("  ", gi.kiOn, fTrue);
    if (gi.nMode != gSector || !is.fSeconds) {
      is.fSeconds = fFalse;
      DrawPrint(SzAltitude(planetalt[s]), gi.kiLite, fTrue);
      is.fSeconds = us.fSeconds;
    }
    if (gi.nMode == gSector) {
      r = GFromO(cp1.obj[s]); s = (int)r + 1;
      if (!is.fSeconds)
        sprintf(sz, " %2d", s);
      else
        sprintf(sz, "%6.3f%c", r + 1.0, pluszone[s] ? '+' : '-');
      DrawPrint(sz, pluszone[s] ? kRainbowB[1] : kMainB[5], fFalse);
    } else
      DrawPrint("", gi.kiOn, fFalse);
  }

  /* Print element table information. */

  DrawPrint("", gi.kiLite, fFalse);
  CreateElemTable(&et);
  sprintf(sz, "Fire: %d,",   et.coElem[eFir]);
  DrawPrint(sz, kElemB[eFir], fTrue);
  sprintf(sz, " Earth: %d,", et.coElem[eEar]);
  DrawPrint(sz, kElemB[eEar], fFalse);
  sprintf(sz, "Air : %d,",   et.coElem[eAir]);
  DrawPrint(sz, kElemB[eAir], fTrue);
  sprintf(sz, " Water: %d",  et.coElem[eWat]);
  DrawPrint(sz, kElemB[eWat], fFalse);
  sprintf(sz, "Car: %d,",  et.coMode[0]); DrawPrint(sz, kModeB(0), fTrue);
  sprintf(sz, " Fix: %d,", et.coMode[1]); DrawPrint(sz, kModeB(1), fTrue);
  sprintf(sz, " Mut: %d",  et.coMode[2]); DrawPrint(sz, kModeB(2), fFalse);
  sprintf(sz, "Yang: %d, Yin: %d", et.coYang, et.coYin);
  DrawPrint(sz, gi.kiLite, fFalse);
  sprintf(sz, "M: %d,",  et.coMC);  DrawPrint(sz, kElemB[eEar], fTrue);
  sprintf(sz, " N: %d,", et.coIC);  DrawPrint(sz, kElemB[eWat], fTrue);
  sprintf(sz, " A: %d,", et.coAsc); DrawPrint(sz, kElemB[eFir], fTrue);
  sprintf(sz, " D: %d",  et.coDes); DrawPrint(sz, kElemB[eAir], fFalse);
  sprintf(sz, "Ang: %d,",  et.coModeH[0]); DrawPrint(sz, kModeB(0), fTrue);
  sprintf(sz, " Suc: %d,", et.coModeH[1]); DrawPrint(sz, kModeB(1), fTrue);
  sprintf(sz, " Cad: %d",  et.coModeH[2]); DrawPrint(sz, kModeB(2), fFalse);
  sprintf(sz, "Learn: %d, Share: %d", et.coLearn, et.coShare);
  DrawPrint(sz, gi.kiLite, fFalse);
  us.fAnsiChar = a;
}


/* This is a subprocedure of XChartWheel() and XChartWheelRelation(). Draw  */
/* the outer sign and house rings for a wheel chart at the specified zodiac */
/* locations and at the given radius values.                                */

void DrawWheel(real *xsign, real *xhouse, int cx, int cy, real unitx,
  real unity, real r1, real r2, real r3, real r4, real r5)
{
  int i;
  real px, py, temp, ra, rb;
  flag fStroke = gs.fPS || gs.fMeta, fSav;

  /* Draw Ascendant/Descendant and Midheaven/Nadir lines across whole chart. */

  if (!gs.fColorHouse) {
    DrawColor(gi.kiLite);
    DrawDash(cx+POINT1(unitx, 0.99, PX(xhouse[sAri])),
             cy+POINT1(unity, 0.99, PY(xhouse[sAri])),
             cx+POINT1(unitx, 0.99, PX(xhouse[sLib])),
             cy+POINT1(unity, 0.99, PY(xhouse[sLib])), !gs.fColor);
    DrawDash(cx+POINT1(unitx, 0.99, PX(xhouse[sCap])),
             cy+POINT1(unity, 0.99, PY(xhouse[sCap])),
             cx+POINT1(unitx, 0.99, PX(xhouse[sCan])),
             cy+POINT1(unity, 0.99, PY(xhouse[sCan])), !gs.fColor);
  } else {
    DrawColor(kSignB(sAri));
    DrawDash(cx+POINT1(unitx, 0.99, PX(xhouse[sAri])),
             cy+POINT1(unity, 0.99, PY(xhouse[sAri])), cx, cy, !gs.fColor);
    DrawColor(kSignB(sLib));
    DrawDash(cx+POINT1(unitx, 0.99, PX(xhouse[sLib])),
             cy+POINT1(unity, 0.99, PY(xhouse[sLib])), cx, cy, !gs.fColor);
    DrawColor(kSignB(sCap));
    DrawDash(cx+POINT1(unitx, 0.99, PX(xhouse[sCap])),
             cy+POINT1(unity, 0.99, PY(xhouse[sCap])), cx, cy, !gs.fColor);
    DrawColor(kSignB(sCan));
    DrawDash(cx+POINT1(unitx, 0.99, PX(xhouse[sCan])),
             cy+POINT1(unity, 0.99, PY(xhouse[sCan])), cx, cy, !gs.fColor);
  }

  /* Draw small five or one degree increments around the zodiac sign ring. */

  fSav = us.fHouse3D; us.fHouse3D = fFalse;
  for (i = 0; i < nDegMax; i++) {
    temp = PZ(HousePlaceInX((real)i, 0.0));
    px = PX(temp); py = PY(temp);
    DrawColor(i%10 ? gi.kiGray : (gs.fColorSign ? kSignB(i/30 + 1) : gi.kiOn));
    if ((gs.fColor || fStroke) && i%5 > 0) {
      ra = r3 + (r4 - r3) * 0.20;
      rb = r3 + (r4 - r3) * 0.80;
    } else {
      ra = r3; rb = r4;
    }
    DrawDash(cx+POINT1(unitx, ra, px), cy+POINT1(unity, ra, py),
      cx+POINT2(unitx, rb, px), cy+POINT2(unity, rb, py),
      (!gs.fColor && !fStroke && i%5 > 0));
  }
  us.fHouse3D = fSav;

  /* Draw circles for the zodiac sign and house rings. */

  DrawColor(gi.kiOn);
  DrawCircle(cx, cy, (int)(unitx*0.95+rRound), (int)(unity*0.95+rRound));
  DrawCircle(cx, cy, (int)(unitx*r4+rRound), (int)(unity*r4+rRound));
  DrawCircle(cx, cy, (int)(unitx*r3+rRound), (int)(unity*r3+rRound));
  DrawCircle(cx, cy, (int)(unitx*r1+rRound), (int)(unity*r1+rRound));

  /* Draw the glyphs for the signs and houses themselves. */

  for (i = 1; i <= cSign; i++) {
    temp = xsign[i];
    DrawColor(gs.fColorSign ? kSignB(i) : gi.kiOn);
    DrawLine(cx+POINT2(unitx, 0.95, PX(temp)),    /* Draw lines separating */
      cy+POINT2(unity, 0.95, PY(temp)),           /* each sign and house   */
      cx+POINT1(unitx, r4, PX(temp)),             /* from each other.      */
      cy+POINT1(unity, r4, PY(temp)));
    DrawColor(gs.fColorHouse ? kSignB(i) : gi.kiOn);
    DrawLine(cx+POINT2(unitx, r3, PX(xhouse[i])),
      cy+POINT2(unity, r3, PY(xhouse[i])),
      cx+POINT1(unitx, r1, PX(xhouse[i])),
      cy+POINT1(unity, r1, PY(xhouse[i])));
    if (gs.fColor && i%3 != 1) {                             /* Lines from */
      DrawColor(gs.fColorHouse ? kSignB(i) : gi.kiGray);     /* each house */
      DrawDash(cx, cy, cx+POINT2(unitx, r1, PX(xhouse[i])),  /* to center  */
        cy+POINT2(unity, r1, PY(xhouse[i])), 1);             /* of wheel.  */
    }
    temp = Midpoint(temp, xsign[Mod12(i+1)]);
    DrawColor(kSignB(i));
    DrawSign(i, cx+POINT1(unitx, r5, PX(temp)),
      cy+POINT1(unity, r5, PY(temp)));
    temp = Midpoint(xhouse[i], xhouse[Mod12(i+1)]);
    DrawHouse(i, cx+POINT1(unitx, r2, PX(temp)),
      cy+POINT1(unity, r2, PY(temp)));
  }
}


/* Another subprocedure of XChartWheel() and XChartWheelRelation(). Draw */
/* a set of planets in a wheel chart, drawing each glyph and a line from */
/* it to a dot indicating the planet's actual location.                  */

void DrawSymbolRing(real *symbol, real *xplanet, real *dir, int cx, int cy,
  real unitx, real unity, real r1, real r2, real r3, real r4)
{
  int i;
  real temp;

  for (i = cObj; i >= 0; i--) if (FProper(i)) {
    if (gs.fLabel) {
      temp = symbol[i];
      DrawColor(dir[i] < 0.0 ? gi.kiGray : gi.kiOn);
      DrawDash(cx+POINT1(unitx, r2, PX(xplanet[i])),
        cy+POINT1(unity, r2, PY(xplanet[i])),
        cx+POINT1(unitx, r3, PX(temp)),
        cy+POINT1(unity, r3, PY(temp)),
        (ret[i] < 0.0 ? 1 : 0) - gs.fColor);
      DrawObject(i, cx+POINT1(unitx, r4, PX(temp)),
        cy+POINT1(unity, r4, PY(temp)));
    } else
      DrawColor(kObjB[i]);
    DrawPoint(cx+POINT1(unitx, r1, PX(xplanet[i])),
      cy+POINT1(unity, r1, PY(xplanet[i])));
  }
}


/*
******************************************************************************
** Map Chart Routines.
******************************************************************************
*/

/* Another stream reader, this one is used by the globe drawing routine: */
/* for the next body of land/water, return its name (and color), its     */
/* longitude and latitude, and a vector description of its outline.      */

flag FReadWorldData(char **nam, char **loc, char **lin)
{
  static char **psz = (char **)szWorldData;
  int i;

  *loc = *psz++;
  *lin = *psz++;
  *nam = *psz++;
  if (*loc[0]) {
    if (gs.fPrintMap && gi.fFile) {
      i = **nam - '0';
      AnsiColor(i ? kRainbowA[i] : kMainA[7]);
      PrintSz(*nam+1); PrintL();
    }
    return fTrue;
  }
  psz = (char **)szWorldData;  /* Reset stream when no data left. */
  return fFalse;
}


/* Given longitude and latitude values on a globe, return the window        */
/* coordinates corresponding to them. In other words, project the globe     */
/* onto the view plane, and return where our coordinates got projected to,  */
/* as well as whether our location is hidden on the back side of the globe. */

flag FGlobeCalc(real x1, real y1, int *u, int *v, int cx, int cy,
  int rx, int ry, int deg)
{
  real j, siny1;

  /* Compute coordinates for a general globe invoked with -XG switch. */

  if (gi.nMode == gGlobe) {
    x1 = Mod(x1+(real)deg);    /* Shift by current globe rotation value. */
    if (gs.rTilt != 0.0) {
      /* Do another coordinate shift if the globe's equator is tilted any. */
      y1 = rDegQuad - y1;
      CoorXform(&x1, &y1, gs.rTilt);
      x1 = Mod(x1); y1 = rDegQuad - y1;
    }
    *v = cy + (int)((real)ry*-RCosD(y1)-rRound);
    *u = cx + (int)((real)rx*-RCosD(x1)*RSinD(y1)-rRound);
    return x1 > rDegHalf;
  }

  /* Compute coordinates for a polar globe invoked with -XP switch. */

  siny1 = RSinD((gs.fSouth ? nDegHalf-y1 : y1) / 2.0);
  j = gs.fSouth ? rDegQuad+x1+deg : 270.0-x1-deg;
  *v = cy + (int)(siny1*(real)ry*RSinD(j)-rRound);
  *u = cx + (int)(siny1*(real)rx*RCosD(j)-rRound);
  return fFalse;
}


/* Draw one "Ley line" on the world map, based coordinates given in terms of */
/* longitude and vertical fractional distance from the center of the earth.  */

void DrawLeyLine(real l1, real f1, real l2, real f2)
{
  l1 = Mod(l1); l2 = Mod(l2);

  /* Convert vertical fractional distance to a corresponding coordinate. */

  f1 = rDegQuad-RAsin(f1)/rPiHalf*rDegQuad;
  f2 = rDegQuad-RAsin(f2)/rPiHalf*rDegQuad;
  DrawWrap((int)(l1*(real)gi.nScale+rRound),
           (int)(f1*(real)gi.nScale+rRound),
           (int)(l2*(real)gi.nScale+rRound),
           (int)(f2*(real)gi.nScale+rRound), 0, gs.xWin-1);
}


/* Draw the main set of planetary Ley lines on the map of the world. This */
/* consists of drawing an icosahedron and then a dodecahedron lattice.    */

void DrawLeyLines(int deg)
{
  real off = (real)deg, phi, h, h1, h2, r, i;

  phi = (RSqr(5.0)+1.0)/2.0;                   /* Icosahedron constants. */
  h = 1.0/(phi*2.0-1.0);
  DrawColor(kMainB[6]);
  for (i = off; i < rDegMax+off; i += 72.0) {  /* Draw icosahedron edges. */
    DrawLeyLine(i, h, i+72.0, h);
    DrawLeyLine(i-36.0, -h, i+36.0, -h);
    DrawLeyLine(i, h, i, 1.0);
    DrawLeyLine(i+36.0, -h, i+36.0, -1.0);
    DrawLeyLine(i, h, i+36.0, -h);
    DrawLeyLine(i, h, i-36.0, -h);
  }
  r = 1.0/RSqr(3.0)/phi/RCos(RFromD(54.0));    /* Dodecahedron constants. */
  h2 = RSqr(1.0-r*r); h1 = h2/(phi*2.0+1.0);
  DrawColor(kMainB[4]);
  for (i = off; i < rDegMax+off; i += 72.0) {  /* Draw docecahedron edges. */
    DrawLeyLine(i-36.0, h2, i+36.0, h2);
    DrawLeyLine(i, -h2, i+72.0, -h2);
    DrawLeyLine(i+36.0, h2, i+36.0, h1);
    DrawLeyLine(i, -h2, i, -h1);
    DrawLeyLine(i+36.0, h1, i+72.0, -h1);
    DrawLeyLine(i+36.0, h1, i, -h1);
  }
}


/* This major routine draws all of Astrolog's map charts. This means       */
/* either the world map or the constellations, in either rectangular or    */
/* globe hemisphere form. The rectangular chart may also be done in a      */
/* Mollewide projection, for six total combinations. We shift the chart by */
/* specified rotational and tilt values, and may draw on the chart each    */
/* planet at its zenith position on Earth or location in constellations.   */

void DrawMap(flag fSky, flag fGlobe, int deg)
{
  char *nam, *loc, *lin, chCmd;
  int X[objMax], Y[objMax], M[objMax], N[objMax],
    cx = gs.xWin/2, cy = gs.yWin/2, rx, ry, lon, lat, unit = 12*gi.nScale,
    x, y, xold, yold, m, n, u, v, i, j, k, l, nScl = gi.nScale;
  flag fNext = fTrue, fSimple;
  real planet1[objMax], planet2[objMax], x1, y1, rT;
#ifdef CONSTEL
  CONST char *pch;
  flag fBlank;
  int isz = 0, nC, xT, yT, xDelta, yDelta, xLo, xHi, yLo, yHi;
#endif

  /* Set up some variables. */
  rx = cx-1; ry = cy-1;
  if (fGlobe)
    fSimple = (gs.rTilt == 0.0 && gi.nMode != gPolar);

#ifdef CONSTEL
  /* Draw a dot grid for large rectangular constellation charts. */
  if (fSky && !fGlobe && !gs.fMollewide && gi.nScale/gi.nScaleT > 2)
    for (yT = 5; yT < nDegHalf; yT += 5)
      for (xT = 5; xT <= nDegMax; xT += 5) {
        DrawColor(xT % 15 == 0 && yT % 10 == 0 ? gi.kiOn : gi.kiGray);
        x = xT+deg;
        if (x >= nDegMax)
          x -= nDegMax;
        DrawPoint(x*nScl, yT*nScl);
      }
#endif

  loop {

    /* Get the next chunk of data to process. Get the starting position, */
    /* map it to the screen, and set the drawing color appropriately.    */

    if (fNext) {
      fNext = fFalse;

      /* For constellations, get data for the next constellation shape. */

      if (fSky) {
#ifdef CONSTEL
        isz++;
        if (isz > cCnstl)
          break;
        DrawColor(gs.fAlt ? kMainB[7] : kRainbowB[6]);
        pch = szDrawConstel[isz];
        lon = nDegMax -
          (((pch[2]-'0')*10+(pch[3]-'0'))*15+(pch[4]-'0')*10+(pch[5]-'0'));
        lat = 90-((pch[6] == '-' ? -1 : 1)*((pch[7]-'0')*10+(pch[8]-'0')));
        pch += 9;
        xLo = xHi = xT = xold = x = lon;
        yLo = yHi = yT = yold = y = lat;
        nC = 0;
        if (fGlobe) {
          FGlobeCalc((real)x, (real)y, &m, &n, cx, cy, rx, ry, deg);
          k = l = fTrue;
        } else {
          xold += deg;
          x += deg;
        }
#else
        ;
#endif

      /* For world maps, get data for the next coastline piece. */

      } else {
        if (!FReadWorldData(&nam, &loc, &lin))
          break;
        i = nam[0]-'0';
        DrawColor((!fGlobe && gi.nMode == gAstroGraph) ? gi.kiOn :
          (gs.fAlt && !gs.fColorHouse ? gi.kiGray :
          (i ? kRainbowB[i] : kMainB[7])));
        lon = (loc[0] == '+' ? 1 : -1)*
          ((loc[1]-'0')*100 + (loc[2]-'0')*10 + (loc[3]-'0'));
        lat = (loc[4] == '+' ? 1 : -1)*((loc[5]-'0')*10 + (loc[6]-'0'));
        if (fGlobe) {
          x = 180-lon;
          y = 90-lat;
          FGlobeCalc((real)x, (real)y, &m, &n, cx, cy, rx, ry, deg);
          k = l = fTrue;
        } else {
          xold = x = 181-lon+deg;
          yold = y = 90-lat;
        }
      }
    }

    /* Get the next unit from the string to draw on the screen as a line. */

    if (fSky) {

      /* For constellations we have a cache of how long we should keep    */
      /* going in the previous direction, as say "u5" for up five should  */
      /* move our pointer up five times without advancing string pointer. */

#ifdef CONSTEL
      if (nC <= 0) {
        if (!(chCmd = *pch)) {
          fNext = fTrue;
          if (gs.fText) {

            /* If we've reached the end of current constellation, compute */
            /* the center location in it based on lower and upper bounds  */
            /* we've maintained, and print the name of the constel there. */

            xT = xLo + (xHi - xLo)*(szDrawConstel[isz][0]-'1')/8;
            yT = yLo + (yHi - yLo)*(szDrawConstel[isz][1]-'1')/8;
            if (xT < 0)
              xT += nDegMax;
            else if (xT > nDegMax)
              xT -= nDegMax;
            if (fGlobe) {
              if (FGlobeCalc((real)xT, (real)yT, &x, &y, cx, cy, rx, ry, deg))
                continue;
            } else {
              xT += deg;
              if (xT > nDegMax)
                xT -= nDegMax;
              if (gs.fMollewide)
                x = 180*nScl + NMultDiv(xT-180, NMollewide(yT-91), 180);
              else
                x = xT*nScl;
              y = yT*nScl;
            }
            DrawColor(gs.fAlt && gi.nMode != gPolar && gi.nMode != gWorldMap ?
              gi.kiGray : kMainB[5]);
            DrawSz(szCnstlAbbrev[isz], x, y, dtCent);
          }
          continue;
        }
        pch++;

        /* Get the next direction and distance from constellation string. */

        if (fBlank = (chCmd == 'b'))
          chCmd = *pch++;
        xDelta = yDelta = 0;
        switch (chCmd) {
        case 'u': yDelta = -1; break;    /* Up    */
        case 'd': yDelta =  1; break;    /* Down  */
        case 'l': xDelta = -1; break;    /* Left  */
        case 'r': xDelta =  1; break;    /* Right */
        case 'U': yDelta = -1; nC = (yT-1)%10+1;    break;  /* Up until    */
        case 'D': yDelta =  1; nC = 10-yT%10;       break;  /* Down until  */
        case 'L': xDelta = -1; nC = (xT+599)%15+1;  break;  /* Left until  */
        case 'R': xDelta =  1; nC = 15-(xT+600)%15; break;  /* Right until */
        default: PrintError("Bad draw.");             /* Shouldn't happen. */
        }
        if (chCmd >= 'a')
          nC = NFromPch(&pch);  /* Figure out how far to draw. */
      }
      nC--;
      xT += xDelta; x += xDelta;
      yT += yDelta; y += yDelta;
      if (fBlank) {
        xold = x; yold = y;    /* We occasionally want to move the pointer */
        l = fFalse;            /* without drawing the line on the screen.  */
        continue;
      }
      if (xT < xLo)         /* Maintain our bounding rectangle for this */
        xLo = xT;           /* constellation if we crossed over it any. */
      else if (xT > xHi)
        xHi = xT;
      if (yT < yLo)
        yLo = yT;
      else if (yT > yHi)
        yHi = yT;
#else
      ;
#endif

    } else {

      /* Get the next unit from the much simpler world map strings. */

      if (!(chCmd = *lin)) {
        fNext = fTrue;
        continue;
      }
      lin++;

      /* Each unit is exactly one character in the coastline string. */

      if (chCmd == 'L' || chCmd == 'H' || chCmd == 'G')
        x--;
      else if (chCmd == 'R' || chCmd == 'E' || chCmd == 'F')
        x++;
      if (chCmd == 'U' || chCmd == 'H' || chCmd == 'E')
        y--;
      else if (chCmd == 'D' || chCmd == 'G' || chCmd == 'F')
        y++;
    }

    /* Transform map coordinates to screen coordinates and draw a line. */

    while (x >= nDegMax)    /* Take care of coordinate wrap around. */
      x -= nDegMax;
    while (x < 0)
      x += nDegMax;
    if (NAbs(x-xold) > nDegHalf)
      xold = x;

    if (fGlobe) {

      /* For globes, we have to go do a complicated transformation, and not */
      /* draw when we're hidden on the back side of the sphere. We're smart */
      /* and try to only do the slow stuff when we know we'll be visible.   */

      if (fSimple) {
        k = x+deg;
        if (k >= nDegMax)
          k -= nDegMax;
        k = gs.fSouth || k <= 180;
      }
      if (k && !FGlobeCalc((real)x, (real)y, &u, &v, cx, cy, rx, ry, deg)) {
        if (l)
          DrawLine(m, n, u, v);
        m = u; n = v;
        l = fTrue;
      } else {
        if (gs.fSouth)
          DrawPoint(u, v);
        l = fFalse;
      }
    } else {

      /* Rectangular maps are much simpler, with screen coordinates      */
      /* proportional to internal coords. For the Mollewide projection   */
      /* we have to apply a factor to the horizontal positioning though. */

      if (gs.fMollewide && gi.nMode != gAstroGraph)
        DrawLine(180*nScl + NMultDiv(xold-180,
          NMollewide(yold-91), 180), (yold-1)*nScl,
          180*nScl + NMultDiv(x-180, NMollewide(y-91), 180), (y-1)*nScl);
      else
        DrawLine(xold*nScl, (yold-1)*nScl, x*nScl, (y-1)*nScl);
      xold = x; yold = y;
    }
  }

  /* Draw the outline of the map, either a circle around globes or a */
  /* Mollewide type ellipse for that type of rectangular chart.      */

  if (gs.fBorder) {
    DrawColor(gi.kiOn);
    if (!fGlobe) {
      if (gs.fMollewide && gi.nMode != gAstroGraph)
        for (j = -1; j <= 1; j += 2)
          for (xold = 0, y = 89; y >= 0; y--, xold = x)
            for (x = NMollewide(y), i = -1; i <= 1; i += 2)
              DrawLine(180*nScl + i*xold - (i==1), (90+j*(y+1))*nScl - (j==1),
                180*nScl + i*x - (i==1), (90+j*y)*nScl - (j==1));
    } else
      DrawEllipse(0, 0, gs.xWin-1, gs.yWin-1);
  }

  /* Now, if we are in an appropriate bonus chart mode, draw each planet at */
  /* its zenith or visible location on the globe or map, if not hidden.     */

  if (!gs.fAlt || gi.nMode == gAstroGraph)
    return;
  rT = gs.fConstel ? rDegHalf - (fGlobe ? 0.0 : (real)deg) :
    (fGlobe ? Lon : Lon - (real)deg);
  if (rT < 0.0)
    rT += rDegMax;
  for (i = 0; i <= cObj; i++) {
    planet1[i] = Tropical(planet[i]);
    planet2[i] = planetalt[i];
    EclToEqu(&planet1[i], &planet2[i]);    /* Calculate zenith long. & lat. */
  }

  /* Compute screen coordinates of each object, if it's even visible. */

  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    x1 = planet1[i];
    if (!fSky)
      x1 = planet1[oMC]-x1;
    if (x1 < 0.0)
      x1 += rDegMax;
    if (x1 > rDegHalf)
      x1 -= rDegMax;
    x1 = Mod(rDegHalf - rT - x1);
    y1 = rDegQuad - planet2[i];
    if (fGlobe) {
      X[i] = FGlobeCalc(x1, y1, &u, &v, cx, cy, rx, ry, deg) ? -1000 : u;
      Y[i] = v;
    } else {
      X[i] = (int)(x1 * (real)nScl);
      Y[i] = (int)(y1 * (real)nScl);
      if (gs.fMollewide)
        X[i] = 180*nScl + NMultDiv(x1-180, NMollewide(y1-90), 180);
    }
    M[i] = X[i]; N[i] = Y[i]+unit/2;
  }

  /* Draw ecliptic equator and zodiac sign wedges. */

  if (us.fHouse3D) {
    if (!gs.fColorSign)
      DrawColor(kMainB[5]);
    for (l = -2; l < cSign; l++) {
      if (gs.fColorSign && l >= 0)
        DrawColor(kSignB(l+1));
      for (i = -90; i < 90; i++) {
        if (gs.fColorSign && l < 0 && i % 30 == 0)
          DrawColor(kSignB((i+90)/30 + (l < -1)*6 + 1));
        if (l >= 0) {
          /* Coordinates for zodiac sign wedge longitude lines. */
          j = l*30; k = i;
        } else {
          /* Coordinates for ecliptic equator latitude line. */
          j = i+90 + (l < -1)*180; k = 0;
        }
        x1 = Tropical((real)j);
        y1 = (real)k;
        EclToEqu(&x1, &y1);
        if (!fSky)
          x1 = planet1[oMC]-(real)x1;
        if (x1 < 0.0)
          x1 += rDegMax;
        if (x1 > rDegHalf)
          x1 -= rDegMax;
        x1 = Mod(rDegHalf - rT - x1);
        y1 = rDegQuad - y1;
        if (fGlobe) {
          j = FGlobeCalc(x1, y1, &u, &v, cx, cy, rx, ry, deg) ? -1000 : u;
          k = v;
        } else {
          j = (int)(x1 * (real)nScl);
          k = (int)(y1 * (real)nScl);
          if (gs.fMollewide)
            j = 180*nScl + NMultDiv(x1-180, NMollewide(y1-91), 180);
        }
        if (j >= 0)
          DrawPoint(j, k);
      }
    }
  }

  /* Now that we have the coordinates of each object, figure out where to   */
  /* draw the glyphs. Again we try not to draw glyphs on top of each other. */

  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    k = l = gs.xWin+gs.yWin;

    /* For each planet, we draw the glyph either right over or right under */
    /* the actual zenith location point. So, find out the closest distance */
    /* of any other planet assuming we place ours at both possibilities.   */

    for (j = 1; j < i; j++) if (FProper(j)) {
      k = Min(k, NAbs(M[i]-M[j]) + NAbs(N[i]-N[j]));
      l = Min(l, NAbs(M[i]-M[j]) + NAbs(N[i]-unit-N[j]));
    }

    /* Normally, we put the glyph right below the actual point. If however  */
    /* another planet is close enough to have their glyphs overlap, and the */
    /* above location is better, then we'll draw the glyph above instead.   */

    if (k < unit || l < unit)
      if (k < l)
        N[i] -= unit;
  }
  for (i = cObj; i >= 0; i--) if (X[i] >= 0 && FProper(i))      /* Draw the */
    DrawObject(i, M[i], N[i]);                                  /* glyphs.  */
  for (i = cObj; i >= 0; i--) if (X[i] >= 0 && FProper(i)) {
    DrawColor(kObjB[i]);
    DrawSpot(X[i], Y[i]);
  }
}


/* Create a chart in the window based on the current graphics chart mode. */
/* This is the main dispatch routine for all of the program's graphics.   */

void DrawChartX()
{
  char sz[cchSzDef];
  int i;
  flag fSky, fSav;

  gi.nScale = gs.nScale/100;

  if (gs.fBitmap || gs.fMeta)
    PrintNotice("Creating graphics chart in memory.");
  DrawClearScreen();
#ifdef CONSTEL
  fSky = gs.fConstel;
#else
  fSky = fFalse;
#endif
  switch (gi.nMode) {
  case gWheel:
  case gHouse:
    if (us.nRel > rcDual)
      XChartWheel();
    else if (us.nRel == rcTriWheel || us.nRel == rcQuadWheel)
      XChartWheelThreeFour();
    else
      XChartWheelRelation();
    break;
  case gGrid:
    if (us.nRel > rcDual)
      XChartGrid();
    else
      XChartGridRelation();
    break;
  case gHorizon:
    if (us.fPrimeVert)
      XChartHorizonSky();
    else
      XChartHorizon();
    break;
  case gOrbit:
    XChartOrbit();
    break;
  case gSector:
    XChartSector();
    break;
  case gDisposit:
    XChartDispositor();
    break;
  case gEsoteric:
    XChartEsoteric();
    break;
  case gAstroGraph:
    DrawMap(fFalse, fFalse, gs.nRot);  /* First draw map of world.           */
    XChartAstroGraph();                /* Then draw astro-graph lines on it. */
    break;
  case gCalendar:
    XChartCalendar();
    break;
  case gEphemeris:
    XChartEphemeris();
    break;
#ifdef WIN
  case gTraTraTim:
  case gTraTraInf:
#endif
  case gTraTraGra:
    XChartTransit(fFalse);
    break;
#ifdef WIN
  case gTraNatTim:
  case gTraNatInf:
#endif
  case gTraNatGra:
    XChartTransit(fTrue);
    break;
  case gSphere:
    XChartSphere();
    break;
  case gWorldMap:
    /* First draw map of world, then maybe Ley lines. */
    DrawMap(fSky, fFalse, gs.nRot);
    if (!fSky && !gs.fMollewide && !gs.fAlt && us.fHouse3D)
      DrawLeyLines(gs.nRot);
    break;
  case gGlobe:
  case gPolar:
    DrawMap(fSky, fTrue, gs.nRot);
    break;
#ifdef BIORHYTHM
  case gBiorhythm:
    XChartBiorhythm();
    break;
#endif
  }

  /* Print text showing chart information at bottom of window. */

  DrawColor(gi.kiLite);
  if (fDrawText) {
    if (FNoTimeOrSpace(ciMain))
      sprintf(sz, "(No time or space)");
    else if (us.nRel == rcComposite)
      sprintf(sz, "(Composite)");
    else {
      fSav = us.fAnsiChar;
      us.fAnsiChar = (!gs.fFont || (!gs.fMeta && !gs.fPS)) << 1;
      i = DayOfWeek(Mon, Day, Yea);
      sprintf(sz, "%.3s %s %s (%cT %s GMT) %s", szDay[i],
        SzDate(Mon, Day, Yea, 2), SzTim(Tim), ChDst(Dst),
        SzZone(Zon), SzLocation(Lon, Lat));
      us.fAnsiChar = fSav;
    }
    DrawSz(sz, gs.xWin/2, gs.yWin-3*gi.nScaleT, dtBottom | dtErase | dtScale2);
  }

  /* Draw a border around the chart if the mode is set and appropriate. */

  if (fDrawBorder)
    DrawEdgeAll();
}
#endif /* GRAPH */

/* xcharts0.cpp */
