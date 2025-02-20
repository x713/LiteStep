/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2002-2006 The LiteStep Development Team

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
#include "../utility/common.h"
#include "../utility/safestr.h"

BOOL WINAPI LSLog(int nLevel, LPCWSTR pwszModule, LPCWSTR pwszMessage)
{
#ifdef LS_COMPAT_LOGGING

    wchar_t wszLogFile[MAX_PATH] = { 0 };
  
    int nLogLevel = GetRCInt(L"LSLogLevel", 2);
    
    // Should this message be logged?
    if (!pwszModule || !pwszMessage ||
        (nLevel > nLogLevel) || (nLevel < 1) || (nLevel > 4))
    {
        return FALSE;
    }

    // Has a log file been assigned?
    if (!GetRCString(L"LSLogFile", wszLogFile, NULL, MAX_PATH))
    {
        return FALSE;
    }

  // If so, open it
  HANDLE hLogFile = CreateFile(wszLogFile,
    GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
        NULL);
    
    // Did open succeed?
    if(hLogFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    
    // Move to the end of the file
    SetFilePointer(hLogFile, 0, NULL, FILE_END);
    
    // Get timestamp
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);
    
    // Add timestamp and module name to message
    LPCWSTR rwszLevel[4] = { L"Error", L"Warning", L"Notice", L"Debug" };
    
    TCHAR wszLine[MAX_LINE_LENGTH] = { 0 };
    size_t cbLine = sizeof(wszLine);
    size_t cbRemaining = 0;
    
    if (SUCCEEDED(StringCbPrintfEx(wszLine, cbLine, NULL, &cbRemaining,
        STRSAFE_IGNORE_NULLS, L"%02d-%02d-%04d %02d:%02d:%02d - %s - %s: %s\r\n",
        st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond,
        rwszLevel[nLevel-1], pwszModule, pwszMessage)))
    {
        DWORD cbToWrite = cbLine - cbRemaining;

        // Write it to the log file
        DWORD dwCount = 0;
        WriteFile(hLogFile, wszLine, cbToWrite, &dwCount, NULL);
    }
    
    // Close the log
    CloseHandle(hLogFile);
#endif // LS_COMPAT_LOGGING
    
    return TRUE;
}



BOOL WINAPIV LSLogPrintf(int nLevel, LPCWSTR pwszModule, LPCWSTR pwszFormat, ...)
{
#ifdef LS_COMPAT_LOGGING

    if (!pwszModule || !pwszFormat)
    {
        return FALSE;
    }

    BOOL bReturn = FALSE;
    wchar_t szMessage[MAX_LINE_LENGTH];

    va_list argList;
    va_start(argList, pwszFormat);
    
    if (SUCCEEDED(StringCchVPrintf(szMessage, MAX_LINE_LENGTH,
        pwszFormat, argList)))
    {
        bReturn = LSLog(nLevel, pwszModule, szMessage);
    }

    va_end(argList);
    
    return bReturn;
#else

    return TRUE;

#endif // LS_COMPAT_LOGGING
}




int GetRCCoordinate(LPCWSTR pwszKeyName, int nDefault, int nMaxVal)
{
  wchar_t strVal[MAX_LINE_LENGTH];

  if (!GetRCString(pwszKeyName, strVal, NULL, MAX_LINE_LENGTH))
    {
        return nDefault;
    }

  return ParseCoordinate(strVal, nDefault, nMaxVal);
}




int ParseCoordinate(LPCWSTR wszString, int nDefault, int nMaxVal)
{
  BOOL negative = false;
  BOOL center = false;
  BOOL percentual = false;

  int value = 0;

  if (!wszString || !wszString[0])
    {
        return nDefault;
    }

  if (wszString[0] == '-')
  {
    negative = true;
    wszString++;
  }
  else if (wszString[0] == '+')
  {
    wszString++;
  }

  size_t length = wcslen(wszString);
  size_t i = 0;

  while (i < length)
  {
    if (wszString[i] >= '0' && wszString[i] <= '9')
      value = (value * 10) + (wszString[i] - '0');
    else
      break;

    ++i;
  }

  if (i == 0)
    return nDefault;

  while (i < length)
  {
    if (wszString[i] == 'c' || wszString[i] == 'C' && !center)
      center = true;

    else if (wszString[i] == '%' && !percentual)
      percentual = true;

    else
      //		{
      //			LSLogPrintf(LOG_WARNING, "SettingsManager", "incorrect coordinate (%s), using \"%s%d%s%s\"", strVal, negative?"-":"",value, percentual?"%":"", center?"c":"");
      break;
    //		}

    ++i;
  }

  if (percentual)
    value = nMaxVal * value / 100;

  if (center)
  {
    if (negative)
      value = (nMaxVal / 2) - value;
    else
      value = (nMaxVal / 2) + value;
  }
  /*	else if (percentual) // percentual positioning ie 30% from left side, -30% to count from right
    {
      if (negative)
        value = nMaxVal*(1-value/100);
      else
        value = nMaxVal*value/100;
    }*/
  else if (negative)
  {
    value = nMaxVal - value;
  }

  return value;
}
