/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 1997-2025 The LiteStep Development Team

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
#include "bangs.h"
#include "../utility/core.hpp"

#include <windows.h>  // For general Windows functions
#include <winuser.h>   // For GWL_* and some GWLP_* constants
#include <winbase.h>  // For other GWLP_* constants (especially if using GWLP_WNDPROC/HINSTANCE)
#include <string>

#include "../utility/safeptr.h"

extern ULONG WINAPI AboutBoxThread(void*);


void SetupBangs()
{
  AddBangCommand(L"!About", BangAbout);
  AddBangCommand(L"!Alert", BangAlert);
  AddBangCommand(L"!CascadeWindows", BangCascadeWindows);
  AddBangCommand(L"!Confirm", BangConfirm);
  AddBangCommand(L"!Execute", BangExecute);
  AddBangCommand(L"!HideModules", BangHideModules);
  AddBangCommand(L"!Logoff", BangLogoff);
  AddBangCommand(L"!MinimizeWindows", BangMinimizeWindows);
  AddBangCommand(L"!Quit", BangQuit);
  AddBangCommand(L"!Recycle", BangRecycle);
  AddBangCommand(L"!Refresh", BangRefresh);
  AddBangCommand(L"!Reload", BangReload);
  AddBangCommand(L"!ReloadModule", BangReloadModule);
  AddBangCommand(L"!RestoreWindows", BangRestoreWindows);
  AddBangCommand(L"!Run", BangRun);
  AddBangCommand(L"!ShowModules", BangShowModules);
  AddBangCommand(L"!Shutdown", BangShutdown);
  AddBangCommand(L"!SwitchUser", BangSwitchUser);
  AddBangCommand(L"!TileWindowsH", BangTileWindowsH);
  AddBangCommand(L"!TileWindowsV", BangTileWindowsV);
  AddBangCommand(L"!ToggleModules", BangToggleModules);
  AddBangCommand(L"!UnloadModule", BangUnloadModule);
}


//
// BangAbout(HWND hCaller, LPCWSTR pwszArgs)
//
void BangAbout(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  DWORD dwThread;

  //CloseHandle(CreateThread(NULL, 0, AboutBoxThread, NULL, 0, &dwThread));
  HANDLE hThread = CreateThread(NULL, 0, AboutBoxThread, NULL, 0, &dwThread);
  if (hThread != NULL)
  {
    CloseHandle(hThread);
  }

}


void BangAlert(HWND hCaller, LPCWSTR pwszArgs)
{
    // Allocate memory on the heap instead of the stack
    wchar_t* wszMessage = new wchar_t[MAX_LINE_LENGTH]();
    wchar_t* wszTitle = new wchar_t[MAX_LINE_LENGTH]();
    LPWSTR awszTokens[] = { wszMessage, wszTitle };

    int nTokenCount = LCTokenize(pwszArgs, awszTokens, 2, 0);

    if (nTokenCount >= 1)
    {
        if (nTokenCount == 1)
        {
            StringCchCopy(wszTitle, MAX_PATH, L"!Alert LiteStep");
        }

        MessageBox(hCaller, wszMessage, wszTitle, MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
    }

    // Free the allocated memory
    delete[] wszMessage;
    delete[] wszTitle;
}


//
// BangCascadeWindows(HWND hCaller, LPCWSTR pwszArgs)
//
void BangCascadeWindows(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  CascadeWindows(NULL, MDITILE_SKIPDISABLED, NULL, 0, NULL);
}


void BangConfirm(HWND hCaller, LPCWSTR pwszArgs)
{
  if (IsValidStringPtr(pwszArgs))
  {
    // Allocate memory on the heap instead of the stack
    wchar_t* wszFirst = new wchar_t[MAX_LINE_LENGTH]();
    wchar_t* wszSecond = new wchar_t[MAX_LINE_LENGTH]();
    wchar_t* wszThird = new wchar_t[MAX_LINE_LENGTH]();
    wchar_t* wszFourth = new wchar_t[MAX_LINE_LENGTH]();
    LPWSTR awszTokens[] = { wszFirst, wszSecond, wszThird, wszFourth };

    int nTokenCount = CommandTokenize(pwszArgs, awszTokens, 4, 0);

    if (nTokenCount >= 3)
    {
      if (nTokenCount == 3)
      {
        StringCchCopy(wszFourth, MAX_LINE_LENGTH, L"!Confirm LiteStep");
      }

      if (MessageBox(hCaller, wszFirst, (nTokenCount == 3) ? wszFourth : wszSecond, MB_YESNO | MB_TOPMOST | MB_SETFOREGROUND) == IDYES)
      {
        LSExecute(hCaller, (nTokenCount == 3) ? wszSecond : wszThird, SW_SHOWNORMAL);
      }
      else
      {
        LSExecute(hCaller, (nTokenCount == 3) ? wszThird : wszFourth, SW_SHOWNORMAL);
      }
    }

    // Free the allocated memory
    delete[] wszFirst;
    delete[] wszSecond;
    delete[] wszThird;
    delete[] wszFourth;
  }
}


//
// BangExecute(HWND hCaller, LPCWSTR pwszArgs)
//
void BangExecute(HWND hCaller, LPCWSTR pwszArgs)
{
  if (IsValidStringPtr(pwszArgs))
  {
    LPCWSTR pwszNextToken = pwszArgs;
    wchar_t wszCommand[MAX_LINE_LENGTH];

    while (GetToken(pwszNextToken, wszCommand, &pwszNextToken, TRUE))
    {
      LSExecute(hCaller, wszCommand, SW_SHOWDEFAULT);
    }
  }
}


//
// BangLogoff(HWND hCaller, LPCWSTR pwszArgs)
//
void BangLogoff(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  HWND hLiteStep = GetLitestepWnd();

  if (hLiteStep)
  {
    PostMessage(hLiteStep, LM_RECYCLE, LR_LOGOFF, 0);
  }
}


//
// CALLBACK WindowsEnumProc(HWND hWnd, LPARAM lParam)
//

static BOOL CALLBACK WindowsEnumProc(HWND hWnd, LPARAM lParam) {
  if (IsWindow(hWnd)) {
    LONG_PTR lUserData = GetWindowLongPtr(hWnd, GWLP_USERDATA); // Use GetWindowLongPtr and GWLP_USERDATA

    if ((lUserData != (LONG_PTR)magicDWord) && // Cast magicDWord to LONG_PTR
      (!(GetWindowLongPtr(hWnd, GWL_STYLE) & WS_CHILD) && // Use GetWindowLongPtr with GWL_STYLE (NO GWLP_STYLE)
        (GetWindowLongPtr(hWnd, GWL_STYLE) & WS_VISIBLE)) && // Use GetWindowLongPtr with GWL_STYLE (NO GWLP_STYLE)
      (!(GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))) { // Use GetWindowLongPtr with GWL_EXSTYLE (NO GWLP_EXSTYLE)
      ShowWindow(hWnd, (UINT)lParam);
    }
  }
  return TRUE;
}

//
// BangMinimizeWindows(HWND hCaller, LPCWSTR pwszArgs)
//
void BangMinimizeWindows(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  EnumWindows(WindowsEnumProc, (LPARAM)SW_MINIMIZE);
}


//
// BangQuit(HWND hCaller, LPCWSTR pwszArgs)
//
void BangQuit(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  HWND hLiteStep = GetLitestepWnd();

  if (hLiteStep)
  {
    PostMessage(hLiteStep, LM_RECYCLE, LR_QUIT, 0);
  }
}


//
// BangRecycle(HWND hCaller, LPCWSTR pwszArgs)
//
void BangRecycle(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  HWND hLiteStep = GetLitestepWnd();

  if (hLiteStep)
  {
    PostMessage(hLiteStep, LM_RECYCLE, LR_RECYCLE, 0);
  }
}


//
// BangRefresh(HWND hCaller, LPCWSTR pwszArgs)
//
void BangRefresh(HWND hCaller, LPCWSTR pwszArgs)
{
  HWND hLiteStep = GetLitestepWnd();

  BangReload(hCaller, pwszArgs);

  if (hLiteStep)
  {
    SendMessage(hLiteStep, LM_REFRESH, 0, 0);
  }
}


void BangReload(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  LSAPIReloadSettings();
}


//
// BangReloadModule(HWND hCaller, LPCSTR pszArgs)
//
void BangReloadModule(HWND /* hCaller */, LPCWSTR pwszArgs)
{
  if (IsValidStringPtr(pwszArgs))
  {
    HWND hLiteStep = GetLitestepWnd();

    if (hLiteStep)
    {
      LPCWSTR pszNextToken = pwszArgs;
      wchar_t szModuleString[MAX_LINE_LENGTH] = { 0 };

      while (GetToken(pszNextToken, szModuleString, &pszNextToken, TRUE))
      {
        SendMessage(hLiteStep, LM_RELOADMODULE, (WPARAM)szModuleString, 0);
      }
    }
  }
}


//
// BangRestoreWindows(HWND hCaller, LPCWSTR pwszArgs)
//
void BangRestoreWindows(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  EnumWindows(WindowsEnumProc, (LPARAM)SW_RESTORE);
}


void BangRun(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  typedef void (WINAPI* RunDlgType)(HWND, HICON, LPCWSTR, LPCWSTR, LPCWSTR, UINT);

  HMODULE hShell32 = GetModuleHandle(L"SHELL32.DLL");
  if (hShell32)
  {
    RunDlgType RunDlg = (RunDlgType)GetProcAddress(hShell32, (LPCSTR)((long)0x003D));
    if (RunDlg)
    {
      RunDlg(NULL, NULL, NULL, NULL, NULL, 0);
    }
  }
}


//
// BangShutdown(HWND hCaller, LPCWSTR pwszArgs)
//
void BangShutdown(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  HWND hLiteStep = GetLitestepWnd();

  if (hLiteStep)
  {
    PostMessage(hLiteStep, LM_RECYCLE, LR_MSSHUTDOWN, 0);
  }
}


void BangSwitchUser(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  typedef BOOL(__stdcall* LockWorkStationType)();

  HMODULE hUser32 = GetModuleHandle(L"USER32.DLL");
  if (hUser32)
  {
    LockWorkStationType LockWorkStation = (LockWorkStationType)GetProcAddress(hUser32, "LockWorkStation");
    if (LockWorkStation)
    {
      LockWorkStation();
    }
  }
}


//
// BangTileWindowsH(HWND hCaller, LPCWSTR pwszArgs)
//
void BangTileWindowsH(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  TileWindows(NULL, MDITILE_HORIZONTAL, NULL, 0, NULL);
}


//
// BangTileWindowsV(HWND hCaller, LPCWSTR pwszArgs)
//
void BangTileWindowsV(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  TileWindows(NULL, MDITILE_VERTICAL, NULL, 0, NULL);
}


//
// BangUnloadModule(HWND hCaller, LPCWSTR pwszArgs)
//
void BangUnloadModule(HWND /* hCaller */, LPCWSTR pwszArgs)
{
  HWND hLiteStep = GetLitestepWnd();

  if (hLiteStep)
  {
    LPCWSTR pwszNextToken = pwszArgs;
    wchar_t wszPath[MAX_LINE_LENGTH] = { 0 };

    while (GetToken(pwszNextToken, wszPath, &pwszNextToken, TRUE))
    {
      SendMessage(hLiteStep, LM_UNLOADMODULE, (WPARAM)wszPath, 0);
    }
  }
}


#define EMP_HIDE   0
#define EMP_SHOW   1
#define EMP_TOGGLE 2

#define DEFAULT_FLAGS (SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE)

static void EMPHandler(HDWP hDwp, HWND hWnd, UINT uMode) {
  if (IsWindow(hWnd)) {
    LONG_PTR lUserData = GetWindowLongPtr(hWnd, GWLP_USERDATA); // Use GetWindowLongPtr and GWLP_USERDATA

    if ((lUserData == (LONG_PTR)magicDWord) && IsWindowVisible(hWnd) && // Cast magicDWord to LONG_PTR
      (uMode == EMP_HIDE || uMode == EMP_TOGGLE)) {
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)HIDEmagicDWord); // Use GetWindowLongPtr and GWLP_USERDATA, cast HIDEmagicDWord
      hDwp = DeferWindowPos(
        hDwp,
        hWnd,
        NULL,
        0, 0,
        0, 0,
        SWP_HIDEWINDOW | DEFAULT_FLAGS
      );
    }
    else if ((lUserData == (LONG_PTR)HIDEmagicDWord) && // Cast HIDEmagicDWord to LONG_PTR
      (uMode == EMP_SHOW || uMode == EMP_TOGGLE)) {
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)magicDWord); // Use GetWindowLongPtr and GWLP_USERDATA, cast magicDWord
      hDwp = DeferWindowPos(
        hDwp,
        hWnd,
        NULL,
        0, 0,
        0, 0,
        SWP_SHOWWINDOW | DEFAULT_FLAGS
      );
    }
  }
}


//
// CALLBACK EnumHideModulesProc(HWND hWnd, LPARAM lParam)
//
static BOOL CALLBACK EnumHideModulesProc(HWND hWnd, LPARAM lParam)
{
  EMPHandler((HDWP)lParam, hWnd, EMP_HIDE);
  return TRUE;
}


//
// CALLBACK EnumShowModulesProc(HWND hWnd, LPARAM lParam)
//
static BOOL CALLBACK EnumShowModulesProc(HWND hWnd, LPARAM lParam)
{
  EMPHandler((HDWP)lParam, hWnd, EMP_SHOW);
  return TRUE;
}


//
// CALLBACK EnumToggleModulesProc(HWND hWnd, LPARAM lParam)
//
static BOOL CALLBACK EnumToggleModulesProc(HWND hWnd, LPARAM lParam)
{
  EMPHandler((HDWP)lParam, hWnd, EMP_TOGGLE);
  return TRUE;
}


//
// BangHideModules(HWND hCaller, LPCWSTR pwszArgs)
//
void BangHideModules(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  HDWP hDwp = BeginDeferWindowPos(5);
  EnumWindows(EnumHideModulesProc, (LPARAM)hDwp);
  EndDeferWindowPos(hDwp);
}


//
// BangShowModules(HWND hCaller, LPCWSTR pwszArgs)
//
void BangShowModules(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  HDWP hDwp = BeginDeferWindowPos(5);
  EnumWindows(EnumShowModulesProc, (LPARAM)hDwp);
  EndDeferWindowPos(hDwp);
}


//
// BangToggleModules(HWND hCaller, LPCWSTR pwszArgs)
//
void BangToggleModules(HWND /* hCaller */, LPCWSTR /* pwszArgs */)
{
  HDWP hDwp = BeginDeferWindowPos(5);
  EnumWindows(EnumToggleModulesProc, (LPARAM)hDwp);
  EndDeferWindowPos(hDwp);
}
