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
#include "DDEWorker.h"
#include "../utility/shellhlp.h"
#include "../utility/core.hpp"
#include <windows.h>
#include <shlwapi.h>
#include <string>   // For std::wstring


DDEWorker::DDEWorker()
{
  wmemset(m_wszCurrentGroup, 0, MAX_PATH);

  SC_HANDLE hSC = OpenSCManager(NULL, NULL, GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
  if (hSC)
  {
    m_bIsUserAnAdmin = TRUE;
  }
  else
  {
    m_bIsUserAnAdmin = FALSE;
  }

  SHFindFiles = (BOOL(__stdcall*)(LPCITEMIDLIST, LPCITEMIDLIST))GetProcAddress(GetModuleHandle(L"SHELL32.DLL"), (LPCSTR)((long)0x005A));
}


DDEWorker::~DDEWorker()
{
}


// Handle incoming DDE requests and pass to relevant functions
BOOL DDEWorker::ParseRequest(LPCWSTR pwszRequest)
{
  // 10 is the Maximum number of parameters passed in PROGMAN DDE call (AddItem)
  LPWSTR pwszParamList[10];      // This holds a list of pointers to args
  LPWSTR pwszTmp;
  LPWSTR pwszWorkRequest;
  BOOL bReturn = FALSE;

  DWORD dwRequest = _MatchRequest(pwszRequest);
  if (dwRequest)
  {
    size_t stLength = wcslen(pwszRequest) + 1;
    pwszWorkRequest = new wchar_t[stLength];

    StringCchCopy(pwszWorkRequest, stLength, pwszRequest);
    pwszWorkRequest[stLength - 3] = '\0';
    pwszWorkRequest = wcschr(pwszWorkRequest, '(');
    pwszWorkRequest++;

    pwszParamList[0] = pwszWorkRequest;

    int nCurIndex = 1;
    pwszTmp = wcschr(pwszWorkRequest, ','); // find a delimiter
    while (NULL != pwszTmp)
    {
      *(pwszTmp++) = '\0'; // null terminate and cut.
      pwszParamList[nCurIndex++] = pwszTmp; // put it in the pointer list
      pwszTmp = wcschr(pwszTmp, ','); // get the next onf
    }


    // Set initial state of the group to act on:
    // bCommon ? Perform function on a common group : Perform function on a private group
    BOOL bCommon = m_bIsUserAnAdmin;

    switch (dwRequest)
    {
      // DDE call: e.g. ExploreFolder(idlist[,object])
    case DDE_REQUEST_EXPLOREFOLDER:
    {
      wchar_t wszParam[MAX_PATH];
      StringCchPrintf(wszParam, MAX_PATH, L"/e,/idlist,%s", pwszParamList[1]);
      bReturn = ((ShellExecute(NULL, L"open", L"explorer.exe", wszParam, NULL, SW_SHOW) > HINSTANCE(32)) ? TRUE : FALSE);
    }
    break;

    // DDE call: e.g. ViewFolder(idlist[,object])
    case DDE_REQUEST_VIEWFOLDER:
    {
      wchar_t wszParam[MAX_PATH];
      StringCchPrintf(wszParam, MAX_PATH, L"/idlist,%s", pwszParamList[1]);
      bReturn = ((ShellExecute(NULL, L"open", L"explorer.exe", wszParam, NULL, SW_SHOW) > HINSTANCE(32)) ? TRUE : FALSE);
      // Will return too soon if we don't sleep here... funky
      Sleep(100);
    }
    break;

    // DDE call: e.g. FindFolder(idlist[,object])
    case DDE_REQUEST_FINDFOLDER:
    {
      bReturn = _FindFiles(pwszParamList[0], TRUE);
    }
    break;

    // DDE call: e.g. OpenFindFile(idlist[,object])
    case DDE_REQUEST_OPENFINDFILE:
    {
      bReturn = _FindFiles(pwszParamList[0], FALSE);
    }
    break;

    // DDE call: e.g. CreateGroup(GroupName[,CommonGroupFlag])
    case DDE_REQUEST_CREATEGROUP:
    {
      if (2 == nCurIndex) // second parameter forces common/private
      {
        bCommon = _wtoi(pwszParamList[1]);
      }
      bReturn = _CreateGroup(pwszParamList[0], bCommon);
    }
    break;

    // DDE call: e.g. DeleteGroup(GroupName[,CommonGroupFlag])
    case DDE_REQUEST_DELETEGROUP:
    {
      if (2 == nCurIndex) // second parameter forces common/private
      {
        bCommon = _wtoi(pwszParamList[1]);
      }
      bReturn = _DeleteGroup(pwszParamList[0], bCommon);
    }
    break;

    // DDE call: e.g. ShowGroup(GroupName,ShowCommand[,CommonGroupFlag])
    case DDE_REQUEST_SHOWGROUP:
    {
      if (nCurIndex >= 2) // show command: maps to SW_
      {
        if (3 == nCurIndex) // third parameter forces common/private
        {
          bCommon = _wtoi(pwszParamList[2]);
        }
        int nShow = _wtoi(pwszParamList[1]);
        bReturn = _ShowGroup(pwszParamList[0], nShow, bCommon);
      }
    }
    break;

    // DDE call: e.g. DeleteItem(ItemName) : applies to current group
    case DDE_REQUEST_DELETEITEM:
    {
      bReturn = _DeleteItem(pwszParamList[0]);
    }
    break;

    //DDE call: AddItem(CmdLine[,Name[,IconPath[,IconIndex[,xPos,yPos[,DefDir[,
    //HotKey[,fMinimize[fSeparateMemSpace] ] ] ] ] ] ]) : applies to current group
    case DDE_REQUEST_ADDITEM:
    {
      BOOL bMinimize = FALSE;
      WORD dwHotKey = 0;
      LPCWSTR pwszDefDir = NULL;
      int nIconIndex = 0;
      LPCWSTR pwszIconPath = NULL;
      LPCWSTR pwszDescription = NULL;

      switch (nCurIndex)
      {
      case 10:  // SeparateMemSpace ignored for now
      case 9:
      { // Minimize
        bMinimize = (BOOL)_wtoi(pwszParamList[8]);
      }
      case 8:
      { // HotKey
        dwHotKey = (WORD)_wtoi(pwszParamList[7]);
      }
      case 7:
      { // DefDir
        pwszDefDir = pwszParamList[6];
      }
      case 6:  //xPos and yPos ignored. Not necessary
      case 5:
      case 4:
      { // IconIndex
        nIconIndex = _wtoi(pwszParamList[3]);
      }
      case 3:
      { // IconPath
        pwszIconPath = pwszParamList[2];
      }
      case 2:
      { // Description
        pwszDescription = pwszParamList[1];
      }

      default:
        break;
      }
      bReturn = _AddItem(pwszParamList[0], pwszDescription, pwszIconPath, nIconIndex, pwszDefDir, dwHotKey, bMinimize);
    }
    break;

    default:
      break;
    }
  }
  return bReturn;
}


// List the program groups
BOOL DDEWorker::ListGroups(LPVOID& pGroupList, UINT& ulSize)
{
  wchar_t wszPath[MAX_PATH];
  pGroupList = NULL;
  ulSize = 0;
  LPWSTR pwszTemp = NULL;
  BOOL bReturn = FALSE;

  // Get user specific folders
  if (GetShellFolderPath(CSIDL_PROGRAMS, wszPath, MAX_PATH))
  {
    PathAppend(wszPath, L"*.*");

    HANDLE hHeap = GetProcessHeap();
    if (hHeap)
    {
      if (_ListGroupsHelper(hHeap, wszPath, pGroupList, ulSize))
      {
        // Get common folders
        if (GetShellFolderPath(CSIDL_COMMON_PROGRAMS, wszPath, MAX_PATH))
        {
          PathAppend(wszPath, L"*.*");

          if (_ListGroupsHelper(hHeap, wszPath, pGroupList, ulSize))
          {
            // add null terminator
            pwszTemp = (wchar_t*)HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pGroupList, ulSize + 1);
            if (pwszTemp != NULL)
            {
              pwszTemp[ulSize] = '\0';
              pGroupList = (LPVOID)pwszTemp;
              ulSize += 1;
              bReturn = TRUE;
            }
          }
        }
      }
    }
  }

  return bReturn;
}


DWORD DDEWorker::_MatchRequest(LPCWSTR pwszCommand)
{
  DWORD dwReturn = DDE_REQUEST_NONE;

  if (wcsstr(pwszCommand, L"[ExploreFolder(") == pwszCommand)
  {
    dwReturn = DDE_REQUEST_EXPLOREFOLDER;
  }
  else if (wcsstr(pwszCommand, L"[ViewFolder(") == pwszCommand)
  {
    dwReturn = DDE_REQUEST_VIEWFOLDER;
  }
  else if (wcsstr(pwszCommand, L"[FindFolder(") == pwszCommand)
  {
    dwReturn = DDE_REQUEST_FINDFOLDER;
  }
  else if (wcsstr(pwszCommand, L"[OpenFindFile(") == pwszCommand)
  {
    dwReturn = DDE_REQUEST_OPENFINDFILE;
  }
  else if (wcsstr(pwszCommand, L"[CreateGroup(") == pwszCommand)
  {
    dwReturn = DDE_REQUEST_CREATEGROUP;
  }
  else if (wcsstr(pwszCommand, L"[DeleteGroup(") == pwszCommand)
  {
    dwReturn = DDE_REQUEST_DELETEGROUP;
  }
  else if (wcsstr(pwszCommand, L"[ShowGroup(") == pwszCommand)
  {
    dwReturn = DDE_REQUEST_SHOWGROUP;
  }
  else if (wcsstr(pwszCommand, L"[AddItem(") == pwszCommand)
  {
    dwReturn = DDE_REQUEST_ADDITEM;
  }
  else if (wcsstr(pwszCommand, L"[DeleteItem(") == pwszCommand)
  {
    dwReturn = DDE_REQUEST_DELETEITEM;
  }

  return dwReturn;
}

/*
BOOL DDEWorker::_FindFiles(LPSTR pszPath, BOOL bFindFolder)
{
  LPMALLOC pMalloc = NULL;
  BOOL bReturn = FALSE;
  WCHAR wzPath[MAX_PATH] = { 0 };

  PathUnquoteSpaces(pszPath);
  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszPath, -1, wzPath, MAX_PATH);

  HRESULT hr = SHGetMalloc(&pMalloc);
  if (SUCCEEDED(hr))
  {
    IShellFolder* psfParent = NULL;

    hr = SHGetDesktopFolder(&psfParent);
    if (SUCCEEDED(hr))
    {
      LPITEMIDLIST pidl = NULL;

      hr = psfParent->ParseDisplayName(NULL, NULL, wzPath, NULL, &pidl, NULL);
      if (SUCCEEDED(hr))
      {
        if (bFindFolder) // FindFolder
        {
          bReturn = (BOOL)SHFindFiles(pidl, NULL);
        }
        else // OpenFindFile
        {
          bReturn = (BOOL)SHFindFiles(NULL, pidl);
        }

        pMalloc->Free(pidl);
      }

      psfParent->Release();
    }

    pMalloc->Release();
  }

  return bReturn;
}
*/

BOOL DDEWorker::_FindFiles(LPWSTR pwszPath, BOOL bFindFolder) {
  LPMALLOC pMalloc = NULL;

  BOOL bReturn = FALSE;
  WCHAR wzPath[MAX_PATH] = { 0 };

  PathUnquoteSpaces(pwszPath);

  HRESULT hr = SHGetMalloc(&pMalloc);
  if (SUCCEEDED(hr))
  {
    IShellFolder* psfParent = NULL;

    hr = SHGetDesktopFolder(&psfParent);
    if (SUCCEEDED(hr))
    {
      LPITEMIDLIST pidl = NULL;

      hr = psfParent->ParseDisplayName(NULL, NULL, wzPath, NULL, &pidl, NULL);
      if (SUCCEEDED(hr))
      {
        if (bFindFolder) // FindFolder
        {
          bReturn = (BOOL)SHFindFiles(pidl, NULL);
        }
        else // OpenFindFile
        {
          bReturn = (BOOL)SHFindFiles(NULL, pidl);
        }

        pMalloc->Free(pidl);
      }

      psfParent->Release();
    }

    pMalloc->Release();
  }

  return bReturn;
}



BOOL DDEWorker::_ShowGroup(LPCWSTR wstrGroupName, int nShow, BOOL bCommon)
{
  wchar_t wszFullPath[MAX_PATH];
  wchar_t wszPath[MAX_PATH];
  BOOL bReturn = FALSE;

  // Get the program group path
  if (GetShellFolderPath(bCommon ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS, wszPath, MAX_PATH))
  {
    StringCchPrintf(wszFullPath, MAX_PATH, L"%s\\%s\\", wszPath, wstrGroupName);
    PathQuoteSpaces(wszFullPath);

    if (PathIsDirectory(wszFullPath))
    {
      // open it up!
      ShellExecute(NULL, L"open", wszFullPath, NULL, NULL, nShow);

      // set our current group to this one, as per Progman DDE
      StringCchCopy(m_wszCurrentGroup, MAX_PATH, wszFullPath);

      bReturn = TRUE;
    }
  }

  return bReturn;
}

// Create the program group
BOOL DDEWorker::_CreateGroup(LPCWSTR wstrGroupName, BOOL bCommon)
{
  wchar_t wszPath[MAX_PATH];
  wchar_t wszFullPath[MAX_PATH];
  BOOL bReturn = FALSE;

  // Get the program group path
  if (GetShellFolderPath(bCommon ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS, wszPath, MAX_PATH))
  {
    StringCchPrintf(wszFullPath, MAX_PATH, L"%s\\%s", wszPath, wstrGroupName);
    PathQuoteSpaces(wszFullPath);

    // standard create directory call
    if (CreateDirectory(wszFullPath, NULL))
    {
      StringCchCopy(m_wszCurrentGroup, MAX_PATH, wszFullPath);

      // Hmmm should we show the group here???? Maybe
      // but not for now.
      //ShellExecute(NULL, "open", szPath, NULL, NULL, nShow);

      // Tell the shell that something changed
      SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, wszFullPath, 0);

      bReturn = TRUE;
    }
    else
    {
      if (ERROR_ALREADY_EXISTS == GetLastError())
      {
        bReturn = TRUE;
      }
    }
  }

  return bReturn;
}



// Remove the group
BOOL DDEWorker::_DeleteGroup(LPCWSTR wstrGroupName, BOOL bCommon)
{
  wchar_t wszTemp[MAX_PATH];
  wchar_t wszPath[MAX_PATH];
  HANDLE hFind;
  WIN32_FIND_DATA FindData;
  BOOL bFindFile = TRUE;
  BOOL bReturn = FALSE;

  // Get the program group path
  if (GetShellFolderPath(bCommon ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS, wszPath, MAX_PATH))
  {
    // Append \*.* for FindFirstFile
    StringCchCopy(wszTemp, MAX_PATH, L"\\");
    StringCchCat(wszTemp, MAX_PATH, wstrGroupName);
    StringCchCat(wszPath, MAX_PATH, wszTemp);

    StringCchCopy(wszTemp, MAX_PATH, wszPath);
    StringCchCat(wszTemp, MAX_PATH, L"\\*.*");

    // Use FindFirstFile to list dir. contents
    hFind = FindFirstFile(wszTemp, &FindData);

    // kill them all off
    while ((INVALID_HANDLE_VALUE != hFind) && bFindFile)
    {
      if (*(FindData.cFileName) != '.')
      {
        PathAppend(wszTemp, FindData.cFileName);
        DeleteFile(wszTemp);
      }
      bFindFile = FindNextFile(hFind, &FindData);
    }

    FindClose(hFind);

    bReturn = RemoveDirectory(wszPath);
  }

  return bReturn;
}




// helper function to do allocation for the list of program groups
BOOL DDEWorker::_ListGroupsHelper(HANDLE hHeap, wchar_t* wszPath, LPVOID& pGroupList, UINT& ulSize)
{
  HANDLE hFind;
  WIN32_FIND_DATA FindData;
  BOOL bFindFile = TRUE;
  wchar_t* pwszTemp = NULL;
  BOOL bReturn = FALSE;

  // allocate the group list with base zero terminator if NULL
  if (NULL == pGroupList)
  {
    pGroupList = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 1);
  }

  // check allocation
  if (pGroupList)
  {
    // Get the first one
    hFind = FindFirstFile(wszPath, &FindData);

    // iterate through the files
    // copy their names into a string in which each field is terminated
    // by \r\n
    while ((INVALID_HANDLE_VALUE != hFind) && bFindFile)
    {
      if (*(FindData.cFileName) != '.' && (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
      {
        int stLen = wcslen(FindData.cFileName);

        pwszTemp = (wchar_t*)HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pGroupList, ulSize + stLen + 2);
        if (pwszTemp != NULL)
        {
          CopyMemory(&pwszTemp[ulSize], FindData.cFileName, stLen);
          CopyMemory(&pwszTemp[ulSize + stLen], L"\r\n", 2);
          pGroupList = (LPVOID)pwszTemp;
          ulSize += (stLen + 2);
          bReturn = TRUE;
        }
        else
        {
          // Cleanup
          FindClose(hFind);
          bReturn = FALSE;
          break;
        }
      }
      // continue to next
      bFindFile = FindNextFile(hFind, &FindData);
    }

    // Cleanup
    FindClose(hFind);
  }
  return bReturn;
}


// kill an item
BOOL DDEWorker::_DeleteItem(LPCWSTR wstrItem)
{
  wchar_t wszPath[MAX_PATH];
  BOOL bReturn = FALSE;

  if (m_wszCurrentGroup[0])
  {
    StringCchPrintf(wszPath, MAX_PATH, L"%s\\%s", m_wszCurrentGroup, wstrItem);
    bReturn = DeleteFile(wszPath);
  }

  return bReturn;
}

// Add an item
BOOL DDEWorker::_AddItem(LPCWSTR wstrCmdLine, LPCTSTR wstrDescription, LPCWSTR wstrIconPath, int nIconIndex, LPCWSTR wstrDefDir, WORD dwHotKey, BOOL bMinimize)
{
  wchar_t wszPath[MAX_PATH];
  HRESULT hr;
  IShellLink* pShellLink;
  IPersistFile* pPersistFile;
  BOOL bReturn = FALSE;

  // check that we've set a current group via showgroup or creategroup
  if (m_wszCurrentGroup[0])
  {
    wchar_t wszDesc[MAX_PATH];
    wchar_t wszArgs[MAX_PATH];
    wchar_t wszCmd[MAX_PATH];

    PathQuoteSpaces((wchar_t*)wstrCmdLine);

    LPWSTR strArgs = PathGetArgs(wstrCmdLine);
    if (strArgs)
    {
      StringCchCopy(wszArgs, MAX_PATH, strArgs);
      PathRemoveArgs((wchar_t*)wstrCmdLine);
    }

    PathUnquoteSpaces((wchar_t*)wstrCmdLine);
    StringCchCopy(wszCmd, MAX_PATH, wstrCmdLine);

    if (wstrDescription)
    {
      StringCchCopy(wszDesc, MAX_PATH, PathFindFileName(wstrCmdLine));
      PathRemoveExtension(wszDesc);
    }
    else
    {
      PathUnquoteSpaces((wchar_t*)wstrDescription);
      StringCchCopy(wszDesc, MAX_PATH, wstrDescription);
    }

    StringCchPrintf(wszPath, MAX_PATH, L"%s\\%s.lnk", m_wszCurrentGroup, wszDesc);

    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
      IID_IShellLink, (LPVOID*)&pShellLink);
    if (SUCCEEDED(hr))
    {
      // Set the shortcut info.
      pShellLink->SetDescription(wszDesc);
      pShellLink->SetHotkey(dwHotKey);
      pShellLink->SetWorkingDirectory(wstrDefDir);
      pShellLink->SetShowCmd(bMinimize ? SW_SHOWMINIMIZED : SW_SHOWNORMAL);
      pShellLink->SetArguments(wszArgs);
      pShellLink->SetPath(wszCmd);

      if (wstrIconPath)
      {
        pShellLink->SetIconLocation(wstrIconPath, nIconIndex);
      }

      // Save it.
      hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
      if (SUCCEEDED(hr))
      {
        WCHAR wsz[MAX_PATH];

        // Ensure that the string is ANSI.
        //MultiByteToWideChar(CP_ACP, 0, wszPath, -1, wsz, MAX_PATH);

        // Save the link by calling IPersistFile::Save.
        hr = pPersistFile->Save(wsz, TRUE);
        if (SUCCEEDED(hr))
        {
          SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, m_wszCurrentGroup, 0);
          SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, wszPath, 0);
          bReturn = TRUE;
        }
        pPersistFile->Release();
      }

      pShellLink->Release();
    }
  }

  return bReturn;
}

