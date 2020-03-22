/*
** Astrolog (Version 6.30) File: xcharts1.cpp
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
** Single Chart Graphics Routines.
******************************************************************************
*/

/* Draw a wheel chart, in which the 12 signs and houses are delineated, and  */
/* the planets are inserted in their proper places. This is the default      */
/* graphics chart to generate, as is done when the -v or -w (or no) switches */
/* are included with -X. Draw the aspects in the middle of chart, too.       */

void XChartWheel()
{
  real xsign[cSign+1], xhouse[cSign+1], xplanet[objMax], symbol[objMax];
  int cx, cy, i, j;
  real unitx, unity;

  /* Set up variables and temporarily automatically decrease the horizontal */
  /* chart size to leave room for the sidebar if that mode is in effect.    */

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  cx = gs.xWin/2 - 1; cy = gs.yWin/2 - 1;
  unitx = (real)cx; unity = (real)cy;
  gi.rAsc = gs.objLeft ? planet[NAbs(gs.objLeft)-1] +
    rDegQuad*(gs.objLeft < 0) : chouse[1];
  if (us.fVedic)
    gi.rAsc = gs.objLeft ? (gs.objLeft < 0 ? 120.0 : -60.0)-gi.rAsc : 0.0;

  /* Fill out arrays with the angular degree on the circle of where to */
  /* place each object, cusp, and sign glyph based on the chart mode.  */

  if (gi.nMode == gWheel) {
    for (i = 1; i <= cSign; i++)
      xhouse[i] = PZ(chouse[i]);
  } else {
    gi.rAsc -= chouse[1];
    for (i = 1; i <= cSign; i++)
      xhouse[i] = PZ(ZFromS(i));
  }
  for (i = 1; i <= cSign; i++)
    xsign[i] = PZ(HousePlaceInX(ZFromS(i), 0.0));
  for (i = 0; i <= cObj; i++)
    xplanet[i] = PZ(HousePlaceInX(planet[i], planetalt[i]));

  /* Go draw the outer sign and house rings. */

  DrawWheel(xsign, xhouse, cx, cy, unitx, unity,
    0.65, 0.70, 0.75, 0.80, 0.875);

  for (i = 0; i <= cObj; i++)    /* Figure out where to put planet glyphs. */
    symbol[i] = xplanet[i];
  FillSymbolRing(symbol, 1.0);

  /* For each planet, draw a small dot indicating where it is, and then */
  /* a line from that point to the planet's glyph.                      */

  DrawSymbolRing(symbol, xplanet, ret, cx, cy, unitx, unity,
    0.50, 0.52, 0.56, 0.60);

  /* Draw lines connecting planets which have aspects between them. */

  if (!gs.fAlt) {                /* Don't draw aspects in bonus mode. */
    if (!FCreateGrid(fFalse))
      return;
    for (j = cObj; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j)) {
          DrawColor(kAspB[grid->n[i][j]]);
          DrawDash(cx+POINT1(unitx, 0.48, PX(xplanet[i])),
            cy+POINT1(unity, 0.48, PY(xplanet[i])),
            cx+POINT1(unitx, 0.48, PX(xplanet[j])),
            cy+POINT1(unity, 0.48, PY(xplanet[j])),
            NAbs(grid->v[i][j]/(60*60*2)));
        }
  }

  /* Go draw sidebar with chart information and positions if need be. */

  DrawSidebar();
}


/* Draw an astro-graph chart on a map of the world, i.e. the draw the     */
/* Ascendant, Descendant, Midheaven, and Nadir lines corresponding to the */
/* time in the chart. This chart is done when the -L switch is combined   */
/* with the -X switch.                                                    */

void XChartAstroGraph()
{
  real planet1[objMax], planet2[objMax],
    end1[cObj*2+2], end2[cObj*2+2],
    symbol1[cObj*2+2], symbol2[cObj*2+2],
    lon = Lon, longm, x, y, z, ad, oa, am, od, dm, lat;
  int unit = gi.nScale, fVector, lat1 = -60, lat2 = 75, y1, y2, xold1, xold2,
    i, j, k, l;

  /* Erase top and bottom parts of map. We don't draw the astro-graph lines */
  /* above certain latitudes, and this gives us room for glyph labels, too. */

  y1 = (90-lat1)*gi.nScale;
  y2 = (90-lat2)*gi.nScale;
  DrawColor(gi.kiOff);
  DrawBlock(0, 1, gs.xWin-1, y2-1);
  DrawBlock(0, y1+1, gs.xWin-1, gs.yWin-2);
  DrawColor(gi.kiLite);
  i = gs.yWin/2;
  DrawDash(0, i, gs.xWin-2, i, 4);    /* Draw equator. */
  DrawColor(gi.kiOn);
  DrawLine(1, y2, gs.xWin-2, y2);
  DrawLine(1, y1, gs.xWin-2, y1);
  for (i = 0; i <= cObj*2+1; i++)
    end1[i] = end2[i] = -rLarge;

  /* Draw small hatches every 5 degrees along edges of world map. */

  DrawColor(gi.kiLite);
  for (i = lat1+5; i < lat2; i += 5) {
    j = (90-i)*gi.nScale;
    k = (2+(i%10 == 0)+2*(i%30 == 0))*gi.nScaleT;
    DrawLine(1, j, k, j);
    DrawLine(gs.xWin-2, j, gs.xWin-1-k, j);
  }
  for (i = -nDegHalf+5; i < nDegHalf; i += 5) {
    j = (nDegHalf-i)*gi.nScale;
    k = (2+(i%10 == 0)+2*(i%30 == 0)+(i%90 == 0))*gi.nScaleT;
    DrawLine(j, y2+1, j, y2+k);
    DrawLine(j, y1-1, j, y1-k);
  }
  if (us.fLatitudeCross) {
    DrawColor(kRainbowB[7]);
    i = (int)((rDegQuad - Lat)*(real)gi.nScale);
    DrawLine(0, i, gs.xWin-1, i);
  }

  /* Calculate zenith locations of each planet. */

  for (i = 0; i <= cObj; i++) if (!ignore[i] || i == oMC) {
    planet1[i] = Tropical(i == oMC ? is.MC : planet[i]);
    planet2[i] = planetalt[i];
    EclToEqu(&planet1[i], &planet2[i]);
  }

  /* Draw the Midheaven lines and zenith location markings. */

  if (lon < 0.0)
    lon += rDegMax;
  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    x = planet1[oMC]-planet1[i];
    if (x < 0.0)
      x += rDegMax;
    if (x > rDegHalf)
      x -= rDegMax;
    z = lon + x;
    if (z > rDegHalf)
      z -= rDegMax;
    j = (int)(Mod(rDegHalf-z+gs.nRot)*(real)gi.nScale);
    if (!ignorez[arMC]) {
      DrawColor(kElemB[eEar]);
      DrawLine(j, y1+unit*4, j, y2-unit*1);
    }
    end2[i*2] = (real)j;
    y = planet2[i];
    k = (int)((rDegQuad-y)*(real)gi.nScale);
    if (FBetween((int)y, lat1, lat2) && !ignorez[arMC]) {
      DrawColor(gi.kiLite);
      DrawBlock(j-gi.nScaleT, k-gi.nScaleT, j+gi.nScaleT, k+gi.nScaleT);
      DrawColor(gi.kiOff);
      DrawBlock(j, k, j, k);
    }

    /* Draw Nadir lines assuming we aren't in bonus chart mode. */

    if (!gs.fAlt && !ignorez[arIC]) {
      j += 180*gi.nScale;
      if (j > gs.xWin-2)
        j -= (gs.xWin-2);
      end1[i*2] = (real)j;
      DrawColor(kElemB[eWat]);
      DrawLine(j, y1+unit*2, j, y2-unit*2);
    }
  }

  /* Now, normally, unless we are in bonus chart mode, we will go on to draw */
  /* the Ascendant and Descendant lines here.                                */

  longm = Mod(planet1[oMC] + lon);
  if (!gs.fAlt && (!ignorez[arAsc] || !ignorez[arDes]))
  for (i = 1; i <= cObj; i++) if (FProper(i)) {
    xold1 = xold2 = -1000;

    /* Hack: Normally we draw the Ascendant and Descendant line segments */
    /* simultaneously. However, for the PostScript and metafile vector   */
    /* graphics, this will cause the file to get inordinately large due  */
    /* to the constant thrashing between the Asc and Desc colors. Hence  */
    /* for these charts only, we'll do two passes for Asc and Desc.      */
    fVector = gs.fPS || gs.fMeta;
    for (l = 0; l <= fVector; l++)

    for (lat = (real)lat1; lat <= (real)lat2;
      lat += 1.0/(real)(gi.nScale/gi.nScaleT)) {

      /* First compute and draw the current segment of Ascendant line. */

      j = (int)((rDegQuad-lat)*(real)gi.nScale);
      ad = RTanD(planet2[i])*RTanD(lat);
      if (ad*ad > 1.0)
        ad = rLarge;
      else {
        ad = RAsin(ad);
        oa = planet1[i] - DFromR(ad);
        if (oa < 0.0)
          oa += rDegMax;
        am = oa - rDegQuad;
        if (am < 0.0)
          am += rDegMax;
        z = longm-am;
        if (z < 0.0)
          z += rDegMax;
        if (z > rDegHalf)
          z -= rDegMax;
        k = (int)(Mod(rDegHalf-z+gs.nRot)*(real)gi.nScale);
        if (!fVector || !l) {
          if (!ignorez[arAsc]) {
            DrawColor(kElemB[eFir]);
            DrawWrap(xold1, j+gi.nScaleT, k, j, 1, gs.xWin-2);
          }
          if (lat == (real)lat1) {                          /* Line segment */
            if (!ignorez[arAsc])                            /* pointing to  */
              DrawLine(k, y1, k, y1+unit*4);                /* Ascendant.   */
            end2[i*2+1] = (real)k;
          }
        } else if (lat == (real)lat1)
          end2[i*2+1] = (real)k;
        xold1 = k;
      }

      /* The curving Ascendant and Descendant lines actually touch at low or */
      /* high latitudes. Sometimes when we start out, a particular planet's  */
      /* lines haven't appeared yet, i.e. we are scanning at a latitude      */
      /* where our planet's lines don't exist. If this is the case, then     */
      /* when they finally do start, draw a thin horizontal line connecting  */
      /* the Ascendant and Descendant lines so they don't just start in      */
      /* space. Note that these connected lines aren't labeled with glyphs.  */

      if (ad == rLarge) {
        if (xold1 >= 0) {
          if ((!fVector || !l) && !ignorez[arAsc] && !ignorez[arDes]) {
            DrawColor(gi.kiGray);
            DrawWrap(xold1, j+1, xold2, j+1, 1, gs.xWin-2);
          }
          lat = rDegQuad;
        }
      } else {

        /* Then compute and draw corresponding segment of Descendant line. */

        od = planet1[i] + DFromR(ad);
        dm = od + rDegQuad;
        z = longm-dm;
        if (z < 0.0)
          z += rDegMax;
        if (z > rDegHalf)
          z -= rDegMax;
        k = (int)(Mod(rDegHalf-z+gs.nRot)*(real)gi.nScale);
        if (xold2 < 0 && lat > (real)lat1 && (!fVector || l) &&
          !ignorez[arDes]) {
          DrawColor(gi.kiGray);
          DrawWrap(xold1, j, k, j, 1, gs.xWin-2);
        }
        if ((!fVector || l) && !ignorez[arDes]) {
          DrawColor(kElemB[eAir]);
          DrawWrap(xold2, j+gi.nScaleT, k, j, 1, gs.xWin-2);
          if (lat == (real)lat1)                            /* Line segment */
            DrawLine(k, y1, k, y1+unit*2);                  /* pointing to  */
        }                                                   /* Descendant.  */
        xold2 = k;
      }
    }

    /* Draw segments pointing to top of Ascendant and Descendant lines. */

    if (ad != rLarge) {
      if (!ignorez[arAsc]) {
        DrawColor(kElemB[eFir]);
        DrawLine(xold1, y2, xold1, y2-unit*1);
      }
      if (!ignorez[arDes]) {
        DrawColor(kElemB[eAir]);
        DrawLine(k, y2, k, y2-unit*2);
      }
      end1[i*2+1] = (real)k;
    }
  }

  DrawColor(kMainB[8]);
  i = (int)((rDegHalf - Lon)*(real)gi.nScale);
  j = (int)((rDegQuad - Lat)*(real)gi.nScale);
  if (us.fLatitudeCross)
    DrawSpot(i, j);
  else
    DrawPoint(i, j);

  /* Determine where to draw the planet glyphs. We have four sets of each    */
  /* planet - each planet's glyph appearing in the chart up to four times -  */
  /* one for each type of line. The Midheaven and Ascendant lines are always */
  /* labeled at the bottom of the chart, while the Nadir and Descendant      */
  /* lines at the top. Therefore we need to place two sets of glyphs, twice. */

  for (i = 0; i <= cObj*2+1; i++) {
    symbol1[i] = end1[i];
    symbol2[i] = end2[i];
  }
  FillSymbolLine(symbol1);
  FillSymbolLine(symbol2);

  /* Now actually draw the planet glyphs. */

  for (i = 0; i <= cObj*2+1; i++) {
    j = i >> 1;
    if (FProper(j)) {
      if ((gi.xTurtle = (int)symbol1[i]) > 0 && gs.fLabel &&
        !ignorez[FOdd(i) ? arDes : arIC]) {
        DrawColor(ret[j] < 0.0 ? gi.kiGray : gi.kiOn);
        DrawDash((int)end1[i], y2-unit*2, (int)symbol1[i], y2-unit*4,
          (ret[i] < 0.0 ? 1 : 0) - gs.fColor);
        DrawObject(j, gi.xTurtle, y2-unit*10);
      }
      if ((gi.xTurtle = (int)symbol2[i]) > 0 && gs.fLabel &&
        !ignorez[FOdd(i) ? arAsc : arMC]) {
        DrawColor(ret[j] < 0.0 ? gi.kiGray : gi.kiOn);
        DrawDash((int)end2[i], y1+unit*4, (int)symbol2[i], y1+unit*8,
          (ret[i] < 0.0 ? 1 : 0) - gs.fColor);
        DrawObject(j, gi.xTurtle, y1+unit*14);
        k = FOdd(i) ? oAsc : oMC;
        l = kObjB[k]; kObjB[k] = kObjB[j];
        DrawObject(k, (int)symbol2[i], y1+unit*24-gi.nScaleT);
        kObjB[k] = l;
      }
    }
  }
}


/* Draw an aspect and midpoint grid in the window, with planets labeled down */
/* the diagonal. This chart is done when the -g switch is combined with the  */
/* -X switch. The chart always has a certain number of cells; hence based    */
/* how the restrictions are set up, there may be blank columns and rows,     */
/* or else only the first number of unrestricted objects will be included.   */

void XChartGrid()
{
  char sz[cchSzDef], szT[cchSzDef];
  int nScale, unit, siz, x, y, i, j, k, l;
  KI c;

  nScale = gi.nScale/gi.nScaleT;
  unit = CELLSIZE*gi.nScale; siz = gi.nGridCell*unit;
  sprintf(szT, "");
  i = us.fSmartCusp; us.fSmartCusp = fFalse;
  if (!FCreateGrid(gs.fAlt))
    return;
  us.fSmartCusp = i;

  /* Loop through each cell in each row and column of grid. */

  for (y = 1, j = oEar-1; y <= gi.nGridCell; y++) {
    do {
      j++;
    } while (!FProper(j) && j <= cObj);
    DrawColor(gi.kiGray);
    DrawDash(0, y*unit, siz, y*unit, !gs.fColor);
    DrawDash(y*unit, 0, y*unit, siz, !gs.fColor);
    if (j <= cObj) for (x = 1, i = oEar-1; x <= gi.nGridCell; x++) {
      do {
        i++;
      } while (!FProper(i) && i <= cObj);
      if (i <= cObj) {
        gi.xTurtle = x*unit-unit/2;
        gi.yTurtle = y*unit-unit/2 - (nScale > 2 ? 5*gi.nScaleT : 0);
        k = grid->n[i][j];

        /* If this is an aspect cell, draw glyph of aspect in effect. */

        if (gs.fAlt ? x > y : x < y) {
          if (k) {
            DrawColor(c = kAspB[k]);
            DrawAspect(k, gi.xTurtle, gi.yTurtle);
          }

        /* If this is a midpoint cell, draw glyph of sign of midpoint. */

        } else if (gs.fAlt ? x < y : x > y) {
          DrawColor(c = kSignB(grid->n[i][j]));
          DrawSign(grid->n[i][j], gi.xTurtle, gi.yTurtle);

        /* For cells on main diagonal, draw glyph of planet. */

        } else {
          DrawColor(gi.kiLite);
          DrawEdge((y-1)*unit, (y-1)*unit, y*unit, y*unit);
          DrawObject(i, gi.xTurtle, gi.yTurtle);
        }

        /* When the scale size is 300+, we can print text in each cell: */

        if (nScale > 2 && gs.fLabel) {
          l = NAbs(grid->v[i][j]); k = l / 60; l %= 60;
          if (nScale > 3 && is.fSeconds)
            sprintf(szT, "%s%02d", x == y ? "'" : "", l);

          /* For the aspect portion, print the orb in degrees and minutes. */

          if (gs.fAlt ? x > y : x < y) {
            if (grid->n[i][j])
              sprintf(sz, "%c%d%c%02d'%s", grid->v[i][j] < 0 ?
                (us.fAppSep ? 'a' : '-') : (us.fAppSep ? 's' : '+'),
                k/60, chDeg2, k%60, szT);
            else
              sprintf(sz, "");

          /* For the midpoint portion, print the degrees and minutes. */

          } else if (gs.fAlt ? x < y : x > y)
            sprintf(sz, "%2d%c%02d'%s", k/60, chDeg2, k%60, szT);

          /* For the main diagonal, print degree and sign of each planet. */

          else {
            c = kSignB(grid->n[i][j]);
            sprintf(sz, "%.3s %02d%s", szSignName[grid->n[i][j]], k, szT);
          }
          DrawColor(c);
          DrawSz(sz, x*unit-unit/2, y*unit-3*gi.nScaleT, dtBottom);
        }
      }
    }
  }
}


/* Translate zodiac position to chart pixel coordinates representing local */
/* horizon position, for the rectangular -Z -X switch chart.               */

void PlotHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  *xp = x1 + (int)((real)xs*(Mod(rDegQuad-lon))/rDegMax + rRound);
  *yp = y1 + (int)((real)ys*(rDegQuad-lat)/rDegHalf + rRound);
}

void EclToLocal(real lonE, real latE, real lonM, real latM,
  int x1, int y1, int xs, int ys, int *xp, int *yp)
{
  real lon, lat;

  lon = Tropical(lonE); lat = latE;
  EclToEqu(&lon, &lat);
  lon = Mod(lonM - lon + rDegQuad);
  EquToLocal(&lon, &lat, rDegQuad - latM);
  lon = rDegMax - lon;
  PlotHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
}


/* Draw the local horizon, and draw in the planets where they are at the */
/* time in question, as done when the -Z is combined with the -X switch. */

void XChartHorizon()
{
  real lonM, latM, lonH, latH;
  int x[objMax], y[objMax], m[objMax], n[objMax],
    cx, cy, unit, x1, y1, x2, y2, xs, ys, xp, yp, i, j, k, l;
  char sz[2];

  unit = Max(12, 6*gi.nScale);
  x1 = unit; y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;
  unit = 12*gi.nScale;
  xs = x2-x1; ys = y2-y1; cx = (x1+x2)/2; cy = (y1+y2)/2;

  /* Calculate the local horizon coordinates of each planet. First convert */
  /* zodiac position and declination to zenith longitude and latitude.     */

  lonM = Tropical(is.MC); latM = 0.0;
  EclToEqu(&lonM, &latM);
  latM = Lat;
  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    EclToLocal(planet[i], planetalt[i], lonM, latM, x1, y1, xs, ys,
      &x[i], &y[i]);
    m[i] = x[i]; n[i] = y[i]+unit/2;
  }

  /* Draw sign and house boundaries if 3D house setting is in effect. */

  if (us.fVedic) {
    /* Draw zodiac sign orange wedges. */
    if (!gs.fColorSign)
      DrawColor(kMainB[7]);
    for (i = 0; i < nDegMax; i++) {
      if (gs.fColorSign && i%30 == 0)
        DrawColor(kSignB(i/30 + 1));
      EclToLocal((real)i, 0.0, lonM, latM, x1, y1, xs, ys, &xp, &yp);
      DrawPoint(xp, yp);
    }
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign)
        DrawColor(kSignB(i/30 + 1));
      for (j = -90; j <= 90; j++) {
        EclToLocal((real)i, (real)j, lonM, latM, x1, y1, xs, ys, &xp, &yp);
        DrawPoint(xp, yp);
      }
    }
    k = gi.nScale;
    gi.nScale = gi.nScaleText * gi.nScaleT;
    for (j = -80; j <= 80; j += 160)
      for (i = 1; i <= cSign; i++) {
        EclToLocal((real)(i-1)*30.0+15.0, (real)j, lonM, latM, x1, y1, xs, ys,
          &xp, &yp);
        if (gs.fColorSign)
          DrawColor(kSignB(i));
        DrawSign(i, xp, yp);
      }
    gi.nScale = k;
  }
  if (us.fHouse3D) {
    /* Draw house orange wedges. */
    if (!gs.fColorHouse)
      DrawColor(kMainB[5]);
    for (j = 2; j <= cSign; j += (j%3 == 0 ? 2 : 1)) {
      if (gs.fColorHouse)
        DrawColor(kSignB(j));
      for (i = 0; i < nDegHalf; i++) {
        lonH = (real)(i + !FBetween(j, sCan, sSag)*180); latH = 0.0;
        k = (j < sCan ? j-1 : (j < sCap ? j-7 : j-13))*30;
        CoorXform(&lonH, &latH, (real)k);
        PlotHorizon(Mod(lonH + rDegQuad), latH, x1, y1, xs, ys,
          &xp, &yp);
        DrawPoint(xp, yp);
      }
    }
    for (i = 1; i <= cSign; i++) {
      xp = FBetween(i, sCan, sSag) * nDegHalf;
      yp = i < sCan ? 15-30*i : (i < sCap ? 30*i-195 : 375-30*i);
      PlotHorizon((real)xp, (real)yp, x1, y1, xs, ys, &xp, &yp);
      if (gs.fColorHouse)
        DrawColor(kSignB(i));
      DrawHouse(i, xp, yp);
    }
  }

  /* Draw vertical lines dividing our rectangle into four areas. In our     */
  /* local space chart, the middle line represents due south, the left line */
  /* due east, the right line due west, and the edges due north. A fourth   */
  /* horizontal line divides that which is above and below the horizon.     */

  if (us.fHouse3D) {
    DrawColor(gs.fColorHouse ? kSignB(sCap) : kMainB[5]);
    DrawDash(cx, y1, cx, cy, 1);
    DrawColor(gs.fColorHouse ? kSignB(sCan) : kMainB[5]);
    DrawDash(cx, cy, cx, y2, 1);
  }
  DrawColor(gi.kiGray);
  if (!us.fHouse3D)
    DrawDash(cx, y1, cx, y2, 1);
  DrawDash((cx+x1)/2, y1, (cx+x1)/2, y2, 1);
  DrawDash((cx+x2)/2, y1, (cx+x2)/2, y2, 1);
  DrawColor(gi.kiOn);
  DrawEdge(x1, y1, x2, y2);
  if (us.fHouse3D) {
    if (gs.fColorHouse)
      DrawColor(kSignB(sAri));
    DrawDash(x1, cy, cx, cy, 1);
    if (gs.fColorHouse)
      DrawColor(kSignB(sLib));
    DrawDash(cx, cy, x2, cy, 1);
  } else
    DrawDash(x1, cy, x2, cy, 1);

  /* Make a slightly smaller rectangle within the window to draw the planets */
  /* in. Make segments on all four edges marking 5 degree increments.        */

  DrawColor(gi.kiLite);
  for (i = 5; i < 180; i += 5) {
    j = y1+(int)((real)i*(real)ys/rDegHalf);
    k = (2+(i%10 == 0)+2*(i%30 == 0))*gi.nScaleT;
    DrawLine(x1+1, j, x1+1+k, j);
    DrawLine(x2-1, j, x2-1-k, j);
  }
  sz[1] = chNull;
  for (i = 0; i <= nDegMax; i += 5) {
    j = x1+(int)((real)i*(real)xs/rDegMax);
    if (i > 0 && i < nDegMax) {
      k = (2+(i%10 == 0)+2*(i%30 == 0))*gi.nScaleT;
      DrawLine(j, y1+1, j, y1+1+k);
      DrawLine(j, y2-1, j, y2-1-k);
    }
    if (i % 90 == 0) {
      *sz = *szDir[i/90 & 3];
      DrawSz(sz, j, y1-2*gi.nScaleT, dtBottom);
    }
  }

  /* As in the DrawGlobe() routine, we now determine where to draw the   */
  /* glyphs in relation to the actual points, so that the glyphs aren't  */
  /* drawn on top of each other if possible. Again, we assume that we'll */
  /* put the glyph right under the point, unless there would be some     */
  /* overlap and the above position is better off.                       */

  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    k = l = gs.xWin+gs.yWin;
    for (j = 0; j < i; j++) if (FProper(j)) {
      k = Min(k, NAbs(m[i]-m[j])+NAbs(n[i]-n[j]));
      l = Min(l, NAbs(m[i]-m[j])+NAbs(n[i]-unit-n[j]));
    }
    if (k < unit || l < unit)
      if (k < l)
        n[i] -= unit;
  }
  for (i = cObj; i >= 0; i--) if (FProper(i))    /* Draw planet's glyph. */
    DrawObject(i, m[i], n[i]);
  for (i = cObj; i >= 0; i--) if (FProper(i)) {
    DrawColor(kObjB[i]);
    if (!gs.fAlt || i > oNorm)
      DrawPoint(x[i], y[i]);      /* Draw small or large dot */
    else                          /* near glyph indicating   */
      DrawSpot(x[i], y[i]);       /* exact local location.   */
  }
}


/* Translate zodiac position to chart pixel coordinates representing local */
/* horizon position, for the circular -Z0 -X switch chart.                 */

void PlotHorizonSky(real lon, real lat, int cx, int cy, real rx, real ry,
  int *xp, int *yp)
{
  real s, x, y;

  s = (rDegQuad-lat)/rDegQuad;
  if (s > 1.0) {
    x = rx * (rSqr2 - 1.0);
    y = ry * (rSqr2 - 1.0);
    if (lon < 45.0 || lon >= rDegMax-45.0 ||
      FBetween(lon, rDegHalf-45.0, rDegHalf+45.0))
      s = 1.0 + (s - 1.0) * (((rx + x)/RAbs(RCosD(lon))-rx) / rx);
    else if (lon < 135.0 || lon >= rDegMax-135.0)
      s = 1.0 + (s - 1.0) * (((ry + y)/RAbs(RCosD(lon-90.0))-ry) / ry);
  }
  *xp = cx+(int)(rx*s*RCosD(rDegHalf+lon)+rRound);
  *yp = cy+(int)(ry*s*RSinD(rDegHalf+lon)+rRound);
}

void EclToLocalSky(real lonE, real latE, real lonM, real latM,
  int cx, int cy, real rx, real ry, int *xp, int *yp)
{
  real lon, lat;

  lon = Tropical(lonE); lat = latE;
  EclToEqu(&lon, &lat);
  lon = Mod(lonM - lon + rDegQuad);
  EquToLocal(&lon, &lat, rDegQuad - latM);
  lon = rDegMax - lon;
  PlotHorizonSky(lon, lat, cx, cy, rx, ry, xp, yp);
}


/* Draw the local horizon, and draw in the planets where they are at the  */
/* time in question. This chart is done when the -Z0 is combined with the */
/* -X switch. This is an identical function to XChartHorizon(); however,  */
/* that routine's chart is entered on the horizon and meridian. Here we   */
/* center the chart around the center of the sky straight up from the     */
/* local horizon, with the horizon itself being an encompassing circle.   */

void XChartHorizonSky()
{
  real lonM, latM, lonH, latH, rx, ry, s;
  int x[objMax], y[objMax], m[objMax], n[objMax],
    cx, cy, unit, x1, y1, x2, y2, xs, ys, xp, yp, i, j, k, l;

  unit = Max(12, 6*gi.nScale);
  x1 = unit; y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;
  unit = 12*gi.nScale;
  xs = x2-x1; ys = y2-y1; cx = (x1+x2)/2; cy = (y1+y2)/2;
  rx = (real)xs/2.0/rSqr2; ry = (real)ys/2.0/rSqr2;

  /* Calculate the local horizon coordinates of each planet. First convert */
  /* zodiac position and declination to zenith longitude and latitude.     */

  lonM = Tropical(is.MC); latM = 0.0;
  EclToEqu(&lonM, &latM);
  latM = Lat;
  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    EclToLocalSky(planet[i], planetalt[i], lonM, latM, cx, cy, rx, ry,
      &x[i], &y[i]);
    m[i] = x[i]; n[i] = y[i]+unit/2;
  }

  /* Draw sign and house boundaries if 3D house setting is in effect. */

  if (us.fVedic) {
    /* Draw zodiac sign orange wedges. */
    if (!gs.fColorSign)
      DrawColor(kMainB[7]);
    for (i = 0; i < nDegMax; i++) {
      if (gs.fColorSign && i%30 == 0)
        DrawColor(kSignB(i/30 + 1));
      EclToLocalSky((real)i, 0.0, lonM, latM, cx, cy, rx, ry, &xp, &yp);
      DrawPoint(xp, yp);
    }
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign)
        DrawColor(kSignB(i/30 + 1));
      for (j = -90; j <= 90; j++) {
        EclToLocalSky((real)i, (real)j, lonM, latM, cx, cy, rx, ry, &xp, &yp);
        DrawPoint(xp, yp);
      }
    }
    k = gi.nScale;
    gi.nScale = gi.nScaleText * gi.nScaleT;
    for (j = -80; j <= 80; j += 160)
      for (i = 1; i <= cSign; i++) {
        EclToLocalSky((real)(i-1)*30.0+15.0, (real)j, lonM, latM, cx, cy,
          rx, ry, &xp, &yp);
        if (gs.fColorSign)
          DrawColor(kSignB(i));
        DrawSign(i, xp, yp);
      }
    gi.nScale = k;
  }
  if (us.fHouse3D) {
    /* Draw house orange wedges. */
    if (!gs.fColorHouse)
      DrawColor(kMainB[5]);
    for (j = 2; j <= cSign; j += (j%3 == 0 ? 2 : 1)) {
      if (gs.fColorHouse)
        DrawColor(kSignB(j));
      for (i = 0; i < nDegHalf; i++) {
        lonH = (real)(i + !FBetween(j, sCan, sSag)*180); latH = 0.0;
        k = (j < sCan ? j-1 : (j < sCap ? j-7 : j-13))*30;
        CoorXform(&lonH, &latH, (real)k);
        PlotHorizonSky(Mod(lonH + rDegQuad), latH, cx, cy,
          rx, ry, &xp, &yp);
        DrawPoint(xp, yp);
      }
    }
    for (i = 1; i <= cSign; i++) {
      xp = FBetween(i, sCan, sSag) * nDegHalf;
      yp = i < sCan ? 15-30*i : (i < sCap ? 30*i-195 : 375-30*i);
      PlotHorizonSky((real)xp, (real)yp, cx, cy, rx, ry, &xp, &yp);
      if (gs.fColorHouse)
        DrawColor(kSignB(i));
      DrawHouse(i, xp, yp);
    }
  }

  /* Draw a circle in window to indicate horizon line, lines dividing   */
  /* the window into quadrants to indicate n/s and w/e meridians, and   */
  /* segments on these lines and the edges marking 5 degree increments. */

  DrawColor(gi.kiGray);
  DrawDash(cx, y1, cx, y2, 1);
  DrawDash(x1, cy, x2, cy, 1);
  DrawColor(gi.kiLite);
  for (i = -125; i <= 125; i += 5) {
    k = (2+(i/10*10 == i ? 1 : 0)+(i/30*30 == i ? 2 : 0))*gi.nScaleT;
    s = 1.0/(rDegQuad*rSqr2);
    j = cy+(int)(s*ys/2*i);
    DrawLine(cx-k, j, cx+k, j);
    j = cx+(int)(s*xs/2*i);
    DrawLine(j, cy-k, j, cy+k);
  }
  for (i = 5; i < 55; i += 5) {
    k = (2+(i/10*10 == i ? 1 : 0)+(i/30*30 == i ? 2 : 0))*gi.nScaleT;
    s = 1.0/(rDegHalf-rDegQuad*rSqr2);
    j = (int)(s*ys/2*i);
    DrawLine(x1, y1+j, x1+k, y1+j);
    DrawLine(x1, y2-j, x1+k, y2-j);
    DrawLine(x2, y1+j, x2-k, y1+j);
    DrawLine(x2, y2-j, x2-k, y2-j);
    j = (int)(s*xs/2*i);
    DrawLine(x1+j, y1, x1+j, y1+k);
    DrawLine(x2-j, y1, x2-j, y1+k);
    DrawLine(x1+j, y2, x1+j, y2-k);
    DrawLine(x2-j, y2, x2-j, y2-k);
  }
  DrawSz("N", cx, y1-2*gi.nScaleT, dtBottom);
  DrawSz("E", x1/2, cy+2*gi.nScaleT, dtCent);
  DrawSz("W", (gs.xWin+x2)/2, cy+2*gi.nScaleT, dtCent);
  if (!gs.fText)
    DrawSz("S", cx, gs.yWin-3*gi.nScaleT, dtBottom);
  DrawColor(gi.kiOn);
  DrawEdge(x1, y1, x2, y2);
  DrawCircle(cx, cy, (int)rx, (int)ry);
  for (i = 0; i < nDegMax; i += 5) {
    k = (2+(i/10*10 == i ? 1 : 0)+(i/30*30 == i ? 2 : 0))*gi.nScaleT;
    DrawLine(cx+(int)((rx-k)*RCosD((real)i)), cy+(int)((ry-k)*RSinD((real)i)),
      cx+(int)((rx+k)*RCosD((real)i)), cy+(int)((ry+k)*RSinD((real)i)));
  }

  /* As in the DrawGlobe() routine, we now determine where to draw the   */
  /* glyphs in relation to the actual points, so that the glyphs aren't  */
  /* drawn on top of each other if possible. Again, we assume that we'll */
  /* put the glyph right under the point, unless there would be some     */
  /* overlap and the above position is better off.                       */

  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    k = l = gs.xWin+gs.yWin;
    for (j = 0; j < i; j++) if (FProper(j)) {
      k = Min(k, NAbs(m[i]-m[j])+NAbs(n[i]-n[j]));
      l = Min(l, NAbs(m[i]-m[j])+NAbs(n[i]-unit-n[j]));
    }
    if (k < unit || l < unit)
      if (k < l)
        n[i] -= unit;
  }
  for (i = cObj; i >= 0; i--) if (FProper(i))  /* Draw glyph. */
    DrawObject(i, m[i], n[i]);
  for (i = cObj; i >= 0; i--) if (FProper(i)) {
    DrawColor(kObjB[i]);
    if (!gs.fAlt || i > oNorm)
      DrawPoint(x[i], y[i]);      /* Draw small or large dot */
    else                          /* near glyph indicating   */
      DrawSpot(x[i], y[i]);       /* exact local location.   */
  }
}


/* Draw a chart depicting an aerial view of the solar system in space, with */
/* all the planets drawn around the Sun, and the specified central planet   */
/* in the middle, as done when the -S is combined with the -X switch.       */

void XChartOrbit()
{
  int x[objMax], y[objMax], m[objMax], n[objMax],
    cx = gs.xWin / 2, cy = gs.yWin / 2, unit, x1, y1, x2, y2, i, j, k, l;
  real sx, sy, sz, xp, yp, a;

  unit = Max(gs.fText*12, 6*gi.nScale);
  x1 = unit; y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;
  unit = 12*gi.nScale;

  /* Determine the scale of the chart. For a scale size of 400, make the */
  /* graphic 1 AU in radius (just out to Earth's orbit). For 300, make   */
  /* the chart 6 AU in radius (enough for inner planets out to asteroid  */
  /* belt). For a scale of 200, make window 30 AU in radius (enough for  */
  /* planets out to Neptune). For scale of 100, make it 90 AU in radius  */
  /* (enough for all planets including the orbits of the Uranians.)      */

  i = gi.nScale/gi.nScaleT;
  sz = i <= 1 ? 90.0 : (i == 2 ? 30.0 : (i == 3 ? 6.0 :
    (gi.nScaleText <= 1 ? 1.0 : 0.006)));
  sx = (real)(cx-x1)/sz; sy = (real)(cy-y1)/sz;
  for (i = 0; i <= oNorm; i++) if (FProper(i)) {
    xp = spacex[i]; yp = spacey[i];
    if (us.fHouse3D) {
      a = RSqr(xp*xp + yp*yp);
      if (a > rSmall)
        a = (log(a / sz * 100.0 + 1.0)/log(101.0)) / (a / sz);
      xp *= a; yp *= a;
    }
    x[i] = cx-(int)(xp*sx); y[i] = cy+(int)(yp*sy);
    m[i] = x[i]; n[i] = y[i]+unit/2;
  }

  /* As in the DrawGlobe() routine, we now determine where to draw the   */
  /* glyphs in relation to the actual points, so that the glyphs aren't  */
  /* drawn on top of each other if possible. Again, we assume that we'll */
  /* put the glyph right under the point, unless there would be some     */
  /* overlap and the above position is better off.                       */

  for (i = 0; i <= oNorm; i++) if (FProper(i)) {
    k = l = gs.xWin+gs.yWin;
    for (j = 0; j < i; j++) if (FProper(j)) {
      k = Min(k, NAbs(m[i]-m[j])+NAbs(n[i]-n[j]));
      l = Min(l, NAbs(m[i]-m[j])+NAbs(n[i]-unit-n[j]));
    }
    if (k < unit || l < unit)
      if (k < l)
        n[i] -= unit;
  }

  /* Draw the 12 sign boundaries from the center body to edges of screen. */

  i = us.objCenter != oSun ? oSun : oEar;
  a = Mod(DFromR(Angle(spacex[i], spacey[i]))-planet[i]);
  if (!gs.fColorSign)
    DrawColor(gi.kiGray);
  for (i = 0; i < cSign; i++) {
    if (gs.fColorSign)
      DrawColor(kSignB(i+1));
    k = cx-2*(int)((real)cx*RCosD((real)i*30.0+a));
    l = cy+2*(int)((real)cy*RSinD((real)i*30.0+a));
    DrawClip(cx, cy, k, l, x1, y1, x2, y2, 1);
  }
  DrawColor(gi.kiLite);
  DrawEdge(x1, y1, x2, y2);
  for (i = oNorm; i >= 0; i--)
    if (FProper(i) && FInRect(m[i], n[i], x1, y1, x2, y2))
      DrawObject(i, m[i], n[i]);
  for (i = oNorm; i >= 0; i--)
    if (FProper(i) && FInRect(x[i], y[i], x1, y1, x2, y2)) {
      DrawColor(kObjB[i]);
      if (!gs.fAlt || i > oNorm)
        DrawPoint(x[i], y[i]);      /* Draw small or large dot */
      else                          /* near glyph indicating   */
        DrawSpot(x[i], y[i]);       /* exact orbital location. */
    }
}


/* Draw a chart showing the 36 Gauquelin sectors, with all the planets    */
/* positioned in their appropriate sector (and at the correct fracton     */
/* across the sector) as done when the -l is combined with the -X switch. */

void XChartSector()
{
  real xplanet[objMax], symbol[objMax];
  byte ignoreSav[objMax], ignoreT[objMax];
  char sz[3];
  int cx, cy, i, j, k;
  real unitx, unity, px, py, temp;

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  cx = gs.xWin/2 - 1; cy = gs.yWin/2 - 1;
  unitx = (real)cx; unity = (real)cy;

  /* Draw lines across the whole chart at the four angles. */

  DrawColor(gi.kiLite);
  DrawDash(cx+POINT1(unitx, 0.99, PX(0.0)),
           cy+POINT1(unity, 0.99, PY(0.0)),
           cx+POINT1(unitx, 0.99, PX(180.0)),
           cy+POINT1(unity, 0.99, PY(180.0)), !gs.fColor);
  DrawDash(cx+POINT1(unitx, 0.99, PX(90.0)),
           cy+POINT1(unity, 0.99, PY(90.0)),
           cx+POINT1(unitx, 0.99, PX(270.0)),
           cy+POINT1(unity, 0.99, PY(270.0)), !gs.fColor);

  /* Draw circles and radial lines delineating the 36 sectors. */

  DrawColor(gi.kiOn);
  for (i = 0; i < nDegMax; i += 10) {
    px = PX((real)i); py = PY((real)i);
    DrawLine(cx+POINT1(unitx, 0.81, px), cy+POINT1(unity, 0.81, py),
      cx+POINT2(unitx, 0.95, px), cy+POINT2(unity, 0.95, py));
  }
  DrawCircle(cx, cy, (int)(unitx*0.95+rRound), (int)(unity*0.95+rRound));
  DrawCircle(cx, cy, (int)(unitx*0.81+rRound), (int)(unity*0.81+rRound));

  /* Label the 36 sectors, with plus zones in red and normal in dark green. */

  k = pluszone[cSector];
  for (i = 1; i <= cSector; i++) {
    j = pluszone[i];
    DrawColor(j ? kRainbowB[1] : kMainB[5]);
    sprintf(sz, "%d", i);
    DrawSz(sz, cx+POINT1(unitx, 0.88, PX((real)(i*10+175)))+
      (FBetween(i, 12, 19) ? -(gi.nScale/* *gi.nScaleT*/) : 0),
      cy+POINT1(unity, 0.88, PY((real)(i*10+175)))+(gi.nScale/* *gi.nScaleT*/),
      dtCent | dtScale);
    sprintf(sz, "%c", j ? '+' : '-');
    DrawSz(sz, cx+POINT1(unitx, 0.97, PX((real)(i*10+175))),
      cy+POINT1(unity, 0.97, PY((real)(i*10+175)))+gi.nScaleT*2, dtCent);
    if (j != k) {
      DrawColor(gi.kiGray);
      DrawDash(cx, cy, cx+POINT2(unitx, 0.81, PX((real)(i*10+170))),
        cy+POINT2(unity, 0.81, PY((real)(i*10+170))), 1);
    }
    k = j;
  }

  if (!gs.fAlt && !FCreateGrid(fFalse))
    return;
  CopyRgb(ignore, ignoreSav, sizeof(ignore));
  CastSectors();    /* Go compute the planets' sector positions. */

  for (i = 0; i <= cObj; i++)    /* Figure out where to put planet glyphs. */
    symbol[i] = xplanet[i] = Mod(rDegHalf - planet[i]);
  FillSymbolRing(symbol, 1.0);

  /* For each planet, draw a small dot indicating where it is, and then */
  /* a line from that point to the planet's glyph.                      */

  for (i = cObj; i >= 0; i--) if (FProper(i)) {
    if (gs.fLabel) {
      temp = symbol[i];
      DrawColor(ret[i] < 0.0 ? gi.kiGray : gi.kiOn);
      DrawDash(cx+POINT1(unitx, 0.67, PX(xplanet[i])),
        cy+POINT1(unity, 0.67, PY(xplanet[i])),
        cx+POINT1(unitx, 0.71, PX(temp)),
        cy+POINT1(unity, 0.71, PY(temp)),
        (ret[i] < 0.0 ? 1 : 0) - gs.fColor);
      DrawObject(i, cx+POINT1(unitx, 0.75, PX(temp)),
        cy+POINT1(unity, 0.75, PY(temp)));
    } else
      DrawColor(kObjB[i]);
    DrawPoint(cx+POINT1(unitx, 0.65, PX(xplanet[i])),
      cy+POINT1(unity, 0.65, PY(xplanet[i])));
  }

  /* Draw lines connecting planets which have aspects between them. */

  if (!gs.fAlt) {                  /* Don't draw aspects in bonus mode. */
    for (j = cObj; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j)) {
          DrawColor(kAspB[grid->n[i][j]]);
          DrawDash(cx+POINT1(unitx, 0.63, PX(xplanet[i])),
            cy+POINT1(unity, 0.63, PY(xplanet[i])),
            cx+POINT1(unitx, 0.63, PX(xplanet[j])),
            cy+POINT1(unity, 0.63, PY(xplanet[j])),
            NAbs(grid->v[i][j]/(60*60*2)));
        }
  }

  CopyRgb(ignore, ignoreT, sizeof(ignore));
  cp1 = cp0;
  CopyRgb(ignoreSav, ignore, sizeof(ignore));
  CastChart(fTrue);
  CopyRgb(ignoreT, ignore, sizeof(ignore));
  DrawSidebar();
  CopyRgb(ignoreSav, ignore, sizeof(ignore));
}


/* Draw an arrow from one point to another, a line with an arrowhead at the */
/* ending point. The size of the arrowhead is based on current scale size,  */
/* and the line segment is actually shorter and doesn't touch either        */
/* endpoint by the same amount. This is used by XChartDispositor() below.   */

void DrawArrow(int x1, int y1, int x2, int y2)
{
  real r, s, a;

  r = DFromR(Angle((real)(x2-x1), (real)(y2-y1)));
  s = (real)(gi.nScale*8);
  x1 += (int)(s*RCosD(r)); y1 += (int)(s*RSinD(r));    /* Shrink line by    */
  x2 -= (int)(s*RCosD(r)); y2 -= (int)(s*RSinD(r));    /* the scale amount. */
  s = (real)(gi.nScale)*4.5;
  DrawLine(x1, y1, x2, y2);                            /* Main segment. */
  for (a = -1.0; a <= 1.0; a += 2.0)
    DrawLine(x2, y2, x2 + (int)(s*RCosD(r + a*135.0)), /* The two arrow     */
      y2 + (int)(s*RSinD(r + a*135.0)));               /* head line pieces. */
}


/* Draw dispositor graphs for the 10 main planets, as done when the -j is   */
/* combined with the -X switch. Four graphs are drawn, one in each screen   */
/* quadrant. A dispositor graph may be based on the sign or house position, */
/* and the planets may be arranged in a hierarchy or a wheel format.        */

void XChartDispositor()
{
  int oDis[oMain+3], dLev[oMain+3], cLev[oMain+3], xo[oMain+3], yo[oMain+3];
  real xCirc[oMain+3], yCirc[oMain+3];
  char sz[cchSzDef];
  int *rgRules, oNum, xLev, yLev, xSub, ySub, cx0, cy0, cx, cy, i, j, k;

  /* Determine rulership and object sets to use. */

  oNum = oMain+2;
  if (ignore7[rrStd] && ignore7[rrEso] && !ignore7[rrHie])
    rgRules = rgSignHie1;
  else if (ignore7[rrStd] && !ignore7[rrEso])
    rgRules = rgSignEso1;
  else {
    rgRules = rules;
    oNum = oMain;
  }

  /* Set up screen positions of the 10 planets for the wheel graphs. */

  cx0 = gs.xWin / 2; cy0 = gs.yWin / 2;
  for (i = 1; i <= oNum; i++) {
    if ((j = (180-(i-1)*360/oNum)) < 0)
      j += nDegMax;
    xCirc[i] = (real)cx0*0.4*RCosD((real)j);
    yCirc[i] = (real)cy0*0.4*RSinD((real)j);
  }

  /* Loop over the two basic dispositor types: sign based and house based. */

  for (xSub = 0; xSub <= 1; xSub++) {
    cx = xSub * cx0 + cx0 / 2;

    /* For each planet, get its dispositor planet for current graph type. */

    for (i = 1; i <= oNum; i++) {
      k = i <= oMain ? i : (i == oMain+1 ? oEar : oVul);
      j = rgRules[xSub ? inhouse[k] : SFromZ(planet[k])];
      if (j == oEar)
        j = oMain+1;
      else if (j == oVul)
        j = oMain+2;
      oDis[i] = (FBetween(j, 1, oNum) ? j : 1);
      dLev[i] = 1;
    }

    /* Determine the final dispositors (including mutual reception loops). */

    do {
      j = fFalse;
      for (i = 1; i <= oNum; i++)
        cLev[i] = fFalse;
      for (i = 1; i <= oNum; i++)
        if (dLev[i])
          cLev[oDis[i]] = fTrue;
      for (i = 1; i <= oNum; i++)     /* A planet isn't a final dispositor */
        if (dLev[i] && !cLev[i]) {    /* if nobody is pointing to it.      */
          dLev[i] = 0;
          j = fTrue;
        }
    } while (j);

    /* Determine the level of each planet, i.e. how many times you have to */
    /* jump to your dispositor before reaching a final, with finals == 1.  */

    do {
      j = fFalse;
      for (i = 1; i <= oNum; i++)
        if (!dLev[i]) {
          if (!dLev[oDis[i]])
            j = fTrue;
          else                              /* If my dispositor already has */
            dLev[i] = dLev[oDis[i]] + 1;    /* a level, mine is one more.   */
        }
    } while (j);

    /* Count the number of planets at each dispositor level. */

    for (i = 1; i <= oNum; i++)
      cLev[i] = 0;
    for (i = 1; i <= oNum; i++)
      cLev[dLev[i]]++;

    /* Count the number of levels total, and max planets on any one level. */

    xLev = yLev = 0;
    for (i = 1; i <= oNum; i++)
      if (cLev[i]) {
        yLev = i;
        if (cLev[i] > xLev)
          xLev = cLev[i];
      }

    /* Loop over our two dispositor display formats: hierarchy and wheel. */

    for (ySub = 0; ySub <= 1; ySub++) {
      cy = ySub * cy0 + cy0 / 2;
      sprintf(sz, "%s dispositor %s", xSub ? "House" : "Sign",
        ySub ? "wheel" : "hierarchy");
      DrawColor(gi.kiLite);
      DrawSz(sz, cx, ySub * cy0 + 3*gi.nScaleT, dtTop);

      if (ySub) {

        /* Draw a graph in wheel format. */

        for (i = 1; i <= oNum; i++) {
          k = i <= oMain ? i : (i == oMain+1 ? oEar : oVul);
          DrawObject(k, cx + (int)xCirc[i], cy + (int)yCirc[i]);
          j = oDis[i];
          if (j != i) {
            if (dLev[i] < 2)
              DrawColor(gi.kiOn);
            else
              DrawColor(kObjB[k]);
            DrawArrow(cx + (int)xCirc[i], cy + (int)yCirc[i],
              cx + (int)xCirc[j], cy + (int)yCirc[j]);
          }
          if (!gs.fAlt && (j == i || dLev[i] < 2)) {
            DrawColor(j == i ? gi.kiOn : gi.kiGray);
            DrawCircle(cx + (int)xCirc[i], cy + (int)yCirc[i],
              7*gi.nScale, 7*gi.nScale);
          }
        }
      } else {

        /* For level hierarchies, first figure out the screen coordinates    */
        /* for each planet, based on its level, total levels, and max width. */

        for (i = 1; i <= oNum; i++) {
          yo[i] = cy0*(dLev[i]*2-1)/(yLev*2);
          k = 0;
          for (j = 1; j < i; j++)
            if (dLev[i] == dLev[j])
              k = j;
          if (k)
            xo[i] = xo[k] + cx0/xLev;    /* One right of last one on level. */
          else
            xo[i] = cx - ((cx0/xLev)*(cLev[dLev[i]]-1)/2);
        }

        /* Draw graph in level hierarchy format. */

        for (i = 1; i <= oNum; i++) {
          k = i <= oMain ? i : (i == oMain+1 ? oEar : oVul);
          DrawObject(k, xo[i], yo[i]);
          j = oDis[i];
          if (j != i) {
            if (dLev[i] < 2) {
              if (NAbs(xo[i] - xo[j]) < cx0/xLev*3/2) {
                DrawColor(gi.kiOn);
                DrawArrow(xo[i], yo[i], xo[j], yo[j]);
              }
              DrawColor(gi.kiGray);
            } else {
              DrawColor(kObjB[k]);
              DrawArrow(xo[i], yo[i], xo[j], yo[j]);
            }
          } else
            DrawColor(gi.kiOn);
          if (!gs.fAlt && dLev[i] < 2)
            DrawCircle(xo[i], yo[i], 7*gi.nScale, 7*gi.nScale);
        }
      }
    }
  }

  /* Draw boundary lines between the four separate dispositor graphs. */

  if (gs.fBorder) {
    DrawColor(gi.kiLite);
    DrawBlock(cx0, 0, cx0, gs.yWin-1);
    DrawBlock(0, cy0, gs.xWin-1, cy0);
  }
}


/* Draw a chart showing a graphical ephemeris of Ray influences for the   */
/* given month or year, with the date on the vertical axis and each Ray   */
/* on the horizontal, as done when the -7 is combined with the -X switch. */

void XChartEsoteric()
{
  real rRay[cRay+2], rRaySav[cRay+2], power1[objMax], power2[objMax],
    power[oNorm+1];
  char sz[cchSzDef];
  int yea, daytot, d = 1, day, mon, monsiz,
    x1, y1, x2, y2, xs, ys, m, n, u, v = 0, i, j, k;

  EnsureRay();
  yea = us.nEphemYears;    /* Doing an entire year or just a month? */
  if (yea) {
    daytot = DayInYear(Yea);
    day = 1; mon = 1; monsiz = 31;
  } else
    daytot = DayInMonth(Mon, Yea);
  x1 = (yea ? 30 : 24) * gi.nScaleText * gi.nScaleT; y1 = 12 * gi.nScaleText;
  x2 = gs.xWin - x1; y2 = gs.yWin - y1;
  xs = x2 - x1; ys = y2 - y1;

  /* Label Rays along the top axis. */

  for (i = 1; i <= cRay+1; i++) {
    m = x1 + NMultDiv(xs, i-1, cRay+1);
    DrawColor(gi.kiGray);
    DrawDash(m, y1, m, y2, 2);
    if (i <= cRay)
      sprintf(sz, "Ray %d", i);
    else
      sprintf(sz, "Average");
    DrawColor(i <= cRay ? kRayB[i] : gi.kiOn);
    DrawSz(sz, x1 + xs*(i-1)/8, y1 - 3*gi.nScaleText,
      dtLeft | dtBottom | dtScale2);
  }

  /* Loop and display Ray influences for one day segment. */

  while (d <= daytot + 1) {
    n = v;
    if (gs.fLabel && yea && mon == Mon && day == 1) {
      v = y1 + NMultDiv(ys, d-2+Day, daytot);
      DrawColor(kMainA[6]);
      DrawLine(x1, v, x2, v);       /* Marker line for specific day. */
    }
    v = y1 + NMultDiv(ys, d-1, daytot);
    if (!yea || day == 1) {
      DrawColor(gi.kiGray);
      DrawDash(x1, v, x2, v, 1);    /* Marker line for day or month. */
    }
    if (d > 1)
      for (i = 1; i <= cRay+1; i++)
        rRaySav[i] = rRay[i];
    ciCore = ciMain;
    if (yea) {
      MM = mon; DD = day;
    } else {
      MM = Mon; DD = d;
    }
    CastChart(fTrue);

    /* Compute Ray influences for current day. */

    for (i = 0; i <= cRay+1; i++)
      rRay[i] = 0.0;
    ComputeInfluence(power1, power2);
    for (i = 0; i <= oNorm; i++) {
      power[i] = power1[i] + power2[i];
      if (FIgnore(i))
        continue;
      k = SFromZ(planet[i]);
      for (j = 1; j <= cRay; j++)
        if (rgSignRay2[k][j]) {
          if (!gs.fAlt)
            rRay[j] += power[i];
          else
            rRay[j] += power[i] / (420 / rgSignRay2[k][j]);
        }
    }
    for (i = 0; i <= cRay; i++)
      rRay[cRay+1] += rRay[i] / 7.0;

    /* Draw a line segment for each Ray during this time section. */

    if (d > 1)
      for (i = 1; i <= cRay+1; i++) {
        k = x1 + (i-1)*xs/8;
        m = k + (int)((real)xs * rRaySav[i] / 8.0 / (real)gs.nRayWidth);
        u = k + (int)((real)xs * rRay[i]    / 8.0 / (real)gs.nRayWidth);
        DrawColor(i <= cRay ? kRayB[i] : gi.kiOn);
        DrawLine(m, n, u, v);
      }

    /* Label months or days in the month along the left and right edges. */

    if (d <= daytot && (!yea || day == 1)) {
      if (yea) {
        sprintf(sz, "%.3s", szMonth[mon]);
        i = (gs.fLabel && mon == Mon);
      } else {
        sprintf(sz, "%2d", d);
        i = (gs.fLabel && d == Day);
      }
      DrawColor(i ? gi.kiOn : gi.kiLite);
      i = v + gi.nScaleT;
      DrawSz(sz,      xFontT,              i, dtLeft | dtTop | dtScale2);
      DrawSz(sz, x2 + xFontT - gi.nScaleT, i, dtLeft | dtTop | dtScale2);
    }

    /* Now increment the day counter. For a month we always go up by one. */
    /* For a year we go up by four or until the end of the month reached. */

    if (yea) {
      i = us.fSeconds ? 1 : 4;
      day += i;
      if (day > monsiz) {
        d += i-(day-monsiz-1);
        if (d <= daytot + 1) {
          mon++;
          monsiz = DayInMonth(mon, Yea);
          day = 1;
        }
      } else
        d += i;
    } else
      d++;
  }
  DrawColor(gi.kiLite);
  DrawEdge(x1, y1, x2, y2);

  ciCore = ciMain;    /* Recast original chart. */
  CastChart(fTrue);
}


/* Draw a graphical calendar for a given month, with numbers in boxes,  */
/* scaled to fit within the given bounds. This is used for single month */
/* -K switch images and is called 12 times for a full year -Ky image.   */

void DrawCalendar(int mon, int X1, int Y1, int X2, int Y2)
{
  char sz[cchSzDef];
  int day, cday, dayHi, cweek, xunit, yunit, xs, ys, x1, y1, x, y, s;

  xs = X2 - X1; ys = Y2 - Y1;
  day = DayOfWeek(mon, 1, Yea);    /* Day of week of 1st of month.     */
  cday = DaysInMonth(mon, Yea);    /* Count of days in the month.      */
  dayHi = DayInMonth(mon, Yea);    /* Number of last day in the month. */
  cweek = us.fCalendarYear ? 6 : (day + cday + 6) / 7;   /* Week rows. */
  xunit = xs/8;                    /* Hor. pixel size of each day box. */
  yunit = ys/(cweek+2);            /* Ver. pixel size of each day box. */
  x1 = X1 + (xs - xunit*7) / 2;    /* Blank space to left of calendar. */
  y1 = Y1 + yunit*3/2;             /* Blank space to top of calendar.  */

  /* Print the month and year in big letters at top of chart. */

  DrawColor(gi.kiOn);
  sprintf(sz, "%s, %d", szMonth[mon], Yea);
  s = gi.nScale;
  gi.nScale = Min((yunit*3/2-yFont*s) / yFont, xs/9/*CchSz(sz)*/ / xFont);
  gi.nScale = Max(gi.nScale-1, 1);
  DrawSz(sz, X1 + xs/2, Y1 + (yunit*3/2-yFont*s)/2, dtCent | dtScale);
  gi.nScale = s;

  /* Draw the grid of boxes for the days. */

  for (x = 0; x <= cWeek; x++) {

    /* Print days of week at top of each column (abbreviated if need be). */

    if (x < cWeek) {
      if (xunit / (xFont*gi.nScale) < 9)
        sprintf(sz, "%.3s", szDay[x]);
      else
        sprintf(sz, "%s", szDay[x]);
      DrawColor(kRainbowB[3]);
      DrawSz(sz, x1 + x*xunit + xunit/2, y1 - s*3, dtBottom | dtScale);
      DrawColor(kRainbowB[5]);
    }
    DrawLine(x1 + x*xunit, y1, x1 + x*xunit, y1 + cweek*yunit);
  }
  for (y = 0; y <= cweek; y++)
    DrawLine(x1, y1 + y*yunit, x1 + 7*xunit, y1 + y*yunit);

  /* Actually draw the day numbers in their appropriate boxes. */

  x = day; y = 0;
  for (day = 1; day <= dayHi; day = AddDay(mon, day, Yea, 1)) {
    sprintf(sz, gs.fText ? "%2d" : "%d", day);
    DrawColor(day == Day && mon == Mon && gs.fLabel ? kRainbowB[4] :
      (x <= 0 || x >= cWeek-1 ? kRainbowB[1] : gi.kiLite));
    if (!gs.fAlt)
      DrawSz(sz, x1 + x*xunit + s*2, y1 + y*yunit + s*4,
        dtLeft | dtTop | dtScale);
    else
      DrawSz(sz, x1 + x*xunit + xunit/2,
        y1 + y*yunit + yunit/2 + gi.nScale, dtCent | dtScale);
    if (++x >= cWeek) {
      x = 0;
      y++;
    }
  }
}


/* Draw a graphical calendar on the screen for the chart month or entire */
/* year, as done when the -K or -Ky is combined with the -X switch.      */

void XChartCalendar()
{
  int xs, ys, xunit, yunit, x1, y1, x, y;

  if (!us.fCalendarYear) {
    DrawCalendar(Mon, 0, 0, gs.xWin, gs.yWin);
    return;
  }

  /* Determine the best sized rectangle of months to draw the year in based */
  /* on the chart dimensions: Either do 6x2 months, or 4x3, 3x4, or 2x6.    */

  if (gs.xWin > gs.yWin) {
    if (gs.xWin > gs.yWin * 3) {
      xs = 6; ys = 2;
    } else {
      xs = 4; ys = 3;
    }
  } else {
    if (gs.yWin > gs.xWin * 2) {
      xs = 2; ys = 6;
    } else {
      xs = 3; ys = 4;
    }
  }
  xunit = gs.xWin / xs; yunit = gs.yWin / ys;
  x1 = (gs.xWin - xunit*xs) / 2;
  y1 = (gs.yWin - yunit*ys) / 2;
  for (y = 0; y < ys; y++)
    for (x = 0; x < xs; x++) {
      DrawCalendar(y * xs + x + 1, x1 + x*xunit, y1 + y*yunit,
        x1 + (x+1)*xunit, y1 + (y+1)*yunit);
    }
}


/* Translate to chart pixel coordinates, that indicate where to draw on a */
/* chart sphere, for the -XX switch chart. Inputs may be local horizon    */
/* altitude and azimuth coordinates, local horizon prime vertical, local  */
/* horizon meridian, or zodiac position and latitude.                     */

flag FSphereLocal(real azi, real alt, int xc, int yc, int xr, int yr,
  int *xp, int *yp)
{
  azi = Mod(rDegQuad*3 - (azi + (real)gs.nRot));
  if (gs.rTilt != 0.0)
    CoorXform(&azi, &alt, gs.rTilt);
  *xp = xc + (int)((real)xr * RCosD(azi) * RCosD(alt) - rRound);
  *yp = yc + (int)((real)yr * RSinD(alt) - rRound);
  return azi >= rDegHalf;
}

flag FSpherePrime(real azi, real alt, int xc, int yc, int xr, int yr,
  int *xp, int *yp)
{
  CoorXform(&azi, &alt, rDegQuad);
  return FSphereLocal(azi + rDegQuad, alt, xc, yc, xr, yr, xp, yp);
}

flag FSphereMeridian(real azi, real alt, int xc, int yc, int xr, int yr,
  int *xp, int *yp)
{
  azi = Mod(azi + rDegQuad);
  CoorXform(&azi, &alt, rDegQuad);
  return FSphereLocal(azi, alt, xc, yc, xr, yr, xp, yp);
}

flag FSphereZodiac(real lon, real lat, real lonMC, real latMC, int xc, int yc,
  int xr, int yr, int *xp, int *yp)
{
  real lonT, latT;

  lonT = Tropical(lon); latT = lat;
  EclToEqu(&lonT, &latT);
  lonT = Mod(lonMC - lonT + rDegQuad);
  EquToLocal(&lonT, &latT, rDegQuad - latMC);
  return FSphereLocal(lonT + rDegQuad, -latT, xc, yc, xr, yr, xp, yp);
}


/* Draw a chart sphere (like a chart wheel but in 3D) as done with the -XX */
/* switch. This is similar to Astrolog's local horizon charts.             */

void XChartSphere()
{
  char sz[cchSzDef];
  int rgx[objMax], rgy[objMax], rgxg[objMax], rgyg[objMax],
    zGlyph, zGlyph2, xc, yc, xr, yr, xo = 0, yo = 0, xp, yp, i, j, k, k2, nSav;
  flag rgf[objMax], fDir = !gs.fSouth, fAny = !gs.fAlt, f;
  real lonMC, latMC;

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;

  /* Avoid default alignments of sphere that don't look as good. */
  if (us.fSmartSave && gs.nRot == 0 && gs.rTilt == 0.0) {
    gs.nRot = 7;
    gs.rTilt = -7.0;
  }

  /* Initialize variables. */
  zGlyph = 7*gi.nScale; zGlyph2 = 14*gi.nScale;
  xc = gs.xWin >> 1; yc = gs.yWin >> 1;
  xr = xc - zGlyph; yr = yc - zGlyph;

  lonMC = Tropical(is.MC); latMC = 0.0;
  EclToEqu(&lonMC, &latMC);
  latMC = Lat;

  /* Draw horizon. */
  if (!gs.fColorHouse)
    DrawColor(gi.kiOn);
  for (i = 0; i <= nDegMax; i++) {
    if (gs.fColorHouse && (i == 0 || i == nDegHalf))
      DrawColor(kSignB(i ? sLib : sAri));
    f = FSphereLocal((real)i, 0.0, xc, yc, xr, yr, &xp, &yp) ^ fDir;
    if (f && i > 0) {
      DrawLine(xo, yo, xp, yp);
      k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
      for (j = -k; j <= k; j += (k << 1)) {
        FSphereLocal((real)i, (real)j / 2.0, xc, yc, xr, yr, &xo, &yo);
        DrawLine(xo, yo, xp, yp);
      }
    } else if (fAny)
      DrawPoint(xp, yp);
    xo = xp; yo = yp;
  }

  /* Draw prime vertical. */
  if (!gs.fColorHouse)
    DrawColor(gi.kiGray);
  for (i = 0; i <= nDegMax; i++) {
    if (gs.fColorHouse)
      DrawColor(kSignB((i-1)/30 + 1));
    f = FSpherePrime((real)i, 0.0, xc, yc, xr, yr, &xp, &yp) ^ fDir;
    if (f && i > 0) {
      DrawLine(xo, yo, xp, yp);
      k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
      for (j = -k; j <= k; j += (k << 1)) {
        FSpherePrime((real)i, (real)j / 2.0, xc, yc, xr, yr, &xo, &yo);
        DrawLine(xo, yo, xp, yp);
      }
    } else if (fAny && !FOdd(i))
      DrawPoint(xp, yp);
    xo = xp; yo = yp;
  }

  /* Draw house wedges and meridian. */
  if (!gs.fColorHouse)
    DrawColor(kMainB[5]);
  for (j = 30; j < nDegMax; j += (j != 150 ? 30 : 60)) {
    if (us.fHouse3D && !(j == 90 || j == 270))
      continue;
    if (gs.fColorHouse)
      DrawColor(kSignB(j/30 + 1));
    for (i = -90; i <= 90; i++) {
      f = FSpherePrime((real)j, (real)i, xc, yc, xr, yr, &xp, &yp) ^ fDir;
      if (f && i > -90) {
        DrawLine(xo, yo, xp, yp);
        if ((j == 90 || j == 270) && i % 30 != 0) {
          k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
          for (k2 = -k; k2 <= k; k2 += (k << 1)) {
            FSphereMeridian((real)(j > 180 ? i+180 : 360-i), (real)k2 / 2.0,
              xc, yc, xr, yr, &xo, &yo);
            DrawLine(xo, yo, xp, yp);
          }
        }
      } else if (fAny && !FOdd(i))
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
  }

  /* Draw sign wedges. */
  if (!us.fVedic) {
    if (!gs.fColorSign)
      DrawColor(kMainB[7]);
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorSign)
        DrawColor(kSignB((i-1)/30 + 1));
      f = FSphereZodiac((real)i, 0.0,
        lonMC, latMC, xc, yc, xr, yr, &xp, &yp) ^ fDir;
      if (f && i > 0) {
        DrawLine(xo, yo, xp, yp);
        if (i % 30 != 0) {
          k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
          for (j = -k; j <= k; j += (k << 1)) {
            FSphereZodiac((real)i, (real)j / 2.0,
              lonMC, latMC, xc, yc, xr, yr, &xo, &yo);
            DrawLine(xo, yo, xp, yp);
          }
        }
      } else if (fAny)
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign)
        DrawColor(kSignB(i/30 + 1));
      for (j = -90; j <= 90; j++) {
        f = FSphereZodiac((real)i, (real)j,
          lonMC, latMC, xc, yc, xr, yr, &xp, &yp) ^ fDir;
        if (f && j > -90)
          DrawLine(xo, yo, xp, yp);
        else if (fAny && !FOdd(i))
          DrawPoint(xp, yp);
        xo = xp; yo = yp;
      }
    }
  }

  /* Draw outer boundary. */
  DrawColor(gi.kiOn);
  DrawCircle(xc, yc, xr, yr);

  /* Label signs. */
  if (!us.fVedic) {
    nSav = gi.nScale;
    gi.nScale = gi.nScaleText * gi.nScaleT;
    for (j = -80; j <= 80; j += 160)
      for (i = 1; i <= cSign; i++) {
        f = FSphereZodiac((real)(i*30-15), (real)j,
          lonMC, latMC, xc, yc, xr, yr, &xp, &yp) ^ fDir;
        if (f || fAny) {
          DrawColor(f ? (gs.fColorSign ? kSignB(i) : kMainB[7]) : gi.kiGray);
          DrawSign(i, xp, yp);
        }
      }
    gi.nScale = nSav;
  }

  /* Label houses. */
  if (!us.fHouse3D) {
    nSav = gi.nScale;
    gi.nScale = gi.nScaleText * gi.nScaleT;
    for (j = -82; j <= 82; j += 164)
      for (i = 1; i <= cSign; i++) {
        f = FSpherePrime((real)(i*30-15), (real)j,
          xc, yc, xr, yr, &xp, &yp) ^ fDir;
        if (f || fAny) {
          DrawColor(f ? (gs.fColorHouse ? kSignB(i) : kMainB[5]) : gi.kiGray);
          DrawHouse(i, xp, yp);
        }
      }
    gi.nScale = nSav;
  }

  /* Label directions. */
  k = zGlyph >> 1;
  for (i = 0; i < nDegMax; i += 90) {
    f = FSphereLocal((real)i, 0.0, xc, yc, xr, yr, &xp, &yp) ^ fDir;
    if (f || fAny) {
      j = i / 90;
      DrawColor(kObjB[oAsc + ((j + 3) & 3)*3]);
      DrawDash(xc, yc, xp, yp, f ? 0 : 2);
      if (gs.fColorHouse)
        DrawColor(f ? gi.kiOn : gi.kiGray);
      FSphereLocal((real)i, 0.0, xc, yc, xr + k, yr + k, &xp, &yp);
      sprintf(sz, "%c", szDir[j][0]);
      DrawSz(sz, xp, yp + gi.nScale, dtCent);
    }
  }
  for (j = -90; j <= 90; j += nDegHalf) {
    f = FSphereLocal(0.0, (real)j, xc, yc, xr + k, yr + k, &xp, &yp) ^ fDir;
    if (f || fAny) {
      DrawColor(gs.fColorHouse ? (f ? gi.kiOn : gi.kiGray) :
        kObjB[j <= 0 ? oMC : oNad]);
      sprintf(sz, "%c", j <= 0 ? 'Z' : 'N');
      DrawSz(sz, xp, yp + gi.nScale, dtCent);
    }
  }

  /* Draw center point. */
  DrawColor(gi.kiOn);
  DrawSpot(xc, yc);

  /* Draw planets. */
  for (i = cObj; i >= 0; i--) if (FProper(i)) {
    f = FSphereZodiac(planet[i], planetalt[i],
      lonMC, latMC, xc, yc, xr, yr, &xp, &yp) ^ fDir;
    rgx[i] = xp; rgy[i] = yp; rgf[i] = f;
    rgxg[i] = xp; rgyg[i] = yp + zGlyph;
  }

  /* As in the DrawGlobe() routine, we now determine where to draw the   */
  /* glyphs in relation to the actual points, so that the glyphs aren't  */
  /* drawn on top of each other if possible. Again, we assume that we'll */
  /* put the glyph right under the point, unless there would be some     */
  /* overlap and the above position is better off.                       */
  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    k = k2 = gs.xWin+gs.yWin;
    for (j = 0; j < i; j++) if (FProper(j)) {
      k = Min(k, NAbs(rgxg[i]-rgxg[j]) + NAbs(rgyg[i]-rgyg[j]));
      k2 = Min(k2, NAbs(rgxg[i]-rgxg[j]) + NAbs(rgyg[i]-zGlyph2-rgyg[j]));
    }
    if ((k < zGlyph2 || k2 < zGlyph2) && k < k2)
      rgyg[i] -= zGlyph2;
  }

  /* Draw lines connecting planets which have aspects between them. */
  if (!FCreateGrid(fFalse))
    return;
  for (j = cObj; j >= 1; j--)
    for (i = j-1; i >= 0; i--)
      if (grid->n[i][j] && FProper(i) && FProper(j) &&
        (fAny || (rgf[i] && rgf[j]))) {
        DrawColor(rgf[i] && rgf[j] ? kAspB[grid->n[i][j]] : gi.kiGray);
        DrawDash(rgx[i], rgy[i], rgx[j], rgy[j],
          NAbs(grid->v[i][j]/(60*60*2)) + (!rgf[i] + !rgf[j])*2);
      }

  /* Draw planet glyphs, and spots for actual local location. */
  for (i = cObj; i >= 0; i--) if (FProper(i)) {
    if (fAny || rgf[i]) {
      if (!rgf[i]) {
        nSav = kObjB[i]; kObjB[i] = gi.kiGray;
      }
      DrawObject(i, rgxg[i], rgyg[i]);
      if (!rgf[i])
        kObjB[i] = nSav;
    }
  }
  for (i = cObj; i >= 0; i--) if (FProper(i)) {
    if (fAny || rgf[i]) {
      DrawColor(rgf[i] ? kObjB[i] : gi.kiGray);
      DrawSpot(rgx[i], rgy[i]);
    }
  }

  DrawSidebar();
}
#endif /* GRAPH */

/* xcharts1.cpp */
