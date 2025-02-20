/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 1997-2006, 2025 The LiteStep Development Team

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
#include "SettingsManager.h"
#include "SettingsFileParser.h"
#include "MathEvaluate.h"
#include "../utility/shellhlp.h"
#include "../utility/core.hpp"

#include <string>

using std::wstring;
SettingsManager::SettingsManager()
{
}


SettingsManager::~SettingsManager()
{
  Lock lock(m_CritSection);

  // check if nasty modules forgot to call LCClose
  for (IteratorSet::iterator itSet = m_Iterators.begin();
    itSet != m_Iterators.end(); ++itSet)
  {
    delete* itSet;
  }

  for (FileMap::iterator itFiles = m_FileMap.begin();
    itFiles != m_FileMap.end(); ++itFiles)
  {
    if (itFiles->second->m_Count == 1)
    {
      delete itFiles->second->m_pSettingsMap;
      delete itFiles->second;
    }
    else
    {
      itFiles->second->m_Count--;
    }
  }
}


void SettingsManager::ParseFile(LPCWSTR pwszFileName)
{
  TRACE(L"Loading config file \"%s\"", pwszFileName);

  FileParser fpParser(&m_SettingsMap);
  fpParser.ParseFile(pwszFileName);
}


BOOL SettingsManager::_FindLine(LPCWSTR pwszName, SettingsMap::iterator& it)
{
  ASSERT_ISNOTNULL(pwszName);
  BOOL bReturn = FALSE;

  // first appearance of a setting takes effect
  it = m_SettingsMap.find(pwszName);

  if (it != m_SettingsMap.end() && _wcsicmp(pwszName, it->first.c_str()) == 0)
  {
    bReturn = TRUE;
  }

  return bReturn;
}


BOOL SettingsManager::GetRCString(LPCWSTR pwszKeyName, LPWSTR pwszValue, LPCWSTR pwszDefStr, int nMaxLen)
{
  SettingsMap::iterator it;
  BOOL bReturn = FALSE;

  if (pwszValue)
  {
    pwszValue[0] = '\0';
  }

  if (pwszKeyName)
  {
    if (_FindLine(pwszKeyName, it))
    {
      bReturn = TRUE;

      if (pwszValue)
      {
        wchar_t szToken[MAX_LINE_LENGTH] = { 0 };
        GetToken(it->second.c_str(), szToken, NULL, FALSE);

        StringSet recursiveVarSet;
        recursiveVarSet.insert(pwszKeyName);
        VarExpansionEx(pwszValue, szToken, nMaxLen, recursiveVarSet);
      }
    }
    else if (pwszDefStr && pwszValue)
    {
      StringCchCopy(pwszValue, nMaxLen, pwszDefStr);

      // for compatibility reasons GetRCString and GetRCLine return FALSE
      // if the default value is returned.
      //bReturn = TRUE;
    }
  }

  return bReturn;
}


BOOL SettingsManager::GetRCLine(LPCWSTR pwszKeyName, LPWSTR pwszValue, int nMaxLen, LPCWSTR pwszDefStr)
{
  SettingsMap::iterator it;
  BOOL bReturn = FALSE;

  if (pwszValue)
  {
    pwszValue[0] = '\0';
  }

  if (pwszKeyName)
  {
    if (_FindLine(pwszKeyName, it))
    {
      bReturn = TRUE;

      if (pwszValue)
      {
        // for compatibility reasons GetRCLine expands $evars$
        StringSet recursiveVarSet;
        recursiveVarSet.insert(pwszKeyName);
        VarExpansionEx(pwszValue, it->second.c_str(), nMaxLen, recursiveVarSet);
      }
    }
    else if (pwszDefStr && pwszValue)
    {
      StringCchCopy(pwszValue, nMaxLen, pwszDefStr);

      // for compatibility reasons GetRCString and GetRCLine return FALSE
      // if the default value is returned.
      //bReturn = TRUE;
    }
  }

  return bReturn;
}


BOOL SettingsManager::GetRCBool(LPCWSTR pwszKeyName, BOOL bIfFound)
{
  SettingsMap::iterator it;

  if (pwszKeyName && _FindLine(pwszKeyName, it))
  {
    wchar_t wszExpanded[MAX_LINE_LENGTH] = { 0 };
    wchar_t wszToken[MAX_LINE_LENGTH] = { 0 };

    StringSet recursiveVarSet;
    recursiveVarSet.insert(pwszKeyName);
    VarExpansionEx(wszExpanded, it->second.c_str(), MAX_LINE_LENGTH, recursiveVarSet);

    if (GetToken(wszExpanded, wszToken, NULL, FALSE))
    {
      if (_wcsicmp(wszToken, L"off") &&
        _wcsicmp(wszToken, L"false") &&
        _wcsicmp(wszToken, L"no"))
      {
        return bIfFound;
      }
    }
    else
    {
      return bIfFound;
    }
  }

  return !bIfFound;
}


BOOL SettingsManager::GetRCBoolDef(LPCWSTR pwszKeyName, BOOL bDefault)
{
  SettingsMap::iterator it;

  if (pwszKeyName && _FindLine(pwszKeyName, it))
  {
    wchar_t wszToken[MAX_LINE_LENGTH] = { 0 };
    wchar_t wszExpanded[MAX_LINE_LENGTH] = { 0 };

    StringSet recursiveVarSet;
    recursiveVarSet.insert(pwszKeyName);
    VarExpansionEx(wszExpanded, it->second.c_str(), MAX_LINE_LENGTH, recursiveVarSet);

    if (GetToken(wszExpanded, wszToken, NULL, FALSE))
    {
      if ((_wcsicmp(wszToken, L"off") == 0) ||
        (_wcsicmp(wszToken, L"false") == 0) ||
        (_wcsicmp(wszToken, L"no") == 0))
      {
        return FALSE;
      }
    }

    return TRUE;
  }

  return bDefault;
}


int SettingsManager::GetRCInt(LPCWSTR pwszKeyName, int nDefault)
{
  SettingsMap::iterator it;
  int nValue = nDefault;

  if (pwszKeyName && _FindLine(pwszKeyName, it))
  {
    wchar_t wszToken[MAX_LINE_LENGTH] = { 0 };
    wchar_t wszExpanded[MAX_LINE_LENGTH] = { 0 };

    StringSet recursiveVarSet;
    recursiveVarSet.insert(pwszKeyName);
    VarExpansionEx(wszExpanded, it->second.c_str(), MAX_LINE_LENGTH, recursiveVarSet);

    if (GetToken(wszExpanded, wszToken, NULL, FALSE))
    {
      nValue = wcstol(wszToken, NULL, 0);
    }
  }

  return nValue;
}


COLORREF SettingsManager::GetRCColor(LPCWSTR pwszKeyName, COLORREF crDefault)
{
  COLORREF crReturn = crDefault;
  SettingsMap::iterator it;

  if (pwszKeyName && _FindLine(pwszKeyName, it))
  {
    wchar_t wszBuffer[MAX_LINE_LENGTH];
    wchar_t wszFirst[MAX_LINE_LENGTH];
    wchar_t wszSecond[MAX_LINE_LENGTH];
    wchar_t wszThird[MAX_LINE_LENGTH];

    LPWSTR lpwszTokens[3] = { wszFirst, wszSecond, wszThird };

    StringSet recursiveVarSet;
    recursiveVarSet.insert(pwszKeyName);
    VarExpansionEx(wszBuffer, it->second.c_str(), MAX_LINE_LENGTH, recursiveVarSet);

    int nCount = LCTokenize(wszBuffer, lpwszTokens, 3, NULL);

    if (nCount >= 3)
    {
      int nRed, nGreen, nBlue;

      nRed = wcstol(wszFirst, NULL, 10);
      nGreen = wcstol(wszSecond, NULL, 10);
      nBlue = wcstol(wszThird, NULL, 10);

      crReturn = RGB(nRed, nGreen, nBlue);
    }
    else if (nCount >= 1)
    {
      crReturn = wcstol(wszFirst, NULL, 16);
      // convert from BGR to RGB
      crReturn = RGB(GetBValue(crReturn), GetGValue(crReturn),
        GetRValue(crReturn));
    }
  }

  return crReturn;
}




BOOL SettingsManager::GetVariable(LPCWSTR pwszKeyName, LPWSTR pwszValue, DWORD dwLength)
{
  // using GetRCString instead of GetRCLine here, again for compatibility reasons.
    // as a side effect this strips any "" quotes around the variable's value.
  return GetRCString(pwszKeyName, pwszValue, NULL, dwLength);
}


void SettingsManager::SetVariable(LPCWSTR pwszKeyName, LPCWSTR pwszValue)
{
  if ((pwszKeyName) && (pwszValue))
  {
    // in order for LSSetVariable to work evars must be redefinable
    SettingsMap::iterator it;
    if (_FindLine(pwszKeyName, it))
    {
      it->second.assign(pwszValue);
    }
    else
    {
      m_SettingsMap.insert(SettingsMap::value_type(pwszKeyName, pwszValue));
    }
  }
}




void SettingsManager::VarExpansionEx(LPWSTR pwszExpandedString, LPCWSTR pwszTemplate, size_t stLength)
{
  StringSet recursiveVarSet;
  VarExpansionEx(pwszExpandedString, pwszTemplate, stLength, recursiveVarSet);
}


void SettingsManager::VarExpansionEx(LPWSTR pwszExpandedString, LPCWSTR pwszTemplate, size_t stLength, const StringSet& recursiveVarSet)
{
  wchar_t wszTempExpandedString[MAX_LINE_LENGTH] = { 0 };
  LPWSTR pwszTempExpandedString = wszTempExpandedString;
  size_t stWorkLength = MAX_LINE_LENGTH; // available working length in szTempExpandedString

  if ((pwszTemplate != NULL) && (pwszExpandedString != NULL) && (stLength > 0))
  {
    //szTempExpandedString[0] = '\0';

    while ((*pwszTemplate != '\0') && (stWorkLength > 0))
    {
      if (*pwszTemplate != '$')
      {
        *pwszTempExpandedString = *pwszTemplate;
        ++pwszTemplate;
        ++pwszTempExpandedString;
        --stWorkLength;
      }
      else
      {
        //
        // This is a variable so we need to find the end of it:
        //
        ++pwszTemplate;

        LPCWSTR pwszVariable = pwszTemplate;

        while ((*pwszTemplate != '$') && (*pwszTemplate != '\0'))
        {
          ++pwszTemplate;
        }

        bool bSucceeded = false;

        if (*pwszTemplate == '\0')
        {
          bSucceeded = SUCCEEDED(
            StringCchCopyN(pwszTempExpandedString,
              stWorkLength,
              pwszVariable, pwszTemplate - pwszVariable));
        }
        else
        {
          //
          // We've found the end of the variable so copy it
          // someplace usefull:
          //
          wchar_t wszVariable[MAX_LINE_LENGTH];

          if (SUCCEEDED(StringCchCopyN(wszVariable, MAX_LINE_LENGTH,
            pwszVariable, pwszTemplate - pwszVariable)) && (wszVariable[0] != '\0'))
          {
            // Check for recursive variable definitions
            if (recursiveVarSet.count(wszVariable) > 0)
            {
              // Output an error message (localize this)
              //Error(LOCALIZE_THIS, L"Error: Variable \"%s\" is defined recursively.", wszVariable);

              pwszExpandedString[0] = '\0';
              return;
            }

            //
            // Get the value, if we can.
            //
            SettingsMap::iterator it;
            if (_FindLine(wszVariable, it))
            {
              // Add this variable to the set to check for recursion
              StringSet newRecursiveVarSet(recursiveVarSet);
              newRecursiveVarSet.insert(wszVariable);

              // FIXME: Should we call GetToken here?!
              TCHAR szTemp[MAX_LINE_LENGTH];
              GetToken(it->second.c_str(), szTemp, NULL, FALSE);
              VarExpansionEx(pwszTempExpandedString, szTemp, stWorkLength, newRecursiveVarSet);
              bSucceeded = true;
            }
            else if (GetEnvironmentVariable(wszVariable,
              pwszTempExpandedString,
              static_cast<DWORD>(stWorkLength)))
            {
              bSucceeded = true;
            }
            else
            {
              pwszTempExpandedString[0] = '\0';
#ifdef LS_COMPAT_MATH
              std::wstring result;

              if (MathEvaluateString(m_SettingsMap, wszVariable, result, recursiveVarSet, MATH_EXCEPTION_ON_UNDEFINED | MATH_VALUE_TO_COMPATIBLE_STRING))
              {
                StringCchCopy(pwszTempExpandedString, stWorkLength, result.c_str());
                bSucceeded = true;
              }
#endif // LS_COMPAT_MATH
            }
          }
        }
        //
        // If we succeeded, adjust our output buffers
        // accordingly:
        //
        if (bSucceeded)
        {
          size_t stTempLen = wcslen(pwszTempExpandedString);
          stWorkLength -= stTempLen;
          pwszTempExpandedString += stTempLen;
        }

        //
        // Move to the next character if we didn't run out of space:
        //
        if (*pwszTemplate != '\0')
        {
          ++pwszTemplate;
        }
      }
    }

    *pwszTempExpandedString = '\0';
    StringCchCopy(pwszExpandedString, stLength, wszTempExpandedString);
  }
}


//
// FILE* LCOpen(LPCWSTR pwszPath)
//
FILE* SettingsManager::LCOpen(LPCWSTR pwszPath)
{
  FILE* pFile = NULL;

  if (pwszPath == NULL)
  {
    SettingsIterator* psiNew = new SettingsIterator(&m_SettingsMap, L"\0");

    if (psiNew)
    {
      m_Iterators.insert(psiNew);
      pFile = (FILE*)psiNew;
    }
  }
  else
  {
    wchar_t wszPath[MAX_PATH] = { 0 };
    VarExpansionEx(wszPath, pwszPath, MAX_PATH);

    Lock lock(m_CritSection);

    FileMap::iterator it = m_FileMap.find(wszPath);

    if (it == m_FileMap.end() && PathFileExists(wszPath))
    {
      FileInfo* pFileInfo = new FileInfo;

      if (pFileInfo)
      {
        pFileInfo->m_pSettingsMap = new SettingsMap;
        pFileInfo->m_Count = 1;

        TRACE(L"Loading config file \"%s\"", wszPath);

        FileParser fpParser(pFileInfo->m_pSettingsMap);
        fpParser.ParseFile(wszPath);

        m_FileMap.insert(FileMap::value_type(wszPath, pFileInfo));

        it = m_FileMap.find(wszPath);
        ASSERT(it != m_FileMap.end());
      }
    }

    if (it != m_FileMap.end())
    {
      SettingsIterator* psiNew =
        new SettingsIterator(it->second->m_pSettingsMap, wszPath);

      if (psiNew)
      {
        it->second->m_Count++;
        m_Iterators.insert(psiNew);
        pFile = (FILE*)psiNew;
      }
    }
  }

  return pFile;
}


//
// BOOL LCClose(FILE* pFile)
//
BOOL SettingsManager::LCClose(FILE* pFile)
{
  BOOL bReturn = FALSE;

  if (pFile != NULL)
  {
    Lock lock(m_CritSection);

    IteratorSet::iterator it = m_Iterators.find((SettingsIterator*)pFile);

    if (it != m_Iterators.end())
    {
      FileMap::iterator fmIt = m_FileMap.find((*it)->get_Path());

      if (fmIt != m_FileMap.end())
      {
        if (fmIt->second->m_Count == 1)
        {
          delete fmIt->second->m_pSettingsMap;
          delete fmIt->second;

          m_FileMap.erase(fmIt);
        }
        else
        {
          fmIt->second->m_Count--;
        }
      }

      delete (*it);

      m_Iterators.erase(it);
    }

    bReturn = TRUE;
  }

  return bReturn;
}


//
// BOOL LCReadNextConfig(FILE *pFile, LPCWSTR pwszConfig, LPWSTR pwszValue, size_t cchValue)
//
BOOL SettingsManager::LCReadNextConfig(FILE* pFile, LPCWSTR pwszConfig, LPWSTR pwszValue, size_t cchValue)
{
  BOOL bReturn = FALSE;
  wchar_t wszTempValue[MAX_LINE_LENGTH];

  if ((pFile != NULL) && IsValidStringPtr(pwszConfig) && IsValidStringPtr(pwszValue, cchValue))
  {
    IteratorSet::iterator it = m_Iterators.find((SettingsIterator*)pFile);
    if (it != m_Iterators.end())
    {
      bReturn = (*it)->ReadNextConfig(pwszConfig, wszTempValue, MAX_LINE_LENGTH);
      if (bReturn)
      {
        VarExpansionEx(pwszValue, wszTempValue, cchValue);
      }
    }
  }

  return bReturn;
}


//
// BOOL LCReadNextLineOrCommand(FILE *pFile, LPWSTR pwszValue, size_t cchValue)
//
BOOL SettingsManager::LCReadNextLineOrCommand(FILE* pFile, LPWSTR pwszValue, size_t cchValue)
{
  BOOL bReturn = FALSE;
  wchar_t szTempValue[MAX_LINE_LENGTH];

  if ((pFile != NULL) && IsValidStringPtr(pwszValue, cchValue))
  {
    IteratorSet::iterator it = m_Iterators.find((SettingsIterator*)pFile);
    if (it != m_Iterators.end())
    {
      bReturn = (*it)->ReadNextLine(szTempValue, MAX_LINE_LENGTH);
      if (bReturn)
      {
        VarExpansionEx(pwszValue, szTempValue, cchValue);
      }
    }
  }

  return bReturn;
}
