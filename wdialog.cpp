/*
** Astrolog (Version 6.00) File: wdialog.cpp
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


#ifdef WIN
/*
******************************************************************************
** Dialog Utility Functions.
******************************************************************************
*/

/* Set the contents of the given edit control in a dialog to a string. */

void SetEditSz(HWND hdlg, int id, char *sz)
{
  while (*sz == ' ')    /* Strip off any extra leading spaces. */
    sz++;
  SetEdit(id, sz);
}


/* Set the contents of the given edit control in a dialog to a floating */
/* point value, with at most 'n' significant fractional digits.         */

void SetEditR(HWND hdlg, int id, real r, int n)
{
  char sz[cchSzDef], szT[8], *pch;

  sprintf(szT, "%%.%df", NAbs(n));
  sprintf(sz, szT, r);
  for (pch = sz; *pch; pch++)
    ;
  while (*(--pch) == '0')  /* Drop off any trailing zero digits. */
    ;
  /* Positive n means ensure at least one fractional digit. */
  pch[n > 0 ? 1 + (*pch == '.') : (*pch != '.')] = chNull;
  SetEdit(id, sz);
}


/* Set the contents of four combo controls and their dropdowns in a dialog */
/* indicating month, day, year, and time fields to the given values.       */

void SetEditMDYT(HWND hdlg, int idMon, int idDay, int idYea, int idTim,
  int mon, int day, int yea, real tim)
{
  char sz[cchSzDef];
  int i;

  ClearCombo(idMon);
  ClearCombo(idDay);
  ClearCombo(idYea);
  ClearCombo(idTim);
  if (!FValidMon(mon))
    mon = 1;
  sprintf(sz, "%c%c%c", chMon3(mon));
  SetEdit(idMon, sz);
  for (i = 1; i <= cSign; i++)
    SetCombo(idMon, szMonth[i]);
  if (!FValidDay(day, mon, yea))
    day = 1;
  SetEditN(idDay, day);
  for (i = 0; i <= 25; i += 5) {
    sprintf(sz, "%d", Max(i, 1)); SetCombo(idDay, sz);
  }
  SetEditN(idYea, yea);
  for (i = 2010; i < 2020; i++) {
    sprintf(sz, "%d", i); SetCombo(idYea, sz);
  }
  sprintf(sz, "%s", SzTim(tim));
  SetEditSz(hdlg, idTim, sz);
  SetCombo(idTim, "Midnight");
  SetCombo(idTim, (char *)(us.fEuroTime ? "6:00" : "6:00am"));
  SetCombo(idTim, "Noon");
  SetCombo(idTim, (char *)(us.fEuroTime ? "18:00" : "6:00pm"));
}


/* Set the contents of four combo controls in a dialog indicating daylight, */
/* time zone, longitude, and latitude fields to the given values.           */

void SetEditSZOA(HWND hdlg, int idDst, int idZon, int idLon, int idLat,
  real dst, real zon, real lon, real lat)
{
  char sz[cchSzDef];
  int i;
  bool fT;

  if (dst == 0.0 || dst == 1.0)
    sprintf(sz, "%s", dst == 0.0 ? "No" : "Yes");
  else
    sprintf(sz, "%.2f", dst);
  SetEdit(idDst, sz);
  SetCombo(idDst, "No"); SetCombo(idDst, "Yes");
  sprintf(sz, "%s", SzZone(zon));
  SetEdit(idZon, (char *)(sz[0] == '+' ? &sz[1] : sz));
  /* For the time zone dropdown, fill it out with all abbreviations of */
  /* three letters that don't reference daylight or war time.          */
  for (i = 0; i < cZone; i++) {
    if (szZon[i][1] && szZon[i][1] != 'D' && szZon[i][1] != 'W' &&
      szZon[i][2] && szZon[i][2] != 'D')
      SetCombo(idZon, szZon[i]);
  }
  fT = us.fAnsiChar; us.fAnsiChar = fFalse;
  sprintf(sz, "%s", SzLocation(lon, lat));
  us.fAnsiChar = fT;
  i = 7 + is.fSeconds*3;
  sz[i] = chNull;
  SetEditSz(hdlg, idLon, &sz[0]);
  SetCombo(idLon, "122W20"); SetCombo(idLon, "0E00");
  SetEditSz(hdlg, idLat, &sz[i+1]);
  SetCombo(idLat, "47N36"); SetCombo(idLat, "0S00");
}


/* Set the contents of a combo control in a dialog indicating a color   */
/* field to the given value, and fill its dropdown with the color list. */

void SetEditColor(HWND hdlg, int id, KI ki)
{
  int i;

  SetEdit(id, szColor[ki]);
  for (i = 0; i < cColor; i++)
    SetCombo(id, szColor[i]);
}


/* Return the contents of a dialog edit control as a floating point value. */

real GetEditR(HWND hdlg, int id)
{
  char sz[cchSzMax];

  GetEdit(id, sz);
  return atof(sz);
}


/* Bring up an error box indicating an illegal value for a dialog field. */

void ErrorEnsure(int n, char *sz)
{
  char szT[cchSzDef];

  sprintf(szT, "The value %d is not a valid %s setting.", n, sz);
  PrintWarning(szT);
}


/* Take many of the user visible settings, and write them out to a new     */
/* command switch file, which may be read in to restore those settings.    */
/* Most often this would be used to create a new astrolog.as default       */
/* settings file. This is called from the File Save Settings menu command. */

bool FOutputSettings()
{
  char sz[cchSzDef];
  FILE *file;
  int i;
  bool f1, f2;

  if (us.fNoWrite)
    return fFalse;
  file = fopen(is.szFileOut, "w");  /* Create and open the file for output. */
  if (file == NULL) {
    sprintf(sz, "Settings file %s can not be created.", is.szFileOut);
    PrintError(sz);
    return fFalse;
  }

  sprintf(sz, "@0309  ; %s (%s) default settings file %s\n\n",
    szAppName, szVersionCore, DEFAULT_INFOFILE); PrintFSz();

  sprintf(sz, "-z %s                ", SzZone(us.zonDef)); PrintFSz();
  PrintF("; Default time zone     [hours W or E of GMT   ]\n");
  sprintf(sz, "-z0 %d                   ", (int)us.dstDef); PrintFSz();
  PrintF("; Default Daylight time [0 standard, 1 daylight]\n");
  f1 = us.fAnsiChar; us.fAnsiChar = 3;
  f2 = is.fSeconds; is.fSeconds = fTrue;
  sprintf(sz, "-zl %s  ", SzLocation(us.lonDef, us.latDef)); PrintFSz();
  us.fAnsiChar = f1; is.fSeconds = f2;
  PrintF("; Default longitude and latitude\n\n");

  sprintf(sz, "-Yz %ld   ", us.lTimeAddition); PrintFSz();
  PrintF(
    "; Time minute addition to be used when \"now\" charts are off.\n-n");
  PrintF(
    "      ; Comment out this line to not start with chart for \"now\".\n\n");

  sprintf(sz, "%cs      ", ChDashF(us.fSidereal)); PrintFSz();
  PrintF(
    "; Zodiac selection          [\"_s\" is tropical, \"=s\" is sidereal]\n");
  sprintf(sz, ":s %.0f    ", us.rZodiacOffset); PrintFSz();
  PrintF(
    "; Zodiac offset value       [Change \"0\" to desired ayanamsa    ]\n");
  sprintf(sz, "-A %d    ", us.nAsp); PrintFSz();
  PrintF(
    "; Number of aspects         [Change \"5\" to desired number      ]\n");
  i = us.nHouseSystem; sprintf(sz, "-c %.4s ", szSystem[i]); PrintFSz();
  PrintF(
    "; House system              [Change \"Plac\" to desired system   ]\n");
  sprintf(sz, "%ck      ", ChDashF(us.fAnsiColor)); PrintFSz();
  PrintF(
    "; Ansi color text           [\"=k\" is color, \"_k\" is normal     ]\n");
  sprintf(sz, ":d %d   ", us.nDivision); PrintFSz();
  PrintF(
    "; Searching divisions       [Change \"48\" to desired divisions  ]\n");
  sprintf(sz, "%cb0     ", ChDashF(us.fSeconds)); PrintFSz();
  PrintF(
    "; Print zodiac seconds      [\"_b0\" to minute, \"=b0\" to second  ]\n");
  sprintf(sz, "%cb      ", ChDashF(us.fEphemFiles)); PrintFSz();
  PrintF(
    "; Use ephemeris files       [\"=b\" uses them, \"_b\" doesn't      ]\n");
  sprintf(sz, "%cC      ", ChDashF(us.fCusp)); PrintFSz();
  PrintF(
    "; Show house cusp objects   [\"_C\" hides them, \"=C\" shows them  ]\n");
  sprintf(sz, "%cv7     ", ChDashF(us.fListingEso)); PrintFSz();
  PrintF(
    "; Show esoteric rulerships  [\"=v7\" shows them, \"_v7\" hides them]\n");
  sprintf(sz, ":w %d    ", us.nWheelRows); PrintFSz();
  PrintF(
    "; Wheel chart text rows     [Change \"4\" to desired wheel rows  ]\n");
  sprintf(sz, ":I %d   ", us.nScreenWidth); PrintFSz();
  PrintF(
    "; Text screen columns       [Change \"80\" to desired columns    ]\n");
  sprintf(sz, "-YQ %d  ", us.nScrollRow); PrintFSz();
  PrintF(
    "; Text screen scroll limit  [Change \"24\" or set to \"0\" for none]\n");
  sprintf(sz, "%cYd     ", ChDashF(us.fEuroDate)); PrintFSz();
  PrintF(
    "; European date format      [\"_Yd\" is MDY, \"=Yd\" is DMY        ]\n");
  sprintf(sz, "%cYt     ", ChDashF(us.fEuroTime)); PrintFSz();
  PrintF(
    "; European time format      [\"_Yt\" is AM/PM, \"=Yt\" is 24 hour  ]\n");
  sprintf(sz, "%cYr     ", ChDashF(us.fRound)); PrintFSz();
  PrintF(
    "; Show rounded positions    [\"=Yr\" rounds, \"_Yr\" doesn't       ]\n");
  sprintf(sz, "%cYC     ", ChDashF(us.fSmartCusp)); PrintFSz();
  PrintF(
    "; Smart cusp displays       [\"=YC\" is smart, \"_YC\" is normal   ]\n");
  sprintf(sz, "%cY8     ", ChDashF(us.fClip80)); PrintFSz();
  PrintF(
    "; Clip text to end of line  [\"=Y8\" clips, \"_Y8\" doesn't clip   ]\n");

  PrintF("\n\n; DEFAULT RESTRICTIONS:\n\n-YR 0 10     ");
  for (i = 0; i <= 10; i++) PrintF(SzNumF(ignore[i]));
  PrintF("   ; Planets\n-YR 11 21    ");
  for (i = 11; i <= 21; i++) PrintF(SzNumF(ignore[i]));
  PrintF("   ; Minor planets\n-YR 22 33    ");
  for (i = 22; i <= 33; i++) PrintF(SzNumF(ignore[i]));
  PrintF(" ; House cusps\n-YR 34 42    ");
  for (i = 34; i <= 42; i++) PrintF(SzNumF(ignore[i]));
  PrintF("       ; Uranians\n\n");

  PrintF("; DEFAULT TRANSIT RESTRICTIONS:\n\n-YRT 0 10    ");
  for (i = 0; i <= 10; i++) PrintF(SzNumF(ignore2[i]));
  PrintF("   ; Planets\n-YRT 11 21   ");
  for (i = 11; i <= 21; i++) PrintF(SzNumF(ignore2[i]));
  PrintF("   ; Minor planets\n-YRT 22 33   ");
  for (i = 22; i <= 33; i++) PrintF(SzNumF(ignore2[i]));
  PrintF(" ; House cusps\n-YRT 34 42   ");
  for (i = 34; i <= 42; i++) PrintF(SzNumF(ignore2[i]));
  PrintF("       ; Uranians\n\n");

  sprintf(sz, "-YR0 %s%s ; Restrict sign, direction changes\n\n\n",
    SzNumF(us.fIgnoreSign), SzNumF(us.fIgnoreDir)); PrintFSz();

  PrintF("; DEFAULT ASPECT ORBS:\n\n-YAo 1 5    ");
  for (i = 1; i <= 5; i++) { sprintf(sz, " %.0f", rAspOrb[i]); PrintFSz(); }
  PrintF("      ; Major aspects\n-YAo 6 11   ");
  for (i = 6; i <= 11; i++) { sprintf(sz, " %.0f", rAspOrb[i]); PrintFSz(); }
  PrintF("    ; Minor aspects\n-YAo 12 18  ");
  for (i = 12; i <= 18; i++) { sprintf(sz, " %.0f", rAspOrb[i]); PrintFSz(); }
  PrintF("  ; Obscure aspects\n\n");

  PrintF("; DEFAULT MAX PLANET ASPECT ORBS:\n\n-YAm 0 10   ");
  for (i = 0; i <= 10; i++) { sprintf(sz, "%4.0f", rObjOrb[i]); PrintFSz(); }
  PrintF("      ; Planets\n-YAm 11 21  ");
  for (i = 11; i <= 21; i++) { sprintf(sz, "%4.0f", rObjOrb[i]); PrintFSz(); }
  PrintF("      ; Minor planets\n-YAm 22 33  ");
  for (i = 22; i <= 33; i++) { sprintf(sz, "%4.0f", rObjOrb[i]); PrintFSz(); }
  PrintF("  ; Cusp objects\n-YAm 34 42  ");
  for (i = 34; i <= 42; i++) { sprintf(sz, "%4.0f", rObjOrb[i]); PrintFSz(); }
  PrintF("              ; Uranians\n-YAm 43 43  ");
  sprintf(sz, "%4.0f", rObjOrb[43]); PrintFSz();
  PrintF("                                              ; Fixed stars\n");

  PrintF("\n; DEFAULT PLANET ASPECT ORB ADDITIONS:\n\n-YAd 0 10   ");
  for (i = 0; i <= 10; i++) { sprintf(sz, " %.0f", rObjAdd[i]); PrintFSz(); }
  PrintF("    ; Planets\n-YAd 11 21  ");
  for (i = 11; i <= 21; i++) { sprintf(sz, " %.0f", rObjAdd[i]); PrintFSz(); }
  PrintF("    ; Minor planets\n-YAd 22 33  ");
  for (i = 22; i <= 33; i++) { sprintf(sz, " %.0f", rObjAdd[i]); PrintFSz(); }
  PrintF("  ; Cusp objects\n-YAd 34 42  ");
  for (i = 34; i <= 42; i++) { sprintf(sz, " %.0f", rObjAdd[i]); PrintFSz(); }
  PrintF("        ; Uranians\n-YAd 43 43  ");
  sprintf(sz, " %.0f", rObjAdd[43]); PrintFSz();
  PrintF("                        ; Fixed stars\n\n\n");

  PrintF("; DEFAULT INFLUENCES:\n\n-Yj 0 10   ");
  for (i = 0; i <= 10; i++) { sprintf(sz, " %.0f", rObjInf[i]); PrintFSz(); }
  PrintF("     ; Planets\n-Yj 11 21  ");
  for (i = 11; i <= 21; i++) { sprintf(sz, " %.0f", rObjInf[i]); PrintFSz(); }
  PrintF("                ; Minor planets\n-Yj 22 33  ");
  for (i = 22; i <= 33; i++) { sprintf(sz, " %.0f", rObjInf[i]); PrintFSz(); }
  PrintF("  ; Cusp objects\n-Yj 34 42  ");
  for (i = 34; i <= 42; i++) { sprintf(sz, " %.0f", rObjInf[i]); PrintFSz(); }
  PrintF("                    ; Uranians\n\n-YjC 1 12  ");

  for (i = 1; i <= cSign; i++)
    { sprintf(sz, " %.0f", rHouseInf[i]); PrintFSz(); }
  PrintF("  ; Houses\n\n-YjA 1 5   ");

  for (i = 1; i <= 5; i++) { sprintf(sz, "%4.1f", rAspInf[i]); PrintFSz(); }
  PrintF("          ; Major aspects\n-YjA 6 11  ");
  for (i = 6; i <= 11; i++) { sprintf(sz, "%4.1f", rAspInf[i]); PrintFSz(); }
  PrintF("      ; Minor aspects\n-YjA 12 18 ");
  for (i = 12; i <= 18; i++) { sprintf(sz, "%4.1f", rAspInf[i]); PrintFSz(); }
  PrintF("  ; Obscure aspects\n\n");

  PrintF("; DEFAULT TRANSIT INFLUENCES:\n\n-YjT 0 10  ");
  for (i = 0; i <= 10; i++)
    { sprintf(sz, " %.0f", rTransitInf[i]); PrintFSz(); }
  PrintF("  ; Planets\n-YjT 11 21 ");
  for (i = 11; i <= 21; i++)
    { sprintf(sz, " %.0f", rTransitInf[i]); PrintFSz(); }
  PrintF("    ; Minor planets\n-YjT 34 42 ");
  for (i = 34; i <= 42; i++)
    { sprintf(sz, " %.0f", rTransitInf[i]); PrintFSz(); }

  sprintf(sz, "      ; Uranians\n\n-Yj0 %.0f %.0f %.0f %.0f ",
    rObjInf[oNorm + 1], rObjInf[oNorm + 2], rHouseInf[cSign + 1],
    rHouseInf[cSign + 2]); PrintFSz();
  PrintF(" ; In ruling sign, exalted sign, ruling house, exalted house\n");
  sprintf(sz, "-Yj7 %.0f %.0f %.0f %.0f %.0f %.0f ", rObjInf[oNorm + 3],
    rObjInf[oNorm + 4], rObjInf[oNorm + 5], rHouseInf[cSign + 3],
    rHouseInf[cSign + 4], rHouseInf[cSign + 5]); PrintFSz();
  PrintF(" ; Esoteric, Hierarchical, Ray ruling - sign, house\n\n\n");

  PrintF("; DEFAULT RAYS:\n\n-Y7C 1 12  ");
  for (i = 1; i <= cSign; i++)
    { sprintf(sz, " %d", rgSignRay[i]); PrintFSz(); }
  PrintF("  ; Signs\n-Y7O 0 10  ");
  for (i = 0; i <= 10; i++)
    { sprintf(sz, " %d", rgObjRay[i]); PrintFSz(); }
  PrintF("             ; Planets\n-Y7O 34 42 ");
  for (i = 34; i <= 42; i++)
    { sprintf(sz, " %d", rgObjRay[i]); PrintFSz(); }
  PrintF("                 ; Uranians\n\n\n");

  PrintF("; DEFAULT COLORS:\n\n-YkO 0 10  ");
  for (i = 0; i <= 10; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("      ; Planet colors\n-YkO 11 21 ");
  for (i = 11; i <= 21; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("      ; Minor colors\n-YkO 22 33 ");
  for (i = 22; i <= 33; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("  ; Cusp colors\n-YkO 34 42 ");
  for (i = 34; i <= 42; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("              ; Uranian colors\n\n-YkA 1 5   ");

  for (i = 1; i <= 5; i++)
    { sprintf(sz, " %.3s", szColor[kAspA[i]]); PrintFSz(); }
  PrintF("          ; Major aspect colors\n-YkA 6 11  ");
  for (i = 6; i <= 11; i++)
    { sprintf(sz, " %.3s", szColor[kAspA[i]]); PrintFSz(); }
  PrintF("      ; Minor aspect colors\n-YkA 12 18 ");
  for (i = 12; i <= 18; i++)
    { sprintf(sz, " %.3s", szColor[kAspA[i]]); PrintFSz(); }
  PrintF("  ; Obscure aspect colors\n\n-YkC       ");

  for (i = eFir; i <= eWat; i++)
    { sprintf(sz, " %.3s", szColor[kElemA[i]]); PrintFSz(); }
  PrintF("                      ; Element colors\n-Yk7 1 7   ");
  for (i = 1; i <= cRay; i++)
    { sprintf(sz, " %.3s", szColor[kRayA[i]]); PrintFSz(); }
  PrintF("          ; Ray colors\n-Yk0 1 7   ");
  for (i = 1; i <= 7; i++)
    { sprintf(sz, " %.3s", szColor[kRainbowA[i]]); PrintFSz(); }
  PrintF("          ; Rainbow colors\n-Yk  0 8   ");
  for (i = 0; i <= 8; i++)
    { sprintf(sz, " %.3s", szColor[kMainA[i]]); PrintFSz(); }
  PrintF("  ; Main colors\n\n\n");

  PrintF("; GRAPHICS DEFAULTS:\n\n");
  sprintf(sz, "%cX              ", ChDashF(us.fGraphics)); PrintFSz();
  PrintF("; Graphics chart flag [\"_X\" is text, \"=X\" is graphics]\n");
  i = gs.xWin; if (fSidebar) i -= SIDESIZE;
  sprintf(sz, ":Xw %d %d     ", i, gs.yWin); PrintFSz();
  PrintF("; Default X and Y resolution\n");
  sprintf(sz, ":Xb%c            ", ChUncap(gs.chBmpMode)); PrintFSz();
  PrintF("; Bitmap file type\n");
  sprintf(sz, ":YXG %d       ", gs.nGlyphs); PrintFSz();
  PrintF("; Glyph selections [Capricorn, Uranus, Pluto, Lilith]\n");
  sprintf(sz, ":YXg %d         ", gs.nGridCell); PrintFSz();
  PrintF("; Aspect grid cells\n");
  sprintf(sz, ":YX7 %d        ", gs.nRayWidth); PrintFSz();
  PrintF("; Esoteric ray column influence width\n");
  sprintf(sz, ":YXf %d          ", gs.fFont); PrintFSz();
  PrintF("; Use actual fonts\n");
  sprintf(sz, ":YXp %d          ", gs.nOrient); PrintFSz();
  PrintF("; PostScript paper orientation\n");
  sprintf(sz, ":YXp0 %.1f %.1f  ", gs.xInch, gs.yInch); PrintFSz();
  PrintF("; PostScript paper X and Y inch sizes\n");
  PrintF(":YX -1 16       ; PC hi-res and lo-res graphics modes\n\n");

  sprintf(sz, "; %s\n", DEFAULT_INFOFILE); PrintFSz();
  fclose(file);
  return fTrue;
}


/*
******************************************************************************
** Windows Dialogs.
******************************************************************************
*/

/* Bring up the Windows standard file open dialog, and process the        */
/* command file specified if any. This is called from the File Open Chart */
/* and File Open Chart #2 menu commands.                                  */

bool API DlgOpenChart()
{
  char sz[cchSzDef];
  CI ciT;

  /* Setup dialog title and settings and get the name of a file from it. */
  if (us.fNoRead) {
    PrintWarning("File input is not allowed now.");
    return fFalse;
  }
  sprintf(sz, "Open Chart #%d", wi.nDlgChart);
  if (wi.nDlgChart < 2)
    sz[10] = chNull;
  ofn.lpstrTitle = sz;
  ofn.lpstrFilter = "Astrolog Files (*.as)\0*.as\0All Files (*.*)\0*.*\0";
  sprintf(szFileName, "*.as");
  if (!GetOpenFileName((LPOPENFILENAME)&ofn))
    return fFalse;

  /* Process the given file based on what open command is being run. */
  ciT = ciCore;
  FInputData(ofn.lpstrFileTitle);
  if (wi.nDlgChart > 1) {
    if (wi.nDlgChart == 2)
      ciTwin = ciCore;
    else if (wi.nDlgChart == 3)
      ciThre = ciCore;
    else
      ciFour = ciCore;
    ciCore = ciT;
  }

  wi.fCast = fTrue;
  return fTrue;
}


/* Bring up the Windows standard file save dialog, get the name of a file   */
/* from the user, and save to it either right away or set variables to      */
/* ensure it will be done later. This is called from all six File Save menu */
/* commands: Save Info, Positions, Text, Bitmap, Picture, and PostScript.   */

bool API DlgSaveChart()
{
  char *pch;

  /* Setup dialog title and settings and get the name of a file from it. */
  if (us.fNoWrite) {
    PrintWarning("File output is not allowed now.");
    return fFalse;
  }
  ofn.lpstrFilter = "Astrolog Files (*.as)\0*.as\0All Files (*.*)\0*.*\0";
  sprintf(szFileName, "*.as");
  switch (wi.wCmd) {
  case cmdSaveChart:
    ofn.lpstrTitle = "Save Chart Info";
    break;
  case cmdSavePositions:
    ofn.lpstrTitle = "Save Chart Positions";
    break;
  case cmdSaveText:
    ofn.lpstrTitle = "Save Chart Text";
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    sprintf(szFileName, "*.txt");
    break;
  case cmdSaveBitmap:
    ofn.lpstrTitle = "Save Chart Bitmap";
    ofn.lpstrFilter =
      "Windows Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    sprintf(szFileName, "*.bmp");
    break;
  case cmdSavePicture:
    ofn.lpstrTitle = "Save Chart Picture";
    ofn.lpstrFilter =
      "Windows Metafiles (*.wmf)\0*.wmf\0All Files (*.*)\0*.*\0";
    sprintf(szFileName, "*.wmf");
    break;
  case cmdSavePS:
    ofn.lpstrTitle = "Save Chart PostScript";
    ofn.lpstrFilter =
      "PostScript Text (*.eps)\0*.eps\0All Files (*.*)\0*.*\0";
    sprintf(szFileName, "*.eps");
    break;
  case cmdSaveSettings:
    ofn.lpstrTitle = "Save Settings";
    ofn.lpstrFilter = "Astrolog Files (*.as)\0*.as\0All Files (*.*)\0*.*\0";
    sprintf(szFileName, "%s", DEFAULT_INFOFILE);
    break;
  }
  if (wi.wCmd != cmdSaveWallTile && wi.wCmd != cmdSaveWallCenter &&
    wi.wCmd != cmdSaveWallStretch && wi.wCmd != cmdSaveWallFit &&
    wi.wCmd != cmdSaveWallFill) {
    if (!GetSaveFileName((LPOPENFILENAME)&ofn))
      return fFalse;
  } else {
    ofn.lpstrFile = (char *)PAllocate(cchSzDef, fTrue, NULL);
    GetWindowsDirectory(ofn.lpstrFile, cchSzDef);
    pch = ofn.lpstrFile + CchSz(ofn.lpstrFile);
    sprintf(pch, "%c%s%s", '\\', szAppName, ".bmp");
  }

  /* Saving chart info, position, or setting command files can be done  */
  /* here. Saving actual chart output isn't done until the next redraw. */
  is.szFileOut = gi.szFileOut = ofn.lpstrFile;
  switch (wi.wCmd) {
  case cmdSaveChart:
    us.fWritePos = fFalse;
    return FOutputData();
  case cmdSavePositions:
    us.fWritePos = fTrue;
    return FOutputData();
  case cmdSaveText:
    is.szFileScreen = ofn.lpstrFile;
    us.fGraphics = fFalse;
    wi.fRedraw = fTrue;
    break;
  case cmdSaveBitmap:
  case cmdSaveWallTile:
  case cmdSaveWallCenter:
  case cmdSaveWallStretch:
  case cmdSaveWallFit:
  case cmdSaveWallFill:
    gs.fBitmap = fTrue;
    gs.fMeta = gs.fPS = fFalse;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSavePicture:
    gs.fMeta = fTrue;
    gs.fBitmap = gs.fPS = fFalse;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSavePS:
    gs.fPS = fTrue;
    gs.fBitmap = gs.fMeta = fFalse;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSaveSettings:
    return FOutputSettings();
  }
  return fTrue;
}


/* Bring up the Windows standard print dialog, receive any printing       */
/* settings from the user, and proceed to print the current graphics or   */
/* text chart as displayed in the window. Called from File Print command. */

bool API DlgPrint()
{
  DLGPROC lpAbortDlg;
  ABORTPROC lpAbortProc;
  HDC hdc;
  LPDEVMODE lpDevMode = NULL;
  LPDEVNAMES lpDevNames;
  LPSTR lpszDriverName;
  LPSTR lpszDeviceName;
  LPSTR lpszPortName;
  DOCINFO doci;
  int xPrint, yPrint;
  int xClient, yClient;

  /* Bring up the Windows print dialog. */
  wi.fNoUpdate = fFalse;
  if (!PrintDlg((LPPRINTDLG)&prd))
    return fTrue;

  /* Get the printer DC. */
  if (prd.hDC)
    hdc = prd.hDC;
  else {
    /* If the dialog didn't just return the DC, try to make one manually. */
    if (!prd.hDevNames)
      return fFalse;
    lpDevNames = (LPDEVNAMES)GlobalLock(prd.hDevNames);
    lpszDriverName = (LPSTR)lpDevNames + lpDevNames->wDriverOffset;
    lpszDeviceName = (LPSTR)lpDevNames + lpDevNames->wDeviceOffset;
    lpszPortName = (LPSTR)lpDevNames + lpDevNames->wOutputOffset;
    GlobalUnlock(prd.hDevNames);
    if (prd.hDevMode)
      lpDevMode = (LPDEVMODE)GlobalLock(prd.hDevMode);
    hdc = CreateDC(lpszDriverName, lpszDeviceName, lpszPortName,
      /*(LPSTR)*/lpDevMode);
    if (prd.hDevMode && lpDevMode)
      GlobalUnlock(prd.hDevMode);
  }
  if (prd.hDevNames) {
    GlobalFree(prd.hDevNames);
    prd.hDevNames = (HGLOBAL)NULL;
  }
  if (prd.hDevMode) {
    GlobalFree(prd.hDevMode);
    prd.hDevMode = (HGLOBAL)NULL;
  }

  /* Setup the abort dialog and start the print job. */
  lpAbortDlg = (DLGPROC)MakeProcInstance((FARPROC)DlgAbort, wi.hinst);
  lpAbortProc = (ABORTPROC)MakeProcInstance((FARPROC)DlgAbortProc, wi.hinst);
  SetAbortProc(hdc, lpAbortProc);
  doci.cbSize = sizeof(DOCINFO);
  doci.lpszDocName = szAppName;
  doci.lpszOutput = NULL;
  if (StartDoc(hdc, &doci) < 0) {
    FreeProcInstance(lpAbortDlg);
    FreeProcInstance(lpAbortProc);
    DeleteDC(hdc);
    return fFalse;
  }
  wi.fAbort = FALSE;
  wi.hwndAbort = CreateDialog(wi.hinst, MAKEINTRESOURCE(dlgAbort), wi.hwnd,
    lpAbortDlg);
  if (!wi.hwndAbort)
    return fFalse;
  ShowWindow(wi.hwndAbort, SW_NORMAL);
  EnableWindow(wi.hwnd, fFalse);
  StartPage(hdc);

  /* Scale the chart to the page. */
  if (us.fGraphics) {
    gs.xWin *= METAMUL; gs.yWin *= METAMUL; gs.nScale *= METAMUL;
  }
  SetMapMode(hdc, MM_ANISOTROPIC);       /* So SetViewportExt can be called */
  xPrint = GetDeviceCaps(hdc, HORZRES);
  yPrint = GetDeviceCaps(hdc, VERTRES);
  SetViewportOrg(hdc, 0, 0);
  SetViewportExt(hdc, xPrint, yPrint);
  xClient = wi.xClient; yClient = wi.yClient;
  wi.xClient = gs.xWin;
  wi.yClient = NMultDiv(wi.xClient, yPrint, xPrint);
  if (gs.yWin > wi.yClient) {
    wi.yClient = gs.yWin;
    wi.xClient = NMultDiv(wi.yClient, xPrint, yPrint);
  }
  wi.hdcPrint = hdc;

  FRedraw();    /* Actually go "draw" the chart to the printer. */

  /* Restore globals that were temporarily changed to print above. */
  wi.hdcPrint = hdcNil;
  wi.xClient = xClient; wi.yClient = yClient;
  if (us.fGraphics) {
    gs.xWin /= METAMUL; gs.yWin /= METAMUL; gs.nScale /= METAMUL;
  }

  /* Finalize and cleanup everything. */
  if (!wi.fAbort) {
    EndPage(hdc);
    EndDoc(hdc);
  }
  EnableWindow(wi.hwnd, fTrue);
  DestroyWindow(wi.hwndAbort);
  FreeProcInstance(lpAbortDlg);
  FreeProcInstance(lpAbortProc);
  DeleteDC(hdc);

  return fTrue;
}


/* Message loop function for the printing abort dialog. Loops until       */
/* printing is completed or the user hits cancel, returning which result. */

bool API DlgAbortProc(HDC hdc, int nCode)
{
  MSG msg;

  if (wi.hwndAbort == (HWND)NULL)
    return fTrue;
  while (!wi.fAbort && PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
    if (!IsDialogMessage(wi.hwndAbort, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  return !wi.fAbort;
}


/* Processing function for the printing abort modeless dialog, as brought */
/* up temporarily when printing via the File Print menu command.          */

BOOL API DlgAbort(HWND hdlg, _int message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
  case WM_INITDIALOG:
    SetFocus(GetDlgItem(hdlg, IDCANCEL));
    return fFalse;

  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      wi.fAbort = fTrue;
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the command switch entry dialog, as brought up */
/* with the Edit Enter Command Line menu command.                         */

bool API DlgCommand(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];

  switch (message) {
  case WM_INITDIALOG:
    SetFocus(GetDlgItem(hdlg, deCo));
    return fFalse;

  case WM_COMMAND:
    if (wParam == IDOK) {
      GetEdit(deCo, sz);
      FProcessCommandLine(sz);
      wi.fCast = wi.fMenuAll = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the color customization dialog, as brought up */
/* with the View Set Colors menu command.                                */

bool API DlgColor(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j, k, l;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 0; i < 4; i++)
      SetEditColor(hdlg, dce0 + i, kElemA[i]);
    for (i = 1; i <= cAspect; i++)
      SetEditColor(hdlg, dca01 + i - 1, kAspA[i]);
    for (i = 0; i < cColor; i++) {
      j = ikPalette[i];
      SetEditColor(hdlg, dck00 + i, j <= 0 ? kMainA[-j] : kRainbowA[j]);
    }
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      for (k = 0; k <= 1; k++) {
        for (i = 0; i < 4; i++) {
          GetEdit(dce0 + i, sz);
          l = NParseSz(sz, pmColor);
          if (k)
            kElemA[i] = l;
          else
            EnsureN(l, FValidColor(l), "element color");
        }
        for (i = 1; i <= cAspect; i++) {
          GetEdit(dca01 + i - 1, sz);
          l = NParseSz(sz, pmColor);
          if (k)
            kAspA[i] = l;
          else
            EnsureN(l, FValidColor(l), "aspect color");
        }
        for (i = 0; i < cColor; i++) {
          GetEdit(dck00 + i, sz);
          l = NParseSz(sz, pmColor);
          if (k) {
            j = ikPalette[i];
            if (j <= 0)
              kMainA[-j] = l;
            else
              kRainbowA[j] = l;
          } else
            EnsureN(l, FValidColor(l), "palette color");
        }
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the chart info entry dialog, as brought up by  */
/* both the Info Set Chart Info and Info Set Chart #2 Info menu commands. */

bool API DlgInfo(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  CI ci;
  char sz[cchSzMax];

  switch (message) {
  case WM_INITDIALOG:
    if (wi.nDlgChart < 2)
      ci = ciMain;
    else {
      sprintf(sz, "Enter Chart #%d Info", wi.nDlgChart);
      SetWindowText(hdlg, sz);
      if (wi.nDlgChart == 2)
        ci = ciTwin;
      else if (wi.nDlgChart == 3)
        ci = ciThre;
      else
        ci = ciFour;
    }
LInit:
    SetEditMDYT(hdlg, dcInMon, dcInDay, dcInYea, dcInTim,
      ci.mon, ci.day, ci.yea, ci.tim);
    SetEditSZOA(hdlg, dcInDst, dcInZon, dcInLon, dcInLat,
      ci.dst, ci.zon, ci.lon, ci.lat);
    SetEditSz(hdlg, deInNam, ciMain.nam);
    SetEditSz(hdlg, deInLoc, ciMain.loc);
    SetFocus(GetDlgItem(hdlg, dcInMon));
    return fFalse;

  case WM_COMMAND:
    if (wParam == dbInNow || wParam == dbInSet) {
      if (wParam == dbInNow) {
        GetTimeNow(&ci.mon, &ci.day, &ci.yea, &ci.tim, us.zonDef-us.dstDef);
        ci.dst = us.dstDef; ci.zon = us.zonDef;
        ci.lon = us.lonDef; ci.lat = us.latDef;
      } else
        ci = ciSave;
      goto LInit;
    }

    if (wParam == IDOK) {
      GetEdit(dcInMon, sz); ci.mon = NParseSz(sz, pmMon);
      GetEdit(dcInDay, sz); ci.day = NParseSz(sz, pmDay);
      GetEdit(dcInYea, sz); ci.yea = NParseSz(sz, pmYea);
      GetEdit(dcInTim, sz); ci.tim = RParseSz(sz, pmTim);
      GetEdit(dcInDst, sz); ci.dst = RParseSz(sz, pmDst);
      GetEdit(dcInZon, sz); ci.zon = RParseSz(sz, pmZon);
      GetEdit(dcInLon, sz); ci.lon = RParseSz(sz, pmLon);
      GetEdit(dcInLat, sz); ci.lat = RParseSz(sz, pmLat);
      EnsureN(ci.mon, FValidMon(ci.mon), "month");
      EnsureN(ci.yea, FValidYea(ci.yea), "year");
      EnsureN(ci.day, FValidDay(ci.day, ci.mon, ci.yea), "day");
      EnsureR(ci.tim, FValidTim(ci.tim), "time");
      EnsureR(ci.dst, FValidZon(ci.dst), "daylight");
      EnsureR(ci.zon, FValidZon(ci.zon), "zone");
      EnsureR(ci.lon, FValidLon(ci.lon), "longitude");
      EnsureR(ci.lat, FValidLat(ci.lat), "latitude");
      GetEdit(deInNam, sz);
      ci.nam = SzPersist(sz);
      GetEdit(deInLoc, sz);
      ci.loc = SzPersist(sz);
      switch (wi.nDlgChart) {
      case 1:  ciMain = ciCore = ci; break;
      case 2:  ciTwin = ci; break;
      case 3:  ciThre = ci; break;
      default: ciFour = ci; break;
      }
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the default chart info dialog, as brought up */
/* with the Info Default Chart Info menu command.                       */

bool API DlgDefault(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  CI ci;
  char sz[cchSzMax];

  switch (message) {
  case WM_INITDIALOG:
    SetEditSZOA(hdlg, dcDeDst, dcDeZon, dcDeLon, dcDeLat,
      us.dstDef, us.zonDef, us.lonDef, us.latDef);
    SetCombo(dcDeDst, "Auto");
    SetEditN(dcDeCor, (int)us.lTimeAddition);
    SetCombo(dcDeCor, "60"); SetCombo(dcDeCor, "-60");
    SetFocus(GetDlgItem(hdlg, dcDeDst));
    return fFalse;

  case WM_COMMAND:
    if (wParam == IDOK) {
      GetEdit(dcDeDst, sz); ci.dst = RParseSz(sz, pmDst);
      GetEdit(dcDeZon, sz); ci.zon = RParseSz(sz, pmZon);
      GetEdit(dcDeLon, sz); ci.lon = RParseSz(sz, pmLon);
      GetEdit(dcDeLat, sz); ci.lat = RParseSz(sz, pmLat);
      GetEdit(dcDeCor, sz); us.lTimeAddition = atol(sz);
      EnsureR(ci.dst, FValidZon(ci.dst), "daylight");
      EnsureR(ci.zon, FValidZon(ci.zon), "zone");
      EnsureR(ci.lon, FValidLon(ci.lon), "longitude");
      EnsureR(ci.lat, FValidLat(ci.lat), "latitude");
      us.dstDef = ci.dst; us.zonDef = ci.zon;
      us.lonDef = ci.lon; us.latDef = ci.lat;
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the charts #3 and #4 info entry dialog, as */
/* brought up by the Info Charts #3 and #4 menu commands.             */

bool API DlgInfoAll(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  DLGPROC dlgproc;
  int i;

  switch (message) {
  case WM_INITDIALOG:
    i = us.nRel;
    if (i > rcDual)
      i = 0;
    else if (i < rcQuadWheel)
      i = rcDual;
    SetRadio(dr01-i, dr01, dr04);
    return fTrue;

  case WM_COMMAND:
    if (FBetween(wParam, dbIa_o1, dbIa_o4)) {
      wi.nDlgChart = wParam - dbIa_o1 + 1;
      DlgOpenChart();
    } else if (FBetween(wParam, dbIa_i1, dbIa_i4)) {
      wi.nDlgChart = wParam - dbIa_i1 + 1;
      WiDoDialog(DlgInfo, dlgInfo);
    }
    if (wParam == IDOK) {
      i = GetCheck(dr01) ? 1 : (GetCheck(dr02) ? 2 : (GetCheck(dr03) ? 3 : 4));
      SetRel(-(i-1));
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the aspect settings dialog, as brought up with */
/* the Setting Aspect Settings menu command.                              */

bool API DlgAspect(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  int i, j;
  real r;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 1; i <= cAspect; i++) {
      SetCheck(dxa01 - 1 + i, i > us.nAsp);
      SetEditR(hdlg, deo01 - 1 + i, rAspOrb[i], -2);
      SetEditR(hdlg, dea01 - 1 + i, rAspAngle[i], -6);
      SetEditR(hdlg, dei01 - 1 + i, rAspInf[i], 2);
    }
    return fTrue;

  case WM_COMMAND:
    switch (wParam) {
    case dbAs_RA0:
      for (i = 1; i <= cAspect; i++)
        SetCheck(dxa01 - 1 + i, fTrue);
      break;
    case dbAs_RA1:
      for (i = 1; i <= cAspect; i++)
        SetCheck(dxa01 - 1 + i, fFalse);
      break;
    case dbAs_RA:
      for (i = 1; i <= 5; i++)
        SetCheck(dxa01 - 1 + i, !GetCheck(dxa01 - 1 + i));
      break;
    }

    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = 1; i <= cAspect; i++) {
          r = GetEditR(hdlg, deo01 - 1 + i);
          if (j)
            rAspOrb[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "orb");
        }
        for (i = 1; i <= cAspect; i++) {
          r = GetEditR(hdlg, dea01 - 1 + i);
          if (j)
            rAspAngle[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "angle");
        }
        for (i = 1; i <= cAspect; i++) {
          r = GetEditR(hdlg, dei01 - 1 + i);
          if (j)
            rAspInf[i] = r;
        }
      }
      for (us.nAsp = cAspect; us.nAsp > 0 && GetCheck(dxa01 - 1 + us.nAsp);
        us.nAsp--)
        ;
      for (i = 1; i <= us.nAsp; i++) {
        if (GetCheck(dxa01 - 1 + i))
          rAspOrb[i] = -rDegHalf;
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the object settings dialog, as brought up with */
/* the Setting Object Settings menu command.                              */

bool API DlgObject(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  int i, j;
  real r;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 0; i <= oCore; i++) {
      SetEditR(hdlg, deo01 + i, rObjOrb[i], -2);
      SetEditR(hdlg, dea01 + i, rObjAdd[i], -1);
      SetEditR(hdlg, dei01 + i, rObjInf[i], -2);
    }
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = 0; i <= oCore; i++) {
          r = GetEditR(hdlg, deo01 - 1 + i);
          if (j)
            rObjOrb[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "max orb");
        }
        for (i = 0; i <= oCore; i++) {
          r = GetEditR(hdlg, dea01 - 1 + i);
          if (j)
            rObjAdd[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "orb addition");
        }
        for (i = 0; i <= oCore; i++) {
          r = GetEditR(hdlg, dei01 - 1 + i);
          if (j)
            rObjInf[i] = r;
        }
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the cusp and uranian object settings dialog, as */
/* brought up with the Setting More Object Settings menu command.          */

bool API DlgObject2(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  int i, j;
  real r;

  switch (message) {
  case WM_INITDIALOG:
    for (i = oAsc; i <= uranHi; i++) {
      SetEditR(hdlg, deo01 - oAsc + i, rObjOrb[i], -2);
      SetEditR(hdlg, dea01 - oAsc + i, rObjAdd[i], -1);
      SetEditR(hdlg, dei01 - oAsc + i, rObjInf[i], -2);
    }
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = oAsc; i <= uranHi; i++) {
          r = GetEditR(hdlg, deo01 - oAsc + i);
          if (j)
            rObjOrb[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "max orb");
        }
        for (i = oAsc; i <= uranHi; i++) {
          r = GetEditR(hdlg, dea01 - oAsc + i);
          if (j)
            rObjAdd[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "orb addition");
        }
        for (i = oAsc; i <= uranHi; i++) {
          r = GetEditR(hdlg, dei01 - oAsc + i);
          if (j)
            rObjInf[i] = r;
        }
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the object restrictions dialog, as invoked with */
/* both the Setting Restrictions and Transit Restrictions menu commands.   */

bool API DlgRestrict(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  byte *lpb, *lpb2;
  int i;

  switch (message) {
  case WM_INITDIALOG:
    if (wi.wCmd == cmdRes)
      lpb = ignore;
    else {
      SetWindowText(hdlg, "Transit Object Restrictions");
      lpb = ignore2;
    }
    for (i = 0; i <= oNorm; i++)
      SetCheck(dx01 + i, lpb[i]);
    return fTrue;

  case WM_COMMAND:
    switch (wParam) {
    case dbRe_R0:
      for (i = 0; i <= oNorm; i++)
        SetCheck(dx01 + i, fTrue);
      break;
    case dbRe_R1:
      for (i = 0; i <= oNorm; i++)
        SetCheck(dx01 + i, fFalse);
      break;
    case dbRe_R:
      for (i = oMain+1; i <= oCore; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbRe_RC:
      for (i = cuspLo; i <= cuspHi; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbRe_Ru:
      for (i = uranLo; i <= uranHi; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbRT:
      lpb2 = wi.wCmd == cmdRes ? ignore2 : ignore;
      for (i = 0; i <= oNorm; i++)
        SetCheck(dx01 + i, lpb2[i]);
      break;
    }

    if (wParam == IDOK) {
      lpb = wi.wCmd == cmdRes ? ignore : ignore2;
      for (i = 0; i <= oNorm; i++)
        lpb[i] = GetCheck(dx01 + i);
      if (!us.fCusp) {
        for (i = cuspLo; i <= cuspHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fCusp = fTrue;
            WiCheckMenu(cmdResCusp, fTrue);
            break;
          }
      } else {
        for (i = cuspLo; i <= cuspHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > cuspHi) {
          us.fCusp = fFalse;
          WiCheckMenu(cmdResCusp, fFalse);
        }
      }
      if (!us.fUranian) {
        for (i = uranLo; i <= uranHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fUranian = fTrue;
            WiCheckMenu(cmdResUranian, fTrue);
            break;
          }
      } else {
        for (i = uranLo; i <= uranHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > uranHi) {
          us.fUranian = fFalse;
          WiCheckMenu(cmdResUranian, fFalse);
        }
      }
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the star restrictions dialog, as brought up with */
/* the Setting Star Restrictions menu command.                              */

bool API DlgStar(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  int i;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 1; i <= cStar; i++)
      SetCheck(dx01 - 1 + i, ignore[oNorm + i]);
    return fTrue;

  case WM_COMMAND:
    switch (wParam) {
    case dbSt_RU0:
      for (i = 1; i <= cStar; i++)
        SetCheck(dx01 - 1 + i, fTrue);
      break;
    case dbSt_RU1:
      for (i = 1; i <= cStar; i++)
        SetCheck(dx01 - 1 + i, fFalse);
      break;
    }

    if (wParam == IDOK) {
      for (i = 1; i <= cStar; i++)
        ignore[oNorm + i] = GetCheck(dx01 - 1 + i);
      if (!us.nStar) {
        for (i = starLo; i <= starHi; i++)
          if (!ignore[i]) {
            us.nStar = fTrue;
            WiCheckMenu(cmdResStar, fTrue);
            break;
          }
      } else {
        for (i = starLo; i <= starHi; i++)
          if (!ignore[i])
            break;
        if (i > starHi) {
          us.nStar = fFalse;
          WiCheckMenu(cmdResStar, fFalse);
        }
      }
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the standard settings dialog, as brought up with */
/* the Setting Calculation Settings menu command.                           */

bool API DlgSetting(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  real r1;
  int n2, n3, n4, n5, n6;

  switch (message) {
  case WM_INITDIALOG:
    SetCombo(dcSe_s, "0.0");        // Fagan-Bradley
    SetCombo(dcSe_s, "0.883333");   // Nirmala Chandra Lahiri (Robert Hand)
    SetCombo(dcSe_s, "0.98");       // Krishnamurti
    SetCombo(dcSe_s, "2.329444");   // B.V. Raman
    SetCombo(dcSe_s, "-3.619166");  // Djwhal Khul
    SetEditR(hdlg, dcSe_s, us.rZodiacOffset, 6);
    SetEditN(deSe_A, us.nAsp);
    SetEditN(deSe_x, us.nHarmonic);
    SetEdit(deSe_h, szObjName[us.objCenter]);
    SetRadio(us.objOnAsc == 0 ? dr01 : (us.objOnAsc > 0 ? dr02 : dr03),
      dr01, dr03);
    SetEdit(deSe_1, szObjName[us.objOnAsc == 0 ? oSun : abs(us.objOnAsc)-1]);
    SetEditN(deSe_I, us.nScreenWidth);
    SetCheck(dxSe_b, us.fEphemFiles);
    SetRadio(us.nDegForm == 0 ? dr04 : (us.nDegForm == 1 ? dr05 : dr06),
      dr04, dr06);
    SetCheck(dxSe_sr, us.fEquator);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      r1 = GetEditR(hdlg, dcSe_s);
      GetEdit(deSe_A, sz); n2 = NParseSz(sz, pmAspect);
      n3 = GetEditN(deSe_x);
      GetEdit(deSe_h, sz); n4 = NParseSz(sz, pmObject);
      GetEdit(deSe_1, sz); n5 = NParseSz(sz, pmObject);
      n6 = GetEditN(deSe_I);
      EnsureR(r1, FValidOffset(r1), "zodiac offset");
      EnsureN(n2, FValidAspect(n2), "aspect count");
      EnsureN(n3, FValidHarmonic(n3), "harmonic factor");
      EnsureN(n4, FValidCenter(n4), "central planet");
      EnsureN(n5, FItem(n5), "Solar chart planet");
      EnsureN(n6, FValidScreen(n6), "text columns");
      us.rZodiacOffset = r1;
      us.nAsp = n2;
      us.nHarmonic = n3;
      us.objCenter = n4;
      us.objOnAsc = GetCheck(dr01) ? 0 : (GetCheck(dr02) ? n5+1 : -n5-1);
      us.nScreenWidth = n6;
      us.fEphemFiles = GetCheck(dxSe_b);
      us.nDegForm = GetCheck(dr04) ? 0 : (GetCheck(dr05) ? 1 : 2);
      us.fEquator = GetCheck(dxSe_sr);
      WiCheckMenu(cmdHeliocentric, us.objCenter != oEar);
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the obscure settings dialog, as brought up with */
/* the Setting Obscure Settings menu command.                              */

bool API DlgObscure(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  int i, n1;

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxOb_Yn, us.fTrueNode);
    SetCheck(dxOb_Yd, us.fEuroDate);
    SetCheck(dxOb_Yt, us.fEuroTime);
    SetCheck(dxOb_YC, us.fSmartCusp);
    SetCheck(dxOb_Y8, us.fClip80);
    SetCheck(dxOb_Yo, us.fWriteOld);
    SetCheck(dxOb_Yc0, us.fHouseAngle);
    SetCheck(dxOb_YR0_s, us.fIgnoreSign);
    SetCheck(dxOb_YR0_d, us.fIgnoreDir);
    SetEditN(deOb_YXg, gs.nGridCell);
    SetCheck(dxOb_YXf, gs.fFont);
    SetEditR(hdlg, deOb_YXp0_x, gs.xInch, 2);
    SetEditR(hdlg, deOb_YXp0_y, gs.yInch, 2);
    SetRadio(gs.nOrient == 0 ? dr12 : (gs.nOrient > 0 ? dr10 : dr11),
      dr10, dr12);
    SetRadio(dr01 + (gs.nGlyphs/1000 == 2), dr01, dr02);
    SetRadio(dr03 + ((gs.nGlyphs/100)%10 == 2), dr03, dr04);
    SetRadio(dr05 + ((gs.nGlyphs/10)%10 -1), dr05, dr07);
    SetRadio(dr08 + (gs.nGlyphs%10 == 2), dr08, dr09);
    for (i = 0; i < 4; i++)
      SetCheck(dx01 + i, ignorez[i]);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      n1 = GetEditN(deOb_YXg);
      EnsureN(n1, FValidGrid(n1), "grid cell");
      us.fTrueNode = GetCheck(dxOb_Yn);
      us.fEuroDate = GetCheck(dxOb_Yd);
      us.fEuroTime = GetCheck(dxOb_Yt);
      us.fSmartCusp = GetCheck(dxOb_YC);
      us.fClip80 = GetCheck(dxOb_Y8);
      us.fWriteOld = GetCheck(dxOb_Yo);
      us.fHouseAngle = GetCheck(dxOb_Yc0);
      us.fIgnoreSign = GetCheck(dxOb_YR0_s);
      us.fIgnoreDir = GetCheck(dxOb_YR0_d);
      gs.nGridCell = n1;
      gs.fFont = GetCheck(dxOb_YXf);
      gs.xInch = GetEditR(hdlg, deOb_YXp0_x);
      gs.yInch = GetEditR(hdlg, deOb_YXp0_y);
      gs.nOrient = GetCheck(dr12) ? 0 : (GetCheck(dr10) ? 1 : -1);
      gs.nGlyphs = (GetCheck(dr01) ? 1 : 2) * 1000 +
        (GetCheck(dr03) ? 1 : 2) * 100 + 
        (GetCheck(dr05) ? 1 : GetCheck(dr06) ? 2 : 3) * 10 +
        (GetCheck(dr08) ? 1 : 2);
      for (i = 0; i < 4; i++)
        ignorez[i] = GetCheck(dx01 + i);
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the transit chart dialog, as brought up with the */
/* Chart Transits menu command.                                             */

bool API DlgTransit(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int mon, day, yea, n1, n2;
  real tim;

  switch (message) {
  case WM_INITDIALOG:
    if (us.fInDay)           n1 = 1;
    else if (us.fInDayInf)   n1 = 2;
    else if (us.fTransit)    n1 = 3;
    else if (us.fTransitInf) n1 = 4;
    else                     n1 = 0;
    SetRadio(dr01 + n1, dr01, dr05);
    SetCheck(dxTr_p, is.fProgress);
    SetCheck(dxTr_r, is.fReturn);
    SetEditMDYT(hdlg, dcTrMon, dcTrDay, dcTrYea, dcTrTim,
      MonT, DayT, YeaT, TimT);
    if (n1 == 1) {
      n2 = is.fProgress || us.fInDayMonth;
      if (n2 == 1 && MonT == 0)
        n2 += 1 + (us.nEphemYears > 1);
    } else if (n1 == 3)
      n2 = 1 + (MonT <= 0) + (MonT < 0);
    else
      n2 = 0;
    SetRadio(dr06 + n2, dr06, dr09);
    SetEditN(deTr_tY, us.nEphemYears);
    SetEditN(deTr_d, us.nDivision);
    SetFocus(GetDlgItem(hdlg, dcTrMon));
    return fFalse;

  case WM_COMMAND:
    if (wParam == dbTr_tn) {
      GetTimeNow(&mon, &day, &yea, &tim, us.zonDef-us.dstDef);
      SetEditMDYT(hdlg, dcTrMon, dcTrDay, dcTrYea, dcTrTim,
        mon, day, yea, tim);
    }

    if (wParam == IDOK) {
      GetEdit(dcTrMon, sz); mon = NParseSz(sz, pmMon);
      GetEdit(dcTrDay, sz); day = NParseSz(sz, pmDay);
      GetEdit(dcTrYea, sz); yea = NParseSz(sz, pmYea);
      GetEdit(dcTrTim, sz); tim = RParseSz(sz, pmTim);
      n1 = GetEditN(deTr_tY);
      n2 = GetEditN(deTr_d);
      EnsureN(mon, FValidMon(mon), "month");
      EnsureN(yea, FValidYea(yea), "year");
      EnsureN(day, FValidDay(day, mon, yea), "day");
      EnsureR(tim, FValidTim(tim), "time");
      EnsureN(n2, FValidDivision(n2), "searching divisions");
      SetCI(ciTran, mon, day, yea, tim,
        us.dstDef, us.zonDef, us.lonDef, us.latDef);
      us.nEphemYears = n1;
      us.nDivision = n2;
      is.fProgress = GetCheck(dxTr_p);
      is.fReturn = GetCheck(dxTr_r);
      n1 = GetCheck(dr01) ? 0 : (GetCheck(dr02) ? 1 : (GetCheck(dr03) ? 2 :
        (GetCheck(dr04) ? 3 : 4)));
      switch (n1) {
      case 1: wi.nMode = gTraTraHit; break;
      case 2: wi.nMode = gTraTraInf; break;
      case 3: wi.nMode = gTraNatHit; break;
      case 4: wi.nMode = gTraNatInf; break;
      default: wi.nMode = gWheel;
      }
      n2 = GetCheck(dr06) ? 0 : (GetCheck(dr07) ? 1 :
        (GetCheck(dr08) ? 2 : 3));
      if (n1 == 1) {
        us.fInDayMonth = (is.fProgress || n2 >= 1);
        if (n2 >= 2) {
          MonT = 0;
          if (n2 == 2)
            us.nEphemYears = 1;
        }
      } else if (n1 == 3) {
        if (n2 == 2)
          MonT = 0;
        else if (n2 == 3) {
          MonT = -1; DayT = us.nEphemYears;
        }
      } else if (n1 == 2) {
        us.fProgress = is.fProgress;
        wi.fCast = fTrue;
      }
      us.fGraphics = fFalse;
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the progression settings dialog, as brought up */
/* with the Chart Progressions menu command.                              */

bool API DlgProgress(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int mon, day, yea;
  real tim, r1;

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxPr_p, us.fProgress);
    SetEditMDYT(hdlg, dcPrMon, dcPrDay, dcPrYea, dcPrTim,
      MonT, DayT, YeaT, TimT);
    SetRadio(dr01 + us.fSolarArc, dr01, dr02);
    SetEditR(hdlg, dcPr_pd, us.rProgDay, 5);
    SetCombo(dcPr_pd, "365.24219");
    SetCombo(dcPr_pd, "27.321661");
    SetCombo(dcPr_pd, "29.530588");
    SetFocus(GetDlgItem(hdlg, dcPrMon));
    return fFalse;

  case WM_COMMAND:
    if (wParam == dbPr_pn) {
      GetTimeNow(&mon, &day, &yea, &tim, us.zonDef-us.dstDef);
      SetEditMDYT(hdlg, dcPrMon, dcPrDay, dcPrYea, dcPrTim,
        mon, day, yea, tim);
    }

    if (wParam == IDOK) {
      GetEdit(dcPrMon, sz); mon = NParseSz(sz, pmMon);
      GetEdit(dcPrDay, sz); day = NParseSz(sz, pmDay);
      GetEdit(dcPrYea, sz); yea = NParseSz(sz, pmYea);
      GetEdit(dcPrTim, sz); tim = RParseSz(sz, pmTim);
      r1 = GetEditR(hdlg, dcPr_pd);
      EnsureN(mon, FValidMon(mon), "month");
      EnsureN(yea, FValidYea(yea), "year");
      EnsureN(day, FValidDay(day, mon, yea), "day");
      EnsureR(tim, FValidTim(tim), "time");
      EnsureR(r1, r1 != 0.0, "degree per day");
      SetCI(ciTran, mon, day, yea, tim,
        us.dstDef, us.zonDef, us.lonDef, us.latDef);
      us.rProgDay = r1;
      us.fProgress = GetCheck(dxPr_p);
      us.fSolarArc = GetCheck(dr02);
      is.JDp = MdytszToJulian(MonT, DayT, YeaT, TimT, us.dstDef, us.zonDef);
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the chart subsettings dialog, as brought up with */
/* the Chart Chart Settings menu command.                                   */

bool API DlgChart(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  int n1, n2, n3;
  bool f;

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxCh_v0, us.fVelocity);
    SetEditN(deCh_w, us.nWheelRows);
    SetCheck(dxCh_w0, us.fWheelReverse);
    SetCheck(dxCh_g0, us.fGridConfig);
    SetCheck(dxCh_a0, us.fAspSummary);
    SetCheck(dxCh_m0, us.fMidSummary);
    SetCheck(dxCh_ma, us.fMidAspect);
    SetCheck(dxCh_Z0, us.fPrimeVert);
    SetCheck(dxCh_l, us.fSectorApprox);
    SetCheck(dxCh_j0, us.fInfluenceSign);
    SetEditN(deCh_L, us.nAstroGraphStep);
    SetCheck(dxCh_L0, us.fLatitudeCross);
    SetCheck(dxCh_Ky, us.fCalendarYear);
    SetEditN(deCh_P, us.nArabicParts);
    SetCheck(dxCh_P0, us.fArabicFlip);
    switch (us.nStar) {
    case 'z': n1 = dr02; break;
    case 'l': n1 = dr03; break;
    case 'n': n1 = dr04; break;
    case 'b': n1 = dr05; break;
    default:  n1 = dr01;
    }
    SetRadio(n1, dr01, dr05);
    switch (us.nArabic) {
    case 'z': n2 = dr07; break;
    case 'n': n2 = dr08; break;
    case 'f': n2 = dr09; break;
    default:  n2 = dr06;
    }
    SetRadio(n2, dr06, dr09);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      n1 = GetEditN(deCh_w);
      n2 = GetEditN(deCh_L);
      n3 = GetEditN(deCh_P);
      EnsureN(n1, FValidWheel(n1), "wheel row");
      EnsureN(n2, FValidAstrograph(n2), "astro-graph step");
      EnsureN(n3, FValidPart(n3), "Arabic part");
      f = GetCheck(dxCh_v0);
      if (us.fVelocity != f) {
        us.fVelocity = f;
        WiCheckMenu(cmdGraphicsSidebar, !f);
      }
      us.nWheelRows = n1;
      us.fWheelReverse = GetCheck(dxCh_w0);
      us.fGridConfig = GetCheck(dxCh_g0);
      us.fAspSummary = GetCheck(dxCh_a0);
      us.fMidSummary = GetCheck(dxCh_m0);
      us.fMidAspect = GetCheck(dxCh_ma);
      us.fPrimeVert = GetCheck(dxCh_Z0);
      us.fSectorApprox = GetCheck(dxCh_l);
      us.fInfluenceSign = GetCheck(dxCh_j0);
      us.nAstroGraphStep = n2;
      us.fLatitudeCross = GetCheck(dxCh_L0);
      us.fCalendarYear = GetCheck(dxCh_Ky);
      us.nArabicParts = n3;
      us.fArabicFlip = GetCheck(dxCh_P0);
      if (us.nStar)
        us.nStar = GetCheck(dr02) ? 'z' : (GetCheck(dr03) ? 'l' :
          (GetCheck(dr04) ? 'n' : (GetCheck(dr05) ? 'b' : fTrue)));
      if (us.nArabic)
        us.nArabic = GetCheck(dr07) ? 'z' : (GetCheck(dr08) ? 'n' :
          (GetCheck(dr09) ? 'f' : fTrue));
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the graphic settings dialog, as brought up with */
/* the Graphics Graphics Settings menu command.                            */

bool API DlgGraphics(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int n1, n2, n3, n5, n6;
  real r4;

  switch (message) {
  case WM_INITDIALOG:
    SetEditN(deGr_Xw_x, gs.xWin);
    SetEditN(deGr_Xw_y, gs.yWin);
    SetEditN(deGr_XW, gs.nRot);
    SetEditR(hdlg, deGr_XG, gs.rTilt, 2);
    SetCheck(dxGr_XW0, gs.fMollewide);
    SetEditN(deGr_WN, wi.nTimerDelay);
    SetRadio(gs.objLeft > 0 ? dr02 :
      (gs.objLeft < 0 ? dr03 : dr01), dr01, dr03);
    SetEdit(deGr_X1, szObjName[gs.objLeft == 0 ? oSun : abs(gs.objLeft)-1]);
    SetCheck(dxGr_Wn, wi.fNoUpdate);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      n1 = GetEditN(deGr_Xw_x);
      n2 = GetEditN(deGr_Xw_y);
      n3 = GetEditN(deGr_XW);
      r4 = GetEditR(hdlg, deGr_XG);
      n5 = GetEditN(deGr_WN);
      GetEdit(deGr_X1, sz); n6 = NParseSz(sz, pmObject);
      EnsureN(n1, FValidGraphx(n1), "horizontal size");
      EnsureN(n2, FValidGraphy(n2), "vertical size");
      EnsureN(n3, FValidRotation(n3), "horizontal rotation");
      EnsureR(r4, FValidTilt(r4), "vertical tilt");
      EnsureN(n5, FValidTimer(n5), "animation delay");
      EnsureN(n6, FItem(n6), "rotation planet");
      if (gs.xWin != n1 || gs.yWin != n2) {
        gs.xWin = n1; gs.yWin = n2;
        if (wi.fWindowChart)
          ResizeWindowToChart();
      }
      gs.nRot = n3; gs.rTilt = r4;
      if (wi.nTimerDelay != (UINT)n5) {
        wi.nTimerDelay = n5;
        if (wi.lTimer != 0)
          KillTimer(wi.hwnd, 1);
        wi.lTimer = SetTimer(wi.hwnd, 1, wi.nTimerDelay, NULL);
      }
      gs.objLeft = GetCheck(dr01) ? 0 : (GetCheck(dr02) ? n6+1 : -n6-1);
      gs.fMollewide = GetCheck(dxGr_XW0);
      wi.fNoUpdate = GetCheck(dxGr_Wn);
      us.fGraphics = wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


/* Processing function for the about dialog, showing copyrights and  */
/* credits, as brought up with the Help About Astrolog menu command. */

bool API DlgAbout(HWND hdlg, _int message, WORD wParam, LONG lParam)
{
  switch (message) {
  case WM_INITDIALOG:
#ifdef DEBUG
    SetWindowText(hdlg, "About Astrolog (DEBUG)");
#endif
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}
#endif /* WIN */

/* wdialog.cpp */
