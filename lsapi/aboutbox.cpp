/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2025 The x7 Dev Team
Copyright (C) 2001-2008 The LiteStep Development Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/****************************************************************************
****************************************************************************/
#include "../utility/common.h"
#include <commctrl.h>
#include <math.h>
#include <string>
#include <ctype.h>
#include "../utility/core.hpp"

#include <strsafe.h> // For StringCchCopy, StringCchPrintf
#include <VersionHelpers.h> // For version helper functions

extern const wchar_t rcsRevision[];

typedef void (*AboutFunction)(HWND);

void AboutBangs(HWND hListView);
void AboutDevTeam(HWND hListView);
void AboutLicense(HWND hEdit);
void AboutModules(HWND hListView);
void AboutRevIDs(HWND hListView);
void AboutSysInfo(HWND hListView);

//
// misc functions
//
BOOL WINAPI AboutBoxProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HFONT CreateSimpleFont(LPCWSTR pwszName, int nSizeInPoints, bool bBold);
int GetClientWidth(HWND hWnd);
void TrimLeft(wchar_t* pwszToTrim);
void FormatBytes(size_t stBytes, LPWSTR pwszBuffer, size_t cchBuffer);


// Global handle to the running AboutBox instance (if any)
HWND g_hAboutbox = NULL;

/* Keep this enum synch'd with aboutOptions */
enum
{
  ABOUT_BANGS = 0
  , ABOUT_DEVTEAM
  , ABOUT_LICENSE
  , ABOUT_MODULES
  , ABOUT_REVIDS
  , ABOUT_SYSINFO
};

struct
{
  const wchar_t* option;
  AboutFunction function;
} aboutOptions[] = {
   {L"Bang Commands",      AboutBangs}
  ,{L"Development Team",   AboutDevTeam}
  ,{L"License",            AboutLicense}
  ,{L"Loaded Modules",     AboutModules}
  ,{L"Revision IDs",       AboutRevIDs}
  ,{L"System Information", AboutSysInfo}
};

struct
{
  const wchar_t* nick;
  const wchar_t* realName;
} theDevTeam[] = {
   {L"Acidfire", L"Alexander Vermaat"}
  ,{L"ilmcuts",  L"Simon"}
  ,{L"jugg",     L"Chris Rempel"}
  ,{L"Maduin",   L"Kevin Schaffer"}
  ,{L"Tobbe",    L"Tobbe Lundberg"}
  ,{L"x713",     L"]x7[13"}
};

const unsigned int aboutOptionsCount = sizeof(aboutOptions) / sizeof(aboutOptions[0]);
const unsigned int theDevTeamCount = sizeof(theDevTeam) / sizeof(theDevTeam[0]);

struct CallbackInfo
{
  HWND hListView;
  int nItem;
};

/* LiteStep license notice */
const wchar_t* lsLicense = \
L"LiteStep is a replacement shell for the standard Windows� Explorer shell.\r\n"
"\r\n"
"Copyright (C) 1997-1998  Francis Gastellu\r\n"
"Copyright (C) 1998-2009  LiteStep Development Team\r\n"
"Copyright (C) 2025 ]x7[ Development Team\r\n"
"\r\n"
"This program is free software; you can redistribute it and/or modify it under "
"the terms of the GNU General Public License as published by the Free Software "
"Foundation; either version 2 of the License, or (at your option) any later "
"version.\r\n"
"\r\n"
"This program is distributed in the hope that it will be useful, but WITHOUT ANY "
"WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A "
"PARTICULAR PURPOSE.  See the GNU General Public License for more details.\r\n"
"\r\n"
"You should have received a copy of the GNU General Public License along with "
"this program; if not, write to the Free Software Foundation, Inc., 51 Franklin "
"Street, Fifth Floor, Boston, MA  02110-1301, USA.\r\n"
"\r\n"
"http://www.lsdev.org/";

//
// AboutBox Dialog Procedure
//
BOOL WINAPI AboutBoxProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_COMMAND:
  {
    if (LOWORD(wParam) == IDC_COMBOBOX &&
      HIWORD(wParam) == CBN_SELCHANGE)
    {
      // delete listview items
      SendDlgItemMessage(hWnd, IDC_LISTVIEW,
        LVM_DELETEALLITEMS, 0, 0);

      int i;

      // delete listview columns
      for (i = 3; i >= 0; i--)
      {
        SendDlgItemMessage(hWnd, IDC_LISTVIEW,
          LVM_DELETECOLUMN, (WPARAM)i, 0);
      }

      // get new selection
      i = (int)SendDlgItemMessage(hWnd, IDC_COMBOBOX, CB_GETCURSEL, 0, 0);

      switch (i)
      {
      default:
        // default to revision IDs
        i = ABOUT_REVIDS;
        SendDlgItemMessage(hWnd, IDC_COMBOBOX, CB_SETCURSEL, ABOUT_REVIDS, 0);
        /* FALL THROUGH */
      case ABOUT_REVIDS:
      case ABOUT_BANGS:
      case ABOUT_DEVTEAM:
      case ABOUT_MODULES:
      case ABOUT_SYSINFO:
        // set the current display to the list view
        aboutOptions[i].function(GetDlgItem(hWnd, IDC_LISTVIEW));
        ShowWindow(GetDlgItem(hWnd, IDC_LISTVIEW), SW_SHOWNA);
        ShowWindow(GetDlgItem(hWnd, IDC_EDIT), SW_HIDE);
        break;

      case ABOUT_LICENSE:
        // set the current display to the edit box
        aboutOptions[i].function(GetDlgItem(hWnd, IDC_EDIT));
        ShowWindow(GetDlgItem(hWnd, IDC_EDIT), SW_SHOWNA);
        ShowWindow(GetDlgItem(hWnd, IDC_LISTVIEW), SW_HIDE);
        break;
      }
    }
    else if (LOWORD(wParam) == IDOK || LOWORD(wParam == IDCANCEL))
    {
      HFONT hTitleFont = (HFONT)SendDlgItemMessage(hWnd, IDC_TITLE,
        WM_GETFONT, 0, 0);

      // close the dialog box
      EndDialog(hWnd, IDOK);

      // release title font
      DeleteObject(hTitleFont);

      return TRUE;
    }
    /* This isn't necessary as we have the edit control set to read only.
     * It just ensures our text doesn't get changed somehow */
    else if (LOWORD(wParam) == IDC_EDIT && HIWORD(wParam) == EN_UPDATE)
    {
      DWORD dwStart;

      SendDlgItemMessage(hWnd, IDC_EDIT, EM_GETSEL, (WPARAM)&dwStart, 0);

      SetDlgItemText(hWnd, IDC_EDIT, lsLicense);

      SendDlgItemMessage(hWnd, IDC_EDIT, EM_SETSEL, dwStart - 1, dwStart - 1);
    }

    return FALSE;
  }

  case WM_CTLCOLOREDIT:
  case WM_CTLCOLORSTATIC:
  {
    HBRUSH hbReturn = FALSE; // special return value to tell the system
    // to perform default message processing

// the header and title need a white (COLOR_WINDOW) background
    int id = GetDlgCtrlID((HWND)lParam);

    if (id == IDC_TITLE || id == IDC_THEME_INFO || id == IDC_EDIT)
    {
      HDC hDC = (HDC)wParam;

      SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
      SetBkColor(hDC, GetSysColor(COLOR_WINDOW));

      hbReturn = GetSysColorBrush(COLOR_WINDOW);
    }
    else if (id == IDC_HEADER || id == IDC_LOGO)
    {
      hbReturn = GetSysColorBrush(COLOR_WINDOW);
    }

    return (BOOL)(hbReturn != NULL);
  }

  case WM_INITDIALOG:
  {
    // save global handle
    g_hAboutbox = hWnd;

    // set title font
    HFONT hTitleFont = CreateSimpleFont(L"Verdana", 14, false); //win98+
    if (!hTitleFont) hTitleFont = CreateSimpleFont(L"Arial", 14, false); //win95
    SendDlgItemMessage(hWnd, IDC_TITLE, WM_SETFONT,
      (WPARAM)hTitleFont, FALSE);

    // set title with LS version
    SetDlgItemText(hWnd, IDC_TITLE, L"LiteStep 0.25.0");

    // set Theme info
    wchar_t themeAuthor[16] = { 0 };
    wchar_t themeName[21] = { 0 };
    wchar_t themeOut[MAX_LINE_LENGTH] = { 0 };

    GetRCString(L"ThemeAuthor", themeAuthor, L"(unknown)", sizeof(themeAuthor));

    if (GetRCString(L"ThemeName", themeName, NULL, sizeof(themeName)))
    {
      StringCchPrintf(themeOut, MAX_LINE_LENGTH,
        L"Theme: %s by %s", themeName, themeAuthor);
    }
    else
    {
      StringCchPrintf(themeOut, MAX_LINE_LENGTH,
        L"Theme by %s", themeAuthor);
    }

    SetDlgItemText(hWnd, IDC_THEME_INFO, themeOut);

    // set compile time
    wchar_t compileTime[42] = { 0 };
    LSGetVariableEx(L"CompileDate", compileTime, 42);
    SetDlgItemText(hWnd, IDC_COMPILETIME, compileTime);

    // set the License Notice text
    SetDlgItemText(hWnd, IDC_EDIT, lsLicense);

    // add options to combo box
    for (unsigned int i = 0; i < aboutOptionsCount; ++i)
    {
      SendDlgItemMessage(hWnd, IDC_COMBOBOX, CB_ADDSTRING,
        0, (LPARAM)aboutOptions[i].option);
    }

    // default to License Notice
    SendDlgItemMessage(hWnd, IDC_COMBOBOX, CB_SETCURSEL, ABOUT_LICENSE, 0);
    // CB_SETCURSEL doesn't notify us via WM_COMMAND, so force it */
    WPARAM wp = MAKEWPARAM(IDC_COMBOBOX, CBN_SELCHANGE);
    SendMessage(hWnd, WM_COMMAND, wp, (LPARAM)GetDlgItem(hWnd, IDC_COMBOBOX));

    // center dialog on screen
    RECT rc;
    GetWindowRect(hWnd, &rc);

    SetWindowPos(hWnd, HWND_TOP,
      (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
      (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
      0, 0, SWP_NOSIZE);

#ifdef __GNUC__
    typedef void(__stdcall* STTWTYPE)(HWND, BOOL);

    static STTWTYPE SwitchToThisWindow = (STTWTYPE)GetProcAddress(
      GetModuleHandle(L"USER32.DLL"), "SwitchToThisWindow");
#endif
    SwitchToThisWindow(hWnd, TRUE);
    SetFocus(GetDlgItem(hWnd, IDC_COMBOBOX));

    return FALSE;
  }

  default:
    break;
  }

  return FALSE;
}


// AboutBox Thread Procedure
//

ULONG WINAPI AboutBoxThread(void*)
{
  if (!g_hAboutbox)
  {
    DialogBox(GetModuleHandle(NULL),
      MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, (DLGPROC)AboutBoxProcedure);

    g_hAboutbox = NULL;
  }
  else
  {
    SetForegroundWindow(g_hAboutbox);
  }

  return 0;
}


// Fill listview with bang command information
//

BOOL __stdcall BangCallback(LPCWSTR pwszName, LPARAM lParam)
{
  CallbackInfo* pCi = (CallbackInfo*)lParam;

  LVITEM itemInfo;
  itemInfo.mask = LVIF_TEXT;
  itemInfo.iItem = pCi->nItem++;
  itemInfo.pszText = (wchar_t*)pwszName;
  itemInfo.iSubItem = 0;

  ListView_InsertItem(pCi->hListView, &itemInfo);

  return TRUE;
}

void AboutBangs(HWND hListView)
{
  LVCOLUMN columnInfo;
  columnInfo.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  columnInfo.fmt = LVCFMT_LEFT;
  columnInfo.cx = GetClientWidth(hListView) - GetSystemMetrics(SM_CXVSCROLL);
  columnInfo.pszText = _wcsdup(L"Bang Command");
  columnInfo.iSubItem = 0;

  ListView_InsertColumn(hListView, 0, &columnInfo);

  CallbackInfo ci = { 0 };
  ci.hListView = hListView;

  EnumLSData(ELD_BANGS, (FARPROC)BangCallback, (LPARAM)&ci);
}


// Fill listview with development team information
//
void AboutDevTeam(HWND hListView)
{
  LVCOLUMN columnInfo;
  int width = GetClientWidth(hListView) - GetSystemMetrics(SM_CXVSCROLL);

  columnInfo.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  columnInfo.fmt = LVCFMT_LEFT;
  columnInfo.cx = width / 3;
  columnInfo.pszText = _wcsdup(L"Nick");
  columnInfo.iSubItem = 0;

  ListView_InsertColumn(hListView, 0, &columnInfo);

  columnInfo.cx = (2 * width) / 3;
  columnInfo.pszText = _wcsdup(L"Real Name");
  columnInfo.iSubItem = 1;

  ListView_InsertColumn(hListView, 1, &columnInfo);

  for (unsigned int i = 0; i < theDevTeamCount; i++)
  {
    LVITEM itemInfo;

    itemInfo.mask = LVIF_TEXT;
    itemInfo.iItem = i;
    itemInfo.pszText = (wchar_t*)theDevTeam[i].nick;
    itemInfo.iSubItem = 0;

    ListView_InsertItem(hListView, &itemInfo);
    ListView_SetItemText(hListView, i, 1, (wchar_t*)theDevTeam[i].realName);
  }
}


// Show License Notice... Nothing to do
//

void AboutLicense(HWND hEdit)
{
  UNREFERENCED_PARAMETER(hEdit);
}


// Fill listview with module information
//

BOOL __stdcall ModulesCallback(LPCWSTR pwszPath, DWORD /* dwFlags */, LPARAM lParam)
{
  CallbackInfo* pCi = (CallbackInfo*)lParam;

  LVITEM itemInfo;
  itemInfo.mask = LVIF_TEXT;
  itemInfo.iItem = pCi->nItem++;
  itemInfo.pszText = (wchar_t*)pwszPath;
  itemInfo.iSubItem = 0;

  ListView_InsertItem(pCi->hListView, &itemInfo);

  return TRUE;
}


void AboutModules(HWND hListView)
{
  LVCOLUMN columnInfo;

  columnInfo.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  columnInfo.fmt = LVCFMT_LEFT;
  columnInfo.cx = GetClientWidth(hListView) - GetSystemMetrics(SM_CXVSCROLL);
  columnInfo.pszText = _wcsdup(L"Module");
  columnInfo.iSubItem = 0;

  ListView_InsertColumn(hListView, 0, &columnInfo);

  CallbackInfo ci = { 0 };
  ci.hListView = hListView;

  EnumLSData(ELD_MODULES, (FARPROC)ModulesCallback, (LPARAM)&ci);
}


// Fill listview with revision ID (LM_GETREVID) information
//

BOOL __stdcall RevIDCallback(LPCWSTR pwszRevID, LPARAM lParam)
{
  CallbackInfo* pCi = (CallbackInfo*)lParam;

  LVITEM itemInfo;
  itemInfo.mask = LVIF_TEXT;
  itemInfo.iItem = pCi->nItem++;
  itemInfo.pszText = (wchar_t*)pwszRevID;
  itemInfo.iSubItem = 0;

  ListView_InsertItem(pCi->hListView, &itemInfo);

  return TRUE;
}


void AboutRevIDs(HWND hListView)
{
  LVCOLUMN columnInfo;
  LVITEM itemInfo;
  int i = 0;
  wchar_t buffer[MAX_PATH];

  columnInfo.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  columnInfo.fmt = LVCFMT_LEFT;
  columnInfo.cx = GetClientWidth(hListView) - GetSystemMetrics(SM_CXVSCROLL);
  columnInfo.pszText = _wcsdup(L"Revision ID");
  columnInfo.iSubItem = 0;

  ListView_InsertColumn(hListView, 0, &columnInfo);

  // special cases: litestep.exe and lsapi.dll
  itemInfo.mask = LVIF_TEXT;
  itemInfo.iItem = i++;
  itemInfo.pszText = buffer;
  itemInfo.iSubItem = 0;

  StringCchCopy(buffer, MAX_PATH, L"lsapi.dll: ");
  StringCchCat(buffer, MAX_PATH, &rcsRevision[11]);
  buffer[wcslen(buffer) - 1] = 0;

  ListView_InsertItem(hListView, &itemInfo);

  CallbackInfo ci = { 0 };
  ci.hListView = hListView;
  ci.nItem = i;

  EnumLSData(ELD_REVIDS, (FARPROC)RevIDCallback, (LPARAM)&ci);
}


// Fill listview with system information
//

void AboutSysInfo(HWND hListView)
{
  LVCOLUMN columnInfo;
  LVITEM itemInfo;
  int i = 0;
  wchar_t buffer[MAX_PATH];

  int width = GetClientWidth(hListView) - GetSystemMetrics(SM_CXVSCROLL);

  std::wstring colName = L"Name";
  columnInfo.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  columnInfo.fmt = LVCFMT_LEFT;
  columnInfo.cx = width / 3 + width / 8;
  columnInfo.pszText = const_cast<LPWSTR>(colName.c_str());
  columnInfo.iSubItem = 0;

  ListView_InsertColumn(hListView, 0, &columnInfo);

  /* Using this odd size, keeps the columns aligned with
   * the other list views, and also gives the text a little
   * more room to keep from being truncated. */
  columnInfo.cx = (2 * width) / 3 - width / 8;
  columnInfo.pszText = _wcsdup(L"Value");
  columnInfo.iSubItem = 1;

  ListView_InsertColumn(hListView, 1, &columnInfo);

  // operating system and version
  itemInfo.mask = LVIF_TEXT;
  itemInfo.iItem = i;
  itemInfo.pszText = _wcsdup(L"Operating System");
  itemInfo.iSubItem = 0;

  ListView_InsertItem(hListView, &itemInfo);


  if (IsWindows10OrGreater()) {
    StringCchCopy(buffer, MAX_PATH, L"Windows 10 or later"); // Or be more specific if needed
  }
  else if (IsWindows8Point1OrGreater()) {
    StringCchCopy(buffer, MAX_PATH, L"Windows 8.1");
  }
  else if (IsWindows8OrGreater()) {
    StringCchCopy(buffer, MAX_PATH, L"Windows 8");
  }
  else if (IsWindows7OrGreater()) {
    StringCchCopy(buffer, MAX_PATH, L"Windows 7");
  }
  else if (IsWindowsVistaOrGreater()) {
    StringCchCopy(buffer, MAX_PATH, L"Windows Vista");
  }
  else if (IsWindowsXPOrGreater()) {
    StringCchCopy(buffer, MAX_PATH, L"Windows XP"); // Consider if you still need this
  }
  else {
    StringCchCopy(buffer, MAX_PATH, L"Older Windows version"); // Handle very old versions
  }

  /*
  TrimLeft(versionInfo.szCSDVersion);

  if (versionInfo.szCSDVersion[0])
  {
    StringCchCat(buffer, MAX_PATH, L" (");
    StringCchCat(buffer, MAX_PATH, versionInfo.szCSDVersion);
    StringCchCat(buffer, MAX_PATH, L")");
  }
  */

  ListView_SetItemText(hListView, i++, 1, buffer);

  // memory status information
  DWORDLONG dwMemoryLoad, dwTotalPhys, dwAvailPhys, dwTotalPageFile, dwAvailPageFile;

  typedef BOOL(WINAPI* GMSExFunctionType)(LPMEMORYSTATUSEX);

  GMSExFunctionType fpGlobalMemoryStatusEx = (GMSExFunctionType)GetProcAddress(
    GetModuleHandle(L"KERNEL32.DLL"), "GlobalMemoryStatusEx");

  if (fpGlobalMemoryStatusEx)
  {
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(MEMORYSTATUSEX);
    fpGlobalMemoryStatusEx(&ms);

    dwMemoryLoad = ms.dwMemoryLoad;
    dwTotalPhys = ms.ullTotalPhys > MAXDWORD ? MAXDWORD : (DWORDLONG)ms.ullTotalPhys;
    dwAvailPhys = ms.ullAvailPhys > MAXDWORD ? MAXDWORD : (DWORDLONG)ms.ullAvailPhys;
    dwTotalPageFile = ms.ullTotalPageFile > MAXDWORD ? MAXDWORD : (DWORDLONG)ms.ullTotalPageFile;
    dwAvailPageFile = ms.ullAvailPageFile > MAXDWORD ? MAXDWORD : (DWORDLONG)ms.ullAvailPageFile;
  }
  else
  {
    MEMORYSTATUS ms;
    ms.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&ms);

    dwMemoryLoad = ms.dwMemoryLoad;
    dwTotalPhys = ms.dwTotalPhys;
    dwAvailPhys = ms.dwAvailPhys;
    dwTotalPageFile = ms.dwTotalPageFile;
    dwAvailPageFile = ms.dwAvailPageFile;
  }

  itemInfo.iItem = i;
  itemInfo.pszText = _wcsdup(L"Memory Load");

  ListView_InsertItem(hListView, &itemInfo);

  StringCchPrintf(buffer, MAX_PATH, L"%d%%", dwMemoryLoad);
  ListView_SetItemText(hListView, i++, 1, buffer);

  itemInfo.iItem = i;
  itemInfo.pszText = _wcsdup(L"Physical Memory Total");

  ListView_InsertItem(hListView, &itemInfo);

  FormatBytes(dwTotalPhys, buffer, 64);
  ListView_SetItemText(hListView, i++, 1, buffer);

  itemInfo.iItem = i;
  itemInfo.pszText = _wcsdup(L"Physical Memory Available");

  ListView_InsertItem(hListView, &itemInfo);

  FormatBytes(dwAvailPhys, buffer, 64);
  ListView_SetItemText(hListView, i++, 1, buffer);

  itemInfo.iItem = i;
  itemInfo.pszText = _wcsdup(L"Swap Space Total");

  ListView_InsertItem(hListView, &itemInfo);

  FormatBytes(dwTotalPageFile, buffer, 64);
  ListView_SetItemText(hListView, i++, 1, buffer);

  itemInfo.iItem = i;
  itemInfo.pszText = _wcsdup(L"Swap Space Available");

  ListView_InsertItem(hListView, &itemInfo);

  FormatBytes(dwAvailPageFile, buffer, 64);
  ListView_SetItemText(hListView, i++, 1, buffer);

}


// Simplified version of CreateFont
//

HFONT CreateSimpleFont(LPCWSTR pwszName, int nSizeInPoints, bool bBold)
{
  ASSERT_ISNOTNULL(pwszName);
  ASSERT(nSizeInPoints > 0);

  // convert size from points to pixels
  HDC hDC = GetDC(NULL);
  int sizeInPixels = -MulDiv(nSizeInPoints,
    GetDeviceCaps(hDC, LOGPIXELSY), 72);

  ReleaseDC(NULL, hDC);

  // fill in LOGFONT structure
  LOGFONT lf = { 0 };
  lf.lfHeight = sizeInPixels;
  lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
  StringCchCopy(lf.lfFaceName, LF_FACESIZE, pwszName);

  // create it
  return CreateFontIndirect(&lf);
}


// Return the width of the window's client area
//

int GetClientWidth(HWND hWnd)
{
  RECT r;
  GetClientRect(hWnd, &r);

  return r.right - r.left;
}


// Trims whitespace from the beginning of a string in-place
//
void TrimLeft(wchar_t* pwszToTrim)
{
  wchar_t* trimmed = pwszToTrim;

  // skip past spaces
  while (*pwszToTrim && iswspace(*pwszToTrim))
    pwszToTrim++;

  if (pwszToTrim != trimmed)
  {
    // copy the rest of the string over
    while (*pwszToTrim)
      *trimmed++ = *pwszToTrim++;

    // null-terminate it
    *trimmed = 0;
  }
}


// Formats a byte count into a string suitable for display to the user
//
// Note: Max value of stBytes is 4 GB, so no need to be concerned of
//       overrunning units[] index.
//
LPCWSTR units[] = { L"bytes", L"KB", L"MB", L"GB", L"TB" };

void FormatBytes(size_t stBytes, LPWSTR pwszBuffer, size_t cchBuffer)
{
  double dValue = (double)stBytes;
  unsigned int uUnit = 0;

  while ((dValue >= 1024) && (uUnit < (sizeof(units) / sizeof(units[0]) - 1)))
  {
    dValue /= 1024;
    ++uUnit;
  }

  // TODO : check for GB and TB
  if (uUnit == 3 || uUnit == 4 )
  {
    StringCchPrintf(
      pwszBuffer, cchBuffer,
      L"%.2f %s",
      dValue, units[uUnit]);
  }
  else
  {
    StringCchPrintf(
      pwszBuffer, cchBuffer,
      L"%d %s",
      (int)floor(dValue + 0.5), units[uUnit]);
  }
}
