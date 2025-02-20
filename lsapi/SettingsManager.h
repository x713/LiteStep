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
#ifndef __SETTINGSMANAGER_H
#define __SETTINGSMANAGER_H

#include "../utility/common.h"
#include "../utility/criticalsection.h"
#include <string>
#include <map>
#include <set>
#include "settingsdefines.h"
#include "settingsiterator.h"

struct FileInfo
{
  SettingsMap* m_pSettingsMap;
  UINT m_Count;
};

typedef std::set<SettingsIterator*> IteratorSet;
typedef std::map<std::wstring, FileInfo*, wstringicmp> FileMap;


class SettingsManager
{
  IteratorSet m_Iterators;
  SettingsMap m_SettingsMap;
  FileMap m_FileMap;

    CriticalSection m_CritSection;

protected:
  BOOL _FindLine(LPCWSTR pszName, SettingsMap::iterator &it);

public:
  SettingsManager();
  ~SettingsManager();

  void ParseFile(LPCWSTR pwszFileName);

  BOOL GetRCString(LPCWSTR pwszKeyName, LPWSTR pwszValue, LPCWSTR pwszDefStr, int nMaxLen);
  BOOL GetRCLine(LPCWSTR pwszKeyName, LPWSTR pwszValue, int nMaxLen, LPCWSTR pwszDefStr);
  BOOL GetRCBool(LPCWSTR pwszKeyName, BOOL bIfFound);
  BOOL GetRCBoolDef(LPCWSTR pwszKeyName, BOOL bDefault);
  int GetRCInt(LPCWSTR pwszKeyName, int nDefault);
  COLORREF GetRCColor(LPCWSTR pwszKeyName, COLORREF crDefault);

  BOOL GetVariable(LPCWSTR pwszKeyName, LPWSTR pwszValue, DWORD dwLength);
  void SetVariable(LPCWSTR pwszKeyName, LPCWSTR pwszValue);

  FILE* LCOpen (LPCWSTR pwszPath);
  BOOL LCClose (FILE *pFile);
  BOOL LCReadNextConfig(FILE *pFile, LPCWSTR pwszConfig, LPWSTR pwszValue, size_t cchValue);
  BOOL LCReadNextLineOrCommand(FILE *pFile, LPWSTR pwszValue, size_t cchValue);

  void VarExpansionEx(LPWSTR pwszExpandedString, LPCWSTR pwszTemplate, size_t stLength);
  void VarExpansionEx(LPWSTR pwszExpandedString, LPCWSTR pwszTemplate, size_t stLength, const StringSet& recursiveVarSet);
};

#endif // __SETTINGSMANAGER_H
