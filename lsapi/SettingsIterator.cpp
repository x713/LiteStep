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
#include "settingsiterator.h"
#include "settingsmanager.h"
#include <algorithm>
#include "../utility/core.hpp"

using std::wstring;


SettingsIterator::SettingsIterator(SettingsMap* pSettingsMap, wstring wszPath)
{
  if (pSettingsMap)
  {
    m_pSettingsMap = pSettingsMap;
    m_pFileIterator = m_pSettingsMap->begin();
    if (wszPath.length())
    {
      m_Path = wszPath;
    }
  }
}


BOOL SettingsIterator::ReadNextLine(LPWSTR pwszValue, size_t cchValue)
{
  BOOL bReturn = FALSE;

  if (IsValidStringPtr(pwszValue, cchValue))
  {
    if (m_pFileIterator != m_pSettingsMap->end())
    {
      StringCchCopy(pwszValue, cchValue, m_pFileIterator->first.c_str());
      StringCchCat(pwszValue, cchValue, L" ");
      StringCchCat(pwszValue, cchValue, m_pFileIterator->second.c_str());
      ++m_pFileIterator;
      bReturn = TRUE;
    }
    else
    {
      pwszValue[0] = '\0';
    }
  }

  return bReturn;
}


//
// BOOL ReadNextConfig(LPCWSTR pwszConfig, LPCSTR pwszValue, size_t cchValue)
//
//
BOOL SettingsIterator::ReadNextConfig(LPCWSTR pwszConfig, LPWSTR pwszValue, size_t cchValue)
{
  BOOL bReturn = FALSE;

  if (IsValidStringPtr(pwszValue, cchValue) && IsValidStringPtr(pwszConfig))
  {
    SettingsMap::iterator itSettings;

    pwszValue[0] = '\0';

    // Has ReadNextConfig been used before for pszConfig?
    IteratorMap::iterator it = m_Iterators.find(pwszConfig);
    if (it == m_Iterators.end())
    {
      // No, so find the first item with a key of pszConfig
      itSettings = m_pSettingsMap->lower_bound(pwszConfig);
      if (_wcsicmp(itSettings->first.c_str(), pwszConfig) == 0) // != m_pSettingsManager->end())
      {
        // Save the iterator for future use and return the value
        it = (m_Iterators.insert(IteratorMap::value_type(pwszConfig, itSettings))).first;

        bReturn = TRUE;
      }
    }
    else
    {
      // Yes so find the last item with a matching key
      itSettings = m_pSettingsMap->upper_bound(pwszConfig);

      // Loop until we either find an item with a matching key or the
      // last matching item
      do
      {
        it->second++;
      } while ((_wcsicmp(pwszConfig, it->first.c_str()) != 0) &&
        (it->second != itSettings));
      // If we found a valid item, return it
      if (it->second != itSettings)
      {
        bReturn = TRUE;
      }
    }
    if (bReturn)
    {
      StringCchCopy(pwszValue, cchValue, it->second->first.c_str());
      StringCchCat(pwszValue, cchValue, L" ");
      StringCchCat(pwszValue, cchValue, it->second->second.c_str());
    }
  }

  return bReturn;
}
