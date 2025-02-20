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
#include "lsapi.h"
#include "lsapiinit.h"
#include "BangCommand.h"

#include <stdio.h> // For _wsplitpath_s
#include "../utility/shellhlp.h"
#include "../utility/core.hpp"

#include "../utility/safeptr.h"
#include "../utility/safestr.h"

#include <string>

static int _Tokenize(LPCWSTR pwszString, LPWSTR* lpwszBuffers, DWORD dwNumBuffers, LPWSTR pwszExtraParameters, BOOL bUseBrackets);

extern const wchar_t rcsRevision[];
const wchar_t rcsRevision[] = L"$Revision: 1.30.1.1 $"; // Our Version
const wchar_t rcsId[] = L"$Id: lsapi.cpp,v 1.30.1.1 2025/02/18 13:18:22 Feb Exp $"; // The Full RCS ID.



BOOL LSAPIInitialize(LPCWSTR pwszLitestepPath, LPCWSTR pwszRcPath)
{
  try
  {
    g_LSAPIManager.Initialize(pwszLitestepPath, pwszRcPath);
  }
  catch (LSAPIException& lse)
  {
    lse.Type();
  }

  return g_LSAPIManager.IsInitialized() ? TRUE : FALSE;
}




void LSAPIReloadBangs(VOID)
{
  try
  {
    g_LSAPIManager.ReloadBangs();
  }
  catch (LSAPIException& lse)
  {
    lse.Type();
  }
}




void LSAPIReloadSettings(VOID)
{
  try
  {
    g_LSAPIManager.ReloadSettings();
  }
  catch (LSAPIException& lse)
  {
    lse.Type();
  }
}





void LSAPISetLitestepWindow(HWND hLitestepWnd)
{
  g_LSAPIManager.SetLitestepWindow(hLitestepWnd);
}






template<typename T>
BOOL AddBangCommandWorker(LPCWSTR pwszCommand, T pfnBangCommand)
{
  BOOL bReturn = FALSE;

  if (IsValidStringPtr(pwszCommand) && 
      IsValidCodePtr((FARPROC)pfnBangCommand))
  {
    DWORD dwCurrentThreadID = GetCurrentThreadId();

    Bang* pBang = new Bang(dwCurrentThreadID, pfnBangCommand, pwszCommand);

    if (IsValidReadPtr(pBang))
    {
      //bBang->AddRef();
      g_LSAPIManager.GetBangManager()->AddBangCommand(pwszCommand, pBang);
      pBang->Release();
      bReturn = TRUE;
    }
  }

  return bReturn;
}




//
// AddBangCommand(LPCWSTR pwszCommand, BangCommand pfnBangCommand)
//
BOOL AddBangCommand(LPCWSTR pwszCommand, BangCommand pfnBangCommand)
{
  return AddBangCommandWorker(pwszCommand, pfnBangCommand);
}





//
// AddBangCommandEx(LPCWSTR pwszCommand, BangCommand pfnBangCommand)
//
BOOL AddBangCommandEx(LPCWSTR pwszCommand, BangCommandEx pfnBangCommand)
{
  return AddBangCommandWorker(pwszCommand, pfnBangCommand);
}





//
// RemoveBangCommand(LPCWSTR pwszCommand)
//
BOOL RemoveBangCommand(LPCWSTR pwszCommand)
{
  BOOL bResult = FALSE;

  if (IsValidStringPtr(pwszCommand))
  {
    bResult = \
      g_LSAPIManager.GetBangManager()->RemoveBangCommand(pwszCommand);
  }

  return bResult;
}





//
// InternalExecuteBangCommand
// Just like ParseBangCommand but without the variable expansion
//
BOOL InternalExecuteBangCommand(HWND hCaller, LPCWSTR pwszCommand, LPCWSTR pwszArgs)
{
  return g_LSAPIManager.GetBangManager()->
    ExecuteBangCommand(pwszCommand, hCaller, pwszArgs);
}






//
// ParseBangCommand(HWND hCaller, LPCWSTR pwszCommand, LPCWSTR pwszArgs)
//
BOOL ParseBangCommand(HWND hCaller, LPCWSTR pwszCommand, LPCWSTR pwszArgs)
{
  wchar_t wszExpandedArgs[MAX_LINE_LENGTH] = { 0 };
  BOOL bReturn = FALSE;

  if (IsValidStringPtr(pwszCommand))
  {
    if (IsValidStringPtr(pwszArgs))
    {
      VarExpansionEx(wszExpandedArgs, pwszArgs, MAX_LINE_LENGTH);
    }

    bReturn = \
      InternalExecuteBangCommand(hCaller, pwszCommand, wszExpandedArgs);
  }

  return bReturn;
}






//
// CommandParse(LPCWSTR pwszCommand, LPWSTR pwszOutCommand, LPWSTR pwszOutArgs, size_t cchOutCommand, size_t cchOutArgs)
//
void CommandParse(LPCWSTR pwszCommand, LPWSTR pwszOutCommand, LPWSTR pwszOutArgs, size_t cchOutCommand, size_t cchOutArgs)
{
  wchar_t wszCommand[MAX_LINE_LENGTH];
  wchar_t wszTempCommand[MAX_LINE_LENGTH];
  LPCWSTR pwszTempArgs = NULL;

  if (IsValidStringPtr(pwszCommand))
  {
    if (IsValidStringPtr(pwszOutCommand, cchOutCommand))
    {
      VarExpansionEx(wszCommand, pwszCommand, MAX_LINE_LENGTH);

      GetToken(wszCommand, wszTempCommand, &pwszTempArgs, TRUE);

      StringCchCopy(pwszOutCommand, cchOutCommand, wszTempCommand);
    }
    if (IsValidStringPtr(pwszOutArgs, cchOutArgs))
    {
      StringCchCopy(pwszOutArgs, cchOutArgs, pwszTempArgs);
    }
  }
}





//
// LSExecuteEx(HWND hOwner, LPCWSTR pwszOperation, LPCWSTR pwszCommand, LPCWSTR pwszArgs, LPCWSTR pwszDirectory, int nShowCmd)
//
HINSTANCE LSExecuteEx(HWND hOwner, LPCWSTR pwszOperation, LPCWSTR pwszCommand, LPCWSTR pwszArgs, LPCWSTR pwszDirectory, int nShowCmd)
{
  HINSTANCE hReturn = HINSTANCE(32);

  if (IsValidStringPtr(pwszCommand))
  {
    if (pwszCommand[0] == '!')
    {
      hReturn = ParseBangCommand(hOwner, pwszCommand, pwszArgs) ? HINSTANCE(33) : HINSTANCE(32);
    }
    else
    {
      DWORD dwType = GetFileAttributes(pwszCommand);
      if ((dwType & FILE_ATTRIBUTE_DIRECTORY) && (dwType != 0xFFFFFFFF))
      {
        hReturn = ShellExecute(hOwner, pwszOperation, pwszCommand, pwszArgs, NULL, nShowCmd ? nShowCmd : SW_SHOWNORMAL);
      }
      else
      {
        SHELLEXECUTEINFO seiCommand = { 0 };
        seiCommand.cbSize = sizeof(SHELLEXECUTEINFO);
        seiCommand.hwnd = hOwner;
        seiCommand.lpVerb = pwszOperation;
        seiCommand.lpFile = pwszCommand;
        seiCommand.lpParameters = pwszArgs;
        seiCommand.lpDirectory = pwszDirectory;
        seiCommand.nShow = nShowCmd;
        seiCommand.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI;

        ShellExecuteEx(&seiCommand);

        hReturn = seiCommand.hInstApp;
      }
    }
  }

  return hReturn;
}




/*
//
// LSExecute(HWND hOwner, LPCWSTR pwszCommand, int nShowCmd)
//
HINSTANCE LSExecute(HWND hOwner, LPCWSTR pwszCommand, int nShowCmd)
{
  wchar_t wszCommand[MAX_LINE_LENGTH];
  wchar_t wszExpandedCommand[MAX_LINE_LENGTH];
  LPCWSTR pwszArgs;
  HINSTANCE hResult = HINSTANCE(32);

  if (IsValidStringPtr(pwszCommand))
  {
    VarExpansionEx(wszExpandedCommand, pwszCommand, MAX_LINE_LENGTH);

    if (GetToken(wszExpandedCommand, wszCommand, &pwszArgs, TRUE))
    {
      if (pwszArgs > (wszExpandedCommand + wcslen(wszExpandedCommand)))
      {
        pwszArgs = NULL;
      }

      if (wszCommand[0] == '!')
      {
        hResult = LSExecuteEx(hOwner, NULL, wszCommand, pwszArgs, NULL, 0);
      }
      else
      {
        wchar_t wszDir[_MAX_DIR];
        wchar_t wszFullDir[_MAX_DIR + _MAX_DRIVE];

        _wsplitpath(wszCommand, wszFullDir, wszDir, NULL, NULL);
        StringCchCat(wszFullDir, _MAX_DIR + _MAX_DRIVE, wszDir);

        hResult = LSExecuteEx(hOwner, NULL, wszCommand, pwszArgs,
          wszFullDir, nShowCmd ? nShowCmd : SW_SHOWNORMAL);
      }
    }
  }

  return hResult;
}

*/

HINSTANCE LSExecute(HWND hOwner, LPCWSTR pwszCommand, int nShowCmd) {
  wchar_t wszCommand[MAX_LINE_LENGTH];
  wchar_t wszExpandedCommand[MAX_LINE_LENGTH];
  LPCWSTR pwszArgs;
  HINSTANCE hResult = HINSTANCE(32);

  if (IsValidStringPtr(pwszCommand)) {
    VarExpansionEx(wszExpandedCommand, pwszCommand, MAX_LINE_LENGTH);

    if (GetToken(wszExpandedCommand, wszCommand, &pwszArgs, TRUE)) {
      if (pwszArgs > (wszExpandedCommand + wcslen(wszExpandedCommand))) {
        pwszArgs = NULL;
      }

      if (wszCommand[0] == '!') {
        hResult = LSExecuteEx(hOwner, NULL, wszCommand, pwszArgs, NULL, 0);
      }
      else {
        wchar_t wszDir[_MAX_DIR]{0};
        wchar_t wszFullDir[_MAX_DIR + _MAX_DRIVE]{0};
        wchar_t wszDrive[_MAX_DRIVE]{0}; // Required for _wsplitpath_s1
        wchar_t wszFname[_MAX_FNAME]{0}; // Required for _wsplitpath_s
        wchar_t wszExt[_MAX_EXT]{0};     // Required for _wsplitpath_s

        errno_t err = _wsplitpath_s(wszCommand, wszDrive, _MAX_DRIVE, wszDir, _MAX_DIR, wszFname, _MAX_FNAME, wszExt, _MAX_EXT);
        if (err != 0) {
          // Handle error (e.g., log it, return an error code, etc.)
          //fwprintf(stderr, L"Error splitting path: %d\n", err); // Example error message
          return hResult; // Or other appropriate error handling
        }

        StringCchCat(wszFullDir, _MAX_DIR + _MAX_DRIVE, wszDir);

        hResult = LSExecuteEx(hOwner, NULL, wszCommand, pwszArgs, wszFullDir, nShowCmd ? nShowCmd : SW_SHOWNORMAL);
      }
    }
  }

  return hResult;
}

//
// SetDesktopArea(int left, int top, int right, int bottom)
//
void SetDesktopArea(int left, int top, int right, int bottom)
{
  RECT r = { left, top, right, bottom };

  SystemParametersInfo(SPI_SETWORKAREA, 0, (PVOID)&r, SPIF_SENDCHANGE);
  SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&r, SPIF_SENDCHANGE);
}




//
//	GetLitestepWnd()
//
HWND GetLitestepWnd()
{
  return g_LSAPIManager.GetLitestepWnd();
}




//
// void GetResStr(HINSTANCE hInstance, UINT uIDText, LPWSTR pwszText, size_t cchText, LPCWSTR pwszDefText)
//
void GetResStr(HINSTANCE hInstance, UINT uIDText, LPWSTR pwszText, size_t cchText, LPCWSTR pwszDefText)
{
  if (IsValidStringPtr(pwszText, cchText))
  {
    if (LoadString(hInstance, uIDText, pwszText, static_cast<int>(cchText)) == 0)
    {
      StringCchCopy(pwszText, cchText, pwszDefText);
    }
  }
}




//
// GetResStrEx(HINSTANCE hInstance, UINT uIDText, LPWSTR pwszText, size_t cchText, LPCWSTR pwszDefText, ...)
//
void GetResStrEx(HINSTANCE hInstance, UINT uIDText, LPWSTR pwszText, size_t cchText, LPCWSTR pwszDefText, ...)
{
  wchar_t wszFormat[MAX_LINE_LENGTH];
  va_list vargs;

  if (IsValidStringPtr(pwszText, cchText))
  {
    GetResStr(hInstance, uIDText, wszFormat, MAX_LINE_LENGTH, pwszDefText);

    va_start(vargs, pwszDefText);
    StringCchVPrintf(pwszText, cchText, wszFormat, vargs);
    va_end(vargs);
  }
}




//
// LSGetLitestepPath(LPWSTR pwszPath, size_t cchPath)
//
BOOL WINAPI LSGetLitestepPath(LPWSTR pwszPath, size_t cchPath)
{
  BOOL bReturn = FALSE;

  if (IsValidStringPtr(pwszPath, cchPath))
  {
    // Default to user defined variable
    if (GetRCString(L"litestepdir", pwszPath, NULL, static_cast<int>(cchPath)))
    {
      bReturn = TRUE;
    }
  }

  return bReturn;
}




//
// LSGetImagePath(LPWSTR pwszPath, size_t cchPath)
//
BOOL WINAPI LSGetImagePath(LPWSTR pwszPath, size_t cchPath)
{
  BOOL bReturn = FALSE;

  if (IsValidStringPtr(pwszPath, cchPath))
  {
    if (GetRCString(L"LSImageFolder", pwszPath, NULL, static_cast<int>(cchPath)))
    {
      bReturn = SUCCEEDED(PathAddBackslashEx(pwszPath, cchPath));
    }
    else
    {
      if (LSGetLitestepPath(pwszPath, cchPath))
      {
        StringCchCat(pwszPath, cchPath, L"images\\");
        bReturn = TRUE;
      }
    }
  }

  return bReturn;
}




static int _Tokenize(LPCWSTR pwszString, LPWSTR* lpwszBuffers, DWORD dwNumBuffers, LPWSTR pwszExtraParameters, BOOL bUseBrackets)
{
  wchar_t wszBuffer[MAX_LINE_LENGTH];
  LPCWSTR pwszNextToken;
  DWORD dwTokens = 0;

  if (IsValidStringPtr(pwszString))
  {
    pwszNextToken = pwszString;

    if ((lpwszBuffers != NULL) && (dwNumBuffers > 0))
    {
      for (dwTokens = 0; pwszNextToken && (dwTokens < dwNumBuffers); dwTokens++)
      {
        GetToken(pwszNextToken, wszBuffer, &pwszNextToken, bUseBrackets);

        if (IsValidStringPtr(lpwszBuffers[dwTokens]))
        {
          StringCchCopy(lpwszBuffers[dwTokens], wcslen(wszBuffer) + 1, wszBuffer);
        }
      }

      for (DWORD dwClearTokens = dwTokens; dwClearTokens < dwNumBuffers; dwClearTokens++)
      {
        if (IsValidStringPtr(lpwszBuffers[dwClearTokens]))
        {
          lpwszBuffers[dwClearTokens][0] = '\0';
        }
      }

      if (IsValidStringPtr(pwszExtraParameters))
      {
        if (pwszNextToken)
        {
          StringCchCopy(pwszExtraParameters, wcslen(pwszNextToken) + 1, pwszNextToken);
        }
        else
        {
          pwszExtraParameters[0] = '\0';
        }
      }
    }
    else
    {
      while (GetToken(pwszNextToken, NULL, &pwszNextToken, bUseBrackets))
      {
        ++dwTokens;
      }
    }
  }

  return dwTokens;
}





int LCTokenize(LPCWSTR wszString, LPWSTR* lpwszBuffers, DWORD dwNumBuffers, LPWSTR wszExtraParameters)
{
  return _Tokenize(wszString, lpwszBuffers, dwNumBuffers, wszExtraParameters, FALSE);
}





BOOL GetToken(LPCWSTR pwszString, LPWSTR pwszToken, LPCWSTR* pwszNextToken, BOOL bUseBrackets)
{
  LPCWSTR pwszCurrent = pwszString;
  LPCWSTR pwszStartMarker = NULL;
  int iBracketLevel = 0;
  WCHAR cQuote = '\0';
  bool bIsToken = false;
  bool bAppendNextToken = false;

  if (pwszString)
  {
    if (pwszToken)
    {
      pwszToken[0] = '\0';
    }

    if (pwszNextToken)
    {
      *pwszNextToken = NULL;
    }

    pwszCurrent += wcsspn(pwszCurrent, WHITESPACE);

    for (; *pwszCurrent; pwszCurrent++)
    {
      if (iswspace(*pwszCurrent) && !cQuote)
      {
        break;
      }

      if (bUseBrackets && wcschr(L"[]", *pwszCurrent) &&
        (!wcschr(L"\'\"", cQuote) || !cQuote))
      {
        if (*pwszCurrent == '[')
        {
          if (bIsToken && !cQuote)
          {
            break;
          }

          iBracketLevel++;
          cQuote = '[';

          if (iBracketLevel == 1)
          {
            continue;
          }
        }
        else
        {
          iBracketLevel--;

          if (iBracketLevel <= 0)
          {
            break;
          }
        }
      }

      if (wcschr(L"\'\"", *pwszCurrent) && (cQuote != '['))
      {
        if (!cQuote)
        {
          if (bIsToken)
          {
            bAppendNextToken = true;
            break;
          }

          cQuote = *pwszCurrent;
          continue;
        }
        else if (*pwszCurrent == cQuote)
        {
          break;
        }
      }

      if (!bIsToken)
      {
        bIsToken = true;
        pwszStartMarker = pwszCurrent;
      }
    }

    if (pwszStartMarker && pwszToken)
    {
      size_t tokenLength = pwszCurrent - pwszStartMarker;

      if (tokenLength < MAX_TOKEN_SIZE) {
        wcsncpy_s(pwszToken, MAX_TOKEN_SIZE, pwszStartMarker, tokenLength);
        pwszToken[tokenLength] = '\0'; // Explicit null termination (important if tokenLength < MAX_TOKEN_SIZE)
      }
      else {
        // Handle the error: Token too long!
        //fwprintf(stderr, L"Error: Token too long.\n");
        pwszToken[0] = '\0'; // Ensure pwszToken is empty (or truncated)
      }
    }

    if (!bAppendNextToken && *pwszCurrent)
    {
      pwszCurrent++;
    }

    pwszCurrent += wcsspn(pwszCurrent, WHITESPACE);

    if (*pwszCurrent && pwszNextToken)
    {
      *pwszNextToken = pwszCurrent;
    }

    if (bAppendNextToken && *pwszCurrent)
    {
      LPWSTR pwszNewToken = pwszToken;

      if (pwszNewToken)
      {
        pwszNewToken += wcslen(pwszToken);
      }

      GetToken(pwszCurrent, pwszNewToken, pwszNextToken, bUseBrackets);
    }

    return pwszStartMarker != NULL;
  }

  return FALSE;
}





int CommandTokenize(LPCWSTR wszString, LPWSTR* lpwszBuffers, DWORD dwNumBuffers, LPWSTR wszExtraParameters)
{
  return _Tokenize(wszString, lpwszBuffers, dwNumBuffers, wszExtraParameters, TRUE);
}





//
// VarExpansion(LPWSTR pwszExpandedString, LPCWSTR pwszTemplate)
//
void VarExpansion(LPWSTR pwszExpandedString, LPCWSTR pwszTemplate)
{
  if (IsValidStringPtr(pwszExpandedString) && IsValidStringPtr(pwszTemplate))
  {
    wchar_t wszTempBuffer[MAX_LINE_LENGTH];

    VarExpansionEx(wszTempBuffer, pwszTemplate, MAX_LINE_LENGTH);

    StringCchCopy(pwszExpandedString, wcslen(wszTempBuffer) + 1, wszTempBuffer);
  }
}





//
// VarExpansionEx(LPWSTR pwszExpandedString, LPCWSTR pwszTemplate, size_t cchLength)
//
// Taken in part from Raptor's rlib\rutil.cpp\UtilExpandString
//
void VarExpansionEx(LPWSTR pwszExpandedString, LPCWSTR pwszTemplate, size_t cchExpandedString)
{
  if (IsValidStringPtr(pwszExpandedString, cchExpandedString) &&
    IsValidStringPtr(pwszTemplate))
  {
    if (g_LSAPIManager.IsInitialized())
    {
      g_LSAPIManager.GetSettingsManager()->VarExpansionEx(pwszExpandedString, pwszTemplate, cchExpandedString);
    }
    else
    {
      StringCchCopy(pwszExpandedString, cchExpandedString, pwszTemplate);
    }
  }
}





//
// EnumLSData
//
// Return values:
//   E_INVALIDARG - Invalid value for uInfo
//   E_POINTER    - Invalid callback
//   E_FAIL       - Unspecified error
//   E_UNEXPECTED - Callback crashed or other catastrophic failure
//   S_OK         - Enumeration successful, callback always returned TRUE
//   S_FALSE      - Enumeration successful, but cancelled by callback
//
HRESULT EnumLSData(UINT uInfo, FARPROC pfnCallback, LPARAM lParam)
{
  HRESULT hr = E_INVALIDARG;

  if (NULL != pfnCallback)
  {
    switch (uInfo)
    {
    case ELD_BANGS:
    {
      hr = g_LSAPIManager.GetBangManager()->
        EnumBangs((LSENUMBANGSPROC)pfnCallback, lParam);
    }
    break;

    case ELD_REVIDS:
    {
      hr = (HRESULT)SendMessage(GetLitestepWnd(), LM_ENUMREVIDS,
        (WPARAM)pfnCallback, lParam);
    }
    break;

    case ELD_MODULES:
    {
      hr = (HRESULT)SendMessage(GetLitestepWnd(), LM_ENUMMODULES,
        (WPARAM)pfnCallback, lParam);
    }
    break;

    default:
      break;
    }
  }
  else
  {
    hr = E_POINTER;
  }

  return hr;
}
