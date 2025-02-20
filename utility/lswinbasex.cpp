/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 1997-2002 The LiteStep Development Team

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

#define STRSAFE_NO_DEPRECATE
#include "lswinbasex.h"

WindowX::WindowX(LPCWSTR className) :
  Window(className),
  hDesktop(NULL)
{
  bAlwaysOnTop = false;
}

WindowX::~WindowX()
{
}

void WindowX::windowProc(Message& message)
{
  BEGIN_MESSAGEPROC
    MESSAGE(onEndSession, WM_ENDSESSION)
    MESSAGE(onEndSession, WM_QUERYENDSESSION)
    MESSAGE(onSysCommand, WM_SYSCOMMAND)
    MESSAGE(onGetRevId, LM_GETREVID)
  default:
    message.lResult = DefWindowProc(hWnd, message.uMsg, message.wParam, message.lParam);
}
}

void WindowX::onEndSession(Message& message)
{
  message.lResult = SendMessage(hParent, message.uMsg, message.wParam, message.lParam);
}

void WindowX::onSysCommand(Message& message)
{
  if (message.wParam == SC_CLOSE)
    PostMessage(hParent, WM_KEYDOWN, LM_SHUTDOWN, 0);
  else
    message.lResult = DefWindowProc(hWnd, message.uMsg, message.wParam, message.lParam);
}

void WindowX::onGetRevId(Message& message)
{
  LPWSTR buf = (LPWSTR)(message.lParam);
  WCHAR path[MAX_PATH + 1] = { 0 };
  int len = 0;

  GetModuleFileName(hInstance, path, MAX_PATH);
  //len = swprintf(buf, L"%s: %s", wcsrchr(path, '\\') + 1, Revision());
  len = swprintf(buf, MAX_PATH, L"%s: %s", wcsrchr(path, '\\') + 1, Revision());

  if (0 > len) // swprintf failed, null out buffer
  {
    buf[0] = '\0';
    len = 0;
  }

  message.lResult = len;
}

void WindowX::onTop(bool bSetAlwaysOnTop)
{
  if ((bSetAlwaysOnTop != bAlwaysOnTop) && (hWnd))
  {
    bAlwaysOnTop = bSetAlwaysOnTop;

    if (!hDesktop)
    {
      hDesktop = FindWindow(L"DesktopBackgroundClass", NULL);
      if (!hDesktop)
      {
        hDesktop = GetDesktopWindow();
      }
    }

    // Set the parent correctly
    //SetWindowLong(GWL_STYLE, (GetStyle() & ~WS_POPUP) | WS_CHILD);
    SetParent(bAlwaysOnTop ? NULL : hDesktop);
    //SetWindowLong(GWL_STYLE, (GetStyle() & ~WS_CHILD) | WS_POPUP);
    SetWindowPos(bAlwaysOnTop ? HWND_TOPMOST : HWND_BOTTOM, 0, 0, 0, 0,
      SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  }
}
