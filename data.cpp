/*
** Astrolog (Version 6.00) File: data.cpp
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


/*
******************************************************************************
** Global Variables.
******************************************************************************
*/

#ifdef __TURBOC__
extern unsigned _stklen = 0x4000;
#endif

US us = {

  /* Chart types */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  /* Chart suboptions */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  /* Table chart types */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  /* Main flags */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,

  /* Main subflags */
#ifdef SWITCHES
  fFalse,
#else
  fTrue,
#endif
  0, 0, 0, 0, 0, 0, 0, 0, 0,

  /* Rare flags */
#ifdef TRUENODE
  fTrue,
#else
  fFalse,
#endif
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  /* Value settings */
  0,
  0,
  0,
  DEFAULT_SYSTEM,
  DEFAULT_ASPECTS,
  oEar,
  0,
  1,
  0,
  0,
  0,
  DIVISIONS,
  SCREENWIDTH,
  0.0,
  DEFAULT_ZONE,
  DEFAULT_LONG,
  DEFAULT_LAT,

  /* Value subsettings */

  4, 5, cPart, 0.0, 365.24219, 1, 1, 24, 0L, 0, BIODAYS};

IS is = {
  fFalse, fFalse, fFalse, fFalse, fFalse, fFalse, fFalse, fFalse,
  NULL, NULL, NULL, NULL, 0, 0, 0, 0.0, 0.0, 0.0,
  NULL, 0.0, 0.0, 0.0, 0.0, 0.0};

CI ciCore = {11, 19, 1971, 11.01, 0.0, 8.0, 122.19984, 47.36584, "", ""};
CI ciMain = {-1, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, "", ""};
CI ciTwin = {9, 11, 1991, 0.01, 0.0, 0.0, 122.19984, 47.36584, "", ""};
CI ciThre = {-1, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, "", ""};
CI ciFour = {-1, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, "", ""};
CI ciTran = {12, 31, 2015, 23.59, 0.0, 0.0, 0.0, 0.0, "", ""};
CI ciSave = {12, 21, 2015, 20.4795, 0.0, 8.0, 122.19984, 47.36584, "", ""};
CP cp0, cp1, cp2;


/*
******************************************************************************
** Global Arrays.
******************************************************************************
*/

real spacex[oNorm+1], spacey[oNorm+1], spacez[oNorm+1], force[objMax];
GridInfo *grid = NULL;
int starname[cStar+1], kObjA[objMax];
char *szMacro[48];

/* Restriction status of each object, as specified with -R switch. */

byte ignore[objMax] = {1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                   /* Planets  */
  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,                /* Minors   */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             /* Cusps    */
  0, 0, 0, 0, 0, 0, 0, 0,                         /* Uranians */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* Stars    */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/* Restriction of objects when transiting, as specified with -RT switch. */

byte ignore2[objMax] = {1,
  0, 1, 0, 0, 0, 0, 0, 0, 0, 0,                   /* Planets  */
  0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1,                /* Minors   */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,             /* Cusps    */
  0, 0, 0, 0, 0, 0, 0, 0,                         /* Uranians */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* Stars    */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

byte ignorez[4] = {0, 0, 0, 0};    /* Restrictions for -Zd chart events. */

/* Gauquelin sector plus zones, as specified with -Yl switch. */

byte pluszone[cSector+1] = {0,
  1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1,
  1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1};


/*
******************************************************************************
** Global Tables.
******************************************************************************
*/

CONST char *szAppName = szAppNameCore;

CONST char *szSignName[cSign+1] = {"",
  "Aries", "Taurus", "Gemini", "Cancer", "Leo",
  "Virgo", "Libra", "Scorpio",
  "Sagittarius", "Capricorn", "Aquarius", "Pisces"};

CONST char *szSignAbbrev[cSign+1] = {"",
  "Ar", "Ta", "Ge", "Cn", "Le", "Vi", "Li", "Sc", "Sg", "Cp", "Aq", "Pi"};

CONST char *szSignEnglish[cSign+1] = {"",
  "Ram", "Bull", "Twins", "Crab", "Lion", "Virgin",
  "Scales", "Scorpion", "Archer", "Sea Goat", "Water Bearer", "Fishes"};

CONST char *szHouseTradition[cSign+1] = {"",
  "Personality", "Money", "Communication", "Home",
  "Children", "Servants", "Marriage", "Death",
  "Long Journeys Over Water", "Career", "Friends", "Troubles"};

CONST char *szObjName[objMax] = {
  "Earth", "Sun", "Moon", "Mercury", "Venus", "Mars",       /* Planets   */
  "Jupiter", "Saturn", "Uranus", "Neptune", "Pluto",
  "Chiron", "Ceres", "Pallas", "Juno", "Vesta",             /* Asteroids */
  "North Node", "South Node",                               /* Nodes     */
  "Lilith", "Fortune", "Vertex", "East Point",              /* Others    */
  "Ascendant", "2nd Cusp", "3rd Cusp", "Nadir",             /* Cusps     */
  "5th Cusp", "6th Cusp", "Descendant", "8th Cusp",
  "9th Cusp", "Midheaven", "11th Cusp", "12th Cusp",
  "Vulcan", "Cupido", "Hades", "Zeus", "Kronos",            /* Uranians  */
  "Apollon", "Admetos", "Vulkanus", "Poseidon",

  "Achernar", "Polaris", "Zeta Retic.", "Pleiades",         /* Stars     */
  "Aldebaran", "Capella", "Rigel", "Bellatrix", "Alnath",
  "Orion", "Betelgeuse", "Menkalinan", "Murzim", "Canopus",
  "Alhena", "Sirius", "Adara", "Wezen", "Castor",
  "Procyon", "Pollux", "Suhail", "Avior", "Miaplacidus",
  "Alphard", "Regulus", "Dubhe", "Acrux", "Gacrux",
  "Becrux", "Alioth", "Spica", "Alkaid", "Agena",
  "Arcturus", "Rigel Kent.", "Antares", "Shaula", "Sargas",
  "Kaus Austr.", "Vega", "Altair", "Peacock", "Deneb",
  "Alnair", "Fomalhaut", "Andromeda"};

CONST char *szSystem[cSystem] = {
  "Placidus", "Koch", "Equal", "Campanus", "Meridian",
  "Regiomontanus", "Porphyry", "Morinus", "Topocentric", "Alcabitius",
  "Equal (MC)", "Neo-Porphyry", "Whole", "Vedic", "Null"};

CONST char *szAspectName[cAspect+1] = {"",
  "Conjunct", "Opposite", "Square", "Trine", "Sextile",
  "Inconjunct", "Semisextile", "Semisquare", "Sesquiquadrate",
  "Quintile", "Biquintile",
  "Semiquintile", "Septile", "Novile",
  "Binovile", "Biseptile", "Triseptile", "Quatronovile"};

CONST char *szAspectAbbrev[cAspect+3] = {"",
  "Con", "Opp", "Squ", "Tri", "Sex",
  "Inc", "SSx", "SSq", "Ses", "Qui", "BQn",
  "SQn", "Sep", "Nov", "BNv", "BSp", "TSp", "QNv",
  "Par", "CPr"};

CONST char *szAspectGlyph[cAspect+1] = {"",
  "Circle with extending line", "Two circles joined by line",
  "Quadrilateral", "Triangle", "Six pointed asterisk",
  "'K' rotated right", "'K' rotated left", "Acute angle",
  "Square with extending lines", "Letter 'Q'", "'+' over '-'",
  "'-' over '+'", "Number '7'", "Number '9'", "'9' under Roman 'II'",
  "'7' under Roman 'II'", "'7' under Roman 'III'", "'9' under Roman 'IV'"};

CONST char *szAspectConfig[cAspConfig+1] = {"",
  "Stellium", "Grand Trine", "T-Square", "Yod", "Grand Cross", "Cradle"};

CONST char *szElem[4] = {"Fire", "Earth", "Air", "Water"};

CONST char *szMode[3] = {"Cardinal", "Fixed", "Mutuable"};

CONST char *szMonth[cSign+1] = {"",
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"};

CONST char *szDay[cWeek] = {"Sunday", "Monday", "Tuesday", "Wednesday",
  "Thursday", "Friday", "Saturday"};

CONST char *szZon[cZone] = {
  "HST", "HT", "H", "CAT", "AHS", "HDT", "AHD", "YST", "YT", "Y", "YDT",
  "PST", "PT", "P", "PDT", "PWT", "MST", "MT", "M", "MDT", "MWT",
  "CST", "CT", "C", "CDT", "CWT", "EST", "ET", "E", "EDT", "EWT",
  "AST", "AT", "A", "ADT", "AWT", "BST", "BT", "B", "BDT", "WAT",
  "GMT", "GT", "G", "WET", "CET", "EET", "UZ3", "UZ4",
  "IST", "IT", "I", "UZ5", "NST", "SST", "CCT", "JST", "JT", "J",
  "SAS", "GST", "UZ1", "NZT", "ZT", "Z", "IDL", "LMT", "LT", "L"};

CONST real rZon[cZone] = {
  10.30, 10.30, 10.30, 10.0, 10.0, 9.30, 9.0, 9.0, 9.0, 9.0, 8.0,
  8.0, 8.0, 8.0, 7.0, 7.0, 7.0, 7.0, 7.0, 6.0, 6.0,
  6.0, 6.0, 6.0, 5.0, 5.0, 5.0, 5.0, 5.0, 4.0, 4.0,
  4.0, 4.0, 4.0, 3.0, 3.0, 3.0, 3.0, 3.0, 2.0, 1.0,
  0.0, 0.0, 0.0, 0.0, -1.0, -2.0, -4.0, -5.0,
  -5.30, -5.30, -5.30, -6.0, -6.30, -7.0, -8.0, -9.0, -9.0, -9.0,
  -9.30, -10.0, -11.0, -11.30, -11.30, -11.30, -12.0, 24.0, 24.0, 24.0};

CONST char *szDir[4] = {"North", "East", "South", "West"};

CONST char *szSuffix[cSign+1] = {"",
  "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "th", "th"};

real rAspAngle[cAspect+1] = {0,
  0.0, 180.0, 90.0, 120.0, 60.0, 150.0, 30.0, 45.0, 135.0, 72.0, 144.0,
  36.0, rDegMax/7.0, 40.0, 80.0, 720.0/7.0, 1080.0/7.0, 160.0};

real rAspOrb[cAspect+1] = {0,
  7.0, 7.0, 7.0, 7.0, 6.0, 3.0, 3.0, 3.0, 3.0, 2.0, 2.0,
  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

real rObjOrb[oNorm+2] = {360.0,
  360.0, 360.0, 360.0, 360.0, 360.0, 360.0, 360.0, 360.0, 360.0, 360.0,
  360.0, 360.0, 360.0, 360.0, 360.0, 2.0, 2.0, 360.0, 360.0, 2.0,
  360.0, 360.0, 360.0, 360.0, 360.0, 360.0,
  360.0, 360.0, 360.0, 360.0, 360.0, 360.0,
  360.0, 360.0, 360.0, 360.0, 360.0, 360.0, 360.0, 360.0, 360.0,
  2.0};

real rObjAdd[oNorm+2] = {0.0,
  1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  0.0};

int ruler1[oNorm+1] = {sSag,
  sLeo, sCan, sGem, sLib, sAri, sSag, sCap, sAqu, sPis, sSco,
  sPis, sTau, sVir, sLib, sSco, sAqu, sLeo, sSco, sPis, sLib, sAri,
  sAri, sTau, sGem, sCan, sLeo, sVir, sLib, sSco, sSag, sCap, sAqu, sPis,
  sVir, sLib, sSco, sLeo, sCap, sSag, sVir, sAri, sSag};
int ruler2[oNorm+1] = {0,
  0, 0, sVir, sTau, sSco, sPis, sAqu, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0};
int exalt[oNorm+1] = {sGem,
  sAri, sTau, sVir, sPis, sCap, sCan, sLib, sSco, sCan, sAqu,
  sCan, sCan, sCap, sLeo, sAqu, sGem, sSag, sPis, sSag, sPis, sCap,
  sLeo, sVir, sLib, sSco, sSag, sCap, sAqu, sPis, sAri, sTau, sGem, sCan,
  sCap, sGem, sVir, sAri, sSag, sAqu, sSco, sLeo, sPis};

  /* This array is the reverse of the ruler arrays:   */
  /* Here, given a sign, return what planet rules it. */
int rules[cSign+1] = {0,
  oMar, oVen, oMer, oMoo, oSun, oMer, oVen, oPlu, oJup, oSat, oUra, oNep};

  /* Esoteric astrology */
int rgObjRay[oNorm+1] = {3,
  2, 4, 4, 5, 6, 2, 3, 7, 6, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0, 0};
int rgSignRay[cSign+1] = {0,
  17, 4, 2, 37, 15, 26, 3, 4, 456, 137, 5, 26};
int rgSignRay2[cSign+1][cRay+1];
int rgObjEso1[oNorm+1] = {sSag,
  sLeo, sVir, sAri, sGem, sSco, sAqu, sCap, sLib, sCan, sPis,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  sTau, 0, 0, 0, 0, 0, 0, 0, 0};
int rgObjEso2[oNorm+1] = {0,
  0, 0, 0, 0, 0, 0, 0, 0, sLeo, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  sVir, 0, 0, 0, 0, 0, 0, 0, 0};
int rgObjHie1[oNorm+1] = {sGem,
  sLeo, sAqu, sSco, sCap, sSag, sVir, sLib, sAri, sCan, sPis,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  sTau, 0, 0, 0, 0, 0, 0, 0, 0};
int rgObjHie2[oNorm+1] = {0,
  0, 0, 0, 0, 0, 0, 0, sLeo, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  sAqu, 0, 0, 0, 0, 0, 0, 0, 0};
int rgSignEso1[cSign+1] = {0,
  oMer, oVul, oVen, oNep, oSun, oMoo, oUra, oMar, oEar, oSat, oJup, oPlu};
int rgSignEso2[cSign+1] = {0,
  0, 0, 0, 0, oNep, 0, 0, 0, 0, 0, 0, 0};
int rgSignHie1[cSign+1] = {0,
  oUra, oVul, oEar, oNep, oSun, oJup, oSat, oMer, oMar, oVen, oMoo, oPlu};
int rgSignHie2[cSign+1] = {0,
  0, 0, 0, 0, oUra, 0, 0, 0, 0, 0, oVul, 0};
int kRayA[cRay+2] = {kBlack,
  kRed, kBlue, kGreen, kYellow, kOrange, kMagenta, kPurple, kWhite};
CONST char *szRayName[cRay+1] = {"",
  "Will & Power", "Love & Wisdom", "Active Creative Intelligence",
  "Harmony Through Conflict", "Concrete Science", "Idealism & Devotion",
  "Order & Ceremonial Magic"};
CONST char *szRayWill[cRay+1] = {"",
  "Initiate", "Unify", "Evolve", "Harmonize", "Act", "Cause", "Express"};

  /* Colors */
CONST char *szColor[cColor+2] = {"Black",
  "Maroon", "DkGreen", "Orange", "DkBlue", "Purple", "DkCyan", "LtGray",
  "Gray", "Red", "Green", "Yellow", "Blue", "Magenta", "Cyan", "White",
  "Element", "Ray"};
int kMainA[9] = {kBlack, kWhite, kLtGray, kDkGray,
  kMaroon, kDkGreen, kDkCyan, kDkBlue, kMagenta};
int kRainbowA[8] = {kWhite,
  kRed, kOrange, kYellow, kGreen, kCyan, kBlue, kPurple};
int kElemA[4] = {kRed, kYellow, kGreen, kBlue};
int kAspA[cAspect+1] = {kWhite,
  kYellow, kBlue, kRed, kGreen, kCyan,
  kMagenta, kMagenta, kOrange, kOrange, kDkCyan, kDkCyan,
  kDkCyan, kMaroon, kPurple, kPurple, kMaroon, kMaroon, kPurple};
int kObjU[oNorm+1] = {kYellow,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  kMagenta, kMagenta, kMagenta, kMagenta, kMagenta,
  kDkCyan, kDkCyan, kDkCyan, kDkCyan, kDkCyan, kDkCyan,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  kPurple,
  kPurple, kPurple, kPurple, kPurple, kPurple, kPurple, kPurple, kPurple};

/* Influence information used by ChartInfluence() follows. The influence of */
/* a planet in its ruling or exalting sign or house is tacked onto the last */
/* two positions of the object and house influence array, respectively.     */

  /* The inherent strength of each planet - */
real rObjInf[oNorm+6] = {0,
  30, 25, 10, 10, 10, 10, 10, 10, 10, 10,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  20, 10, 10, 10, 10, 10, 10, 10, 10, 15, 10, 10,
  3, 3, 3, 3, 3, 3, 3, 3,
  20, 10, 10, 10, 10};

  /* The inherent strength of each house - */
real rHouseInf[cSign+6]  = {0,
  20, 0, 0, 10, 0, 0, 5, 0, 0, 15, 0, 0,
  15, 5, 5, 5, 5};

  /* The inherent strength of each aspect - */
real rAspInf[cAspect+1] = {0.0,
  1.0, 0.8, 0.8, 0.6, 0.6, 0.4, 0.4, 0.2, 0.2,
  0.2, 0.2, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};

  /* The inherent strength of each planet when transiting - */
real rTransitInf[oNorm+3] = {0,
  10, 4, 8, 9, 20, 30, 35, 40, 45, 50,
  30, 15, 15, 15, 15, 30,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  50, 50, 50, 50, 50, 50, 50, 50};

  /* Informational astronomical data for planets - */
CONST real rObjDist[oVes+1] = {149.59787, 0.0, 0.3844,
  57.91, 108.2, 227.94, 778.33, 1426.98, 2870.99, 4497.07, 5913.52,
  13.670*149.59787, 2.767*149.59787, 2.770*149.59787, 2.669*149.59787,
  2.361*149.59787};
CONST real rObjYear[oVes+1] = {1, 0, 27.32166/365.25,
  87.969/365.25, 224.701/365.25, 686.98/365.25, 11.8623,
  29.458, 84.01, 164.79, 248.54,
  51.0, 4.60, 4.61, 4.36, 3.63};
CONST real rObjDiam[oVes+1] = {12756.0, 1392000.0, 3476.0,
  4878.0, 12102.0, 6786.0, 142984.0, 120536.0, 51118.0, 49528.0, 2372.0,
  320.0, 955.0, 538.0, 226.0, 503.0};
CONST real rObjDay[oPlu+1] = {24.0, 30.0*24.0, 27.322*24.0,
  58.65*24.0, 243.01*24.0, 24.6229, 9.841, 10.233, 17.9, 19.2, 6.3872*24.0};
CONST real rObjMass[oPlu+1] = {1.0, 322946.0, 0.0123,
  0.0553, 0.8149, 0.1074, 317.938, 95.181, 14.531, 17.135, 0.0022};
CONST real rObjAxis[oPlu+1] = {23.5, 0.0, 6.7,
  2.0, 2.7, 25.19, 3.12, 26.73, 82.14, 29.6, 57.54};
CONST byte cSatellite[oPlu+1] = {1, 9, 0,
  0, 0, 2, 16, 18, 15, 8, 1};

#ifdef ARABIC
CONST AI ai[cPart] = {
  {"    02 01F ", "Fortune"},
  {"    01 02F ", "Spirit"},
  {"    06 S F ", "Victory"},
  {"    F  05F ", "Valor & Bravery"},
  {"    05 03Fh", "Mind & Administrators"},
  {"   h02r02Fh", "Property & Goods"},
  {"    06 07 h", "Siblings"},
  {"   j03 01Fh", "Death of Siblings"},
  {"    06 07Fh", "Death of Parents"},
  {"    07h02Fh", "Grandparents"},
  {"    06 03Fh", "Real Estate"},
  {"    07 06Fh", "Children & Life"},
  {"   R02 02 h", "Expected Birth"},
  {"    05 03 h", "Disease & Defects (1)"},
  {"    05 07 h", "Disease & Defects (2)"},
  {"   R07 07 h", "Captivity"},
  {"    02 03 h", "Servants"},
  {"   h07 04 h", "Partners"},
  {"   h08 02 h", "Death"},
  {"    05 07Fh", "Sickness & Murder"},
  {"    03 07Fh", "Danger, Violence & Debt"},
  {"   h09r09 h", "Journeys"},
  {"   105 07Fh", "Travel by Water"},
  {"    03 02Fh", "Faith, Trust & Belief"},
  {"    02 07Fh", "Deep Reflection"},
  {"    01 07Fh", "Understanding & Wisdom"},
  {"    06 01Fh", "Fame & Recognition"},
  {"    02 05Fh", "Rulers & Disassociation"},
  {"    07 01Fh", "Father, Fate & Karma"}, /* Combust */
  {"    F  07Fh", "Sudden Advancement"},
  {"    01 07 h", "Celebrity of Rank"},
  {"    07 05Fh", "Surgery & Accident"},
  {"    04 03Fh", "Merchants & Their Work"},
  {"    F  S Fh", "Merchandise (Exchange)"}, /* Moon */
  {"    02 04Fh", "Mother"},
  {"    S  F Fh", "Glory & Constancy"},
  {"    01 F Fh", "Honorable Acquaintances"},
  {"    06 F Fh", "Success"},
  {"    04 F Fh", "Worldliness"},
  {"    03 02 h", "Acquaintances"},
  {"    03 S  h", "Violence"},
  {"    01 03Fh", "Liberty of Person"},
  {"    04 06Fh", "Praise & Acceptance"},
  {"   h12r12 h", "Enmity"},
  {"    F  S  h", "Bad Luck"},
  {"    05 F F ", "Debilitated Bodies"},
  {"    02D  F ", "Boldness & Violence"},
  {"    S  03F ", "Trickery & Deceit"},
  {"   h03 05  ", "Necessities"},
  {"    03 F   ", "Realization of Needs"},
  {"    01 05F ", "Retribution"},
  {"    06 02  ", "Children (Male)"},
  {"    04 02  ", "Children (Female)"},
  {"    05 04  ", "Play & Variety"}, /* Change */
  {"    07 03 h", "Stability"},
  {"   h05 06Fh", "Speculation"},
  {"    03 04Fh", "Art"},
  {"   h05r05 h", "Sexual Attraction"},
  {"    10 04 h", "Sex Drive & Stimulation"},
  {"    05 01 h", "Passion"},
  {"    05 04 h", "Emotion & Affection"},
  {"   r08 07 h", "Most Perilous Year"},
  {"   h08 07 h", "Peril"},
  {"    09 08 h", "Occultism"},
  {"    03 01 h", "Commerce"},
  {"h09h03 04 h", "Marriage Contracts"},
  {"   h09r09 h", "Travel by Land"},
  {"    08H08 h", "Travel by Air"},
  {" 30 01 02Fh", "Destiny"},
  {" 30 02 01Fh", "Vocation & Status"},
  {"   019 01 h", "Honor, Nobility (Day)"},
  {"   033 02 h", "Honor, Nobility (Night)"},
  {"    10 01 h", "Organization"},
  {"    04h07 h", "Divorce"},
  {"    08 01 h", "Ostracism & Loss"},
  {"    02 08Fh", "Friends"},
  {"    07 01 h", "Tragedy & Brethren"},
  {"    02 10Fh", "Race (Consciousness)"},
  {"    02D02Fh", "Bondage & Slavery"},
  {"    F  09 h", "Imprisonment & Sorrow"},
  {"    04 08 h", "Perversion"},
  {"   h12 09 h", "Self-Undoing"},
  {"    09 01 h", "Treachery & Entrapment"},
  {"h12r12 09 h", "Bereavement"},
  {"    06h12 h", "Suicide (Yang)"},
  {"   h08 09 h", "Suicide (Yin)"},
  {"    06 09 h", "Depression"},
  {" 05 09 08 h", "Assassination (Yang)"},
  {"   r12 09 h", "Assassination (Yin)"},
  {"    09 06  ", "Cancer (Disease)"},
  {"    08 07  ", "Catastrophe"},
  {"    07 08  ", "Foolhardiness"},
  {"    03 05  ", "Release & Luck"},
  {"    06 03  ", "Benevolence & Assurance"},
  {"    03 06  ", "Hope & Sensitivity"},
  {"    03 07  ", "Aptness & Aloofness"},
  {"    08 09  ", "Charm & Personality"},
  {"    02 03F ", "Faith & Good Manners"},
  {"    01 03  ", "Temperament"},
  {"    04 03  ", "Security & Treasure"},
  {"    08 03  ", "Originality"},
  {"    03 08  ", "Eccentricity, Astrology"},
  {"    09 03  ", "Divination"},
  {"    03 09  ", "Intrusion"},
  {"    05 06  ", "Negotiation"},
  {"    06 05  ", "Discord & Controversy"},
  {"    05 08  ", "Coincidence"},
  {"    08 05  ", "Unpreparedness"},
  {"    05 09  ", "Popularity"},
  {"    09 05  ", "Misunderstanding"},
  {"    04 06  ", "Sentiment & Marriage"},
  {"    06 04  ", "Loneliness"},
  {"    04 07  ", "Success in Investment"},
  {"    07 04  ", "Frugality & Labor"},
  {"    08 04  ", "Wastefulness"},
  {"    04 09  ", "Vanity"},
  {"    09 04  ", "Corruptness"},
  {"    01 05  ", "Initiative"},
  {"    05 02F ", "Memory"},
  {"    04 01  ", "Love, Beauty & Peace"},
  {"    01 04  ", "Disinterest & Boredom"},
  {"    01 06  ", "Accomplishment"},
  {"    07 02F ", "Influence"},
  {"    06 01  ", "Increase & Impression"},
  {"    09 07  ", "Caution"},
  {"    07 09  ", "Timidity"},
  {"    08 06  ", "Entertainment"},
  {"    06 08  ", "Bequest"},
  {"    01 09  ", "Genius"},
  {"    02 09F ", "Revelation"},
  {"    09 02F ", "Delusion"},
  {"    08 02F ", "Misinterpretation"},
  {"    01 08  ", "Intellectuality"},
  {"    06 07 E", "Earth"},
  {"    04 02 E", "Water"},
  {"   r04 03 E", "Air & Wind"},
  {"    05 01 E", "Fire"},
  {"    07 05FE", "Clouds"},
  {"    04 02FE", "Rains"},
  {"    07 03FE", "Cold"},
  {"    06 01FC", "Wheat"},
  {"    06 02FC", "Barley & Meats"},
  {"    04 06FC", "Rice & Millet"},
  {"    07 06FC", "Corn"},
  {"    07 05FC", "Lentils, Iron, Pungents"},
  {"    05 07FC", "Beans & Onions"},
  {"    01 03FC", "Chick Peas"},
  {"    04 07FC", "Sesame & Grapes"},
  {"    03 04FC", "Sugar & Legumes"},
  {"    01 02FC", "Honey"},
  {"    02 05FC", "Oils"},
  {"    04 05FC", "Nuts & Flax"},
  {"    02 03FC", "Olives"},
  {"    05 07FC", "Apricots & Peaches"},
  {"    03 06FC", "Melons"},
  {"    05 02FC", "Salt"},
  {"    03 01FC", "Sweets"},
  {"    07 03FC", "Astrigents & Purgatives"},
  {"    04 03FC", "Silk & Cotton"},
  {"    05 07FC", "Purgatives (Bitter)"},
  {"    06 07FC", "Purgatives (Acid)"},
  {"    30D   H", "Secrets"},
  {"    02 03FH", "Information True/False"},
  {"    F D  FH", "Injury to Business"},
  {" 03 07 06 H", "Freedmen & Servants"},
  {" 02 07 06 H", "Employers"},
  {"   h07 04 H", "Marriage"},
  {"    06 01 H", "Time for Action/Success"},
  {"    07 01 H", "Time Occupied in Action"},
  {" 07 06 01 H", "Dismissal & Resignation"},
  {"    05 02 H", "Life/Death of Absentee"},
  {"    05 01 H", "Lost Animal (Light)"},
  {"    05 07 H", "Lost Animal (Dark)"},
  {"    03 05 H", "Lawsuit"},
  {"h08 05 02 H", "Decapitation"},
  {" 30 07 02 H", "Torture"},
  {" 02h04D   H", "Lost Objects"}
};
#endif /* ARABIC */

/* data.cpp */
