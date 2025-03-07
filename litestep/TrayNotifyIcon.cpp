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
#include "TrayNotifyIcon.h"
#include "../utility/safestr.h"


#include <algorithm> // For std::min

NotifyIcon::IcVtr NotifyIcon::s_icVtr;


//
// ReadMe
//
// The Notification Icons are an interesting beast.  Especially when
// we try to implement a persistent store of them.  Notification Icons
// contain a number of different member data types, each with a different
// lifetime.
//
// The persistent non-volatile members are:
//
// uID and hWnd
//
// Those two members are used to identify a specific Notification Icon
// and are use to communicate back with the owner of the Notification Icon.
//
// The persistent volatile members are:
//
// uFlags, uCallbackMessage, hIcon, szTip, dwState
//
// The only required valid member is hIcon.  The Notification Icon will
// not be displayed until hIcon has been populated.  However, the calling
// application will see a success during NIM_ADD even when hIcon is NULL
// with the exception if NIS_SHAREDICON is set then hIcon must be populated
// for NIM_ADD to return success.
//
// dwState/dwStateMask may also keep the Notification Icon from being
// displayed when set to NIS_HIDDEN.
//
// The rest of the members are non-persistent and are not stored.  They
// are passed on to the systray modules one time and are not repopulated
// on enumeration of the Notification Icons.
//

NotifyIcon::NotifyIcon(const NID_XX& nidSource)
  :m_hWnd(nidSource.hWnd)
  , m_uID(nidSource.uID)
  , m_uFlags(0)
  , m_uCallbackMessage(0)
  , m_hIcon(NULL)
  , m_hOriginalIcon(NULL)
  , m_dwState(0)
{
  m_wszTip[0] = 0;
  Update(nidSource);

  s_icVtr.push_back(this);
}

NotifyIcon::~NotifyIcon()
{
  if (NULL != m_hIcon)
  {
    DestroyIcon(m_hIcon);
  }

  for (IcVtr::iterator it = s_icVtr.begin(); it != s_icVtr.end(); it++)
  {
    if (*it == this)
    {
      s_icVtr.erase(it);
      break;
    }
  }
}


void NotifyIcon::Update(const NID_XX& nidSource)
{
  //
  // Copy persistent values only
  //

  // state values
  copy_state(&nidSource);

  // callback message
  copy_message(&nidSource);

  // icon (note, this depends on copy_state)
  copy_icon(&nidSource);

  // tool tip string
  copy_tip(&nidSource);
}


void NotifyIcon::copy_message(PCNID_XX pnidSource)
{
  if (NIF_MESSAGE & pnidSource->uFlags)
  {
    m_uCallbackMessage = pnidSource->uCallbackMessage;
    m_uFlags |= NIF_MESSAGE;
  }
}

void NotifyIcon::copy_icon(PCNID_XX pnidSource)
{
  // ignore if we are being told to use the same icon as before.
  if (NIF_ICON & pnidSource->uFlags && m_hOriginalIcon != pnidSource->hIcon)
  {
    HICON hNewIcon = NULL;

    if (IsShared())
    {
      for (IcVtr::const_iterator it = s_icVtr.begin(); it != s_icVtr.end(); it++)
      {
        const NotifyIcon* p = *it;

        if (p->m_hOriginalIcon == pnidSource->hIcon)
        {
          m_hSharedWnd = (HANDLE)p->m_hWnd;
          m_hSharedID = (HANDLE)p->m_uID;
          hNewIcon = CopyIcon(p->m_hIcon);
          break;
        }
      }
    }
    else
    {
      hNewIcon = CopyIcon(pnidSource->hIcon);
    }

    // Update if we have a new icon, or we were told
    // to clear the current icon.
    if (hNewIcon || !pnidSource->hIcon)
    {
      if (m_hIcon)
      {
        DestroyIcon(m_hIcon);
      }

      m_hIcon = hNewIcon;
      m_hOriginalIcon = pnidSource->hIcon;
    }

    if (!m_hIcon)
    {
      m_uFlags &= ~NIF_ICON;
    }
    else
    {
      m_uFlags |= NIF_ICON;
    }
  }
}

void NotifyIcon::copy_tip(PCNID_XX pnidSource)
{
  if (NIF_TIP & pnidSource->uFlags)
  {
    switch (pnidSource->cbSize)
    {

#ifdef _UNICODE
    case NID_6W_SIZE:
    case NID_5W_SIZE:
    {
      LPCWSTR pwzSrc = ((NID_5W*)(pnidSource))->szTip;
      //WCHAR szTemp[128];
      std::wstring wszTipSrc(pwzSrc); // Create a std::wstring from the source

      // 1. Determine the number of characters to copy (safely)
      //size_t charsToCopy = std::min(wszTipSrc.length(), (size_t)TRAY_MAX_TIP_LENGTH - 1); // -1 for null terminator

      size_t charsToCopy = (std::min)(wszTipSrc.length(), (size_t)TRAY_MAX_TIP_LENGTH - 1); // -1 for null terminator
      // 2. Copy the characters
      wcsncpy_s(m_wszTip, TRAY_MAX_TIP_LENGTH, wszTipSrc.c_str(), charsToCopy);

      // 3. Ensure null termination (just in case)
      m_wszTip[charsToCopy] = L'\0';

      // ... No ANSI conversion needed if m_wszTip is WCHAR[]    
    }
    break;
    default:
    case NID_4W_SIZE:
    {
      //WCHAR szTemp[64];
      LPCWSTR pwzSrc = ((NID_4W*)(pnidSource))->szTip;

      std::wstring wszTemp(pwzSrc); // Create a std::wstring from the source

      size_t charsToCopy = (std::min)(wszTemp.length(), (size_t)TRAY_MAX_TIP_LENGTH - 1); // -1 for null terminator

      wcsncpy_s(m_wszTip, TRAY_MAX_TIP_LENGTH, wszTemp.c_str(), charsToCopy);

      m_wszTip[charsToCopy] = L'\0';

    }
#else
    case NID_6A_SIZE:
    case NID_5A_SIZE:
    {
      LPCSTR pwzSrc = ((NID_5A*)(pnidSource))->szTip;

      HRESULT hr = StringCchCopy(m_szTip, 128, pwzSrc);

      if (FAILED(hr))
      {
        m_szTip[0] = 0;
      }
    }
    break;

    default:
    case NID_4A_SIZE:
    {
      LPCSTR pwzSrc = ((NID_4A*)(pnidSource))->szTip;

      HRESULT hr = StringCchCopy(m_wszTip, 64, pwzSrc);

      if (FAILED(hr))
      {
        m_wszTip[0] = 0;
      }
      break;
    }
#endif

    }
    if (0 == m_wszTip[0])
    {
      m_uFlags &= ~NIF_TIP;
    }
    else
    {
      m_uFlags |= NIF_TIP;
    }
  }
}

void NotifyIcon::copy_state(PCNID_XX pnidSource)
{
  switch (pnidSource->cbSize)
  {
  case NID_6W_SIZE:
  case NID_5W_SIZE:
  {
    NID_5W* pnid = (NID_5W*)pnidSource;

    if (NIF_STATE & pnid->uFlags)
    {
      update_state(pnid->dwState, pnid->dwStateMask);
    }
  }
  break;

  case NID_6A_SIZE:
  case NID_5A_SIZE:
  {
    NID_5A* pnid = (NID_5A*)pnidSource;

    if (NIF_STATE & pnid->uFlags)
    {
      update_state(pnid->dwState, pnid->dwStateMask);
    }
  }
  break;

  default:
    break;
  }
}

void NotifyIcon::update_state(DWORD dwState, DWORD dwMask)
{
  DWORD dwTemp = (m_dwState & ~dwMask) \
    | (dwState & dwMask);

  m_dwState = dwTemp;

  if (0 != m_dwState)
  {
    m_uFlags |= NIF_STATE;
  }
  else
  {
    m_uFlags &= ~NIF_STATE;
  }
}

void NotifyIcon::CopyLSNID(LSNOTIFYICONDATA* plsnid, UINT uFlagMask) const
{
  plsnid->cbSize = sizeof(LSNOTIFYICONDATA);
  plsnid->hWnd = m_hWnd;
  plsnid->uID = m_uID;
  plsnid->uFlags = 0;

  if (NIF_MESSAGE & m_uFlags & uFlagMask)
  {
    plsnid->uCallbackMessage = m_uCallbackMessage;
    plsnid->uFlags |= NIF_MESSAGE;
  }

  if (NIF_ICON & m_uFlags & uFlagMask)
  {
    // Use our copy of the icon
    plsnid->hIcon = m_hIcon;
    plsnid->uFlags |= NIF_ICON;
  }

  if (NIF_TIP & m_uFlags & uFlagMask)
  {
    // Make a copy of the string
    HRESULT hr = StringCchCopy(plsnid->szTip, TRAY_MAX_TIP_LENGTH, m_wszTip);

    if (SUCCEEDED(hr))
    {
      plsnid->uFlags |= NIF_TIP;
    }
  }
}
