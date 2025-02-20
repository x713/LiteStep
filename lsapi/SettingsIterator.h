/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2025 The x7 Dev Team
Copyright (C) 2002-2003,2005 The LiteStep Development Team

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
#ifndef __SETTINGSITERATOR_H
#define __SETTINGSITERATOR_H

#include "../utility/common.h"
#include "settingsdefines.h"


class SettingsIterator
{

public:
  SettingsIterator(SettingsMap* pSettingsMap, std::wstring wszPath);

  BOOL ReadNextLine(LPWSTR pwszValue, size_t cchValue);
  BOOL ReadNextConfig(LPCWSTR pwszConfig, LPWSTR pwszValue, size_t cchValue);

  std::wstring get_Path()
  {
    return m_Path;
  };

private:
  SettingsMap* m_pSettingsMap;
  SettingsMap::iterator m_pFileIterator;
  IteratorMap m_Iterators;
  std::wstring m_Path;
};



#endif // __SETTINGSITERATOR_H
