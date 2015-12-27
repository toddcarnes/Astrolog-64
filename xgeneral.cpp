/*
** Astrolog (Version 6.00) File: xgeneral.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2015 by
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
** Last code change made 12/20/2015.
*/

#include "astrolog.h"


#ifdef GRAPH
/*
******************************************************************************
** Core Graphic Procedures.
******************************************************************************
*/

/* Set the current color to use in drawing on the screen or bitmap array. */

void DrawColor(KI col)
{
#ifdef WIN
  HPEN hpenT;
#endif
#ifdef MACG
  RGBColor kv;
#endif

  if (gi.fFile) {
#ifdef PS
    if (gs.fPS) {
      if (gi.kiCur != col) {
        PsStrokeForce();      /* Render existing path with current color */
        fprintf(gi.file, "%.2f %.2f %.2f c\n",
          (real)RGBR(rgbbmp[col])/255.0, (real)RGBG(rgbbmp[col])/255.0,
          (real)RGBB(rgbbmp[col])/255.0);
      }
    }
#endif
#ifdef META
    if (gs.fMeta)
      gi.kiLineDes = col;
#endif
  }
#ifdef X11
  else
    XSetForeground(gi.disp, gi.gc, rgbind[col]);
#endif
#ifdef WIN
  else {
    if (gi.kiCur != col) {
      hpenT = wi.hpen;
      wi.hpen = CreatePen(PS_SOLID, gi.nScaleT, (COLORREF)rgbbmp[col]);
      SelectObject(wi.hdc, wi.hpen);
      if (hpenT != (HPEN)NULL)
        DeleteObject(hpenT);
    }
  }
#endif
#ifdef MSG
  else
    _setcolor(col);
#endif
#ifdef BGI
  else
    setcolor(col);
#endif
#ifdef MACG
  else {
    kv.red   = RGBR(rgbbmp[col]) << 8;
    kv.green = RGBG(rgbbmp[col]) << 8;
    kv.blue  = RGBB(rgbbmp[col]) << 8;
    RGBForeColor(&kv);
    RGBBackColor(&kv);
  }
#endif
  gi.kiCur = col;
}


/* Set a single point on the screen. This is the most basic graphic function */
/* and is called by all the more complex routines. Based on what mode we are */
/* in, we either set a cell in the bitmap array or a pixel on the window.    */

void DrawPoint(int x, int y)
{
  if (gi.fFile) {
    if (gs.fBitmap) {
      /* Force the coordinates to be within the bounds of the bitmap array. */

      if (x < 0)
        x = 0;
      else if (x >= gs.xWin)
        x = gs.xWin-1;
      if (y < 0)
        y = 0;
      else if (y >= gs.yWin)
        y = gs.yWin-1;
      if (gi.yBand) {
        y -= gi.yOffset;
        if (y < 0 || y >= gi.yBand)
          return;
      }
      BmSet(gi.bm, x, y, gi.kiCur);
    }
#ifdef PS
    else if (gs.fPS) {
      DrawColor(gi.kiCur);
      PsLineCap(fTrue);
      fprintf(gi.file, "%d %d d\n", x, y);
      PsStroke(2);
    }
#endif
#ifdef META
    else {
      gi.kiFillDes = gi.kiCur;
      MetaSelect();
      MetaEllipse(x-gi.nPenWid/2, y-gi.nPenWid/2,
        x+gi.nPenWid/2, y+gi.nPenWid/2);
    }
#endif
  }
#ifdef X11
  else
    XDrawPoint(gi.disp, gi.pmap, gi.gc, x, y);
#endif
#ifdef WIN
  else {
    if (wi.hdcPrint == hdcNil)
      SetPixel(wi.hdc, x, y, (COLORREF)rgbbmp[gi.kiCur]);
    else {
      MoveTo(wi.hdc, x, y);
      LineTo(wi.hdc, x, y);
    }
  }
#endif
#ifdef MSG
  else
    _setpixel(gi.xOffset + x, gi.yOffset + y);
#endif
#ifdef BGI
  else
    putpixel(gi.xOffset + x, gi.yOffset + y, gi.kiCur);
#endif
#ifdef MACG
  else {
    MoveTo(x, y); LineTo(x, y);
  }
#endif
}


/* Draw dot a little larger than just a single pixel at specified location. */

void DrawSpot(int x, int y)
{
#ifdef PS
  if (gs.fPS) {
    PsLineWidth((int)(gi.rLineWid*3.0));
    DrawPoint(x, y);
    PsLineWidth((int)(gi.rLineWid/3.0));
    return;
  }
#endif
#ifdef META
  if (gs.fMeta) {
    gi.kiFillDes = gi.kiCur;
    MetaSelect();
    MetaEllipse(x-gi.nPenWid, y-gi.nPenWid, x+gi.nPenWid, y+gi.nPenWid);
    return;
  }
#endif
  DrawPoint(x, y);
  DrawPoint(x, y-1);
  DrawPoint(x-1, y);
  DrawPoint(x+1, y);
  DrawPoint(x, y+1);
}


/* Draw a filled in block, defined by the corners of its rectangle. */

void DrawBlock(int x1, int y1, int x2, int y2)
{
  int x, y;
#ifdef MACG
  Rect rc;
#endif

  if (gi.fFile) {
    if (gs.fBitmap) {
      /* Force the coordinates to be within the bounds of the bitmap band. */

      if (x1 < 0)
        x1 = 0;
      if (x2 >= gs.xWin)
        x2 = gs.xWin-1;
      if (gi.yBand) {
        y1 -= gi.yOffset;
        if (y1 < 0)
          y1 = 0;
        y2 -= gi.yOffset;
        if (y2 >= gi.yBand)
          y2 = gi.yBand-1;
      }
      for (y = y1; y <= y2; y++)           /* For bitmap, we have to  */
        for (x = x1; x <= x2; x++)         /* just fill in the array. */
          BmSet(gi.bm, x, y, gi.kiCur);
    }
#ifdef PS
    else if (gs.fPS) {
      DrawColor(gi.kiCur);
      fprintf(gi.file, "%d %d %d %d rf\n",
        Max(x1-gi.nPenWid/4, 0), Max(y1-gi.nPenWid/4, 0),
        x2-x1+gi.nPenWid/4, y2-y1+gi.nPenWid/4);
    }
#endif
#ifdef META
    else {
      gi.kiFillDes = gi.kiCur;
      MetaSelect();
      MetaRectangle(x1-gi.nPenWid/2, y1-gi.nPenWid/2,
        x2+gi.nPenWid/2, y2+gi.nPenWid/2);
    }
#endif
  }
#ifdef X11
  else
    XFillRectangle(gi.disp, gi.pmap, gi.gc, x1, y1, x2-x1, y2-y1);
#endif
#ifdef WIN
  else {
    wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[gi.kiCur]);
    SelectObject(wi.hdc, wi.hbrush);
    PatBlt(wi.hdc, x1, y1, x2-x1 + 1, y2-y1 + 1, PATCOPY);
    SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
    DeleteObject(wi.hbrush);
  }
#endif
#ifdef MSG
  else
    _rectangle(_GFILLINTERIOR,
      gi.xOffset + x1, gi.yOffset + y1, gi.xOffset + x2, gi.yOffset + y2);
#endif
#ifdef BGI
  else {
    setfillstyle(SOLID_FILL, gi.kiCur);
    bar(gi.xOffset + x1, gi.yOffset + y1, gi.xOffset + x2, gi.yOffset + y2);
  }
#endif
#ifdef MACG
  else {
    SetRect(&rc, x1, y1, x2+1, y2+1);
    EraseRect(&rc);
  }
#endif
}


/* Draw a rectangle on the screen with specified thickness. This is just   */
/* like DrawBlock() except that we are only drawing the edges of the area. */

void DrawBox(int x1, int y1, int x2, int y2, int xsiz, int ysiz)
{
#ifdef META
  if (gs.fMeta)
    /* For thin boxes in metafiles, we can just output one rectangle record */
    /* instead of drawing each side separately as we have to do otherwise.  */
    if (xsiz <= 1 && ysiz <= 1) {
      gi.kiFillDes = kNull;          /* Specify a hollow fill brush. */
      MetaSelect();
      MetaRectangle(x1, y1, x2, y2);
      return;
    }
#endif
  DrawBlock(x1, y1, x2, y1 + ysiz - 1);
  DrawBlock(x1, y1 + ysiz, x1 + xsiz - 1, y2 - ysiz);
  DrawBlock(x2 - xsiz + 1, y1 + ysiz, x2, y2 - ysiz);
  DrawBlock(x1, y2 - ysiz + 1, x2, y2);
}


/* Clear and erase the graphics screen or bitmap contents. */

void DrawClearScreen()
{
#ifdef PS
  if (gs.fPS) {
    /* For PostScript charts first output page orientation information. */
    if (!gi.fEps) {
      if (gs.nOrient == 0)
        gs.nOrient = gs.xWin > gs.yWin ? -1 : 1;
      if (gs.nOrient < 0) {
        /* chartx and charty are reversed for Landscape mode. */
        fprintf(gi.file, "%d %d translate\n",
          ((int)(gs.xInch*72.0+rRound) + gs.yWin)/2,
          ((int)(gs.yInch*72.0+rRound) + gs.xWin)/2);
        fprintf(gi.file, "-90 rotate\n");
      } else {
        /* Most charts are in Portrait mode */
        fprintf(gi.file, "%d %d translate\n",
          ((int)(gs.xInch*72.0+rRound) - gs.xWin)/2,
          ((int)(gs.yInch*72.0+rRound) + gs.yWin)/2);
      }
    } else
      fprintf(gi.file, "0 %d translate\n", gs.yWin);
    fprintf(gi.file, "1 -1 scale\n");
    gs.nScale *= PSMUL; gs.xWin *= PSMUL; gs.yWin *= PSMUL; gi.nScale *= PSMUL;
    fprintf(gi.file, "1 %d div dup scale\n", PSMUL);
  }
#endif
#ifdef META
  if (gs.fMeta)
    MetaInit();    /* For metafiles first go write our header information. */
#endif

  /* Don't actually erase the screen if the -Xj switch is in effect. */
  if (gs.fJetTrail)
    return;

#ifdef MSG
  if (!gi.fFile)
    _clearscreen(_GCLEARSCREEN);
#endif
#ifdef BGI
  if (!gi.fFile)
    clearviewport();
#endif
  DrawColor(gi.kiOff);
  DrawBlock(0, 0, gs.xWin - 1, gs.yWin - 1);    /* Clear bitmap screen. */
}


/* Draw a line on the screen, specified by its endpoints. In addition, we */
/* have specified a skip factor, which allows us to draw dashed lines.    */

void DrawDash(int x1, int y1, int x2, int y2, int skip)
{
  int x = x1, y = y1, xadd, yadd, yinc, xabs, yabs, i, j = 0;

  if (skip < 0)
    skip = 0;
#ifdef ISG
  if (!gi.fFile) {
    if (!skip) {
#ifdef X11
      /* For non-dashed X window lines, let's have the Xlib do it for us. */

      XDrawLine(gi.disp, gi.pmap, gi.gc, x1, y1, x2, y2);
#endif
#ifdef WIN
      /* For Windows lines, we have to manually draw the last pixel. */

      MoveTo(wi.hdc, x1, y1);
      LineTo(wi.hdc, x2, y2);
      SetPixel(wi.hdc, x2, y2, (COLORREF)rgbbmp[gi.kiCur]);
#endif
#ifdef PCG
      /* For non-dashed lines, let's have the graphics library do it for us. */

      PcMoveTo(gi.xOffset + x1, gi.yOffset + y1);
      PcLineTo(gi.xOffset + x2, gi.yOffset + y2);
#endif
#ifdef MACG
      MoveTo(x1, y1);
      LineTo(x2, y2);
#endif
      return;
    }
#ifdef WIN
    if (skip && wi.hdcPrint != hdcNil)
      skip = (skip + 1)*METAMUL - 1;
#endif
  }
#endif /* ISG */

#ifdef PS
  if (gs.fPS) {

    /* For PostScript charts we can save file size if we output a LineTo  */
    /* command when the start vertex is the same as the end vertex of the */
    /* previous line drawn, instead of writing out both vertices.         */

    PsLineCap(fTrue);
    PsDash(skip);
    if (gi.xPen != x1 || gi.yPen != y1)
      fprintf(gi.file, "%d %d %d %d l\n", x1, y1, x2, y2);
    else
      fprintf(gi.file, "%d %d t\n", x2, y2);
    gi.xPen = x2; gi.yPen = y2;
    PsStroke(2);
    return;
  }
#endif

#ifdef META
  if (gs.fMeta) {

    /* For metafile charts we can really save file size for consecutive */
    /* lines sharing endpoints by consolidating them into a PolyLine.   */

    if (gi.xPen != x1 || gi.yPen != y1) {
      if (x1 != x2 || y1 != y2) {
        gi.kiLineDes = (gi.kiLineDes & 15) + 16*(skip > 3 ? 3 : skip);
        MetaSelect();
        gi.pwPoly = gi.pwMetaCur;
        MetaRecord(8, 0x325);      /* Polyline */
        MetaWord(2); MetaWord(x1); MetaWord(y1);
      } else {
        DrawPoint(x1, y1);
        return;
      }
    } else {
      *gi.pwPoly += 2;
      (*(gi.pwPoly+3))++;
      /* Note: We should technically update the max record size in the   */
      /* file header if need be here too, but it doesn't seem necessary. */
    }
    MetaWord(x2); MetaWord(y2);
    gi.xPen = x2; gi.yPen = y2;
    return;
  }
#endif

  /* If none of the above cases hold, we have to draw the line dot by dot. */

  xadd = x2 - x1 >= 0 ? 1 : 3;
  yadd = y2 - y1 >= 0 ? 2 : 4;
  xabs = abs(x2 - x1);
  yabs = abs(y2 - y1);

  /* Technically what we're doing here is drawing a line which is more    */
  /* horizontal then vertical. We always increment x by 1, and increment  */
  /* y whenever a fractional variable passes a certain amount. For lines  */
  /* that are more vertical than horizontal, we just swap x and y coords. */

  if (xabs < yabs) {
    SwapN(xadd, yadd);
    SwapN(xabs, yabs);
  }
  yinc = (xabs >> 1) - ((xabs & 1 ^ 1) && xadd > 2);
  for (i = xabs + 1; i; i--) {
    if (j < 1)
      DrawPoint(x, y);
    j = j < skip ? j+1 : 0;
    switch (xadd) {
    case 1: x++; break;
    case 2: y++; break;
    case 3: x--; break;
    case 4: y--; break;
    }
    yinc += yabs;
    if (yinc - xabs >= 0) {
      yinc -= xabs;
      switch (yadd) {
      case 1: x++; break;
      case 2: y++; break;
      case 3: x--; break;
      case 4: y--; break;
      }
    }
  }
}


/* Draw a normal line on the screen; however, if the x coordinates are close */
/* to either of the two given bounds, then we assume that the line runs off  */
/* one side and reappears on the other, so draw the appropriate two lines    */
/* instead. This is used by the Ley line and astro-graph routines, which     */
/* draw lines running around the world and hence off the edges of the maps.  */

void DrawWrap(int x1, int y1, int x2, int y2, int xmin, int xmax)
{
  int xmid, ymid, i, j, k;

  if (x1 < 0) {           /* Special case for drawing world map. */
    DrawPoint(x2, y2);
    return;
  }
  j = (xmin < 0);    /* Negative xmin means always draw forward. */
  if (j)
    neg(xmin);
  xmid = (xmax-xmin) / 2;
  if (j)
    k = (x1 < x2 ? xmid*7 : xmid)/4;
  else
    k = xmid;

  /* If endpoints aren't near opposite edges, just draw the line and return. */

  if (NAbs(x2-x1) < k) {
    DrawLine(x1, y1, x2, y2);
    return;
  }
  if ((i = (xmax-xmin+1) + (!j && x1 < xmid ? x1-x2 : x2-x1)) == 0)
    i = 1;

  /* Determine vertical coordinate where our line runs off edges of screen. */

  ymid = y1+(int)((real)(y2-y1)*
    (!j && x1 < xmid ? (real)(x1-xmin) : (real)(xmax-x1))/(real)i + rRound);
  DrawLine(x1, y1, !j && x1 < xmid ? xmin : xmax, ymid);
  DrawLine(j || x2 < xmid ? xmin : xmax, ymid, x2, y2);
}


/* This routine, and its companion below, clips a line defined by its  */
/* endpoints to either above some line y=c, or below some line y=c. By */
/* passing in parameters in different orders, we can clip to vertical  */
/* lines, too. These are used by the DrawClip() routine below.         */

void ClipLesser(int *x1, int *y1, int *x2, int *y2, int s)
{
  *x1 -= (int)((long)(*y1-s)*(*x2-*x1)/(*y2-*y1));
  *y1 = s;
}

void ClipGreater(int *x1, int *y1, int *x2, int *y2, int s)
{
  *x1 += (int)((long)(s-*y1)*(*x2-*x1)/(*y2-*y1));
  *y1 = s;
}


/* Draw a line on the screen. This is just like DrawLine() routine earlier; */
/* however, first clip the endpoints to the given viewport before drawing.  */

void DrawClip(int x1, int y1, int x2, int y2, int xl, int yl, int xh, int yh,
  int skip)
{
  if (x1 < xl)
    ClipLesser (&y1, &x1, &y2, &x2, xl);    /* Check left side of window. */
  if (x2 < xl)
    ClipLesser (&y2, &x2, &y1, &x1, xl);
  if (y1 < yl)
    ClipLesser (&x1, &y1, &x2, &y2, yl);    /* Check top side of window.  */
  if (y2 < yl)
    ClipLesser (&x2, &y2, &x1, &y1, yl);
  if (x1 > xh)
    ClipGreater(&y1, &x1, &y2, &x2, xh);    /* Check right of window.  */
  if (x2 > xh)
    ClipGreater(&y2, &x2, &y1, &x1, xh);
  if (y1 > yh)
    ClipGreater(&x1, &y1, &x2, &y2, yh);    /* Check bottom of window. */
  if (y2 > yh)
    ClipGreater(&x2, &y2, &x1, &y1, yh);
  DrawDash(x1, y1, x2, y2, skip);           /* Go draw the line.       */
}


/* Draw a circle or ellipse inside the given bounding rectangle. */

void DrawEllipse(int x1, int y1, int x2, int y2)
{
  int x, y, rx, ry, m, n, u, v, i;
#ifdef MACG
  Rect rc;
#endif

  if (gi.fFile) {
    x = (x1+x2)/2; y = (y1+y2)/2; rx = (x2-x1)/2; ry = (y2-y1)/2;
    if (gs.fBitmap) {
      m = x + rx; n = y;
      for (i = 0; i <= nDegMax; i += DEGINC) {
        u = x + (int)(((real)rx+rRound)*RCosD((real)i));
        v = y + (int)(((real)ry+rRound)*RSinD((real)i));
        DrawLine(m, n, u, v);
        m = u; n = v;
      }
    }
#ifdef PS
    else if (gs.fPS) {
      PsLineCap(fFalse);
      PsStrokeForce();
      PsDash(0);
      fprintf(gi.file, "%d %d %d %d el\n", rx, ry, x, y);
    }
#endif
#ifdef META
    else {
      gi.kiFillDes = kNull;    /* Specify a hollow fill brush. */
      MetaSelect();
      MetaEllipse(x1+gi.nPenWid/3, y1+gi.nPenWid/3,
        x2+1+gi.nPenWid/3, y2+1+gi.nPenWid/3);
    }
#endif
  }
#ifdef X11
  else
    XDrawArc(gi.disp, gi.pmap, gi.gc, x1, y1, x2-x1, y2-y1, 0, nDegMax*64);
#endif
#ifdef WIN
  else
    Ellipse(wi.hdc, x1, y1, x2+1, y2+1);
#endif
#ifdef MSG
  else
    _ellipse(_GBORDER, gi.xOffset + x1, gi.yOffset + y1,
      gi.xOffset + x2, gi.yOffset + y2);
#endif
#ifdef BGI
  else
    ellipse(gi.xOffset + (x1+x2)/2, gi.yOffset + (y1+y2)/2,
      0, 360, (x2-x1)/2, (y2-y1)/2);
#endif
#ifdef MACG
  else {
    SetRect(&rc, x1, y1, x2, y2);
    FrameOval(&rc);
  }
#endif
}


/* Print a string of text on the graphic window at specified location. To  */
/* do this we either use Astrolog's own "font" (6x10) and draw each letter */
/* separately, or else specify system fonts for PostScript and metafiles.  */

void DrawSz(CONST char *sz, int x, int y, int dt)
{
  int s = gi.nScale, c = gi.kiCur, cch, i;

  cch = CchSz(sz);
  if (!(dt & dtScale))
    gi.nScale = gi.nScaleT;
  x += gi.nScale;
  if (!(dt & dtLeft))
    x -= cch*xFont*gi.nScale/2;
  if (dt & dtBottom)
    y -= (yFont-3)*gi.nScale;
  else if (!(dt & dtTop))
    y -= yFont*gi.nScale/2;
  if (dt & dtErase) {
    DrawColor(gi.kiOff);
    DrawBlock(x, y, x+xFont*gi.nScale*cch-1, y+(yFont-2)*gi.nScale);
  }
  DrawColor(c);
#ifdef PS
  if (gs.fPS && gs.fFont) {
    PsFont(4);
    fprintf(gi.file, "%d %d(%s)center\n",
      x + xFont*gi.nScale*cch/2, y + yFont*gi.nScale/2, sz);
    gi.nScale = s;
    return;
  }
#endif
  while (*sz) {
#ifdef META
    if (gs.fMeta && gs.fFont) {
      gi.nFontDes = 3;
      gi.kiTextDes = gi.kiCur;
      gi.nAlignDes = 0x6 | 0 /* Center | Top */;
      MetaSelect();
      MetaTextOut(x, y, 1);
      MetaWord(WFromBB(*sz, 0));
    } else
#endif
    {
      i = (_char)*sz-' ';
      if (i < 256-32)
        DrawTurtle(szDrawCh[i], x, y);
    }
    x += xFont*gi.nScale;
    sz++;
  }
  gi.nScale = s;
}


/* Draw the glyph of a sign at particular coordinates on the screen.    */
/* To do this we either use Astrolog's turtle vector representation or  */
/* we may specify a system font character for PostScript and metafiles. */

void DrawSign(int i, int x, int y)
{
#ifdef PS
  if (gs.fPS && gs.fFont) {
    PsFont(1);
    fprintf(gi.file, "%d %d(%c)center\n", x, y, 'A' + i - 1);
    return;
  }
#endif
#ifdef META
  if (gs.fMeta && gs.fFont) {
    gi.nFontDes = 1;
    gi.kiTextDes = gi.kiCur;
    gi.nAlignDes = 0x6 | 0x8 /* Center | Bottom */;
    MetaSelect();
    MetaTextOut(x, y+4*gi.nScale, 1);
    MetaWord(WFromBB('^' + i - 1, 0));
    return;
  }
#endif
  if (i == sCap && gs.nGlyphs/1000 > 1)
    i = cSign+1;
  if ((gi.nScale & 1) || !szDrawSign2[i][0])
    DrawTurtle(szDrawSign[i], x, y);
  else {
    gi.nScale >>= 1;
    DrawTurtle(szDrawSign2[i], x, y);  /* Special hi-res sign glyphs. */
    gi.nScale <<= 1;
  }
}


/* Draw the number of a house at particular coordinates on the screen. */
/* We either use a turtle vector or write a number in a system font.   */

void DrawHouse(int i, int x, int y)
{
#ifdef PS
  if (gs.fPS && gs.fFont) {
    PsFont(3);
    fprintf(gi.file, "%d %d(%d)center\n", x, y, i);
    return;
  }
#endif
#ifdef META
  if (gs.fMeta && gs.fFont) {
    gi.nFontDes = 2;
    gi.kiTextDes = gi.kiCur;
    gi.nAlignDes = 0x6 | 0x8 /* Center | Bottom */;
    MetaSelect();
    MetaTextOut(x, y+3*gi.nScale, 1 + (i>9));
    MetaWord(WFromBB(i > 9 ? '1' : '0'+i, i > 9 ? '0'+i-10 : 0));
    return;
  }
#endif
  if ((gi.nScale & 1) || !szDrawHouse2[i][0])
    DrawTurtle(szDrawHouse[i], x, y);
  else {
    gi.nScale >>= 1;
    DrawTurtle(szDrawHouse2[i], x, y);  /* Special hi-res house numbers. */
    gi.nScale <<= 1;
  }
}


/* Draw the glyph of an object at particular coordinates on the screen. */

void DrawObject(int obj, int x, int y)
{
  char szGlyph[4];
  int ich;

  if (!gs.fLabel)    /* If we are inhibiting labels, then do nothing. */
    return;
  DrawColor(kObjB[obj]);
  if (obj <= oNorm) {
#ifdef STROKE
    ich = obj;
#endif
#ifdef PS
    if (gs.fPS && gs.fFont == 1 && obj < uranLo && szObjectFont[ich] != ' ') {
      PsFont(2);
      fprintf(gi.file, "%d %d(%c)center\n", x, y, szObjectFont[ich]);
      return;
    }
#endif
#ifdef META
    if (gs.fMeta && gs.fFont == 1 &&
      obj < uranLo && szObjectFont[ich] != ' ') {
      gi.nFontDes = 4;
      gi.kiTextDes = gi.kiCur;
      gi.nAlignDes = 0x6 | 0x8 /* Center | Bottom */;
      MetaSelect();
      MetaTextOut(x, y+5*gi.nScale, 1);
      MetaWord(WFromBB(szObjectFont[ich], 0));
      return;
    }
#endif
    if (obj == oUra) {
      if ((gs.nGlyphs/100)%10 > 1)
        obj = oNorm + 1;
    } else if (obj == oPlu) {
      if ((gs.nGlyphs/10)%10 > 1)
        obj = oNorm + (((gs.nGlyphs/10)%10 > 2) ? 4 : 2);
    } else if (obj == oLil) {
      if (gs.nGlyphs%10 > 1)
        obj = oNorm + 3;
    }
    if ((gi.nScale & 1) || !szDrawObject2[obj][0])
      DrawTurtle(szDrawObject[obj], x, y);
    else {
      gi.nScale >>= 1;
      DrawTurtle(szDrawObject2[obj], x, y); /* Special hi-res object glyphs. */
      gi.nScale <<= 1;
    }

  /* Normally we can just go draw the glyph; however, stars don't have */
  /* glyphs, so for these draw their three letter abbreviation.        */

  } else {
    sprintf(szGlyph, "%c%c%c", chObj3(obj));
#ifdef CONSTEL
    /* If doing constellations, give a couple stars more correct */
    /* astronomical names.                                       */

    if (gs.fConstel) {
      if (obj == oOri)
        sprintf(szGlyph, "Aln");    /* Alnilam, normally "Orion" */
      else if (obj == oAnd)
        sprintf(szGlyph, "M31");    /* M31, normally "Andromeda" */
    }
#endif
    DrawSz(szGlyph, x, y, dtCent);
  }
}


/* Draw the glyph of an aspect at particular coordinates on the screen. */
/* Again we either use Astrolog's turtle vector or a system Astro font. */

void DrawAspect(int asp, int x, int y)
{
#ifdef PS
  if (gs.fPS && gs.fFont == 1 && szAspectFont[asp-1] != ' ') {
    PsFont(2);
    fprintf(gi.file, "%d %d(%s%c)center\n", x, y,
      asp == aSSq || asp == aSes ? "\\" : "", szAspectFont[asp-1]);
    return;
  }
#endif
#ifdef META
  if (gs.fMeta && gs.fFont == 1 && szAspectFont[asp-1] != ' ') {
    gi.nFontDes = 4;
    gi.kiTextDes = gi.kiCur;
    gi.nAlignDes = 0x6 | 0x8 /* Center | Bottom */;
    MetaSelect();
    MetaTextOut(x, y+5*gi.nScale, 1);
    MetaWord(WFromBB(szAspectFont[asp-1], 0));
    return;
  }
#endif
  if (us.fParallel && asp <= aOpp)
    asp += cAspect;
  if ((gi.nScale & 1) || !szDrawAspect2[asp][0])
    DrawTurtle(szDrawAspect[asp], x, y);
  else {
    gi.nScale >>= 1;
    DrawTurtle(szDrawAspect2[asp], x, y);  /* Special hi-res aspect glyphs. */
    gi.nScale <<= 1;
  }
}


/* Convert a string segment to a positive number, updating the string to  */
/* point beyond the number chars. Return 1 if the string doesn't point to */
/* a numeric value. This is used by the DrawTurtle() routine to extract   */
/* motion vector quantities from draw strings, e.g. the "12" in "U12".    */

int NFromPch(CONST char **str)
{
  int num = 0, i = 0;

  loop {
    if (**str < '0' || **str > '9')
      return num > 0 ? num : (i < 1 ? 1 : 0);
    num = num*10+(**str)-'0';
    (*str)++;
    i++;
  }
}


/* This routine is used to draw complicated objects composed of lots of line */
/* segments on the screen, such as all the glyphs and coastline pieces. It   */
/* is passed in a string of commands defining what to draw in relative       */
/* coordinates. This is a copy of the format of the BASIC draw command found */
/* in PC's. For example, "U5R10D5L10" means go up 5 dots, right 10, down 5,  */
/* and left 10 - draw a box twice as wide as it is high.                     */

void DrawTurtle(CONST char *sz, int x0, int y0)
{
  int i, x, y, deltax, deltay;
  bool fBlank, fNoupdate;
  char chCmd;

  gi.xTurtle = x0; gi.yTurtle = y0;
  while (chCmd = ChCap(*sz)) {
    sz++;

    /* 'B' prefixing a command means just move the cursor, and don't draw. */

    if (fBlank = (chCmd == 'B')) {
      chCmd = ChCap(*sz);
      sz++;
    }

    /* 'N' prefixing a command means don't update cursor when done drawing. */

    if (fNoupdate = (chCmd == 'N')) {
      chCmd = ChCap(*sz);
      sz++;
    }

    /* Here we process the eight directional commands. */

    switch (chCmd) {
    case 'U': deltax =  0; deltay = -1; break;      /* Up    */
    case 'D': deltax =  0; deltay =  1; break;      /* Down  */
    case 'L': deltax = -1; deltay =  0; break;      /* Left  */
    case 'R': deltax =  1; deltay =  0; break;      /* Right */
    case 'E': deltax =  1; deltay = -1; break;      /* NorthEast */
    case 'F': deltax =  1; deltay =  1; break;      /* SouthEast */
    case 'G': deltax = -1; deltay =  1; break;      /* SouthWest */
    case 'H': deltax = -1; deltay = -1; break;      /* NorthWest */
    default: PrintError("Bad draw.");       /* Shouldn't happen. */
    }
    x = gi.xTurtle;
    y = gi.yTurtle;
    i = NFromPch(&sz)*gi.nScale;    /* Figure out how far to draw. */
    if (fBlank) {
      gi.xTurtle += deltax*i;
      gi.yTurtle += deltay*i;
    } else {
      gi.xTurtle += deltax*i;
      gi.yTurtle += deltay*i;
      DrawLine(x, y, gi.xTurtle, gi.yTurtle);
      if (fNoupdate) {
        gi.xTurtle = x;
        gi.yTurtle = y;
      }
    }
  }
}
#endif /* GRAPH */

/* xgeneral.cpp */
