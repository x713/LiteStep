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
/**********************************************************************
 
  Copyright (C) 1998 (e)
 
**********************************************************************/
#ifndef __DDEWORKER_H
#define __DDEWORKER_H

#include "../utility/common.h"
#include <ddeml.h>
#include <shlobj.h>

#define DDE_REQUEST_NONE			0x0000
#define DDE_REQUEST_CREATEGROUP		0x0001
#define DDE_REQUEST_DELETEGROUP		0x0002
#define DDE_REQUEST_SHOWGROUP		0x0004
#define DDE_REQUEST_ADDITEM			0x0006
#define DDE_REQUEST_DELETEITEM		0x0008
#define DDE_REQUEST_EXPLOREFOLDER	0x0010
#define DDE_REQUEST_VIEWFOLDER		0x0020
#define DDE_REQUEST_FINDFOLDER		0x0040
#define DDE_REQUEST_OPENFINDFILE	0x0060

class DDEWorker
{
public:
  DDEWorker();
  ~DDEWorker();

  BOOL ParseRequest(LPCWSTR pwszRequest);
  BOOL ListGroups(LPVOID& pGroupList, UINT& ulSize);

private:

  BOOL _FindFiles(LPWSTR pwszPath, BOOL bFindFolder);
  BOOL _ShowGroup(LPCWSTR wstrGroupName, int nShow, BOOL bCommon);
  BOOL _CreateGroup(LPCWSTR wstrGroupName, BOOL bCommon);
  BOOL _DeleteGroup(LPCWSTR wstrGroupName, BOOL bCommon);
  BOOL _DeleteItem(LPCWSTR wstrItem);
  BOOL _AddItem(LPCWSTR wstrCmdLine, LPCWSTR pwszDescription, LPCWSTR pwszIconPath, int nIconIndex, LPCWSTR pwszDefDir, WORD dwHotKey, BOOL bMinimize);
  BOOL _ListGroupsHelper(HANDLE hHeap, wchar_t* wszPath, LPVOID& pGroupList, UINT& ulSize);
  DWORD _MatchRequest(LPCWSTR pwszCommand);

  wchar_t m_wszCurrentGroup[MAX_PATH];
  BOOL m_bIsUserAnAdmin;

  BOOL (__stdcall *SHFindFiles)(LPCITEMIDLIST, LPCITEMIDLIST);
};

#endif // __DDEWROKER_H

