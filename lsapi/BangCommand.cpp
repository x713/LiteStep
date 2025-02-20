/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2025 The x7 Dev Team
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
#include "BangCommand.h"
#include "ThreadedBangCommand.h"
#include "../utility/core.hpp"


Bang::Bang(DWORD dwThread, BangCommand pfnBang, LPCWSTR pwszCommand)
  :m_wszCommand(pwszCommand)
{
  m_bEX = false;
  m_bBang = pfnBang;
  m_dwThreadID = dwThread;
}

Bang::Bang(DWORD dwThread, BangCommandEx pfnBang, LPCWSTR pwszCommand)
  :m_wszCommand(pwszCommand)
{
  m_bEX = true;
  m_bBangEX = pfnBang;
  m_dwThreadID = dwThread;
}

Bang::~Bang()
{
}

void Bang::Execute(HWND hCaller, LPCWSTR pwszParams)
{
  DWORD dwThreadID = GetCurrentThreadId();

  if (dwThreadID != m_dwThreadID)
  {
    ThreadedBangCommand* pInfo = \
      new ThreadedBangCommand(hCaller, m_wszCommand.c_str(), pwszParams);

    if (pInfo != NULL)
    {
      // target thread releases pInfo
      PostThreadMessage(m_dwThreadID, LM_THREAD_BANGCOMMAND, (WPARAM)pInfo, 0);
    }
  }
  else
  {
#if !defined(LS_NO_EXCEPTION)
    try
    {
#endif /* LS_NO_EXCEPTION */
      if (m_bEX)
      {
        m_bBangEX(hCaller, m_wszCommand.c_str(), pwszParams);
      }
      else
      {
        m_bBang(hCaller, pwszParams);
      }
#if !defined(LS_NO_EXCEPTION)
    }
    catch (...)
    {
      const wchar_t* m1 = m_wszCommand.c_str();
      const wchar_t* pp = pwszParams != NULL ? pwszParams : L"(null)";
      /*
      * TODO : fix this
      ErrorEx(m_wszCommand.c_str(), IDS_LITESTEP_BANGEXCEPTION,
        L"Exception during bang command execution.\nPlease contact the module author.\n\nBang parameters: %s",
        pp);
        */
    }
#endif /* LS_NO_EXCEPTION */
  }
}

