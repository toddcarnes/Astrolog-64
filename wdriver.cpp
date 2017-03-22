/*
** Astrolog (Version 6.20) File: wdriver.cpp
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


#ifdef WIN
/*
******************************************************************************
** Windows Command Processing.
******************************************************************************
*/

/* Process one command line switch passed to the program dealing with the   */
/* Windows features. This is just like the processing of each switch in the */
/* main program, however here each switch has been prefixed with an 'W'.    */

int NProcessSwitchesW(int argc, char **argv, int pos,
  flag fOr, flag fAnd, flag fNot)
{
  int darg = 0, i;
  char sz[cchSzDef], ch1;

  ch1 = argv[0][pos+1];
  switch (argv[0][pos]) {
  case chNull:
    if (argc <= 1) {
      ErrorArgc("W");
      return tcError;
    }
    i = atoi(argv[1]);
    PostMessage(wi.hwnd, WM_COMMAND, i, 0L);
    darg++;
    break;

  case 'N':
    if (argc <= 1) {
      ErrorArgc("WN");
      return tcError;
    }
    i = atoi(argv[1]);
    if (!FValidTimer(i)) {
      ErrorValN("WN", i);
      return tcError;
    }
    wi.nTimerDelay = i;
    darg++;
    break;

  case 'M':
    if (argc <= 2) {
      ErrorArgc("WM");
      return tcError;
    }
    i = atoi(argv[1]);
    if (!FValidMacro(i)) {
      ErrorValN("WM", i);
      return tcError;
    }
    i--;
    sprintf(sz, "%s\t%sF%d", argv[2], i < 12 ? "" : (i < 24 ? "Shift+" :
      (i < 36 ? "Ctrl+" : "Alt+")), i % 12 + 1);
    ModifyMenu(wi.hmenu, (WORD)(cmdMacro01 + i), MF_BYCOMMAND | MF_STRING,
      (WORD)(cmdMacro01 + i), sz);
    darg += 2;
    break;

  case 'n':
    SwitchF(wi.fNoUpdate);
    break;

  case 'o':
    SwitchF(wi.fAutoSave);
    break;

  case 'S':
    if (ch1 == 'g')
      FCreateProgramGroup(fFalse);
    else if (ch1 == 'G')
      FCreateProgramGroup(fTrue);
    else if (ch1 == 'd')
      FCreateDesktopIcon();
    else if (ch1 == 'x')
      FRegisterExtensions();
    else if (ch1 == 'u')
      FUnregisterExtensions();
    break;

  default:
    ErrorSwitch(argv[0]);
    return tcError;
  }
  /* 'darg' contains the value to be added to argc when we return. */
  return darg;
}


/* Change the pixel size of the window so its internal drawable area is the */
/* dimensions of the current graphics chart. Both the upper left and lower  */
/* right corners of the window may change depending on the scroll position. */

void ResizeWindowToChart()
{
  HDC hdc;
  RECT rcOld, rcCli, rcNew;
  POINT pt;
  int xScr, yScr;

  if (!us.fGraphics || gs.xWin == 0 || gs.yWin == 0)
    return;
  hdc = GetDC(wi.hwnd);
  xScr = GetDeviceCaps(hdc, HORZRES);
  yScr = GetDeviceCaps(hdc, VERTRES);
  ReleaseDC(wi.hwnd, hdc);
  GetWindowRect(wi.hwnd, &rcOld);
  GetClientRect(wi.hwnd, &rcCli);
  pt.x = pt.y = 0;
  ClientToScreen(wi.hwnd, &pt);
  rcNew.left = rcOld.left + gi.xOffset;
  rcNew.top  = rcOld.top  + gi.yOffset;
  rcNew.right = rcNew.left + gs.xWin + (gi.nMode == 0 ? SIDESIZE : 0) +
    (rcOld.right - rcOld.left - rcCli.right);
  rcNew.bottom = rcNew.top + gs.yWin +
    (rcOld.bottom - rcOld.top - rcCli.bottom);
  if (rcNew.right > xScr)
    OffsetRect(&rcNew, xScr - rcNew.right, 0);
  if (rcNew.bottom > yScr)
    OffsetRect(&rcNew, 0, yScr - rcNew.bottom);
  if (rcNew.left < 0)
    OffsetRect(&rcNew, -rcNew.left, 0);
  if (rcNew.top < 0)
    OffsetRect(&rcNew, 0, -rcNew.top);
  MoveWindow(wi.hwnd, rcNew.left, rcNew.top,
    rcNew.right - rcNew.left, rcNew.bottom - rcNew.top, fTrue);
}


/* Launch an external application and have it open the specified file. */

void BootExternal(CONST char *szApp, CONST char *szFile)
{
  char szCmd[cchSzDef], sz[cchSzDef];

  if (FileOpen(szFile, 2, sz) != NULL) {
    if (szApp != NULL) {
      sprintf(szCmd, "%s %s", szApp, sz);
      WinExec(szCmd, SW_SHOW);
    } else
      ShellExecute(NULL, NULL, sz, NULL, NULL, SW_SHOW);
  } else {
    sprintf(szCmd, "File %s not found!", szFile);
    PrintWarning(szCmd);
  }
}


/* Given a relationship chart mode, return the menu command that sets it. */

WORD WCmdFromRc(int rc)
{
  switch (rc) {
  case rcTriWheel: case rcQuadWheel: /* Fall through */
  case rcDual:       return cmdRelComparison;
  case rcSynastry:   return cmdRelSynastry;
  case rcComposite:  return cmdRelComposite;
  case rcMidpoint:   return cmdRelMidpoint;
  case rcDifference: return cmdRelDate;
  case rcBiorhythm:  return cmdRelBiorhythm;
  case rcTransit:    return cmdRelTransit;
  case rcProgress:   return cmdRelProgressed;
  default:           return cmdRelNo;
  }
}


/* Change relationship chart modes. Given a new mode, we put a check by its */
/* menu command, and erase the check by the menu command for the old mode.  */

void SetRel(int rc)
{
  CheckMenu(WCmdFromRc(us.nRel), fFalse);
  us.nRel = rc;
  if (rc == rcMidpoint)  /* Turn off so don't move to midpoint again. */
    rc = rcNone;
  CheckMenu(WCmdFromRc(rc), fTrue);
  wi.fCast = fTrue;
}


/* The main program, the starting point of Astrolog for Windows, follows.   */
/* This is like the "main" function in standard C. The program initializes  */
/* here, then spins in a tight message processing loop until it terminates. */

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpszCmdLine, int nCmdShow)
{
  MSG msg;
  WNDCLASS wndclass;
#ifdef BETA
  char sz[cchSzMax];
#endif

  /* Set up the window class shared by all instances of Astrolog. */

  wi.hinst = hInstance;
  if (!hPrevInstance) {
    ClearB((lpbyte)&wndclass, sizeof(WNDCLASS));
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = wi.hinst;
    wndclass.hIcon = LoadIcon(wi.hinst, MAKEINTRESOURCE(icon));
    wndclass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wndclass.lpszMenuName = MAKEINTRESOURCE(menu);
    wndclass.lpszClassName = szAppName;
    if (!RegisterClass(&wndclass)) {
      PrintError("The window class could not be registered.");
      return -1L;
    }
  }

  /* Create the actual window to be used and drawn on by this instance. */

  wi.hmenu = LoadMenu(wi.hinst, MAKEINTRESOURCE(menu));
  wi.hwndMain = CreateWindow(
    szAppName,
    szAppNameCore " " szVersionCore,
    WS_CAPTION |
    WS_SYSMENU |
    WS_MINIMIZEBOX |
    WS_MAXIMIZEBOX |
    WS_THICKFRAME |
    WS_VSCROLL |
    WS_HSCROLL |
    WS_CLIPCHILDREN |
    WS_OVERLAPPED,
    CW_USEDEFAULT, 0,
    CW_USEDEFAULT, 0,
    (HWND)NULL,
    wi.hmenu,
    wi.hinst,
    (LPSTR)NULL);
  if (wi.hwndMain == (HWND)NULL) {
    PrintError("The window could not be created.");
    return -1L;
  }

  /* Set up some globals that can't be initialized at compile time. */

  CoInitialize(NULL);
  is.S = stdout;
  ofn.hwndOwner = wi.hwndMain;
  ofn.lpstrFile = szFileName;
  ofn.lpstrFileTitle = szFileTitle;
  prd.hwndOwner = wi.hwndMain;
  wi.haccel = LoadAccelerators(wi.hinst, MAKEINTRESOURCE(accelerator));
  wi.kiPen = gi.kiLite;

  /* Process the astrolog.as file and the Windows command line. */

  FProcessSwitchFile(DEFAULT_INFOFILE, NULL);
  FProcessCommandLine(lpszCmdLine);
  ciTran = ciFour = ciThre = ciTwin = ciCore;

  /* Actually bring up and display the window for the first time. */

  ResizeWindowToChart();
  ShowWindow(wi.hwndMain, nCmdShow);
  RedoMenu();
  wi.lTimer = SetTimer(wi.hwnd, 1, wi.nTimerDelay, NULL);
#ifdef BETA
  sprintf(sz, "This is a beta version of %s %s! "
    "That means changes are still being made and testing is not complete. "
    "If this is being run after %s %d, %d, "
    "it should be replaced with the finished release.",
    szAppName, szVersionCore, szMonth[ciSave.mon], ciSave.day, ciSave.yea);
  PrintWarning(sz);
#endif

  /* Process window messages until the program is told to terminate. */

  while (GetMessage(&msg, (HWND)NULL, 0, 0)) {
    if (!TranslateAccelerator(wi.hwndMain, wi.haccel, &msg))
      TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  /* Cleanup and exit Astrolog for Windows. */

  UnregisterClass(szAppName, wi.hinst);
  return (int)msg.wParam;
}


/* This is the main message processor for the Astrolog window. Given a */
/* user input or other message, do the appropriate action and updates. */

LRESULT API WndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
  HDC hdc;
  HPEN hpen, hpenOld;
  HBRUSH hbr;
  RECT rc;
  int x, y;

  wi.hwnd = hwnd;
  switch (wMsg) {

    /* A command, most likely a menu option, was given. */

    case WM_COMMAND:
      switch (NWmCommand((WORD)wParam)) {
      case 1:
        goto LWM_CLOSE;
      case -1:
        return DefWindowProc(hwnd, wMsg, wParam, lParam);
      default:
        ProcessState();
      }
      break;

    /* When a part of a window is uncovered, Windows quickly blanks it out, */
    /* usually with white, before having the app redraw that section. Most  */
    /* apps don't do anything here, however Astrolog can quickly draw with  */
    /* a more appropriate color since we know our chart's background color. */

    case WM_ERASEBKGND:
      if (!(wi.fNoUpdate & 1) && !wi.fBuffer) {
        GetClipBox((HDC)wParam, &rc);
        hbr = CreateSolidBrush(rgbbmp[gs.fInverse ? kWhite : kBlack]);
        FillRect((HDC)wParam, &rc, hbr);
        DeleteObject(hbr);
      }
      return fTrue;

    /* The window was just created. Setup the scrollbars. */

    case WM_CREATE:
      SetScrollRange(hwnd, SB_HORZ, 0, nScrollDiv, fFalse);
      SetScrollRange(hwnd, SB_VERT, 0, nScrollDiv, fFalse);
      break;

    /* The window has been resized. Change the chart size if need be. */

    case WM_SIZE:
      wi.xClient = LOWORD(lParam);
      wi.yClient = HIWORD(lParam);
      if (wi.fWindowChart || wi.fChartWindow) {
        gs.xWin = wi.xClient; gs.yWin = wi.yClient;
      }
      break;

    /* All or part of the window needs to be redrawn. Go do so. */

    case WM_PAINT:
      if (!(wi.fNoUpdate & 1))
        FRedraw();
      break;

#ifdef MOUSE
    /* The mouse has been left clicked or dragged over the window. */

    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:

      /* Treat dragging with the mouse down as a Shift+left click. */
      if (wMsg == WM_MOUSEMOVE) {
        if ((wParam & MK_LBUTTON) == 0 ||
          (wParam & MK_SHIFT) || (wParam & MK_CONTROL))
          break;
        wParam = MK_SHIFT;
      }
      x = WLo(lParam);
      y = WHi(lParam);

      /* Alt+click on a world map chart means relocate the chart there. */
      if (wMsg == WM_LBUTTONDOWN && GetKeyState(VK_MENU) < 0) {
        if (fMap && gs.nRot == 0 && !gs.fConstel && !gs.fMollewide) {
          Lon = rDegHalf-(real)(x-gi.xOffset)/(real)gs.xWin*rDegMax;
          if (Lon < -rDegHalf)
            Lon = -rDegHalf;
          else if (Lon > rDegHalf)
            Lon = rDegHalf;
          Lat = rDegQuad-(real)(y-gi.yOffset)/(real)gs.yWin*rDegHalf;
          if (Lat < -rDegQuad)
            Lat = -rDegQuad;
          else if (Lat > rDegQuad)
            Lat = rDegQuad;
          wi.xMouse = -1;
          wi.fCast = fTrue;
        }
        break;
      }
      hdc = GetDC(hwnd);
      hpen = (HPEN)CreatePen(PS_SOLID, 0, (COLORREF)rgbbmp[wi.kiPen]);
      hpenOld = (HPEN)SelectObject(hdc, hpen);

      /* Ctrl+click means draw a rectangle. Ctrl+Shift+click does ellipse. */
      if (wParam & MK_CONTROL) {
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        if (wParam & MK_SHIFT)
          Ellipse(hdc, wi.xMouse, wi.yMouse, x, y);
        else
          Rectangle(hdc, wi.xMouse, wi.yMouse, x, y);

      /* Shift+click means draw a line from the last to current position. */
      } else if (wParam & MK_SHIFT) {
        if (wi.xMouse >= 0) {
          MoveTo(hdc, wi.xMouse, wi.yMouse);
          LineTo(hdc, x, y);
          if (wMsg == WM_MOUSEMOVE) {
            wi.xMouse = x; wi.yMouse = y;
          }
        }

      /* A simple click means set a pixel and remember that location. */
      } else {
        SetPixel(hdc, x, y, (COLORREF)rgbbmp[wi.kiPen]);
        wi.xMouse = x; wi.yMouse = y;
      }
      SelectObject(hdc, hpenOld);
      DeleteObject(hpen);
      ReleaseDC(hwnd, hdc);
      break;

    /* The mouse has been right clicked on the window. If on a world map  */
    /* or astro-graph chart, relocate the chart to the mouse coordinates. */

    case WM_RBUTTONDOWN:
      x = WLo(lParam);
      y = WHi(lParam);
      if (us.fGraphics && (us.fAstroGraph || gi.nMode == gWorldMap) &&
        gs.nRot == 0 && !gs.fConstel && !gs.fMollewide) {
        Lon = rDegHalf - (real)(x-gi.xOffset) / (real)gs.xWin*rDegMax;
        if (Lon < -rDegHalf)
          Lon = -rDegHalf;
        else if (Lon > rDegHalf)
          Lon = rDegHalf;
        Lat = rDegQuad - (real)(y-gi.yOffset) / (real)gs.yWin*rDegHalf;
        if (Lat < -rDegQuad)
          Lat = -rDegQuad;
        else if (Lat > rDegQuad)
          Lat = rDegQuad;
        ciCore = ciMain;
        wi.fCast = fTrue;
        ProcessState();
      }
      break;
#endif

    /* A timer message is received at a defined regular interval. */

    case WM_TIMER:
      if (gs.nAnim < 1 || wi.fPause)
        break;
      Animate(gs.nAnim, wi.nDir);  /* Update chart if animation mode on. */
      wi.fRedraw = fTrue;
      ProcessState();
      break;

    /* Define the minimum and maximum size the window may be resized to. */

    case WM_GETMINMAXINFO:
      ((MINMAXINFO FAR *)lParam)->ptMinTrackSize.x = BITMAPX1;
      ((MINMAXINFO FAR *)lParam)->ptMinTrackSize.y = BITMAPY1;
      ((MINMAXINFO FAR *)lParam)->ptMaxTrackSize.x = BITMAPX;
      ((MINMAXINFO FAR *)lParam)->ptMaxTrackSize.x = BITMAPY;
      break;

    /* The horizontal scrollbar was clicked on or used in some way. */

    case WM_HSCROLL:
      x = wi.xScroll;
      switch (wParam) {
      case SB_LINEUP:
        wi.xScroll--;
        break;
      case SB_LINEDOWN:
        wi.xScroll++;
        break;
      case SB_PAGEUP:
        wi.xScroll -= nScrollPage;
        break;
      case SB_PAGEDOWN:
        wi.xScroll += nScrollPage;
        break;
      case SB_THUMBPOSITION:
        wi.xScroll = LOWORD(lParam);
        break;
      default:
        return fFalse;
      }
      wi.xScroll = max(0, min(wi.xScroll, nScrollDiv));
      if (wi.xScroll == x)
        break;
      SetScrollPos(hwnd, SB_HORZ, wi.xScroll, fTrue);
      wi.fRedraw = fTrue;
      ProcessState();
      break;

    /* The vertical scrollbar was clicked on or used in some way. */

    case WM_VSCROLL:
      y = wi.yScroll;
      switch(wParam) {
      case SB_LINEUP:
        wi.yScroll--;
        break;
      case SB_LINEDOWN:
        wi.yScroll++;
        break;
      case SB_PAGEUP:
        wi.yScroll -= nScrollPage;
        break;
      case SB_PAGEDOWN:
        wi.yScroll += nScrollPage;
        break;
      case SB_THUMBPOSITION:
        wi.yScroll = LOWORD(lParam);
        break;
      default:
        return fFalse;
      }
      wi.yScroll = max(0, min(wi.yScroll, nScrollDiv));
      if (wi.yScroll == y)
        break;
      SetScrollPos(hwnd, SB_VERT, wi.yScroll, fTrue);
      wi.fRedraw = fTrue;
      ProcessState();
      break;

    /* The window is being terminated. Clean up and free program data. */

    case WM_CLOSE:
LWM_CLOSE:
      if (us.fNoQuit) {
        PrintWarning("Program exiting is not allowed now.");
        break;
      }
      if (wi.hpen != (HPEN)NULL)
        DeleteObject(wi.hpen);
      if (wi.lTimer != 0)
        KillTimer(hwnd, 1);
      DestroyWindow(hwnd);
      if (hwnd == wi.hwndMain)
        PostQuitMessage(0);
      break;

    /* Messages not processed here are handled by Windows in a default way. */

    default:
      return DefWindowProc(hwnd, wMsg, wParam, lParam);
    }
  return fFalse;
}


/* This is called after some action has been done that probably changed the */
/* chart state, such as a menu command was run. Update anything needing it. */

void ProcessState()
{
  if (wi.fMenuAll) {       /* Redetermine all menu checks if need be. */
    RedoMenu();
    wi.fMenu = fTrue;
  }
  if (wi.fMenu) {          /* Update menu if we added/removed check marks. */
    DrawMenuBar(wi.hwnd);
    wi.fMenu = fFalse;
  }
  if (wi.fCast)            /* Recasting a chart implies redrawing it too. */
    wi.fRedraw = fTrue;

  /* If the chart type was changed, clear all the setting flags, then set */
  /* the appropriate core switch settings based on the new chart type.    */

  if (wi.nMode) {
    ClearB((lpbyte)&us.fListing,
      (int)((lpbyte)&us.fVelocity - (lpbyte)&us.fListing));
    ClearB((lpbyte)&us.fCredit,
      (int)((lpbyte)&us.fLoop - (lpbyte)&us.fCredit));
    us.nArabic = gi.nMode = 0;
    switch (wi.nMode) {
      case gBiorhythm:
      case gWheel:      us.fListing    = fTrue; break;
      case gHouse:      us.fWheel      = fTrue; break;
      case gGrid:       us.fGrid       = fTrue; break;
      case gHorizon: us.fHorizon = fTrue; us.fHorizonSearch = fFalse; break;
      case gOrbit:      us.fOrbit      = fTrue; break;
      case gSector:     us.fSector     = fTrue; break;
      case gAstroGraph: us.fAstroGraph = fTrue; break;
      case gEphemeris:  us.fEphemeris  = fTrue; break;
      case gWorldMap:   gi.nMode = gWorldMap; break;
      case gGlobe:      gi.nMode = gGlobe;    break;
      case gPolar:      gi.nMode = gPolar;    break;
      case gCalendar:   us.fCalendar   = fTrue; break;
      case gDisposit:   us.fInfluence  = fTrue; break;
      case gEsoteric:   us.fEsoteric   = fTrue; break;
      case gAspect:     us.fAspList    = fTrue; break;
      case gMidpoint:   us.fMidpoint   = fTrue; break;
      case gArabic:     us.nArabic     = 1; break;
      case gSign:       us.fSign       = fTrue; break;
      case gObject:     us.fObject     = fTrue; break;
      case gHelpAsp:    us.fAspect     = fTrue; break;
      case gConstel:    us.fConstel    = fTrue; break;
      case gPlanet:     us.fOrbitData  = fTrue; break;
      case gRay:        us.fRay        = fTrue; break;
      case gMeaning:    us.fMeaning    = fTrue; break;
      case gSwitch:     us.fSwitch     = fTrue; break;
      case gObscure:    us.fSwitchRare = fTrue; break;
      case gKeystroke:  us.fKeyGraph   = fTrue; break;
      case gCredit:     us.fCredit     = fTrue; break;
      case gRising:     us.fHorizon = us.fHorizonSearch = fTrue; break;
      case gTraTraHit:  us.fInDay      = fTrue; break;
      case gTraTraInf:  us.fInDayInf   = fTrue; break;
      case gTraNatHit:  us.fTransit    = fTrue; break;
      case gTraNatInf:  us.fTransitInf = fTrue; break;
    }
    wi.nMode = 0;
    wi.fRedraw = fTrue;
  }
  if (wi.fRedraw)    /* Send the window a redraw message if need be. */
    RedrawWindow(wi.hwnd, NULL, (HRGN)NULL, RDW_INVALIDATE);
}


/* Given a command, process it, changing any appropriate program settings. */
/* Return values are 0, meaning it was one of Astrolog's menu commands and */
/* action was taken; 1, meaning it's the special end program command; or   */
/* -1, meaning it's not a custom command and Windows can deal with it.     */

int NWmCommand(WORD wCmd)
{
  char sz[cchSzDef];
  DLGPROC dlgproc;
  int i;
  long l;
  flag fGraphics, fT;

  wi.wCmd = wCmd;
  fGraphics = us.fGraphics;
  switch (wCmd) {

  /* File Menu */

  case cmdFileExit:
    return 1;

  case cmdOpenChart:
    wi.nDlgChart = 1;
    DlgOpenChart();
    break;

  case cmdOpenChart2:
    wi.nDlgChart = 2;
    DlgOpenChart();
    break;

  case cmdSaveChart:
  case cmdSavePositions:
  case cmdSaveText:
  case cmdSaveBitmap:
#ifdef META
  case cmdSavePicture:
#endif
#ifdef PS
  case cmdSavePS:
#endif
  case cmdSaveSettings:
  case cmdSaveWallTile:
  case cmdSaveWallCenter:
  case cmdSaveWallStretch:
  case cmdSaveWallFit:
  case cmdSaveWallFill:
    DlgSaveChart();
    break;

  case cmdPrint:
    if (!DlgPrint())
      PrintWarning("The printing was not completed successfully.");
    break;

  case cmdPrintSetup:
    l = prd.Flags;
    prd.Flags |= PD_PRINTSETUP;
    PrintDlg((LPPRINTDLG)&prd);
    prd.Flags = l;
    break;

  /* Edit Menu */

  case cmdCommand:
    WiDoDialog(DlgCommand, dlgCommand);
    break;

  case cmdColor:
    WiDoDialog(DlgColor, dlgColor);
    break;

  case cmdCopyText:
    is.szFileScreen = szFileTemp;
    us.fGraphics = fFalse;
    wi.fRedraw = fTrue;
    break;

  case cmdCopyBitmap:
#ifdef META
  case cmdCopyPicture:
#endif
#ifdef PS
  case cmdCopyPS:
#endif
    if (us.fNoWrite)
      break;
    gi.szFileOut = szFileTemp;
    gs.fBitmap = wi.wCmd == cmdCopyBitmap;
    gs.fMeta = wi.wCmd == cmdCopyPicture;
    gs.fPS = wi.wCmd == cmdCopyPS;
    if (wCmd == cmdCopyBitmap)
      gs.chBmpMode = 'B';
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  /* View Menu */

  case cmdGraphics:
    inv(us.fGraphics);
    wi.fRedraw = fTrue;
    break;

  case cmdColoredText:
    inv(us.fAnsiColor); inv(us.fAnsiChar);
    WiCheckMenu(cmdColoredText, us.fAnsiColor);
    us.fGraphics = fFalse;
    wi.fRedraw = fTrue;
    break;

  case cmdWinBuffer:
    inv(wi.fBuffer);
    WiCheckMenu(cmdWinBuffer, wi.fBuffer);
    break;

  case cmdWinRedraw:
    if (wi.fNoUpdate)
      wi.fNoUpdate = 2;
    wi.fMenuAll = wi.fRedraw = fTrue;
    break;

  case cmdWinClear:
    fT = gs.fJetTrail;
    gs.fJetTrail = fFalse;
    wi.hdc = GetDC(wi.hwnd);
    if (us.fGraphics)
      DrawClearScreen();
    else
      TextClearScreen();
    ReleaseDC(wi.hwnd, wi.hdc);
    gs.fJetTrail = fT;
    break;

  case cmdWinHourglass:
    inv(wi.fHourglass);
    WiCheckMenu(cmdWinHourglass, wi.fHourglass);
    break;

  case cmdChartResizesWindow:
    inv(wi.fChartWindow);
    WiCheckMenu(cmdChartResizesWindow, wi.fChartWindow);
    wi.fRedraw = fTrue;
    break;

  case cmdWindowResizesChart:
    inv(wi.fWindowChart);
    WiCheckMenu(cmdWindowResizesChart, wi.fWindowChart);
    wi.fRedraw = fTrue;
    break;

  case cmdSizeChartToWindow:
    gs.xWin = wi.xClient;
    gs.yWin = wi.yClient;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdSizeWindowToChart:
    ResizeWindowToChart();
    break;

  case cmdScrollPageUp:
    PostMessage(wi.hwnd, WM_VSCROLL, SB_PAGEUP, 0);
    break;

  case cmdScrollPageDown:
    PostMessage(wi.hwnd, WM_VSCROLL, SB_PAGEDOWN, 0);
    break;

  case cmdScrollHome:
    PostMessage(wi.hwnd, WM_HSCROLL, SB_THUMBPOSITION, 0);
    PostMessage(wi.hwnd, WM_VSCROLL, SB_THUMBPOSITION, 0);
    break;

  case cmdScrollEnd:
    PostMessage(wi.hwnd, WM_HSCROLL, SB_THUMBPOSITION, nScrollDiv);
    PostMessage(wi.hwnd, WM_VSCROLL, SB_THUMBPOSITION, nScrollDiv);
    break;

#ifdef INTERPRET
  case cmdInterpret:
    inv(us.fInterpret);
    WiCheckMenu(cmdInterpret, us.fInterpret);
    us.fGraphics = fFalse;
    wi.fRedraw = fTrue;
    break;
#endif

  case cmdSecond:
    inv(us.fSeconds);
    is.fSeconds = us.fSeconds;
    WiCheckMenu(cmdSecond, us.fSeconds);
    wi.fRedraw = fTrue;
    break;

  case cmdApplying:
    inv(us.fAppSep);
    WiCheckMenu(cmdApplying, us.fAppSep);
    wi.fRedraw = fTrue;
    break;

  case cmdParallel:
    inv(us.fParallel);
    WiCheckMenu(cmdParallel, us.fParallel);
    wi.fRedraw = fTrue;
    break;

  /* Info Menu */

  case cmdSetInfo:
    wi.nDlgChart = 1;
    WiDoDialog(DlgInfo, dlgInfo);
    break;

#ifdef TIME
  case cmdNow:
    FInputData(szNowCore);
    wi.fCast = fTrue;
    break;
#endif

  case cmdDefaultInfo:
    WiDoDialog(DlgDefault, dlgDefault);
    break;

  case cmdSetInfo2:
    wi.nDlgChart = 2;
    WiDoDialog(DlgInfo, dlgInfo);
    break;

  case cmdSetInfoAll:
    WiDoDialog(DlgInfoAll, dlgInfoAll);
    break;

  case cmdRelNo:
  case cmdRelComparison:
    SetRel(us.nRel ? rcNone : rcDual);
    break;

  case cmdRelSynastry:
    SetRel(rcSynastry);
    break;

  case cmdRelComposite:
    SetRel(rcComposite);
    break;

  case cmdRelMidpoint:
    SetRel(rcMidpoint);
    break;

  case cmdRelDate:
    SetRel(rcDifference);
    gi.nMode = gWheel;
    us.fGraphics = fFalse;
    break;

#ifdef BIORHYTHM
  case cmdRelBiorhythm:
    SetRel(rcBiorhythm);
    gi.nMode = gBiorhythm;
    break;
#endif

  case cmdRelTransit:
    SetRel(rcTransit);
    break;

  case cmdRelProgressed:
    SetRel(rcProgress);
    break;

  /* Settings Menu */

  case cmdSidereal:
    inv(us.fSidereal);
    WiCheckMenu(cmdSidereal, us.fSidereal);
    wi.fCast = fTrue;
    break;

  case cmdHeliocentric:
    inv(us.objCenter);
    WiCheckMenu(cmdHeliocentric, us.objCenter != oEar);
    wi.fCast = fTrue;
    break;

  case cmdHouse00:
  case cmdHouse01:
  case cmdHouse02:
  case cmdHouse03:
  case cmdHouse04:
  case cmdHouse05:
  case cmdHouse06:
  case cmdHouse07:
  case cmdHouse08:
  case cmdHouse09:
  case cmdHouse10:
  case cmdHouse11:
  case cmdHouse12:
  case cmdHouse13:
  case cmdHouse14:
  case cmdHouse15:
  case cmdHouse16:
  case cmdHouse17:
    WiCheckMenu(cmdHouse00 + us.nHouseSystem, fFalse);
    us.nHouseSystem = (int)(wCmd - cmdHouse00);
    WiCheckMenu(wCmd, fTrue);
    wi.fCast = fTrue;
    break;

  case cmdHouseSetSolar:
    us.objOnAsc = us.objOnAsc ? 0 : oSun+1;
    WiCheckMenu(cmdHouseSetSolar, us.objOnAsc);
    wi.fCast = fTrue;
    break;

  case cmdHouseSet3D:
    inv(us.fHouse3D);
    WiCheckMenu(cmdHouseSet3D, us.fHouse3D);
    wi.fCast = fTrue;
    break;

  case cmdHouseSetDecan:
    inv(us.fDecan);
    WiCheckMenu(cmdHouseSetDecan, us.fDecan);
    wi.fCast = fTrue;
    break;

  case cmdHouseSetFlip:
    inv(us.fFlip);
    WiCheckMenu(cmdHouseSetFlip, us.fFlip);
    wi.fCast = fTrue;
    break;

  case cmdHouseSetGeodetic:
    inv(us.fGeodetic);
    WiCheckMenu(cmdHouseSetGeodetic, us.fGeodetic);
    wi.fCast = fTrue;
    break;

  case cmdHouseSetVedic:
    inv(us.fVedic);
    WiCheckMenu(cmdHouseSetVedic, us.fVedic);
    wi.fRedraw = fTrue;
    break;

  case cmdHouseSetNavamsa:
    inv(us.fNavamsa);
    WiCheckMenu(cmdHouseSetNavamsa, us.fNavamsa);
    wi.fCast = fTrue;
    break;

  case cmdAspect:
    WiDoDialog(DlgAspect, dlgAspect);
    break;

  case cmdObject:
    WiDoDialog(DlgObject, dlgObject);
    break;

  case cmdObject2:
    WiDoDialog(DlgObject2, dlgObject2);
    break;

  case cmdRes:
  case cmdResTransit:
    WiDoDialog(DlgRestrict, dlgRestrict);
    break;

  case cmdResMinor:
    for (i = oChi; i <= oEP; i++)
      if (i != oNod)
        inv(ignore[i]);
    WiCheckMenu(cmdResMinor, !ignore[oChi]);
    wi.fCast = fTrue;
    break;

  case cmdResCusp:
    inv(us.fCusp);
    for (i = cuspLo; i <= cuspHi; i++)
      ignore[i] = !us.fCusp || !ignore[i];
    WiCheckMenu(cmdResCusp, us.fCusp);
    wi.fCast = fTrue;
    break;

  case cmdResUranian:
    inv(us.fUranian);
    for (i = uranLo; i <= uranHi; i++)
      ignore[i] = !us.fUranian || !ignore[i];
    WiCheckMenu(cmdResUranian, us.fUranian);
    wi.fCast = fTrue;
    break;

  case cmdResStar:
    us.nStar = !us.nStar;
    for (i = starLo; i <= starHi; i++)
      ignore[i] = !us.nStar || !ignore[i];
    WiCheckMenu(cmdResStar, us.nStar);
    wi.fCast = fTrue;
    break;

  case cmdStar:
    WiDoDialog(DlgStar, dlgStar);
    break;

  case cmdSettingMore:
    WiDoDialog(DlgSetting, dlgSetting);
    break;

  case cmdDisplay:
    WiDoDialog(DlgDisplay, dlgDisplay);
    break;

  /* Chart Menu */

  case cmdChartList:
    wi.nMode = gWheel;
    break;

  case cmdChartWheel:
    wi.nMode = gHouse;
    break;

  case cmdChartGrid:
    wi.nMode = gGrid;
    break;

  case cmdChartAspect:
    wi.nMode = gAspect;
    us.fGraphics = fFalse;
    break;

  case cmdChartMidpoint:
    wi.nMode = gMidpoint;
    us.fGraphics = fFalse;
    break;

  case cmdChartHorizon:
    wi.nMode = gHorizon;
    break;

  case cmdChartOrbit:
    wi.nMode = gOrbit;
    break;

  case cmdChartSector:
    wi.nMode = gSector;
    break;

  case cmdChartCalendar:
    wi.nMode = gCalendar;
    break;

  case cmdChartInfluence:
    wi.nMode = gDisposit;
    break;

  case cmdChartEsoteric:
    wi.nMode = gEsoteric;
    break;

  case cmdChartAstroGraph:
    wi.nMode = gAstroGraph;
    break;

  case cmdChartEphemeris:
    wi.nMode = gEphemeris;
    break;

#ifdef ARABIC
  case cmdChartArabic:
    wi.nMode = gArabic;
    us.fGraphics = fFalse;
    break;
#endif

  case cmdChartRising:
    wi.nMode = gRising;
    us.fGraphics = fFalse;
    break;

  case cmdProgress:
    WiDoDialog(DlgProgress, dlgProgress);
    break;

  case cmdTransit:
    WiDoDialog(DlgTransit, dlgTransit);
    break;

  case cmdChartSettings:
    WiDoDialog(DlgChart, dlgChart);
    break;

  /* Graphics Menu */

  case cmdChartMap:
    wi.nMode = gWorldMap;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdChartGlobe:
    wi.nMode = gGlobe;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdChartPolar:
    wi.nMode = gPolar;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

#ifdef CONSTEL
  case cmdConstellation:
    if (!fMap && gi.nMode != gGlobe && gi.nMode != gPolar)
      wi.nMode = gWorldMap;
    inv(gs.fConstel);
    WiCheckMenu(cmdConstellation, gs.fConstel);
    us.fGraphics = wi.fRedraw = fTrue;
    break;
#endif

  case cmdGraphicsReverse:
    inv(gs.fInverse);
    WiCheckMenu(cmdGraphicsReverse, gs.fInverse);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsMonochrome:
    inv(gs.fColor);
    WiCheckMenu(cmdGraphicsMonochrome, !gs.fColor);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsBorder:
    inv(gs.fBorder);
    WiCheckMenu(cmdGraphicsBorder, gs.fBorder);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsText:
    inv(gs.fText);
    WiCheckMenu(cmdGraphicsText, gs.fText);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsSidebar:
    inv(us.fVelocity);
    if (!us.fVelocity) {
      gs.fText = fTrue;
      WiCheckMenu(cmdGraphicsText, fTrue);
    }
    WiCheckMenu(cmdGraphicsSidebar, !us.fVelocity);
    wi.nMode = gWheel;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsLabel:
    inv(gs.fLabel);
    WiCheckMenu(cmdGraphicsLabel, gs.fLabel);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsSquare:
    SquareX(&gs.xWin, &gs.yWin, fTrue);
    ResizeWindowToChart();
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdScale1:
  case cmdScale2:
  case cmdScale3:
  case cmdScale4:
    WiCheckMenu(cmdScale1 + gs.nScale/100 - 1, fFalse);
    gs.nScale = (int)(wCmd - cmdScale1) + 1;
    gs.nScale *= 100;
    WiCheckMenu(wCmd, fTrue);
    wi.fRedraw = fTrue;
    break;

  case cmdScaleDecrease:
    if (gs.nScale > 100) {
      WiCheckMenu(cmdScale1 + gs.nScale/100 - 1, fFalse);
      gs.nScale -= 100;
LScale:
      WiCheckMenu(cmdScale1 + gs.nScale/100 - 1, fTrue);
      wi.fRedraw = fTrue;
    }
    break;

  case cmdScaleIncrease:
    if (gs.nScale < MAXSCALE) {
      WiCheckMenu(cmdScale1 + gs.nScale/100 - 1, fFalse);
      gs.nScale += 100;
      goto LScale;
    }
    break;

  case cmdTiltZero:
    if (gi.nMode != gGlobe)
      wi.nMode = gGlobe;
    if (gs.rTilt != 0.0) {
      gs.rTilt = 0.0;
      wi.fRedraw = fTrue;
    }
    us.fGraphics = fTrue;
    break;

  case cmdTiltDecrease:
    if (gi.nMode != gGlobe)
      wi.nMode = gGlobe;
    if (gs.rTilt > -rDegQuad) {
      gs.rTilt = gs.rTilt > -rDegQuad ? gs.rTilt-TILTSTEP : -rDegQuad;
      wi.fRedraw = fTrue;
    }
    us.fGraphics = fTrue;
    break;

  case cmdTiltIncrease:
    if (gi.nMode != gGlobe)
      wi.nMode = gGlobe;
    if (gs.rTilt < rDegQuad) {
      gs.rTilt = gs.rTilt < rDegQuad ? gs.rTilt+TILTSTEP : rDegQuad;
      wi.fRedraw = fTrue;
    }
    us.fGraphics = fTrue;
    break;

  case cmdGraphicsModify:
    inv(gs.fAlt);
    WiCheckMenu(cmdGraphicsModify, gs.fAlt);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdChartModify:
    inv(us.fPrimeVert);
    inv(us.fCalendarYear);
    inv(us.nEphemYears);
    inv(gs.fMollewide);
    gi.nMode = (gi.nMode == gWheel ? gHouse :
      (gi.nMode == gHouse ? gWheel : gi.nMode));
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdPen00:
  case cmdPen01:
  case cmdPen02:
  case cmdPen03:
  case cmdPen04:
  case cmdPen05:
  case cmdPen06:
  case cmdPen07:
  case cmdPen08:
  case cmdPen09:
  case cmdPen10:
  case cmdPen11:
  case cmdPen12:
  case cmdPen13:
  case cmdPen14:
  case cmdPen15:
    WiCheckMenu(cmdPen00 + wi.kiPen, fFalse);
    wi.kiPen = (int)(wCmd - cmdPen00);
    WiCheckMenu(wCmd, fTrue);
    break;

  case cmdSettingGraphics:
    WiDoDialog(DlgGraphics, dlgGraphics);
    break;

  /* Animate Menu */

  case cmdAnimateNo:
    neg(gs.nAnim);
    WiCheckMenu(cmdAnimateNo, gs.nAnim > 0);
    if (gs.nAnim > 0) {
      WiCheckMenu(cmdWinBuffer, fTrue);
      wi.fBuffer = wi.fRedraw = fTrue;
    }
    break;

  case cmdAnimateNow:
  case cmdAnimateNo1:
  case cmdAnimateNo2:
  case cmdAnimateNo3:
  case cmdAnimateNo4:
  case cmdAnimateNo5:
  case cmdAnimateNo6:
  case cmdAnimateNo7:
  case cmdAnimateNo8:
  case cmdAnimateNo9:
    WiCheckMenu(cmdAnimateNo + NAbs(gs.nAnim), fFalse);
    gs.nAnim = (gs.nAnim < 0 ? -1 : 1)*((int)(wCmd - cmdAnimateNo1) + 1);
    WiCheckMenu(wCmd, fTrue);
    break;

  case cmdAnimateF1:
  case cmdAnimateF2:
  case cmdAnimateF3:
  case cmdAnimateF4:
  case cmdAnimateF5:
  case cmdAnimateF6:
  case cmdAnimateF7:
  case cmdAnimateF8:
  case cmdAnimateF9:
    WiCheckMenu(cmdAnimateF1 + abs(wi.nDir) - 1, fFalse);
    wi.nDir = (wi.nDir > 0 ? 1 : -1)*(int)(wCmd - cmdAnimateF1) + 1;
    WiCheckMenu(wCmd, fTrue);
    break;

  case cmdAnimateReverse:
    neg(wi.nDir);
    WiCheckMenu(cmdAnimateReverse, wi.nDir < 0);
    if (gs.nAnim < 0) {
      gs.nAnim = 10;
      WiCheckMenu(cmdWinBuffer, fTrue);
      WiCheckMenu(cmdAnimateNo, fTrue);
      WiCheckMenu(cmdAnimateNow, fTrue);
      wi.fBuffer = fTrue;
    }
    wi.fRedraw = fTrue;
    break;

  case cmdAnimatePause:
    inv(wi.fPause);
    WiCheckMenu(cmdAnimatePause, wi.fPause);
    break;

  case cmdTimedExposure:
    inv(gs.fJetTrail);
    WiCheckMenu(cmdTimedExposure, gs.fJetTrail);
    break;

  case cmdStepForward:
    Animate(NAbs(gs.nAnim) >= 10 ? 4 : gs.nAnim, abs(wi.nDir));
    wi.fCast = fTrue;
    break;

  case cmdStepBackward:
    Animate(NAbs(gs.nAnim) >= 10 ? 4 : gs.nAnim, -abs(wi.nDir));
    wi.fCast = fTrue;
    break;

  case cmdStore:
    ciSave = ciMain;
    break;

  case cmdRecall:
    ciMain = ciCore = ciSave;
    wi.fCast = fTrue;
    break;

  /* Help Menu */

  case cmdDocDefault:
    BootExternal("Notepad", DEFAULT_INFOFILE);
    break;

  case cmdDocUpdate:
    BootExternal(NULL, "changes.htm");
    break;

  case cmdDocHelpfile:
    BootExternal(NULL, "astrolog.htm");
    break;

  case cmdDocLicense:
    BootExternal(NULL, "license.htm");
    break;

  case cmdDocWebsite:
    BootExternal(NULL, "astrolog.url");
    break;

  case cmdDocWebsite2:
    BootExternal(NULL, "astrlog2.url");
    break;

  case cmdSetupAll:
  case cmdSetupUser:
    FCreateProgramGroup(wCmd == cmdSetupAll);
    break;

  case cmdSetupDesktop:
    FCreateDesktopIcon();
    break;

  case cmdSetupExtension:
    FRegisterExtensions();
    break;

  case cmdUnsetup:
    FUnregisterExtensions();
    break;

  case cmdHelpSign:
    wi.nMode = gSign;
    us.fGraphics = fFalse;
    break;

  case cmdHelpObject:
    wi.nMode = gObject;
    us.fGraphics = fFalse;
    break;

  case cmdHelpAspect:
    wi.nMode = gHelpAsp;
    us.fGraphics = fFalse;
    break;

#ifdef CONSTEL
  case cmdHelpConstellation:
    wi.nMode = gConstel;
    us.fGraphics = fFalse;
    break;
#endif

  case cmdHelpPlanetInfo:
    wi.nMode = gPlanet;
    us.fGraphics = fFalse;
    break;

  case cmdHelpRay:
    wi.nMode = gRay;
    us.fGraphics = fFalse;
    break;

#ifdef INTERPRET
  case cmdHelpMeaning:
    wi.nMode = gMeaning;
    us.fGraphics = fFalse;
    break;
#endif

  case cmdHelpSwitch:
    wi.nMode = gSwitch;
    us.fGraphics = fFalse;
    break;

  case cmdHelpObscure:
    wi.nMode = gObscure;
    us.fGraphics = fFalse;
    break;

  case cmdHelpKeystroke:
    wi.nMode = gKeystroke;
    us.fGraphics = fFalse;
    break;

  case cmdHelpCredit:
    wi.nMode = gCredit;
    us.fGraphics = fFalse;
    break;

  case cmdHelpAbout:
    WiDoDialog(DlgAbout, dlgAbout);
    break;

  default:
    if (FBetween(wCmd, cmdMacro01, cmdMacro48)) {
      i = (int)(wCmd - cmdMacro01);
      if (szMacro[i]) {
        FProcessCommandLine(szMacro[i]);
        wi.fCast = wi.fMenuAll = fTrue;
      } else if (i == 0/*F1*/ || i == 39/*Alt+F4*/) {
        /* F1 shows help if macro not already defined for it. */
        if (i == 0)
          BootExternal(NULL, "astrolog.htm");
      } else {
        sprintf(sz, "Macro number %d is not defined.", i+1);
        PrintWarning(sz);
      }
      break;
    }
    return -1;
  }

  if (us.fNoGraphics)
    us.fGraphics = fFalse;
  if (us.fGraphics != fGraphics)
    WiCheckMenu(cmdGraphics, us.fGraphics);
  return 0;
}


/* For each menu command that can have a check mark by it, determine its   */
/* state and set or clear appropriately. This is called when the program   */
/* is first started, and after commands that may change many settings that */
/* can't be kept track of, such as running macro commands or script files. */

void API RedoMenu()
{
  WORD cmd;

  CheckMenu(cmdGraphics, us.fGraphics);
  CheckMenu(cmdColoredText, us.fAnsiColor);
  CheckMenu(cmdWinBuffer, wi.fBuffer);
  CheckMenu(cmdWinHourglass, wi.fHourglass);
  CheckMenu(cmdChartResizesWindow, wi.fChartWindow);
  CheckMenu(cmdWindowResizesChart, wi.fWindowChart);
#ifdef INTERPRET
  CheckMenu(cmdInterpret, us.fInterpret);
#endif
  CheckMenu(cmdSecond, us.fSeconds);
  CheckMenu(cmdApplying, us.fAppSep);
  CheckMenu(cmdParallel, us.fParallel);
  for (cmd = cmdRelNo; cmd <= cmdRelProgressed; cmd++)
    CheckMenu(cmd, fFalse);
  CheckMenu(WCmdFromRc(us.nRel), fTrue);
  CheckMenu(cmdSidereal, us.fSidereal);
  CheckMenu(cmdHeliocentric, us.objCenter != oEar);
  for (cmd = cmdHouse00; cmd <= cmdHouse13; cmd++)
    CheckMenu(cmd, fFalse);
  CheckMenu(cmdHouse00 + us.nHouseSystem, fTrue);
  CheckMenu(cmdHouseSetSolar, us.objOnAsc);
  CheckMenu(cmdHouseSet3D, us.fHouse3D);
  CheckMenu(cmdHouseSetDecan, us.fDecan);
  CheckMenu(cmdHouseSetFlip, us.fFlip);
  CheckMenu(cmdHouseSetGeodetic, us.fGeodetic);
  CheckMenu(cmdHouseSetVedic, us.fVedic);
  CheckMenu(cmdHouseSetNavamsa, us.fNavamsa);
  CheckMenu(cmdResMinor, !ignore[oChi]);
  CheckMenu(cmdResCusp, us.fCusp);
  CheckMenu(cmdResUranian, us.fUranian);
  CheckMenu(cmdResStar, us.nStar);
#ifdef CONSTEL
  CheckMenu(cmdConstellation, gs.fConstel);
#endif
  CheckMenu(cmdGraphicsReverse, gs.fInverse);
  CheckMenu(cmdGraphicsMonochrome, !gs.fColor);
  CheckMenu(cmdGraphicsBorder, gs.fBorder);
  CheckMenu(cmdGraphicsText, gs.fText);
  CheckMenu(cmdGraphicsSidebar, !us.fVelocity);
  CheckMenu(cmdGraphicsLabel, gs.fLabel);
  for (cmd = cmdScale1; cmd <= cmdScale4; cmd++)
    CheckMenu(cmd, fFalse);
  CheckMenu(cmdScale1 + gs.nScale/100 - 1, fTrue);
  CheckMenu(cmdGraphicsModify, gs.fAlt);
  for (cmd = cmdPen00; cmd <= cmdPen15; cmd++)
    CheckMenu(cmd, fFalse);
  CheckMenu(cmdPen00 + wi.kiPen, fTrue);
  for (cmd = cmdAnimateNo; cmd <= cmdAnimateNow; cmd++)
    CheckMenu(cmd, fFalse);
  CheckMenu(cmdAnimateNo + NAbs(gs.nAnim), fTrue);
  for (cmd = cmdAnimateF1; cmd <= cmdAnimateF9; cmd++)
    CheckMenu(cmd, fFalse);
  CheckMenu(cmdAnimateF1 + NAbs(wi.nDir) - 1, fTrue);
  CheckMenu(cmdAnimateReverse, wi.nDir < 0);
  CheckMenu(cmdAnimatePause, wi.fPause);
  CheckMenu(cmdTimedExposure, gs.fJetTrail);
  wi.fMenuAll = fFalse;
}


/* This important routine is the bottleneck to redraw the window and call */
/* into the program to draw or do action with a particular chart type.    */

flag API FRedraw(void)
{
  /* Local variables used in drawing on the screen. */
  PAINTSTRUCT ps;
  HDC hdcWin;
  HCURSOR hcurOld;
  HBITMAP hbmp, hbmpOld;
  HFONT hfontOld;
  int nScrollRow, i;
  flag fSmartSave, fAnsiColor, fAnsiChar;

  /* Local variables used for copying to the Windows clipboard. */
  HFILE hfile;
  LONG lSize, l;
  HGLOBAL hglobal, hmfp;
  byte *hpb;
  METAFILEPICT mfp;
  HMETAFILE hmf;

  fSmartSave = us.fSmartSave &&
    (wi.wCmd == cmdSaveText || wi.wCmd == cmdCopyText);
  if (wi.fHourglass)
    hcurOld = SetCursor(LoadCursor((HINSTANCE)NULL, IDC_WAIT));
  ClearB((lpbyte)&ps, sizeof(PAINTSTRUCT));
  if (wi.hdcPrint != hdcNil)
    wi.hdc = wi.hdcPrint;
  else {
    hdcWin = BeginPaint(wi.hwnd, &ps);
    if (wi.fBuffer) {
      wi.hdc = CreateCompatibleDC(hdcWin);
      hbmp = CreateCompatibleBitmap(hdcWin, wi.xClient, wi.yClient);
      hbmpOld = (HBITMAP)SelectObject(wi.hdc, hbmp);
      if (gs.fJetTrail)
        BitBlt(wi.hdc, 0, 0, wi.xClient, wi.yClient, hdcWin, 0, 0, SRCCOPY);
    } else
      wi.hdc = hdcWin;
  }

  if (us.fGraphics) {
    /* Set up a graphics chart. */
    if (wi.fWindowChart && wi.hdcPrint == hdcNil) {
      gs.xWin = wi.xClient; gs.yWin = wi.yClient;
    }
    gi.nScale = gs.nScale/100;
    gi.nScaleText = gs.nScaleText/100;
    gi.kiCur = -1;
  } else {
    /* Set up a text chart. */
    SetWindowOrg(wi.hdc, 0, 0);
    SetWindowExt(wi.hdc, wi.xClient, wi.yClient);
    SetBkMode(wi.hdc, TRANSPARENT);
    if (wi.hdcPrint == hdcNil)
      TextClearScreen();
    i = gs.nScale/100;
    wi.xChar = i < 2 ? 6 : (i < 3 ? 8 : (i < 4 ? 10 : 12));
    wi.yChar = i < 2 ? 8 : (i < 3 ? 12 : (i < 4 ? 18 : 16));
    wi.hfont = CreateFont(wi.yChar /*nHeight*/, wi.xChar /*nWidth*/,
      0 /*nEscapement*/, 0 /*nOrientation*/, FW_DONTCARE,
      0 /*fbItalic*/, 0 /*fbUnderline*/, 0 /*fbStrikeOut*/,
      DEFAULT_CHARSET, OUT_RASTER_PRECIS, CLIP_DEFAULT_PRECIS,
      DRAFT_QUALITY, FIXED_PITCH | FF_DONTCARE, "Terminal");
    hfontOld = (HFONT)SelectObject(wi.hdc, wi.hfont);
    /* If printing, set the number of text rows per page. */
    if (wi.hdcPrint != hdcNil) {
      nScrollRow = us.nScrollRow;
      us.nScrollRow = wi.yClient / wi.yChar;
    }
  }
  if (fSmartSave) {
    fAnsiColor = us.fAnsiColor; fAnsiChar = us.fAnsiChar;
    us.fAnsiColor = fFalse; us.fAnsiChar = fFalse;
  }

  Action();    /* Actually go and create the chart here. */

  if (wi.fAutoSave && us.fGraphics && !gs.fBitmap) {
    gs.fBitmap = fTrue;
    gs.fMeta = gs.fPS = fFalse;
    gi.szFileOut = szFileAutoCore;
    Action();
    gi.szFileOut = NULL;
    gs.fBitmap = fFalse;
  }

  /* Cleanup and copy from the buffer to the screen if need be. */

  if (fSmartSave) {
    us.fAnsiColor = fAnsiColor; us.fAnsiChar = fAnsiChar;
  }
  if (!us.fGraphics) {
    if (wi.hdcPrint != hdcNil)
      us.nScrollRow = nScrollRow;
    SelectObject(wi.hdc, hfontOld);
    DeleteObject(wi.hfont);
  }
  if (wi.hdcPrint == hdcNil) {
    if (wi.fBuffer) {
      BitBlt(hdcWin, 0, 0, wi.xClient, wi.yClient,
        wi.hdc, -gi.xOffset, -gi.yOffset, SRCCOPY);
      SelectObject(wi.hdc, hbmpOld);
      DeleteObject(hbmp);
      DeleteDC(wi.hdc);
    }
    EndPaint(wi.hwnd, &ps);
  }
  if (wi.fHourglass)
    SetCursor(hcurOld);

  /* If all text was scrolled off the top of the screen, scroll up. */

  if (!us.fGraphics && is.S == stdout && is.cchRow - wi.yScroll * 10 < 0)
    PostMessage(wi.hwnd, WM_VSCROLL, SB_THUMBPOSITION, is.cchRow / 10 - 2);

  /* Sometimes creating a chart means saving it to a file instead of     */
  /* drawing it on screen. If we were in file mode, cleanup things here. */

  if (is.szFileScreen != NULL || gs.fBitmap || gs.fMeta || gs.fPS) {
    is.szFileScreen = NULL;
    if (gs.fMeta) {
      gs.xWin /= METAMUL; gs.yWin /= METAMUL; gs.nScale /= METAMUL;
    } else if (gs.fPS) {
      gs.xWin /= PSMUL; gs.yWin /= PSMUL; gs.nScale /= PSMUL;
    }
    gs.fBitmap = gs.fMeta = gs.fPS = fFalse;

    /* To copy charts to the clipboard, Astrolog saves the chart to a temp */
    /* file, then copies the contents of that file to the clipboard.       */

    if (wi.wCmd == cmdCopyText || wi.wCmd == cmdCopyBitmap ||
      wi.wCmd == cmdCopyPicture || wi.wCmd == cmdCopyPS) {
      hfile = _lopen(szFileTemp, OF_READ);
      if (hfile == HFILE_ERROR)
        return fFalse;
      lSize = _llseek(hfile, 0, 2);
      /* For bitmap and metafile charts, skip over the file header bytes. */
      l = wi.wCmd == cmdCopyBitmap ? sizeof(BITMAPFILEHEADER) :
        (wi.wCmd == cmdCopyPicture ? 22 : 0);
      hglobal = GlobalAlloc(GMEM_MOVEABLE, lSize - l);
      if (hglobal == (HGLOBAL)NULL)
        return fFalse;
      hpb = (byte *)GlobalLock(hglobal);
      _llseek(hfile, l, 0);
      _hread(hfile, hpb, lSize - l);
      _lclose(hfile);
      GlobalUnlock(hglobal);
      if (!OpenClipboard(wi.hwnd))
        return fFalse;
      EmptyClipboard();
      if (wi.wCmd == cmdCopyText || wi.wCmd == cmdCopyPS)
        SetClipboardData(CF_TEXT, hglobal);
      else if (wi.wCmd == cmdCopyBitmap)
        SetClipboardData(CF_DIB, hglobal);
      else {
        mfp.mm = MM_ANISOTROPIC;         /* For metafiles special structure */
        mfp.xExt = (gs.xWin-1)*-METAMUL; /* with a pointer to picture data  */
        mfp.yExt = (gs.yWin-1)*-METAMUL; /* needs to be allocated and used. */
        hpb = (byte *)GlobalLock(hglobal);
        hmf = SetMetaFileBitsEx(lSize - l, hpb);
        GlobalUnlock(hglobal);
        mfp.hMF = hmf;
        hmfp = GlobalAlloc(GMEM_MOVEABLE, sizeof(METAFILEPICT));
        if (hmfp == (HGLOBAL)NULL)
          return fFalse;
        hpb = (byte *)GlobalLock(hmfp);
        *(METAFILEPICT FAR *)hpb = mfp;
        GlobalUnlock(hmfp);
        SetClipboardData(CF_METAFILEPICT, hmfp);
      }
      CloseClipboard();
      _unlink(szFileTemp);
      wi.wCmd = 0;
    }
    ProcessState();
  }
  if (fSmartSave)
    wi.wCmd = 0;

  wi.fRedraw = fFalse;
  if (wi.fNoUpdate)
    wi.fNoUpdate = fTrue;
  return fTrue;
}


/*
******************************************************************************
** Windows Setup.
******************************************************************************
*/

/* Create a Windows shortcut file, a link pointing to another file. Called */
/* from FCreateProgramGroup() and FCreateDesktopIcon() to setup program.   */

flag FCreateShortcut(CONST char *szDir, CONST char *szName,
  CONST char *szFile, CONST char *szDesc, int nIcon)
{
  IShellLinkA *pisl = NULL;
  IPersistFile *pipf = NULL;
  char sz[cchSzMax], *pch;
  WCHAR wsz[cchSzMax];
  HRESULT hr;
  flag fRet = fFalse;

  /* Define the link data */
  hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
    IID_IShellLink, (LPVOID *)&pisl);
  if (FAILED(hr))
    goto LExit;
  GetModuleFileName(wi.hinst, sz, cchSzDef);
  if (nIcon >= 0) {
    hr = pisl->SetIconLocation(sz, nIcon);
    if (FAILED(hr))
      goto LExit;
  }
  for (pch = sz; *pch; pch++)
    ;
  while (pch > sz && *pch != '\\')
    pch--;
  *pch = chNull;
  hr = pisl->SetWorkingDirectory(sz);
  if (FAILED(hr))
    goto LExit;
  sprintf(pch, "\\%s", szFile);
  hr = pisl->SetPath(sz);
  if (FAILED(hr))
    goto LExit;
  if (szDesc == NULL)
    szDesc = szName;
  hr = pisl->SetDescription(szDesc);
  if (FAILED(hr))
    goto LExit;

  /* Save it to a link file */
  hr = pisl->QueryInterface(IID_IPersistFile, (LPVOID *)&pipf);
  if (FAILED(hr))
    goto LExit;
  sprintf(sz, "%s", szDir);
  for (pch = sz; *pch; pch++)
    ;
  sprintf(pch, "\\%s.lnk", szName);
  MultiByteToWideChar(CP_ACP, 0, sz, -1, wsz, cchSzMax);
  hr = pipf->Save(wsz, fTrue);
  if (FAILED(hr))
    goto LExit;

  fRet = fTrue;
LExit:
  if(pipf)
    pipf->Release();
  if(pisl)
    pisl->Release();
  return fRet;
}


/* Delete a Windows shortcut file, a link pointing to another file. Called */
/* from FCreateProgramGroup() to setup the program.                        */

void DeleteShortcut(CONST char *szDir, CONST char *szFile)
{
  char sz[cchSzMax];

  sprintf(sz, "%s\\%s.lnk", szDir, szFile);
  _unlink(sz);
}


/* Add an icon pointing to the Astrolog executable on the Windows desktop. */

flag FCreateDesktopIcon()
{
  char szDir[cchSzMax], szName[cchSzMax];
  LPITEMIDLIST pidl;
  flag fRet;

  SHGetSpecialFolderLocation(wi.hwnd, CSIDL_DESKTOPDIRECTORY, &pidl);
  SHGetPathFromIDList(pidl, szDir);
  sprintf(szName, "%s %s", szAppName, szVersionCore);
  fRet = FCreateShortcut(szDir, szName,
    "astrolog.exe", "Astrolog executable", 0);
  if (!fRet)
    PrintError("Failed to create desktop icon.");
  return fRet;
}


/* Create a Windows program group folder containing icons for Astrolog. */

flag FCreateProgramGroup(flag fAll)
{
  char szDir[cchSzMax], szName[cchSzMax], *pch;
  LPITEMIDLIST pidl;

  /* Create program group folder */
  SHGetSpecialFolderLocation(wi.hwnd,
    fAll ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS, &pidl);
  SHGetPathFromIDList(pidl, szDir);
  for (pch = szDir; *pch; pch++)
    ;
  sprintf(pch, "\\Astrolog");
  if (!CreateDirectory(szDir, NULL)) {
    if (GetLastError() != ERROR_ALREADY_EXISTS)
      goto LError;
  }

  /* Delete old previous version shortcuts from folder */
  DeleteShortcut(szDir, "Astrolog 6.00");
  DeleteShortcut(szDir, "Astrolog 6.10");

  /* Add main shortcuts in folder */
  sprintf(szName, "%s %s", szAppName, szVersionCore);
  if (!FCreateShortcut(szDir, szName,
    "astrolog.exe", "Astrolog executable", -1))
    goto LError;
  if (!FCreateShortcut(szDir, "Astrolog documentation",
    "astrolog.htm", NULL, -1))
    goto LError;
  if (!FCreateShortcut(szDir, "Astrolog changes", "changes.htm", NULL, -1))
    goto LError;
  if (!FCreateShortcut(szDir, "Astrolog website", "astrolog.url", NULL, 3))
    goto LError;

  return fTrue;
LError:
  PrintError("Failed to create program group.");
  return fFalse;
}


/* Add info about Astrolog specific file extensions to the Windows registry. */

flag FRegisterExtensions()
{
  HKEY hkey;
  char szExe[cchSzMax], szIco[cchSzMax], *pch;

  GetModuleFileName(wi.hinst, szExe, cchSzDef);
  sprintf(szIco, "%s,1", szExe);
  for (pch = szExe; *pch; pch++)
    ;
  sprintf(pch, "%s", " %1");

  /* Define .as extension and point it to Astrolog File Type */
  if (RegCreateKey(HKEY_CLASSES_ROOT, ".as", &hkey) != ERROR_SUCCESS)
    goto LError;
  if (RegSetValue(hkey, NULL, REG_SZ, "Astrolog.as", 11) != ERROR_SUCCESS)
    goto LError;
  RegCloseKey(hkey);

  /* Set File Type name for .as extensions */
  if (RegCreateKey(HKEY_CLASSES_ROOT, "Astrolog.as", &hkey) != ERROR_SUCCESS)
    goto LError;
  if (RegSetValue(hkey, NULL, REG_SZ, "Astrolog Settings", 17) !=
    ERROR_SUCCESS)
    goto LError;
  RegCloseKey(hkey);

  /* Make .as extension files be opened by Astrolog */
  if (RegCreateKey(HKEY_CLASSES_ROOT, "Astrolog.as\\shell\\open\\command",
    &hkey) != ERROR_SUCCESS)
    goto LError;
  if (RegSetValue(hkey, NULL, REG_SZ, szExe, CchSz(szExe)) != ERROR_SUCCESS)
    goto LError;
  RegCloseKey(hkey);

  /* Make .as extension files be edited by Windows Notepad */
  if (RegCreateKey(HKEY_CLASSES_ROOT, "Astrolog.as\\shell\\edit\\command",
    &hkey) != ERROR_SUCCESS)
    goto LError;
  if (RegSetValue(hkey, NULL, REG_SZ, "Notepad %1", 10) != ERROR_SUCCESS)
    goto LError;
  RegCloseKey(hkey);

  /* Set icon for .as extension files */
  if (RegCreateKey(HKEY_CLASSES_ROOT, "Astrolog.as\\DefaultIcon", &hkey) !=
    ERROR_SUCCESS)
    goto LError;
  if (RegSetValue(hkey, NULL, REG_SZ, szIco, CchSz(szIco)) != ERROR_SUCCESS)
    goto LError;
  RegCloseKey(hkey);

  return fTrue;
LError:
  PrintError("Failed to register Astrolog file extensions.\n"
    "You may need to run Astrolog as Administrator for registering to "
    "succeed.\nThis error is ignorable and all features of Astrolog will "
    "still work.");
  return fFalse;
}


/* Remove Astrolog specific data from the Windows registry. */

flag FUnregisterExtensions()
{
  /* Delete .as extension pointing to Astrolog File Type */
  if (SHDeleteKey(HKEY_CLASSES_ROOT, ".as") != ERROR_SUCCESS)
    goto LError;

  /* Delete icon for .as extension files */
  if (SHDeleteKey(HKEY_CLASSES_ROOT, "Astrolog.as") != ERROR_SUCCESS)
    goto LError;

  return fTrue;
LError:
  PrintError("Failed to unregister Astrolog file extensions.\n"
    "You may need to run Astrolog as Administrator for unregistering to "
    "succeed.");
  return fFalse;
}
#endif /* WIN */

/* wdriver.cpp */
