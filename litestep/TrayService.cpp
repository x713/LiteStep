/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 1997-2006 The LiteStep Development Team

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
#include "TrayService.h"
#include <regstr.h>
#include <shlobj.h>
#include <algorithm>
#include "../utility/macros.h"
#include "../utility/shellhlp.h"
#include "../utility/core.hpp"

#include "../utility/safestr.h"
#include <docobj.h>


#include <VersionHelpers.h> // Include for Version Helper functions

#ifdef __GNUC__
using std::max;
using std::min;
#endif

FUNC_PVOID__HANDLE_DWORD fpSHLockShared;
FUNC_BOOL__PVOID fpSHUnlockShared;

#ifndef REGSTR_PATH_SHELLSERVICEOBJECTDELAYED
#define REGSTR_PATH_SHELLSERVICEOBJECTDELAYED L"Software\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad"
#endif

static const wchar_t szTrayClass[] = L"Shell_TrayWnd";
static const wchar_t szTrayTitle[] = L"LiteStep Tray Manager";
static const wchar_t szNotifyClass[] = L"TrayNotifyWnd";

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// TrayService
//
TrayService::TrayService() :
  m_bWin2000(false), m_uWorkAreaDirty(0), m_hNotifyWnd(NULL), m_hTrayWnd(NULL),
  m_hLiteStep(NULL), m_hInstance(NULL)
{
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ~TrayService
//
TrayService::~TrayService()
{
}


bool IsWin2000OrLater() {
  return IsWindowsVersionOrGreater(5, 0, 0); // Check for version 5.0 or greater
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Start()
//
HRESULT TrayService::Start()
{
  ASSERT_ISNULL(m_hTrayWnd);
  HRESULT hr = E_FAIL;

  fpSHLockShared = (FUNC_PVOID__HANDLE_DWORD)GetProcAddress(
    GetModuleHandle(L"SHELL32")
    , (LPCSTR)((long)0x0209)
  );
  fpSHUnlockShared = (FUNC_BOOL__PVOID)GetProcAddress(
    GetModuleHandle(L"SHELL32")
    , (LPCSTR)((long)0x020A)
  );

  m_hLiteStep = GetLitestepWnd();
  m_hInstance = GetModuleHandle(NULL);

  if (m_hLiteStep && m_hInstance)
  {
    /*
    OSVERSIONINFO OsVersionInfo = { 0 };
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&OsVersionInfo);

    if (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
      OsVersionInfo.dwMajorVersion >= 5)
    {
      m_bWin2000 = true;
    }
    */


    // Usage:
    m_bWin2000 = IsWin2000OrLater();

    // clear work area of primary monitor
    SetRect(&m_rWorkAreaDef,
      0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    setWorkArea(&m_rWorkAreaDef);
    CopyRect(&m_rWorkAreaCur, &m_rWorkAreaDef);

    if (createWindows())
    {
#ifdef _WIN64
      SetWindowLongPtr(m_hTrayWnd, GWLP_USERDATA, (LONG_PTR)magicDWord); // Correct on x64
#else
      SetWindowLong(m_hTrayWnd, GWL_USERDATA, magicDWord);
      //SetWindowLongPtr(m_hTrayWnd, GWLP_USERDATA, (LONG_PTR)magicDWord); // Correct on x86 (GWLP_USERDATA == GWL_USERDATA)
#endif
      SetWindowLongPtr(m_hTrayWnd, 0, (LONG_PTR)this);

      // http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=1427677&SiteID=1
      //
      // Re: TaskbarCreated message (not working in Vista)
      //
      //  > The code is still in there to broadcast the message.
      //  > SendNotifyMessage(HWND_BROADCAST, RegisterWindowMessage(TEXT("TaskbarCreated")), 0, 0);
      //
      //  >> It looks as though applications running as admin cannot receive the message.
      //  >> Presumably, this is because Explorer, which publishes the message, is not
      //  >> running as admin
      //
      //  >>> The message is blocked by User Interface Privilege Isolation, Administrative
      //  >>> applications that need to see it can allow it through by calling
      //  >>> ChangeWindowMessageFilter after making sure the necessary security precautions
      //  >>> are in place. 

      // tell apps to reregister their icons
      SendNotifyMessage(HWND_BROADCAST,
        RegisterWindowMessage(L"TaskbarCreated"), 0, 0);

      if (m_bWin2000)
      {
        loadShellServiceObjects();
      }

      hr = S_OK;
    }
  }

  return hr;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Stop()
//
HRESULT TrayService::Stop()
{
  HRESULT hr = S_OK;

  if (m_bWin2000 && m_hTrayWnd)
  {
    unloadShellServiceObjects();
  }

  destroyWindows();

  while (!m_siVector.empty())
  {
    delete m_siVector.back();
    m_siVector.pop_back();
  }

  while (!m_abVector.empty())
  {
    delete m_abVector.back();
    m_abVector.pop_back();
  }

  // clear the work area of primary monitor
  SetRect(&m_rWorkAreaDef,
    0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
  setWorkArea(&m_rWorkAreaDef);

  m_bWin2000 = false;
  m_hLiteStep = NULL;
  m_hInstance = NULL;
  fpSHLockShared = NULL;
  fpSHUnlockShared = NULL;

  return hr;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// createWindows
//
bool TrayService::createWindows()
{
  bool bReturn = false;

  //
  // Register tray window class
  //
  WNDCLASSEX wc = { 0 };
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.cbWndExtra = sizeof(TrayService*);
  wc.lpfnWndProc = TrayService::WindowTrayProc;
  wc.hInstance = m_hInstance;
  wc.lpszClassName = szTrayClass;
  wc.style = CS_DBLCLKS;

  if (!RegisterClassEx(&wc))
  {
    RESOURCE_MSGBOX(m_hInstance, IDS_LITESTEP_REGISTERCLASS_ERROR,
      L"Error registering window class.", szTrayClass);
  }
  else
  {
    //
    // Window which receives the tray messages
    //
    m_hTrayWnd = CreateWindowEx(
      WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
      szTrayClass, szTrayTitle,
      WS_POPUP,
      0, 0, 0, 0,
      NULL, NULL,
      m_hInstance,
      NULL);

    if (NULL == m_hTrayWnd)
    {
      UnregisterClass(szTrayClass, m_hInstance);

      RESOURCE_MSGBOX(m_hInstance, IDS_LITESTEP_CREATEWINDOW_ERROR,
        L"Unable to create window.", szTrayTitle);
    }
    else
    {
      // Our main window is enough to start up, we can do without the
      // TrayNotifyWnd if necessary.
      bReturn = true;

      //
      // Register "TrayNotifyWnd" class and create window
      //
      ZeroMemory(&wc, sizeof(wc));
      wc.cbSize = sizeof(WNDCLASSEX);
      wc.lpfnWndProc = TrayService::WindowNotifyProc;
      wc.hInstance = m_hInstance;
      wc.lpszClassName = szNotifyClass;
      wc.style = CS_DBLCLKS;

      if (RegisterClassEx(&wc))
      {
        m_hNotifyWnd = CreateWindowEx(
          0,
          szNotifyClass, NULL,
          WS_CHILD,
          0, 0, 0, 0,
          m_hTrayWnd, NULL,
          m_hInstance,
          NULL);

        if (!m_hNotifyWnd)
        {
          UnregisterClass(szNotifyClass, m_hInstance);
        }
      }
    }
  }

  return bReturn;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// destroyWindows
//
void TrayService::destroyWindows()
{
  if (m_hNotifyWnd)
  {
    DestroyWindow(m_hNotifyWnd);
    m_hNotifyWnd = NULL;

    UnregisterClass(szNotifyClass, m_hInstance);
  }

  if (m_hTrayWnd)
  {
    DestroyWindow(m_hTrayWnd);
    m_hTrayWnd = NULL;

    UnregisterClass(szTrayClass, m_hInstance);
  }
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// loadShellServiceObjects
//
// Start the COM based shell services listed in the registry.
//
void TrayService::loadShellServiceObjects()
{
  HKEY hkeyServices;
  int nCounter = 0;

  LONG lErrorCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
    REGSTR_PATH_SHELLSERVICEOBJECTDELAYED,
    0, KEY_READ, &hkeyServices);


  while (lErrorCode == ERROR_SUCCESS)
  {
    TCHAR szValueName[32] = { 0 };
    char szData[40] = { 0 };
    DWORD cchValueName = sizeof(szValueName) / sizeof(szValueName[0]);
    DWORD cbData = sizeof(szData);
    DWORD dwDataType;

    lErrorCode = RegEnumValue(hkeyServices, nCounter, szValueName,
      &cchValueName, NULL, &dwDataType, (LPBYTE)szData, &cbData);

    if (lErrorCode == ERROR_SUCCESS)
    {
      WCHAR wszCLSID[40] = { 0 };
      CLSID clsid;
      IOleCommandTarget* pCmdTarget = NULL;

      MultiByteToWideChar(CP_ACP, 0, szData, cbData, wszCLSID, 40);

      CLSIDFromString(wszCLSID, &clsid);

      HRESULT hr = CoCreateInstance(clsid, NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
        IID_IOleCommandTarget, (void**)&pCmdTarget);

      if (SUCCEEDED(hr))
      {
        pCmdTarget->Exec(&CGID_ShellServiceObject, OLECMDID_NEW,
          OLECMDEXECOPT_DODEFAULT, NULL, NULL);

        m_ssoVector.push_back(pCmdTarget);
      }
    }

    ++nCounter;
  }

  RegCloseKey(hkeyServices);
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// unloadShellServiceObjects
//
// Shut down the COM based shell services.
//
//
void TrayService::unloadShellServiceObjects()
{
  while (!m_ssoVector.empty())
  {
    m_ssoVector.back()->Exec(&CGID_ShellServiceObject, OLECMDID_SAVE,
      OLECMDEXECOPT_DODEFAULT, NULL, NULL);

    m_ssoVector.back()->Release();
    m_ssoVector.pop_back();
  }
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// WindowTrayProc
//
// Redirects all messages to the appropriate handler
//
LRESULT CALLBACK TrayService::WindowTrayProc(HWND hWnd, UINT uMsg,
  WPARAM wParam, LPARAM lParam)
{
  static TrayService* pTrayService = NULL;

  if (!pTrayService)
  {
    pTrayService = (TrayService*)GetWindowLongPtr(hWnd, 0);
  }

  if (pTrayService)
  {
    LRESULT lResult = 0;

    switch (uMsg)
    {
    case WM_COPYDATA:
    {
      //
      // Undocumented: This is how we can make our own system tray
      // and handle app bar windows. Once a window in the system has
      // the "Shell_TrayWnd" class name, it receives this message!
      //
      COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)lParam;

      switch (pcds->dwData)
      {
      case SH_APPBAR_DATA:
      {
        TRACE(L"SH_APPBAR_DATA: %u", pcds->cbData);

        //
        // Application Bar Message
        //
        switch (pcds->cbData)
        {
          case sizeof(APPBARMSGDATAV2) :
          {
            PAPPBARMSGDATAV2 pamd = (PAPPBARMSGDATAV2)pcds->lpData;

            TRACE(L"APPBARMSGDATAV2: %u", pamd->abd.cbSize);

            if (sizeof(APPBARDATAV2) != pamd->abd.cbSize)
              break;

            TRACE(L"dwMessage: %u", pamd->dwMessage);

            SHELLAPPBARDATA sbd((APPBARDATAV1&)(pamd->abd));
            sbd.dwMessage = pamd->dwMessage;
            sbd.hSharedMemory = pamd->hSharedMemory;
            sbd.dwSourceProcessId = pamd->dwSourceProcessId;

            lResult = pTrayService->HandleAppBarMessage(&sbd);
          }
          break;

          case sizeof(APPBARMSGDATAV1) :
          {
            PAPPBARMSGDATAV1 pamd = (PAPPBARMSGDATAV1)pcds->lpData;

            TRACE(L"APPBARMSGDATAV1: %u", pamd->abd.cbSize);

            if (sizeof(APPBARDATAV1) != pamd->abd.cbSize)
              break;

            TRACE(L"dwMessage: %u", pamd->dwMessage);

            SHELLAPPBARDATA sbd((APPBARDATAV1&)(pamd->abd));
            sbd.dwMessage = pamd->dwMessage;
            sbd.hSharedMemory = pamd->hSharedMemory;
            sbd.dwSourceProcessId = pamd->dwSourceProcessId;

            lResult = pTrayService->HandleAppBarMessage(&sbd);
          }
          break;

          default:
          {
            TRACE(L"Unknown APPBARMSGDATA size: %u", pcds->cbData);
          }
          break;
        }
      }
      break;

      case SH_TRAY_DATA:
      {
        //
        // System Tray Notification
        //
        PSHELLTRAYDATA pstd = (PSHELLTRAYDATA)pcds->lpData;

        lResult = (LRESULT)pTrayService->HandleNotification(pstd);
      }
      break;

      case SH_LOADPROC_DATA:
      default:
      {
        TRACE(L"Unsupported tray message: %u", pcds->dwData);
      }
      break;
      }
    }
    break;

    case ABP_NOTIFYPOSCHANGED:
    {
      const BarVector abTempV = pTrayService->m_abVector;
      BarVector::const_reverse_iterator rit;

      HMONITOR hMon = (HMONITOR)lParam;
      HWND hSkip = (HWND)wParam;
      AppBar* p = NULL;

      for (rit = abTempV.rbegin(); rit != abTempV.rend(); rit++)
      {
        if (hSkip != (*rit)->hWnd())
        {
          if ((*rit)->hMon() == hMon && (!p || p->uEdge() != (*rit)->uEdge()))
          {
            SendMessage(
              (*rit)->hWnd()
              , (*rit)->uMsg()
              , ABN_POSCHANGED
              , 0
            );
          }
        }
        else
        {
          p = *rit;
        }
      }

      pTrayService->adjustWorkArea(hMon);
    }
    break;

    case ABP_NOTIFYSTATECHANGE:
    {
      const BarVector abTempV = pTrayService->m_abVector;
      BarVector::const_reverse_iterator rit;

      for (rit = abTempV.rbegin(); rit != abTempV.rend(); rit++)
      {
        if (!(*rit)->IsOverLap())
        {
          SendMessage(
            (*rit)->hWnd()
            , (*rit)->uMsg()
            , ABN_STATECHANGE
            , 0
          );
        }
      }
    }
    break;

    case ABP_RAISEAUTOHIDEHWND:
    {
      HWND hRaise = (HWND)wParam;
      //UINT uEdge = (UINT)lParam;

      SetWindowPos(
        hRaise, HWND_TOP
        , 0, 0, 0, 0
        , SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER
      );
    }
    break;

    case WM_SETTINGCHANGE:
    {
      if (SPI_SETWORKAREA == wParam)
      {
        RECT rc;

        SystemParametersInfo(
          SPI_GETWORKAREA
          , 0
          , &rc
          , 0
        );

        // Always update the Current workarea
        CopyRect(&pTrayService->m_rWorkAreaCur, &rc);

        // This will be the case when we updated the workarea ourselves.
        if (pTrayService->m_uWorkAreaDirty)
        {
          pTrayService->m_uWorkAreaDirty--;
          break;
        }

        // Only update the Default workarea when set externally.
        CopyRect(&pTrayService->m_rWorkAreaDef, &rc);

        // Clean up any dead app bars before repositioning
        pTrayService->removeDeadAppBars();

        // Now reposition our appbars based on the new default workarea.
        PostMessage(
          pTrayService->m_hTrayWnd
          , ABP_NOTIFYPOSCHANGED
          , (WPARAM)NULL
          , (LPARAM)LSMonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY)
        );
      }
    }
    break;

    default:
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return lResult;
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// WindowNotifyProc
//
// Redirects all messages to the appropriate handler
//
LRESULT CALLBACK TrayService::WindowNotifyProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// HandleAppBarMessage
//
// Handler for all AppBar Messages
//
LRESULT TrayService::HandleAppBarMessage(PSHELLAPPBARDATA psad)
{
  LRESULT lResult = 0;

  removeDeadAppBars();

  switch (psad->dwMessage)
  {
  case ABM_NEW:
    lResult = barCreate(psad->abd);
    break;

  case ABM_REMOVE:
    lResult = barDestroy(psad->abd);
    break;

  case ABM_QUERYPOS:
    lResult = barQueryPos(psad);
    break;

  case ABM_SETPOS:
    lResult = barSetPos(psad);
    break;

  case ABM_GETSTATE:
    lResult = barGetTaskBarState();
    break;

  case ABM_GETTASKBARPOS:
    lResult = barGetTaskBarPos(psad);
    break;

  case ABM_ACTIVATE:
    lResult = barActivate(psad->abd);
    break;

  case ABM_GETAUTOHIDEBAR:
    lResult = barGetAutoHide(psad->abd);
    break;

  case ABM_SETAUTOHIDEBAR:
    lResult = barSetAutoHide(psad->abd);
    break;

  case ABM_WINDOWPOSCHANGED:
    lResult = barPosChanged(psad->abd);
    break;

  case ABM_SETSTATE:
    lResult = barSetTaskBarState(psad->abd);
    break;

  default:
    TRACE(L"ABM unknown: %u", psad->dwMessage);
    break;
  }

  return lResult;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// NotifyRudeApp
//
// External interface to let us know a window has gone/left full screen mode
//
void TrayService::NotifyRudeApp(bool bIsFullScreen) const
{
  const BarVector abTempV = m_abVector;
  BarVector::const_reverse_iterator rit;

  for (rit = abTempV.rbegin(); rit != abTempV.rend(); rit++)
  {
    SendMessage(
      (*rit)->hWnd()
      , (*rit)->uMsg()
      , ABN_FULLSCREENAPP
      , (LPARAM)(bIsFullScreen ? 1 : 0)
    );
  }
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// removeDeadAppBars
//
// Removes all "dead" appBars.  ie. Those that no longer have a valid HWND
// associated with them.
//
void TrayService::removeDeadAppBars()
{
  BarVector::iterator it = m_abVector.begin();

  while (it != m_abVector.end())
  {
    if (IsWindow((*it)->hWnd()))
    {
      it++;
      continue;
    }

    delete* it;
    it = m_abVector.erase(it);
  }

  return;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// barCreate
//
// Creates a new AppBar.
//
// ABM_NEW
//
// Valid APPBARDATA members:
//
//  cbSize
//  hWnd
//  uCallbackMessage
//
LRESULT TrayService::barCreate(const APPBARDATAV1& abd)
{
  LRESULT lResult = 0;

  if (IsWindow(abd.hWnd) && !isBar(abd.hWnd))
  {
    m_abVector.push_back(new AppBar(abd.hWnd, abd.uCallbackMessage));
    lResult = 1;
  }

  return lResult;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// barDestroy
//
// Removes the specified AppBar.
//
// ABM_REMOVE
//
// Valid APPBARDATA members:
//
//  cbSize
//  hWnd
//
LRESULT TrayService::barDestroy(const APPBARDATAV1& abd)
{
  BarVector::iterator itBar;
  LRESULT lResult = 0;

  if (getBar(abd.hWnd, itBar))
  {
    lResult = 1;

    HMONITOR hMon = (*itBar)->hMon();

    delete* itBar;
    m_abVector.erase(itBar);

    PostMessage(
      m_hTrayWnd
      , ABP_NOTIFYPOSCHANGED
      , (WPARAM)NULL
      , (LPARAM)hMon
    );
  }

  return lResult;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// barQueryPos
//
// Queries for an acceptable position for the specified appbar
//
// ABM_QUERYPOS
//
// Valid APPBARDATA members:
//
//  cbSize
//  hWnd
//  uEdge
//  rc
//
LRESULT TrayService::barQueryPos(PSHELLAPPBARDATA psad)
{
  LRESULT lResult = 0;
  PAPPBARDATAV1 pabd = ABLock(psad);
  const APPBARDATAV1& abd = psad->abd;

  if (pabd)
  {
    AppBar* p;

    if (getBar(abd.hWnd, p))
    {
      lResult = 1;

      if (p->IsOverLap())
      {
        modifyOverlapBar(pabd->rc, abd.rc, abd.uEdge);
      }
      else
      {
        modifyNormalBar(pabd->rc, abd.rc, abd.uEdge, abd.hWnd);
      }
    }

    ABUnLock(pabd);
  }

  return lResult;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// barSetPos
//
// Sets the position of the specified appbar
//
// ABM_SETPOS
//
// Valid APPBARDATA members:
//
//  cbSize
//  hWnd
//  uEdge
//  rc
//
LRESULT TrayService::barSetPos(PSHELLAPPBARDATA psad)
{
  LRESULT lResult = 0;
  PAPPBARDATAV1 pabd = ABLock(psad);
  const APPBARDATAV1& abd = psad->abd;

  if (pabd)
  {
    AppBar* p;

    if (getBar(abd.hWnd, p))
    {
      lResult = 1;

      if (p->IsOverLap())
      {
        modifyOverlapBar(pabd->rc, abd.rc, abd.uEdge);
      }
      else
      {
        modifyNormalBar(pabd->rc, abd.rc, abd.uEdge, abd.hWnd);

        // If this is the first time to position the bar or if
        // the new position is different than the previous, then
        // notify all other appbars.
        if ((ABS_CLEANRECT != (ABS_CLEANRECT & p->lParam()))
          || !EqualRect(&p->GetRectRef(), &pabd->rc))
        {
          // Notify other bars
          PostMessage(
            m_hTrayWnd
            , ABP_NOTIFYPOSCHANGED
            , (WPARAM)abd.hWnd
            , (LPARAM)LSMonitorFromRect(&pabd->rc, MONITOR_DEFAULTTOPRIMARY)
          );
        }
      }

      // Update the appbar stored parameters
      CopyRect(&p->GetRectRef(), &pabd->rc);
      p->lParam(p->lParam() | ABS_CLEANRECT);
      p->uEdge(abd.uEdge);
      p->hMon(LSMonitorFromRect(&p->GetRectRef(), MONITOR_DEFAULTTOPRIMARY));
    }

    ABUnLock(pabd);
  }

  return lResult;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// barGetTaskBarState
//
// Returns the current TaskBar state (autohide, alwaysontop)
//
// ABM_GETSTATE
//
// Valid APPBARDATA members:
//
//  cbSize
//
LRESULT TrayService::barGetTaskBarState()
{
  // Zero means neither always-on-top nor autohide,
  // which is the safest guess for most LS setups
  return 0;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// barGetTaskBarPos
//
// Gets the current TaskBar position
//
// ABM_GETTASKBARPOS
//
// Valid APPBARDATA members:
//
//  cbSize
//  hWnd
//  uEdge (undocumented)
//
LRESULT TrayService::barGetTaskBarPos(PSHELLAPPBARDATA psad)
{
  LRESULT lResult = 0;
  PAPPBARDATAV1 pabd = ABLock(psad);

  if (pabd)
  {
    if (GetWindowRect(m_hNotifyWnd, &pabd->rc))
    {
      lResult = 1;

      MONITORINFO mi;
      mi.cbSize = sizeof(mi);

      if (!LSGetMonitorInfo(LSMonitorFromWindow(m_hNotifyWnd, MONITOR_DEFAULTTOPRIMARY), &mi))
      {
        SetRect(&mi.rcMonitor,
          0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
      }

      INT nHeight, nWidth, nScreenHeight, nScreenWidth;

      nHeight = pabd->rc.bottom - pabd->rc.top;
      nWidth = pabd->rc.right - pabd->rc.left;

      nScreenHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
      nScreenWidth = mi.rcMonitor.right - mi.rcMonitor.left;

      if (nHeight > nWidth)
      {
        if (pabd->rc.left > nScreenWidth / 2)
        {
          pabd->uEdge = ABE_RIGHT;
        }
        else
        {
          pabd->uEdge = ABE_LEFT;
        }
      }
      else
      {
        if (pabd->rc.top > nScreenHeight / 2)
        {
          pabd->uEdge = ABE_BOTTOM;
        }
        else
        {
          pabd->uEdge = ABE_TOP;
        }
      }
    }

    ABUnLock(pabd);
  }

  return lResult;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// barActivate
//
// Ensures that any autohide appbar has the topmost zOrder when any appbar is
// activated.
//
// ABM_ACTIVATE
//
// Valid APPBARDATA members:
//
//  cbSize
//  hWnd
//
LRESULT TrayService::barActivate(const APPBARDATAV1& abd)
{
  LRESULT lResult = 0;
  AppBar* p;

  if (getBar(abd.hWnd, p))
  {
    lResult = 1;

    BarVector::iterator itBar = findBar(p->hMon(), p->uEdge(), ABS_AUTOHIDE);

    if (itBar != m_abVector.end())
    {
      PostMessage(
        m_hTrayWnd
        , ABP_RAISEAUTOHIDEHWND
        , (WPARAM)(*itBar)->hWnd()
        , (LPARAM)(*itBar)->uEdge()
      );
    }
  }

  return lResult;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// barGetAutoHide
//
// Returns the HWND of the autohide appbar on the specified screen edge
//
// ABM_GETAUTOHIDEBAR
//
// Valid APPBARDATA members:
//
//  cbSize
//  hWnd
//  uEdge
//
LRESULT TrayService::barGetAutoHide(const APPBARDATAV1& abd)
{
  LRESULT lResult = 0;
  BarVector::iterator itBar = findBar(LSMonitorFromWindow(abd.hWnd, MONITOR_DEFAULTTOPRIMARY), abd.uEdge, ABS_AUTOHIDE);

  if (itBar != m_abVector.end())
  {
    lResult = (LRESULT)(*itBar)->hWnd();
  }

  return lResult;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// barSetAutoHide
//
// Sets the specified AppBar to autohide if it already exists, otherwise
// if the specified AppBar does not exist, it creates it and sets it to as
// an overlap autohide AppBar.
//
// ABM_SETAUTOHIDEBAR
//
// Valid APPBARDATA members:
//
//  cbSize
//  hWnd
//  uEdge
//  lParam
//
LRESULT TrayService::barSetAutoHide(const APPBARDATAV1& abd)
{
  LRESULT lResult = 0;

  BarVector::iterator itBar = findBar(abd.hWnd);
  BarVector::iterator itAutoHideBar = findBar(LSMonitorFromWindow(abd.hWnd, MONITOR_DEFAULTTOPRIMARY), abd.uEdge, ABS_AUTOHIDE);

  if (abd.lParam) // Set Auto Hide
  {
    lResult = 1;

    // Does an autohide bar already exist on this edge?
    if (itAutoHideBar != m_abVector.end())
    {
      // Return true if this appbar is the autohide bar.
      return (itBar == itAutoHideBar);
    }

    // if bar doesn't exist, create it and set it as an overlap bar
    if (itBar == m_abVector.end())
    {
      if (!barCreate(abd))
      {
        return 0;
      }

      itBar = m_abVector.end() - 1;
      (*itBar)->lParam((*itBar)->lParam() | ABS_OVERLAPAUTOHIDE);
    }

    // set its assigned edge, and set it as an autohide bar
    (*itBar)->uEdge(abd.uEdge);
    (*itBar)->lParam((*itBar)->lParam() | ABS_AUTOHIDE);
  }
  else // Clear Auto Hide
  {
    // if the bar was the auto hide bar proceed
    if (itBar != m_abVector.end() && itBar == itAutoHideBar)
    {
      lResult = 1;

      // if bar was overlap, destroy it
      if ((*itBar)->IsOverLap())
      {
        barDestroy(abd);
      }
      else
      {
        // otherwise clear its autohide status.
        (*itBar)->lParam((*itBar)->lParam() & ~ABS_AUTOHIDE);
      }
    }
  }

  return lResult;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// barPosChanged
//
// Ensures that any autohide appbar has the topmost zOrder when any non autohide
// appbar's position has changed.
//
// ABM_WINDOWPOSCHANGED
//
// Valid APPBARDATA members:
//
//  cbSize
//  hWnd
//
LRESULT TrayService::barPosChanged(const APPBARDATAV1& abd)
{
  LRESULT lResult = 0;
  AppBar* p;

  if (getBar(abd.hWnd, p) && !p->IsAutoHide())
  {
    lResult = 1;

    BarVector::iterator itBar = findBar(p->hMon(), p->uEdge(), ABS_AUTOHIDE);

    if (itBar != m_abVector.end())
    {
      PostMessage(
        m_hTrayWnd
        , ABP_RAISEAUTOHIDEHWND
        , (WPARAM)(*itBar)->hWnd()
        , (LPARAM)(*itBar)->uEdge()
      );
    }
  }

  return lResult;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// barSetTaskBarState
//
// Sets the current TaskBar state (autohide, alwaysontop)
//
// ABM_SETSTATE
//
// Valid APPBARDATA members:
//
//  cbSize
//  hWnd
//  lParam
//
LRESULT TrayService::barSetTaskBarState(const APPBARDATAV1& abd)
{
  return 0;
  UNREFERENCED_PARAMETER(abd);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// modifyOverlapBar
//
// Helper function to barSetPos and barQueryPos
//
void TrayService::modifyOverlapBar(RECT& rcDst, const RECT& rcOrg, UINT uEdge)
{
  // Use entire screen for default rectangle
  HMONITOR hMon = LSMonitorFromRect(&rcOrg, MONITOR_DEFAULTTOPRIMARY);

  MONITORINFO mi;
  mi.cbSize = sizeof(mi);

  if (LSGetMonitorInfo(hMon, &mi))
  {
    CopyRect(&rcDst, &mi.rcMonitor);
  }
  else
  {
    SetRect(&rcDst,
      0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
  }

  // Set the bar's extent
  modifyBarExtent(rcDst, rcOrg, uEdge);

  // The bar's position is anchored at the desktop edge - so nothing to do

  // Set the bar's breadth
  modifyBarBreadth(rcDst, rcOrg, uEdge);

  return;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// modifyNormalBar
//
// Helper function to barSetPos and barQueryPos
//
void TrayService::modifyNormalBar(RECT& rcDst, const RECT& rcOrg, UINT uEdge, HWND hWnd)
{
  BarVector::const_iterator it;
  AppBar* p;
  bool bFound = FALSE;

  // Use entire screen for default rectangle
  HMONITOR hMon = LSMonitorFromRect(&rcOrg, MONITOR_DEFAULTTOPRIMARY);
  HMONITOR hMonPrimary = LSMonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);

  if (hMonPrimary == hMon)
  {
    // Use only the original workarea for the default rectangle
    CopyRect(&rcDst, &m_rWorkAreaDef);
  }
  else
  {
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);

    if (LSGetMonitorInfo(hMon, &mi))
    {
      CopyRect(&rcDst, &mi.rcMonitor);
    }
    else
    {
      ASSERT(FALSE);

      // Use only the original workarea for the default rectangle
      CopyRect(&rcDst, &m_rWorkAreaDef);
      hMon = hMonPrimary;
    }
  }

  // Set the bar's extent
  modifyBarExtent(rcDst, rcOrg, uEdge);

  // Set the bar's position
  for (it = m_abVector.begin(); it != m_abVector.end(); it++)
  {
    p = *it;

    // ignore overlap bars and invalid windows and other monitors
    if (!p || p->IsOverLap() || hMon != p->hMon())
    {
    }
    // if this is our appbar, mark it found and continue
    else if (hWnd == p->hWnd())
    {
      bFound = true;
    }
    // ignore bars that haven't been initialized
    else if (ABS_CLEANRECT != (ABS_CLEANRECT & p->lParam()))
    {
    }
    // Ignore other edge appbars for the most part
    else if (uEdge != p->uEdge())
    {
      // Left/Right bars must resize to top/bottom bars
      if (ABE_HORIZONTAL != (ABE_HORIZONTAL & uEdge)
        &&
        ABE_HORIZONTAL == (ABE_HORIZONTAL & p->uEdge()))
      {
        RECT rc; // dummy

        // If our destination rectangle currently intersects
        // then resize it
        if (IntersectRect(&rc, &p->GetRectRef(), &rcDst))
        {
          if (ABE_TOP == p->uEdge())
          {
            rcDst.top = p->GetRectRef().bottom;
          }
          else
          {
            rcDst.bottom = p->GetRectRef().top;
          }
        }
      }
    }
    // Keep moving the destination appbar while we are looking
    // at same edge appbars before our own.  Once we find our
    // own, then we ignore all same edge appbars after ours.
    else if (!bFound)
    {
      switch (uEdge)
      {
      case ABE_LEFT:
        rcDst.left = p->GetRectRef().right;
        break;
      case ABE_TOP:
        rcDst.top = p->GetRectRef().bottom;
        break;
      case ABE_RIGHT:
        rcDst.right = p->GetRectRef().left;
        break;
      case ABE_BOTTOM:
        rcDst.bottom = p->GetRectRef().top;
        break;
      }
    }
  }

  // Set the bar's breadth
  modifyBarBreadth(rcDst, rcOrg, uEdge);

  return;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// modifyBarExtent
//
// Helper function to modifyNormalBar and modifyOverlapBar
//
void TrayService::modifyBarExtent(RECT& rcDst, const RECT& rcOrg, UINT uEdge)
{
  switch (uEdge)
  {
  case ABE_LEFT:
  case ABE_RIGHT:
    rcDst.top = max(rcDst.top, rcOrg.top);
    rcDst.bottom = min(rcDst.bottom, rcOrg.bottom);
    break;
  case ABE_TOP:
  case ABE_BOTTOM:
    rcDst.left = max(rcDst.left, rcOrg.left);
    rcDst.right = min(rcDst.right, rcOrg.right);
    break;
  }

  return;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// modifyBarBreadth
//
// Helper function to modifyBarNormal and modifyBarOverlap
//
void TrayService::modifyBarBreadth(RECT& rcDst, const RECT& rcOrg, UINT uEdge)
{
  switch (uEdge)
  {
  case ABE_LEFT:
    rcDst.right = rcDst.left + (rcOrg.right - rcOrg.left);
    break;
  case ABE_TOP:
    rcDst.bottom = rcDst.top + (rcOrg.bottom - rcOrg.top);
    break;
  case ABE_RIGHT:
    rcDst.left = rcDst.right - (rcOrg.right - rcOrg.left);
    break;
  case ABE_BOTTOM:
    rcDst.top = rcDst.bottom - (rcOrg.bottom - rcOrg.top);
    break;
  }

  return;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// adjustWorkArea
//
// Helper function to barSetPos (via ABP_NOTIFYPOSCHANGED)
//
void TrayService::adjustWorkArea(HMONITOR hMon)
{
  RECT rcWorker, rcMonitor, rcWorkArea;

  HMONITOR hMonPrimary = LSMonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);

  if (hMon != hMonPrimary)
  {
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);

    if (!LSGetMonitorInfo(hMon, &mi))
    {
      return;
    }

    CopyRect(&rcMonitor, &mi.rcMonitor);
    CopyRect(&rcWorkArea, &mi.rcWork);
  }
  else
  {
    CopyRect(&rcMonitor, &m_rWorkAreaDef); // yes, we actually want this
    CopyRect(&rcWorkArea, &m_rWorkAreaCur);
  }

  CopyRect(&rcWorker, &rcMonitor);

  for (
    BarVector::iterator it = m_abVector.begin()
    ; it != m_abVector.end()
    ; it++
    )
  {
    AppBar* p = *it;

    if (p && !p->IsOverLap() && p->hMon() == hMon && (ABS_CLEANRECT == (ABS_CLEANRECT & p->lParam())))
    {
      if (!IsWindow(p->hWnd())) continue;

      RECT rcBarEx;

      CopyRect(&rcBarEx, &rcMonitor);

      switch (p->uEdge())
      {
      case ABE_LEFT:
        rcBarEx.right = p->GetRectRef().right;
        break;
      case ABE_RIGHT:
        rcBarEx.left = p->GetRectRef().left;
        break;
      case ABE_TOP:
        rcBarEx.bottom = p->GetRectRef().bottom;
        break;
      case ABE_BOTTOM:
        rcBarEx.top = p->GetRectRef().top;
        break;
      }

      SubtractRect(&rcWorker, &rcWorker, &rcBarEx);
    }
  }

  if (!EqualRect(&rcWorker, &rcWorkArea))
  {
    setWorkArea(&rcWorker);
  }

  return;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// setWorkArea
//
// Helper function for setting the specified work area
//
void TrayService::setWorkArea(LPRECT prcWorkArea)
{
  if (m_hTrayWnd)
  {
    m_uWorkAreaDirty++;
  }

  SystemParametersInfo(
    SPI_SETWORKAREA
    , 1 // readjust maximized windows
    , prcWorkArea
    , 0 // SPIF_SENDCHANGE - see below
  );
  SendNotifyMessage(    // used in place of SPIF_SENDCHANGE above to avoid lockups
    HWND_BROADCAST   // see http://blogs.msdn.com/oldnewthing/archive/2005/03/10/392118.aspx
    , WM_SETTINGCHANGE
    , SPI_SETWORKAREA
    , 0
  );
  return;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ABLock
//
// Calls SHLockShared API for the specified Shell AppBar memory
//
// Returns a pointer to shared memory location for an AppBarData structure
//
PAPPBARDATAV1 TrayService::ABLock(PSHELLAPPBARDATA psad)
{
  if (NULL != fpSHLockShared && NULL != psad->hSharedMemory)
  {
    return (PAPPBARDATAV1)fpSHLockShared(psad->hSharedMemory, psad->dwSourceProcessId);
  }

  return NULL;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ABUnlock
//
// Calls SHUnlockShared API for the specified AppBarData* memory
//
void TrayService::ABUnLock(PAPPBARDATAV1 pabd)
{
  if (NULL != fpSHUnlockShared && NULL != pabd)
  {
    fpSHUnlockShared(pabd);
  }
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// FindAppBarPredicate_hWnd
//
// Predicate for std::find_if, used by findBar variants
// Needs to be at global scope because of mingw issues
//
// Find the appbar matching the specified hWnd
//
struct FindAppBarPredicate_hWnd
{
  FindAppBarPredicate_hWnd(HWND hWnd) : m_hWnd(hWnd)
  {
  }

  bool operator() (const AppBar* pab) const
  {
    return (pab->hWnd() == m_hWnd);
  }

private:
  const HWND m_hWnd;
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// FindAppBarPredicate_MatchLParam
//
// Predicate for std::find_if, used by findBar variants
// Needs to be at global scope because of mingw issues
//
// Find the appbar on the specified edge that has a matching lParam value
//
struct FindAppBarPredicate_MatchLParam
{
  FindAppBarPredicate_MatchLParam(HMONITOR hMon, UINT uEdge, LPARAM lParam) : m_hMon(hMon), m_uEdge(uEdge), m_lParam(lParam)
  {
  }

  bool operator() (const AppBar* pab) const
  {
    return (pab->hMon() == m_hMon && pab->uEdge() == m_uEdge && m_lParam == (pab->lParam() & m_lParam));
  }

private:
  const HMONITOR m_hMon;
  const UINT m_uEdge;
  const LPARAM m_lParam;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// findBar
//
// Looks up an AppBar in the AppBarList
//
BarVector::iterator TrayService::findBar(HWND hWnd)
{
  return std::find_if(m_abVector.begin(), m_abVector.end(),
    FindAppBarPredicate_hWnd(hWnd));
}
BarVector::iterator TrayService::findBar(HMONITOR hMon, UINT uEdge, LPARAM lParam)
{
  return std::find_if(m_abVector.begin(), m_abVector.end(),
    FindAppBarPredicate_MatchLParam(hMon, uEdge, lParam));
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// isBar
//
// Determines whether the HWND is associated with any current AppBar.
//
bool TrayService::isBar(HWND hWnd)
{
  return m_abVector.end() != findBar(hWnd);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// getBar
//
// Looks up an BarVector iterator based on the HWND
//
// returns true if found, otherwise false.
//
bool TrayService::getBar(HWND hWnd, BarVector::iterator& itBar)
{
  bool bReturn = false;

  itBar = findBar(hWnd);

  if (itBar != m_abVector.end())
  {
    bReturn = true;
  }

  return bReturn;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// getBar
//
// Looks up an AppBar* based on the HWND
//
// Returns true if found, otherwise false.
//
bool TrayService::getBar(HWND hWnd, AppBar*& pBarRef)
{
  bool bReturn = false;
  BarVector::iterator itBar;

  if (getBar(hWnd, itBar))
  {
    pBarRef = *itBar;
    bReturn = true;
  }

  return bReturn;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// HandleNotification
//
// Handler for all system tray notifications
//
BOOL TrayService::HandleNotification(PSHELLTRAYDATA pstd)
{
  bool bReturn = false;

  switch (pstd->dwMessage)
  {
  case NIM_ADD:
  {
    bReturn = addIcon(pstd->nid);
  }
  break;

  case NIM_MODIFY:
  {
    bReturn = modifyIcon(pstd->nid);
  }
  break;

  case NIM_DELETE:
  {
    bReturn = deleteIcon(pstd->nid);
  }
  break;

  case NIM_SETFOCUS:
  {
    bReturn = setFocusIcon(pstd->nid);
  }
  break;

  case NIM_SETVERSION:
  {
    bReturn = setVersionIcon(pstd->nid);
  }
  break;

  default:
  {
    TRACE(L"NIM unknown: %u", pstd->dwMessage);
  }
  break;
  }

  return bReturn ? TRUE : FALSE;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// notify
//
// Notify all listeners of a systray event
//
bool TrayService::notify(DWORD dwMessage, PCLSNOTIFYICONDATA pclsnid) const
{
  return 0 != SendMessage(m_hLiteStep, LM_SYSTRAY, dwMessage, (LPARAM)pclsnid);
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// SendSystemTray
//
// Resend all icon data; systray modules will request this via LM_SYSTRAYREADY
// during their startup.
//
HWND TrayService::SendSystemTray()
{
  removeDeadIcons();

  for (IconVector::const_iterator it = m_siVector.begin();
    it != m_siVector.end(); ++it)
  {
    if ((*it)->IsValid())
    {
      LSNOTIFYICONDATA lsnid = { 0 };

      (*it)->CopyLSNID(&lsnid);

      notify(NIM_ADD, &lsnid);
    }
  }

  return m_hNotifyWnd;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// removeDeadIcons
//
// Removes all "dead" icons.  ie. Those that no longer have a valid HWND
// associated with them.
//
void TrayService::removeDeadIcons()
{
  IconVector::iterator it = m_siVector.begin();

  while (it != m_siVector.end())
  {
    if (IsWindow((*it)->GetHwnd()))
    {
      it++;
      continue;
    }

    delete* it;
    it = m_siVector.erase(it);
  }

  return;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// extendNIDCopy
//
// Copies all non-persistent data that is not stored in NotifyIcon.
//
bool TrayService::extendNIDCopy(LSNOTIFYICONDATA& lsnid, const NID_XX& nid) const
{
  // Copy Info Tips
  if (nid.uFlags & NIF_INFO)
  {
    switch (nid.cbSize)
    {
#ifdef _UNICODE
    case NID_6W_SIZE:
    case NID_5W_SIZE:
    {
      NID_5W* pnid = (NID_5W*)&nid;

      int nReturn;
      //WCHAR szTemp[256];
     //szTemp[63] = 0;

      // 1. Handle szInfo (Unicode)
      std::wstring wszInfoSrc(pnid->szInfo);

      // 2. Truncate if necessary (using std::min)
      size_t charsToCopyInfo = (std::min)(wszInfoSrc.length(), (size_t)TRAY_MAX_INFO_LENGTH - 1);

      // 3. Copy safely to WCHAR array (using wcsncpy_s)
      wcsncpy_s(lsnid.szInfo, TRAY_MAX_INFO_LENGTH, wszInfoSrc.c_str(), charsToCopyInfo);

      // 4. Ensure null termination (important!)
      lsnid.szInfo[charsToCopyInfo] = L'\0';


      // 5. Handle szInfoTitle (Unicode)
      std::wstring wszInfoTitleSrc(pnid->szInfoTitle);

      // 6. Truncate if necessary
      size_t charsToCopyTitle = (std::min)(wszInfoTitleSrc.length(), (size_t)TRAY_MAX_INFOTITLE_LENGTH - 1);

      // 7. Copy safely to WCHAR array
      wcsncpy_s(lsnid.szInfoTitle, TRAY_MAX_INFOTITLE_LENGTH, wszInfoTitleSrc.c_str(), charsToCopyTitle);

      // 8. Ensure null termination
      lsnid.szInfoTitle[charsToCopyTitle] = L'\0';

      lsnid.dwInfoFlags = pnid->dwInfoFlags;
      lsnid.uTimeout = pnid->uTimeout;
      lsnid.uFlags |= NIF_INFO;
    }
    break;
#else
    case NID_6A_SIZE:
    case NID_5A_SIZE:
    {
      NID_5A* pnid = (NID_5A*)&nid;

      HRESULT hr;

      hr = StringCchCopy(lsnid.szInfo, TRAY_MAX_INFO_LENGTH, pnid->szInfo);

      if (FAILED(hr))
      {
        lsnid.szInfo[0] = 0;
      }

      hr = StringCchCopy(lsnid.szInfoTitle, TRAY_MAX_INFOTITLE_LENGTH, pnid->szInfoTitle);

      if (FAILED(hr))
      {
        lsnid.szInfoTitle[0] = 0;
      }

      lsnid.dwInfoFlags = pnid->dwInfoFlags;
      lsnid.uTimeout = pnid->uTimeout;
      lsnid.uFlags |= NIF_INFO;
    }
    break;
#endif
    default:
      break;
    }
  }

  // Copy GUID
  if (nid.uFlags & NIF_GUID)
  {
    switch (nid.cbSize)
    {
    case NID_6W_SIZE:
    {
      NID_6W* pnid = (NID_6W*)&nid;

      lsnid.guidItem = pnid->guidItem;
      lsnid.uFlags |= NIF_GUID;
    }
    break;
    case NID_6A_SIZE:
    {
      NID_6A* pnid = (NID_6A*)&nid;

      lsnid.guidItem = pnid->guidItem;
      lsnid.uFlags |= NIF_GUID;
    }
    break;
    default:
      break;
    }
  }

  return true;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// addIcon
//
bool TrayService::addIcon(const NID_XX& nid)
{
  bool bReturn = false;

  if (m_siVector.end() == findIcon(nid))
  {
    NotifyIcon* pni = new NotifyIcon(nid);

    if (pni)
    {
      // Fail shared icons, unless a valid hIcon exists
      if (IsWindow(pni->GetHwnd()) && (!pni->IsShared() || pni->HasIcon()))
      {
        m_siVector.push_back(pni);

        if (pni->IsValid())
        {
          LSNOTIFYICONDATA lsnid = { 0 };

          pni->CopyLSNID(&lsnid);
          extendNIDCopy(lsnid, nid);

          notify(NIM_ADD, &lsnid);
        }
        bReturn = true;
      }
      else
      {
        delete pni;
      }
    }
  }

  return bReturn;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// modifyIcon
//
bool TrayService::modifyIcon(const NID_XX& nid)
{
  bool bReturn = false;

  IconVector::iterator it = findIcon(nid);

  if (m_siVector.end() != it && *it)
  {
    NotifyIcon* pni = *it;

    bool bWasValid = pni->IsValid();

    // Update stored NotifyIcon
    pni->Update(nid);

    if (pni->IsValid())
    {
      LSNOTIFYICONDATA lsnid = { 0 };

      if (!bWasValid)
      {
        // This is a "new" icon, send entire stored NotifyIcon
        pni->CopyLSNID(&lsnid);
        extendNIDCopy(lsnid, nid);

        notify(NIM_ADD, &lsnid);
      }
      else
      {
        // This icon already exists, just send updated flags
        pni->CopyLSNID(&lsnid, nid.uFlags);
        extendNIDCopy(lsnid, nid);

        notify(NIM_MODIFY, &lsnid);
      }
    }
    else if (bWasValid)
    {
      LSNOTIFYICONDATA lsnid = {
           sizeof(LSNOTIFYICONDATA)
          ,pni->GetHwnd()
          ,pni->GetuID()
          ,0
      };

      // This icon is no longer visible, remove
      notify(NIM_DELETE, &lsnid);
    }

    bReturn = true;
  }

  return bReturn;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// deleteIcon
//
// Remove an icon from the icon list
//
bool TrayService::deleteIcon(const NID_XX& nid)
{
  bool bReturn = false;

  IconVector::iterator it = findIcon(nid);

  if (m_siVector.end() != it)
  {
    LSNOTIFYICONDATA lsnid = {
         sizeof(LSNOTIFYICONDATA)
        ,(*it)->GetHwnd()
        ,(*it)->GetuID()
        ,0
    };

    notify(NIM_DELETE, &lsnid);

    delete* it;
    m_siVector.erase(it);

    bReturn = true;
  }

  return bReturn;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// setFocusIcon
//
// Sets selection focus to the specified icon
//
bool TrayService::setFocusIcon(const NID_XX& nid)
{
  bool bReturn = false;

  IconVector::iterator it = findIcon(nid);

  if (m_siVector.end() != it)
  {
    LSNOTIFYICONDATA lsnid = {
         sizeof(LSNOTIFYICONDATA)
        ,(*it)->GetHwnd()
        ,(*it)->GetuID()
        ,0
    };

    bReturn = notify(NIM_SETFOCUS, &lsnid);
  }

  return bReturn;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// setVersionIcon
//
// Sets compatibility version for specified icon
//
bool TrayService::setVersionIcon(const NID_XX& nid)
{
  bool bReturn = false;

  IconVector::iterator it = findIcon(nid);

  if (m_siVector.end() != it)
  {
    LSNOTIFYICONDATA lsnid = {
         sizeof(LSNOTIFYICONDATA)
        ,(*it)->GetHwnd()
        ,(*it)->GetuID()
        ,0
    };

    switch (nid.cbSize)
    {
    case NID_6W_SIZE:
    case NID_5W_SIZE:
      lsnid.uVersion = ((NID_5W&)nid).uVersion;
      break;

    case NID_6A_SIZE:
    case NID_5A_SIZE:
      lsnid.uVersion = ((NID_5A&)nid).uVersion;
      break;

    default:
      break;
    }

    bReturn = notify(NIM_SETVERSION, &lsnid);
  }

  return bReturn;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// FindIconPredicate
//
// Predicate for std::find_if, used by findIcon
// Needs to be at global scope because of mingw issues
//
struct FindIconPredicate
{
  FindIconPredicate(HWND hWnd, UINT uID) : m_hWnd(hWnd), m_uID(uID) {}

  bool operator() (const NotifyIcon* pni) const
  {
    return (pni->GetHwnd() == m_hWnd && pni->GetuID() == m_uID);
  }

private:
  const HWND m_hWnd;
  const UINT m_uID;
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// findIcon
//
// Looks up an icon in the SystrayIconList
//
IconVector::iterator TrayService::findIcon(HWND hWnd, UINT uId)
{
  return std::find_if(m_siVector.begin(),
    m_siVector.end(), FindIconPredicate(hWnd, uId));
}

