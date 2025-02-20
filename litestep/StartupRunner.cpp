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
#include "StartupRunner.h"
#include "../utility/shellhlp.h"
#include <regstr.h>
#include "../utility/core.hpp"

#include <VersionHelpers.h>

#ifdef __GNUC__ // mingw
enum RESTRICTIONS
{
    REST_NOLOCALMACHINERUN     = 0x40000046,
    REST_NOCURRENTUSERRUN      = 0x40000047,
    REST_NOLOCALMACHINERUNONCE = 0x40000048,
    REST_NOCURRENTUSERRUNONCE  = 0x40000049
};
#endif

#define ERK_NONE				0x0000
#define ERK_RUNSUBKEYS			0x0001 // runs key and its subkeys
#define ERK_DELETE				0x0002
#define ERK_WAITFOR_QUIT        0x0004 // wait until process exits
#define ERK_WAITFOR_IDLE        0x0008 // wait until process waits for input
#define REGSTR_PATH_RUN_POLICY	REGSTR_PATH_POLICIES L"\\Explorer\\Run"

StartupRunner::StartupRunner()
{}

StartupRunner::~StartupRunner()
{}

DWORD StartupRunner::Run(void* pvVoid)
{
    bool bRunStartup = _IsFirstRunThisSession();
    BOOL bForceStartup = (BOOL)pvVoid;
    
    // by keeping the call to _IsFirstRunThisSession() above we make sure the
    // regkey is created even if we're in "force startup" mode
    if (bRunStartup || bForceStartup)
    {
        bool bHKLMRun = true;
        bool bHKCURun = true;
        bool bHKLMRunOnce = true;
        bool bHKCURunOnce = true;
        
        //
        // SHRestricted is not available on Windows 95
        //
        typedef DWORD (WINAPI* SHREST_PROC)(RESTRICTIONS);

        SHREST_PROC pSHRestricted = (SHREST_PROC)GetProcAddress(
            GetModuleHandle(L"shell32.dll"), (LPCSTR)((long)0x0064));

        if (pSHRestricted)
        {
            bHKLMRun = !pSHRestricted(REST_NOLOCALMACHINERUN);
            bHKCURun = !pSHRestricted(REST_NOCURRENTUSERRUN);
            bHKLMRunOnce = !pSHRestricted(REST_NOLOCALMACHINERUNONCE);
            bHKCURunOnce = !pSHRestricted(REST_NOCURRENTUSERRUNONCE);
        }
        
        if (bHKLMRunOnce)
        {
            _RunRegKeys(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUNONCE,
                (ERK_RUNSUBKEYS | ERK_DELETE | ERK_WAITFOR_QUIT));
        }

        _RunRunOnceEx();
        
        if (bHKLMRun)
        {
            _RunRegKeys(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUN, ERK_NONE);
        }

        _RunRegKeys(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUN_POLICY, ERK_NONE);
        _RunRegKeys(HKEY_CURRENT_USER, REGSTR_PATH_RUN_POLICY, ERK_NONE);
        
        if (bHKCURun)
        {
            _RunRegKeys(HKEY_CURRENT_USER, REGSTR_PATH_RUN, ERK_NONE);
        }
        
        _RunStartupMenu();
        
        if (bHKCURunOnce)
        {
            _RunRegKeys(HKEY_CURRENT_USER, REGSTR_PATH_RUNONCE,
                (ERK_RUNSUBKEYS | ERK_DELETE));
        }
    }
    
    return bRunStartup;
}

void StartupRunner::_RunRunOnceEx()
{
    TCHAR szArgs[MAX_PATH] = { 0 };
    UINT uChars = GetSystemDirectory(szArgs, MAX_PATH);
    
    if (uChars > 0 && uChars < MAX_PATH)
    {
        if (SUCCEEDED(StringCchCat(szArgs, MAX_PATH, _T("\\iernonce.dll"))))
        {
            // The file doesn't exist on NT4
            if (PathFileExists(szArgs) && SUCCEEDED(StringCchCat(szArgs,
                MAX_PATH, _T(",RunOnceExProcess"))))
            {
                ShellExecute(NULL, _T("open"), _T("rundll32.exe"), szArgs, NULL,
                    SW_NORMAL);
            }
        }
    }
}

void StartupRunner::_RunStartupMenu()
{
  const UINT STARTUPMENU_TABLE[] =
    {
        CSIDL_COMMON_STARTUP,
        CSIDL_COMMON_ALTSTARTUP,
        CSIDL_STARTUP,
        CSIDL_ALTSTARTUP
    };
  const ULONG STARTUPMENU_SIZE = sizeof(STARTUPMENU_TABLE) / sizeof(UINT);

  TCHAR tzPath[MAX_PATH];

  for (ULONG i = 0; i < STARTUPMENU_SIZE; ++i)
  {
    if (GetShellFolderPath(STARTUPMENU_TABLE[i], tzPath, MAX_PATH))
    {
      _RunFolderContents(tzPath);
    }
  }
}

void StartupRunner::_RunFolderContents(LPCTSTR ptzPath)
{
  TCHAR tzSearchPath[MAX_PATH];
  WIN32_FIND_DATA findData;
  HANDLE hSearch;

  if (IsValidStringPtr(ptzPath) && (ptzPath[0] != _T('\0')))
  {
    StringCchCopy(tzSearchPath, MAX_PATH, ptzPath);
    PathAppend(tzSearchPath, _T("*.*"));

    hSearch = FindFirstFile(tzSearchPath, &findData);
    while (hSearch != INVALID_HANDLE_VALUE)
    {
      if (!PathIsDirectory(findData.cFileName) &&
                !(findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
                !(findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
      {
                SHELLEXECUTEINFO seiCommand = { 0 };
        seiCommand.cbSize = sizeof(SHELLEXECUTEINFO);

        seiCommand.lpFile = findData.cFileName;
        seiCommand.lpDirectory = ptzPath;
        seiCommand.nShow = SW_SHOWNORMAL;
        seiCommand.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI;

                ShellExecuteEx(&seiCommand);
      }

      if (!FindNextFile(hSearch, &findData))
      {
        FindClose(hSearch);
        hSearch = INVALID_HANDLE_VALUE;
      }
    }
  }
}

bool IsWindowsNT() {
  return IsWindowsVersionOrGreater(4, 0, 0); // Check for NT 4.0 or greater
}

  // On NT systems, the SessionInfo subkey will be the AuthenticationID

bool StartupRunner::_IsFirstRunThisSession()
{
    bool bReturn = false;
    HKEY hkExplorer;
    TCHAR tzSessionInfo[30];

    // Check if the OS is Windows NT
    if (IsWindowsNT())
    {
        HANDLE hToken;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) != 0)
        {
            TOKEN_STATISTICS tsStats;
            DWORD dwOutSize;

            if (GetTokenInformation(hToken, TokenStatistics,
                &tsStats, sizeof(tsStats), &dwOutSize))
            {
                StringCchPrintf(tzSessionInfo, 30, _T("SessionInfo\\%08x%08x"),
                    tsStats.AuthenticationId.HighPart,
                    tsStats.AuthenticationId.LowPart);

                // Create the SessionInfo and StartUpHasBeenRun keys
                LONG lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
                    REGSTR_PATH_EXPLORER, 0, NULL, REG_OPTION_NON_VOLATILE,
                    MAXIMUM_ALLOWED, NULL, &hkExplorer, NULL);

                if (lResult == ERROR_SUCCESS)
                {
                    HKEY hkSessionInfo;

                    lResult = RegCreateKeyEx(hkExplorer, tzSessionInfo, 0, NULL,
                        REG_OPTION_VOLATILE, KEY_WRITE, NULL, &hkSessionInfo,
                        NULL);

                    if (lResult == ERROR_SUCCESS)
                    {
                        DWORD dwDisposition;
                        HKEY hkStartup;
                        lResult = RegCreateKeyEx(hkSessionInfo,
                            _T("StartupHasBeenRun"), 0, NULL,
                            REG_OPTION_VOLATILE, KEY_WRITE, NULL,
                            &hkStartup, &dwDisposition);

                        RegCloseKey(hkStartup);

                        if (dwDisposition == REG_CREATED_NEW_KEY)
                        {
                            bReturn = true;
                        }
                    }

                    RegCloseKey(hkSessionInfo);
                }

                RegCloseKey(hkExplorer);
            }
        }

        CloseHandle(hToken);
    }
    else
    {
        bReturn = true;
    }

    return bReturn;
}


//
// RunRegKeys(HKEY hkParent, LPCSTR pszSubKey, DWORD dwFlags)
//
void StartupRunner::_RunRegKeys(HKEY hkParent, LPCTSTR ptzSubKey, DWORD dwFlags)
{
    HKEY hkSubKey;

  LONG lResult = RegOpenKeyEx(hkParent, ptzSubKey, 0, MAXIMUM_ALLOWED,
        &hkSubKey);

  if (lResult == ERROR_SUCCESS)
  {
        //
        // Run the key itself
        //
        for (DWORD dwLoop = 0; ; ++dwLoop)
        {
            TCHAR tzNameBuffer[MAX_PATH] = { 0 };
            TCHAR tzValueBuffer[MAX_LINE_LENGTH] = { 0 };
            
            DWORD dwNameSize = MAX_PATH;
            DWORD dwValueSize = MAX_LINE_LENGTH;
            DWORD dwType;

            lResult = RegEnumValue(hkSubKey, dwLoop, tzNameBuffer, &dwNameSize,
                NULL, &dwType, (LPBYTE)tzValueBuffer, &dwValueSize);
            
            if (lResult == ERROR_MORE_DATA)
            {
                // tzNameBuffer too small?
                continue;
            }
            else if (lResult == ERROR_SUCCESS)
            {
                if ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ))
                {
                    if (tzValueBuffer[0])
                    {
                        _SpawnProcess(tzValueBuffer, dwFlags);
                    }

                    if ((dwFlags & ERK_DELETE) && (tzNameBuffer[0] != _T('!')))
                    {
                        if (RegDeleteValue(hkSubKey, tzNameBuffer) ==
                            ERROR_SUCCESS)
                        {
                            --dwLoop;
                        }
                    }
                }
            }
            else
            {
                break;
            }
        }

        //
        // Run subkeys as well?
        //
        if (dwFlags & ERK_RUNSUBKEYS)
    {
            dwFlags &= ~(ERK_RUNSUBKEYS);

      for (DWORD dwLoop = 0; ; ++dwLoop)
      {
                TCHAR tzNameBuffer[MAX_PATH] = { 0 };
                lResult = RegEnumKey(hkSubKey, dwLoop, tzNameBuffer, MAX_PATH);
        
                if (lResult == ERROR_MORE_DATA)
        {
          // tzNameBuffer too small?
                    continue;
        }
        else if (lResult == ERROR_SUCCESS)
        {
          _RunRegKeys(hkSubKey, tzNameBuffer, dwFlags);

          if (dwFlags & ERK_DELETE)
          {
            if (RegDeleteKey(hkSubKey, tzNameBuffer) ==
                            ERROR_SUCCESS)
            {
                            --dwLoop;
                        }
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
    
    RegCloseKey(hkSubKey);
}


void StartupRunner::_SpawnProcess(LPTSTR ptzCommandLine, DWORD dwFlags)
{
    ASSERT(!(dwFlags & ERK_WAITFOR_QUIT && dwFlags & ERK_WAITFOR_IDLE));
    
    //
    // The following cases need to be supported:
    //
    // 1. "C:\Program Files\App\App.exe" -params
    // 2. C:\Program Files\App\App.exe -params
    // 3. App.exe -params   (app.exe is in %path% or HKLM->REGSTR_PATH_APPPATHS)
    // and all the above cases without arguments.
    //
    // CreateProcess handles 1 and 2, ShellExecuteEx handles 1 and 3.
    // So if the first token doesn't contain path characters (':' or '\')
    // ShellExecuteEx is used. That's really ugly but it *should* work.
    //

    TCHAR tzToken[MAX_LINE_LENGTH] = { 0 };
    LPCTSTR ptzArgs = NULL;

    GetToken(ptzCommandLine, tzToken, &ptzArgs, FALSE);

    HANDLE hProcess;

    if (wcschr(tzToken, _T('\\')) || wcschr(tzToken, _T(':')))
    {
        hProcess = _CreateProcess(ptzCommandLine);
    }
    else
    {
        hProcess = _ShellExecuteEx(tzToken, ptzArgs);
    }

    if (hProcess != NULL)
    {
        if (dwFlags & ERK_WAITFOR_QUIT)
        {
            WaitForSingleObject(hProcess, INFINITE);
        }
        else if (dwFlags & ERK_WAITFOR_IDLE)
        {
            WaitForInputIdle(hProcess, INFINITE);
        }

        CloseHandle(hProcess);
    }
}


HANDLE StartupRunner::_CreateProcess(LPTSTR ptzCommandLine)
{
    HANDLE hReturn = NULL;

    STARTUPINFO suInfo = { 0 };
    PROCESS_INFORMATION procInfo = { 0 };
    
    suInfo.cb = sizeof(suInfo);
    
    if (CreateProcess(NULL, ptzCommandLine, NULL, NULL, FALSE,
        CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS,
        NULL, NULL, &suInfo, &procInfo))
    {
        CloseHandle(procInfo.hThread);

        hReturn = procInfo.hProcess;
    }
    
    return hReturn;
}


HANDLE StartupRunner::_ShellExecuteEx(LPCTSTR ptzExecutable, LPCTSTR ptzArgs)
{
    HANDLE hReturn = NULL;

    SHELLEXECUTEINFO sei = { 0 };
    sei.cbSize = sizeof(sei);

    sei.lpFile = ptzExecutable;
    sei.lpParameters = ptzArgs;
    sei.nShow = SW_SHOWNORMAL;
    sei.fMask = \
        SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
    
    if (ShellExecuteEx(&sei))
    {
        hReturn = sei.hProcess;
    }

    return hReturn;
}
