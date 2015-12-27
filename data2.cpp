/*
** Astrolog (Version 6.00) File: data2.cpp
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
** Constellation Tables.
******************************************************************************
*/

#ifdef CONSTEL
CONST char *szCnstlName[cCnstl+1] = {"",
  "Andromeda", "Antilia", "Apus", "Aquarius",
  "Aquila", "Ara", "Aries", "Auriga",
  "Bootes", "Caelum", "Camelopardalis", "Cancer",
  "Canes Venatici", "Canis Major", "Canis Minor", "Capricornus",
  "Carina", "Cassiopeia", "Centaurus", "Cepheus",
  "Cetus", "Chamaeleon", "Circinus", "Columba",
  "Coma Berenices", "Corona Australis", "Corona Borealis", "Corvus",
  "Crater", "Crux", "Cygnus", "Delphinus",
  "Dorado", "Draco", "Equuleus", "Eridanus",
  "Fornax", "Gemini", "Grus", "Hercules",
  "Horologium", "Hydra", "Hydrus", "Indus",
  "Lacerta", "Leo", "Leo Minor", "Lepus",
  "Libra", "Lupus", "Lynx", "Lyra",
  "Mensa", "Microscopium", "Monoceros", "Musca",
  "Norma", "Octans", "Ophiuchus", "Orion",
  "Pavo", "Pegasus", "Perseus", "Phoenix",
  "Pictor", "Pisces", "Piscis Austrinus", "Puppis",
  "Pyxis", "Reticulum", "Sagitta", "Sagittarius",
  "Scorpius", "Sculptor", "Scutum", "Serpens Caput/Cauda",
  "Sextans", "Taurus", "Telescopium", "Triangulum",
  "Triangulum Australe", "Tucana", "Ursa Major", "Ursa Minor",
  "Vela", "Virgo", "Volans", "Vulpecula"};

CONST char *szCnstlAbbrev[cCnstl+1] = {"",
  "And", "Ant", "Aps", "Aqu", "Aql", "Ara", "Ari", "Aur",
  "Boo", "Cae", "Cam", "Cnc", "CVn", "CMa", "CMi", "Cap",
  "Car", "Cas", "Cen", "Cep", "Cet", "Cha", "Cir", "Col",
  "Com", "CrA", "CrB", "Crv", "Crt", "Cru", "Cyg", "Del",
  "Dor", "Dra", "Equ", "Eri", "For", "Gem", "Gru", "Her",
  "Hor", "Hya", "Hyi", "Ind", "Lac", "Leo", "LMi", "Lep",
  "Lib", "Lup", "Lyn", "Lyr", "Men", "Mic", "Mon", "Mus",
  "Nor", "Oct", "Oph", "Ori", "Pav", "Peg", "Per", "Phe",
  "Pic", "Psc", "PsA", "Pup", "Pyx", "Ret", "Sge", "Sgr",
  "Sco", "Scl", "Sct", "Ser", "Sex", "Tau", "Tel", "Tri",
  "TrA", "Tuc", "UMa", "UMi", "Vel", "Vir", "Vol", "Vul"};

CONST char *szCnstlMeaning[cCnstl+1] = {"",
  "Chained Maiden", "Air Pump", "Bird of Paradise", "Water Bearer",
  "Eagle", "Altar", "Ram", "Charioteer",
  "Herdsman", "Chisel", "Giraffe", "Crab",
  "Hunting Dogs", "Great Dog", "Little Dog", "Sea Goat",
  "Keel", "Queen", "Centaur", "King",
  "Sea Monster", "Chameleon", "Drawing Compass", "Dove",
  " Berenice's Hair", "Southern Crown", "Northern Crown", "Crow",
  "Cup", "Southern Cross", "Swan", "Dolphin",
  "Dolphinfish", "Dragon", "Little Horse", "River",
  "Furnace", "Twins", "Crane", "Strongman",
  "Clock", "Sea Serpent", "Water Snake", "Indian",
  "Lizard", "Lion", "Little Lion", "Hare",
  "Scales", "Wolf", "Lynx", "Lyre",
  "Table Mountain", "Microscope", "Unicorn", "Fly",
  "Level", "Octant", "Serpent Bearer", "Hunter",
  "Peacock", "Winged Horse", "Hero", "Phoenix",
  "Painter", "Fishes", "Southern Fish", "Stern",
  "Compass", "Net", "Arrow", "Archer",
  "Scorpion", "Sculptor", "Shield", " Head/Tail of the Snake",
  "Sextant", "Bull", "Telescope", "Triangle",
  "Southern Triangle", "Toucan", "Great Bear", "Little Bear",
  "Sail", "Virgin", "Flying Fish", "Fox"};

CONST char *szCnstlGenitive[cCnstl+1] = {"",
  "", "", "2odis", "", "", "", "1tis", "",
  "2is", "", "", "2ri", "2um1orum", " is", " is", "",
  "", "", "", "", "", "tis", "", "",
  "", "", "", "", "is", "", "", "",
  "1us", "nis", "", "", "", "1orum", "1is", "2is",
  "", "", "", "", "", "nis", "nis0is", "2oris",
  "", "", "", "", "", "", "1tis", "",
  "", "1is", "", "is", "nis", "", "", "",
  "is", "2ium", "", "", "1dis", "", "", "",
  "", "is", "", "!1is", "1tis", "", "", "",
  "2i1is", "", " is", " is", "1orum", "1inis", "1tis", ""};
#endif /* CONSTEL */


/*
******************************************************************************
** Object Calculation Tables.
******************************************************************************
*/

#ifdef MATRIX
CONST byte rErrorCount[oPlu-oJup+1] = {11, 5, 4, 4, 4};
CONST byte rErrorOffset[oPlu-oJup+1] = {0, 72, 72+51, 72+51+42, 72+51+42+42};

CONST real rErrorData[72+51+42*3] = {
-.001,-.0005,.0045,.0051,581.7,-9.7,-.0005,2510.7,-12.5,-.0026,1313.7,-61.4,
0.0013,2370.79,-24.6,-.0013,3599.3,37.7,-.001,2574.7,31.4,-.00096,6708.2,
-114.5,-.0006,5499.4,-74.97,-.0013,1419,54.2,.0006,6339.3,-109,.0007,4824.5,
-50.9,.0020,-.0134,.0127,-.0023,676.2,.9,.00045,2361.4,174.9,.0015,1427.5,
-188.8,.0006,2110.1,153.6,.0014,3606.8,-57.7,-.0017,2540.2,121.7,-.00099,
6704.8,-22.3,-.0006,5480.2,24.5,.00096,1651.3,-118.3,.0006,6310.8,-4.8,.0007,
4826.6,36.2, /* Jupiter error */

-.0009,.0037,0,.0134,1238.9,-16.4,-.00426,3040.9,-25.2,.0064,1835.3,36.1,
-.0153,610.8,-44.2,-.0015,2480.5,-69.4,-.0014,.0026,0,.0111,1242.2,78.3,
-.0045,3034.96,62.8,-.0066,1829.2,-51.5,-.0078,640.6,24.2,-.0016,2363.4,
-141.4,.0006,-.0002,0,-.0005,1251.1,43.7,.0005,622.8,13.7,.0003,1824.7,-71.1,
.0001,2997.1,78.2, /* Saturn error */

-.0021,-.0159,0,.0299,422.3,-17.7,-.0049,3035.1,-31.3,-.0038,945.3,60.1,
-.0023,1227,-4.99,.0134,-.02186,0,.0317,404.3,81.9,-.00495,3037.9,57.3,.004,
993.5,-54.4,-.0018,1249.4,79.2,-.0003,.0005,0,.0005,352.5,-54.99,.0001,3027.5,
54.2,-.0001,1150.3,-88, /* Uranus error */

0.1832,-.6718,.2726,-.1923,175.7,31.8,.0122,542.1,189.6,.0027,1219.4,178.1,
-.00496,3035.6,-31.3,-.1122,.166,-.0544,-.00496,3035.3,58.7,.0961,177.1,-68.8,
-.0073,630.9,51,-.0025,1236.6,78,.00196,-.0119,.0111,.0001,3049.3,44.2,-.0002,
893.9,48.5,.00007,1416.5,-25.2, /* Neptune error */

-.0426,.073,-.029,.0371,372,-331.3,-.0049,3049.6,-39.2,-.0108,566.2,318.3,
0.0003,1746.5,-238.3,-.0603,.5002,-.6126,.049,273.97,89.97,-.0049,3030.6,61.3,
0.0027,1075.3,-28.1,-.0007,1402.3,20.3,.0145,-.0928,.1195,.0117,302.6,-77.3,
0.00198,528.1,48.6,-.0002,1000.4,-46.1 /* Pluto error */
};

OE rgoe[oVes+cUran-2] = {
{358.4758,35999.0498,-.0002,.01675,-.4E-4,0,1,101.2208,1.7192,.00045,0,0,0,0,
0,0}, /* Earth/Sun */
{102.2794,149472.515,0,.205614,.2E-4,0,.3871,28.7538,.3703,.0001,47.1459,
1.1852,0.0002,7.009,.00186,0}, /* Mercury */
{212.6032,58517.8039,.0013,.00682,-.5E-4,0,.7233,54.3842,.5082,-.14E-2,
75.7796,0.8999,.4E-3,3.3936,.1E-2,0}, /* Venus */
{319.5294,19139.8585,.2E-3,.09331,.9E-4,0,1.5237,285.4318,1.0698,.1E-3,
48.7864,0.77099,0,1.8503,-.7E-3,0}, /* Mars */

{225.4928,3033.6879,0,.04838,-.2E-4,0,5.2029,273.393,1.3383,0,99.4198,1.0583,
0,1.3097,-.52E-2,0}, /* Jupiter */
{174.2153,1223.50796,0,.05423,-.2E-3,0,9.5525,338.9117,-.3167,0,112.8261,
.8259,0,2.4908,-.0047,0}, /* Saturn */
{74.1757,427.2742,0,.04682,.00042,0,19.2215,95.6863,2.0508,0,73.5222,.5242,0,
0.7726,.1E-3,0}, /* Uranus */
{30.13294,240.45516,0,.00913,-.00127,0,30.11375,284.1683,-21.6329,0,130.68415,
1.1005,0,1.7794,-.0098,0}, /* Neptune */
{229.781,145.1781,0,.24797,.002898,0,39.539,113.5366,.2086,0,108.944,1.3739,0,
17.1514,-.0161,0}, /* Pluto */

{34.6127752,713.5756219,0,.382270369,-.004694073,0,13.66975144,337.407213,
2.163306646,0,208.1482658,1.247724355,0,6.911179715,.011236955,0}, /* Chiron */
{108.2925,7820.36556,0,.0794314,0,0,2.7672273,71.0794444,0,0,80.23555556,
1.3960111,0,10.59694444,0,0}, /* Ceres */
{106.6641667,7806.531667,0,.2347096,0,0,2.7704955,310.166111,0,0,172.497222,
1.39601111,0,34.81416667,0,0}, /* Pallas Athena */
{267.685,8256.081111,0,.2562318,0,0,2.6689897,245.3752778,0,0,170.137777,
1.396011111,.0003083333,13.01694444,0,0}, /* Juno */
{138.7733333,9924.931111,0,.0902807,0,0,2.360723,149.6386111,0,0,103.2197222,
1.396011111,.000308333,7.139444444,0,0}, /* Vesta */

{104.5959,138.5369,0,0,0,0,40.99837, 0,0,0,0,0,0,0,0,0}, /* Cupido   */
{337.4517,101.2176,0,0,0,0,50.667443,0,0,0,0,0,0,0,0,0}, /* Hades    */
{104.0904,80.4057, 0,0,0,0,59.214362,0,0,0,0,0,0,0,0,0}, /* Zeus     */
{17.7346, 70.3863, 0,0,0,0,64.816896,0,0,0,0,0,0,0,0,0}, /* Kronos   */
{138.0354,62.5,    0,0,0,0,70.361652,0,0,0,0,0,0,0,0,0}, /* Apollon  */
{-8.678,  58.3468, 0,0,0,0,73.736476,0,0,0,0,0,0,0,0,0}, /* Admetos  */
{55.9826, 54.2986, 0,0,0,0,77.445895,0,0,0,0,0,0,0,0,0}, /* Vulkanus */
{165.3595,48.6486, 0,0,0,0,83.493733,0,0,0,0,0,0,0,0,0}  /* Poseidon */
};
#endif /* MATRIX */

CONST real rStarBright[cStar+1] = {0,
  0.46, 2.02, 5.24, 5.09, 0.85, 0.08, 0.12, 1.64, 1.65, 1.70,
  0.50, 1.90, 1.98,-0.72, 1.93,-1.46, 1.50, 1.84, 1.59, 0.38,
  1.14, 1.78, 1.86, 1.68, 1.98, 1.35, 1.79, 1.58, 1.63, 1.25,
  1.77, 0.98, 1.86, 0.61,-0.04,-0.01, 0.96, 1.63, 1.87, 1.85,
  0.03, 0.77, 1.94, 1.25, 1.74, 1.16, 4.61};

CONST real rStarData[cStar*6] = {
 1,37,42.9,-57,14,12,   2,31,50.5, 89,15,51,   3,17,46.1,-62,34,32,
 3,49,11.1, 24, 8,12,   4,35,55.2, 16,30,33,   5,16,41.3, 45,59,53,
 5,14,32.2, -8,12, 6,   5,25, 7.8,  6,20,59,   5,26,17.5, 28,36,27,
 5,36,12.7, -1,12, 7,   5,55,10.3,  7,24,25,   5,59,31.7, 44,56,51,
 6,22,41.9,-17,57,22,   6,23,57.2,-52,41,44,   6,37,42.7, 16,23,57,
 6,45, 8.9,-16,42,58,   6,58,37.5,-28,58,20,   7, 8,23.4,-26,23,35,
 7,34,35.9, 31,53,18,   7,39,18.1,  5,13,30,   7,45,18.9, 28, 1,34,
 8, 9,31.9,-47,20,12,   8,22,30.8,-59,30,34,   9,13,12.1,-69,43, 2,
 9,27,35.2, -8,39,31,  10, 8,22.3, 11,58, 2,  11, 3,43.6, 61,45, 3,
12,26,35.9,-63, 5,56,  12,31, 9.9,-57, 6,47,  12,47,43.3,-59,41,19,
12,54, 1.7, 55,57,35,  13,25,11.5,-11, 9,41,  13,47,32.3, 49,18,48,
14, 3,49.4,-60,22,22,  14,15,39.6, 19,10,57,  14,39,36.2,-60,50, 7,
16,29,24.4,-26,25,55,  17,33,36.4,-37, 6,13,  17,37,19.0,-42,59,52,
18,24,10.3,-34,23, 5,  18,36,56.2, 38,47, 1,  19,50,46.9,  8,52, 6,
20,25,38.8,-56,44, 7,  20,41,25.8, 45,16,49,  22, 8,13.9,-46,57,40,
22,57,39.0,-29,37,20,   0,42, 7.0, 41,16, 0};


#ifdef INTERPRET
/*
******************************************************************************
** Interpretation Tables.
******************************************************************************
*/

char *szMindPart[oNorm+1] = {"arena of practical life experience",
  "spirit, ego, image of self, and sense of aliveness",
  "emotional nature, moods, feelings, and caring tendencies",
  "thoughts, intellect, and communicative activity",
  "creativity, tendencies for affection and calmness, and relationship needs",
  "will, energy, activity, and aggressive, assertive tendencies",
  "enthusiastic, faithful, wise, expansive, spontaneous nature",
  "part of mind that is disciplined, respectful, and solitary",
  "individuality, desires for change, and tendency to go against social norms",
  "intuitive, spiritual, compassionate, psychic nature",
  "destiny, and capacity to transform the self and the outer world",
  "ability to help, heal, and teach others, and where one has much experience",
  "tendency to direct energy to instinctive, creative, reproductive goals",
  "tendency to direct energy away from emotional and into mental pursuits",
  "tendency to give away individual power for the benefit of a relationship",
  "capacity to direct creative energy into personal or devotional goals",
  "karmic goals, and best direction of evolutionary growth",
  "karmic past, and area of experience but little growth",
  "capacity to let go and accept their natural darker side",
  "place where opportunity and success can be found",
  "karmic fate and method of reception of energies",
  "generalized projected personality and outward appearance",
  "personality and outward appearance, as projected to the world",
  "", "", "", "", "", "", "", "",
  "view as seen in the eyes of others, reputation, and social standing",
  "", "",
  "will to purify and forge both physically and spiritually",
  "group associations and matters relating to art",
  "dark, secretive, past, shameful side",
  "fiery, creative, leadership tendencies",
  "lofty, extraordinary parts",
  "progressive, abundant energies",
  "intense, focused nature",
  "greatly forceful energies",
  "idealistic, honorable side"};

char *szDesc[cSign+1] = {"",
  "forceful, energetic, direct, courageous",
  "practical, often skeptical and stubborn",
  "inquisitive, witty, perceptive, adaptable",
  "introspective, emotional, protective",
  "proud, gregarious, dramatic, dignified",
  "analytical, critical, modest, helpful",
  "affectionate, tolerant, often indecisive, appreciates beauty",
  "penetrating, suspicious, introspective",
  "jovial, open minded, loves freedom",
  "industrious, practical, disciplined",
  "progressive, erratic, revolutionary, idealistic, humanitarian, inventive",
  "imaginative, other worldly, impressionable"};

char *szDesire[cSign+1] = {"",
  "seeks adventure and challenge",
  "loves serenity and inner peace",
  "seeks out information",
  "wants inner and outer security",
  "desires self expression",
  "works toward perfection",
  "seeks balance, relationship, and calmness",
  "desires to transform and remove outer masks",
  "seeks meaning and new experience",
  "works for solitude and personal integrity",
  "desires individuality",
  "seeks transcendence of self"};

char *szLifeArea[cSign+1] = {"",
  "establishment of personal identity",
  "self image, self worth, and material security",
  "communicating to and receiving from the environment",
  "imagination, fantasies, inner feelings, and domestic life",
  "finding joy, pleasure, and creative expression",
  "work and feeling talented and useful",
  "personal relationships and intimacy",
  "sex, death, the occult, and other hard to grasp topics",
  "changes, exploration, and the breaking of routine",
  "career, social stature, and destiny",
  "the future, life goals, and association with friends and groups",
  "things that disrupt or cause disassociation with the personality"};

char *szInteract[cAspectInt+1] = {"",
  "is %sconnected and fused together with",
  "%sopposes and creates tension with",
  "is %sin conflict with",
  "is %sin harmony with",
  "has %sopportunity for growth, exchange, and harmony in relation with",
  "is %sdifferent from",
  "%sgets new perspectives in relation with",
  "%screates internal friction with",
  "%screates internal agitation with",
  "%screatively relates externally with",
  "%screatively relates internally with"};

char *szTherefore[cAspectInt+1] = {"",
  "Both parts are prominent in their psyche", "Balance is needed",
  "Adaptation is required by both sides", "", "",
  "They can often relate in a discordant way", "", "", "", "", ""};

/* Modifier array makes the interpretation stronger for narrower orbs. */

char *szModify[3][cAspectInt] =
  {{"always ", "always ", "irreconcilably ", "always ", "much ",
  "completely ", "often ", "often ", "often ", "often ", "often "},
  {"", "", "", "", "", "", "", "", "", "", ""},
  {"somewhat ", "somewhat ", "somewhat ", "somewhat ", "some ", "somewhat ",
  "sometimes ", "sometimes ", "sometimes ", "sometimes ", "sometimes "}};
#endif /* INTERPRET */

/* data2.cpp */
