#pragma once
/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2025 The x7 Dev Team
Copyright (C) 1997-2005 The LiteStep Development Team

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
#ifndef __BANGMANAGER_H
#define __BANGMANAGER_H

#include "../utility/common.h"
#include "../lsapi/BangCommand.h"
#include "../utility/criticalsection.h"
#include "../utility/stringutility.h"
#include <map>
#include <string>


class BangManager
{
private:
  typedef std::map<std::wstring, Bang*, wstringicmp> BangMap;

  BangMap bang_map;
  mutable CriticalSection m_cs;

public:
  BangManager();
  virtual ~BangManager();

  BOOL AddBangCommand(LPCWSTR pwszName, Bang* pbbBang);
  BOOL RemoveBangCommand(LPCWSTR pwszName);
  void ClearBangCommands();
  BOOL ExecuteBangCommand(LPCWSTR pwszName, HWND hCaller, LPCWSTR pwszParams);
  HRESULT EnumBangs(LSENUMBANGSPROC pfnCallback, LPARAM lParam) const;
};

#endif // __BANGMANAGER_H
