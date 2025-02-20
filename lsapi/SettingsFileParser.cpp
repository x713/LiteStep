//
//  This is a part of the LiteStep Shell source code.
//
//  Copyright (C) 1997-2003,2005 The LiteStep Development Team.
//
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
#include "SettingsFileParser.h"
#include "../utility/core.hpp"
#include "MathEvaluate.h"




//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// FileParser constructor
//
FileParser::FileParser(SettingsMap* pSettingsMap)
  : m_pSettingsMap(pSettingsMap), m_phFile(NULL)
{
  ASSERT(NULL != m_pSettingsMap);
}





//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// FileParser destructor
//
FileParser::~FileParser()
{
}





//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ParseFile
//
void FileParser::ParseFile(LPCWSTR pwzFileName)
{
  ASSERT(NULL == m_phFile);    // in case of possible threading problem?
  ASSERT(NULL != pwzFileName);

  TCHAR tzExpandedPath[MAX_PATH_LENGTH];

  VarExpansionEx(tzExpandedPath, pwzFileName, MAX_PATH_LENGTH);
  PathUnquoteSpaces(tzExpandedPath);

  DWORD dwLen;
  dwLen = GetFullPathName(tzExpandedPath, MAX_PATH_LENGTH, m_wzFullPath, NULL);

  if (0 == dwLen || dwLen > MAX_PATH_LENGTH)
  {
    TRACE(L"Error: Can not get full path for \"%s\"", tzExpandedPath);
    return;
  }

  
  //FILE* hFile; // Note: No initialization here
  //m_phFile = _wfopen(m_wzFullPath, L"r");
  errno_t err = _wfopen_s(&m_phFile, m_wzFullPath, L"rb"); // Use _wfopen_s


  //if (NULL == m_phFile)  
  if (err != 0) {
    TRACE(L"Error: Can not open file \"%s\" (Defined as \"%s\").", m_wzFullPath, pwzFileName);
    return;
  }

  TRACE(L"Parsing \"%s\"", m_wzFullPath);

  fseek(m_phFile, 0, SEEK_SET);

  TCHAR tzKey[MAX_RCCOMMAND] = { 0 };
  TCHAR tzValue[MAX_LINE_LENGTH] = { 0 };

  m_uLineNumber = 0;

  while (_ReadLineFromFile(tzKey, tzValue))
  {
    _ProcessLine(tzKey, tzValue);
  }

  fclose(m_phFile);
  m_phFile = NULL;

  TRACE(L"Finished Parsing \"%s\"", m_wzFullPath);
}




//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// _ReadLineFromFile
//
// ptzName must be MAX_RCCOMMAND size
// ptzValue must be MAX_LINE_LENGTH size (or NULL)
//
bool FileParser::_ReadLineFromFile(LPWSTR pwzName, LPWSTR pwzValue)
{
  ASSERT(NULL != m_phFile);
  ASSERT(NULL != pwzName);

  wchar_t tzBuffer[MAX_LINE_LENGTH];
  bool bReturn = false;

  while (!feof(m_phFile) && !bReturn)
  {
    if (!fgetws(tzBuffer, MAX_LINE_LENGTH, m_phFile))
    {
      // End Of File or an Error occured. We don't care which.
      break;
    }

    m_uLineNumber++;

    LPWSTR pwzCurrent = tzBuffer;

    // Jump over any initial whitespace
    pwzCurrent += wcsspn(pwzCurrent, WHITESPACE);

    // Ignore empty lines, and comments
    if (pwzCurrent[0] && pwzCurrent[0] != ';')
    {
      // End on first reserved character or whitespace
      size_t stEndConfig = wcscspn(pwzCurrent, WHITESPACE RESERVEDCHARS);

      // If the character is not whitespace or a comment
      // then the line has an invalid format.  Ignore it.
      // TODO  : This is a bit of a hack, to check 
      if (wcschr(WHITESPACE ";", pwzCurrent[stEndConfig]) == NULL)
      {
        TRACE(L"Syntax Error (%s, %d): Invalid line format",
          m_wzFullPath, m_uLineNumber);
        continue;
      }

      // Copy directive name to ptzName.
      if (stEndConfig && SUCCEEDED(StringCchCopyN(pwzName, MAX_RCCOMMAND, pwzCurrent, stEndConfig)))
      {
        // If ptzValue is NULL, then the caller doesn't want the value,
        // however, we still will return TRUE.  If the caller does want
        // the value, then we need to ensure we put something into the
        // buffer, even if its is zero length.
        if (pwzValue != NULL)
        {
          LPWSTR pwzValueStart = pwzCurrent + stEndConfig;

          // Avoid expensive in-place copy from _StripString
          // Simply increment passed any whitespace, here.
          pwzValueStart += wcsspn(pwzValueStart, WHITESPACE);

          // Removing trailing whitespace and comments
          _StripString(pwzValueStart);

          StringCchCopy(pwzValue, MAX_LINE_LENGTH, pwzValueStart);
        }

        bReturn = true;
      }
    }
  }

  return bReturn;
}




//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// _StripString
//
void FileParser::_StripString(LPWSTR pwzString)
{
  ASSERT(NULL != pwzString);

  LPWSTR pwzCurrent = pwzString;
  LPWSTR pwzStart = NULL;
  LPWSTR pwzLast = NULL;
  size_t stQuoteLevel = 0;
  TCHAR tLastQuote = '\0';

  while (*pwzCurrent != '\0')
  {
    if (wcschr(WHITESPACE, *pwzCurrent) == NULL)
    {
      if (pwzStart == NULL)
      {
        pwzStart = pwzCurrent;
      }

      pwzLast = NULL;
    }
    else if (pwzLast == NULL)
    {
      pwzLast = pwzCurrent;
    }

    if (pwzStart != NULL)
    {
      if (*pwzCurrent == '[')
      {
        ++stQuoteLevel;
      }
      else if (*pwzCurrent == ']')
      {
        if (stQuoteLevel > 0)
        {
          --stQuoteLevel;
        }
      }
      else if ((*pwzCurrent == '"') || (*pwzCurrent == '\''))
      {
        if (tLastQuote == *pwzCurrent)
        {
          ASSERT(stQuoteLevel > 0);
          --stQuoteLevel;
          tLastQuote = 0;
        }
        else if (!tLastQuote)
        {
          ++stQuoteLevel;
          tLastQuote = *pwzCurrent;
        }
      }
      else if (*pwzCurrent == ';')
      {
        if (!stQuoteLevel)
        {
          pwzLast = pwzCurrent;
          break;
        }
      }
    }

    ++pwzCurrent;
  }

  if (pwzLast != NULL)
  {
    while (pwzLast > pwzString && wcschr(WHITESPACE, *(pwzLast - 1)))
    {
      --pwzLast;
    }

    *pwzLast = '\0';
  }

  if ((pwzCurrent != pwzString) && pwzStart)
  {
    StringCchCopy(pwzString, wcslen(pwzString) + 1, pwzStart);
  }
}




//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// _ProcessLine
//
void FileParser::_ProcessLine(LPCWSTR pwszName, LPCWSTR pwszValue)
{
  ASSERT(NULL != m_pSettingsMap);
  ASSERT(NULL != pwszName);
  ASSERT(NULL != pwszValue);

  if (_wcsicmp(pwszName, L"if") == 0)
  {
    _ProcessIf(pwszValue);
  }
#ifdef _DEBUG
  // In a release build ignore dangling elseif/else/endif.
  // Too much overhead just for error handling
  else if (
    (_wcsicmp(pwszName, L"else") == 0)
    || (_wcsicmp(pwszName, L"elseif") == 0)
    || (_wcsicmp(pwszName, L"endif") == 0)
    )
  {
    TRACE(L"Error: Dangling pre-processor directive (%s, line %d): \"%s\"",
      m_wzFullPath, m_uLineNumber, pwszName);
  }
#endif
  else if (_wcsicmp(pwszName, L"include") == 0)
  {
    TCHAR tzPath[MAX_PATH_LENGTH] = { 0 };

    if (!GetToken(pwszValue, tzPath, NULL, FALSE))
    {
      TRACE(L"Syntax Error (%s, %d): Empty \"Include\" directive",
        m_wzFullPath, m_uLineNumber);
      return;
    }

    TRACE(L"Include (%s, line %d): \"%s\"",
      m_wzFullPath, m_uLineNumber, tzPath);

    FileParser fpParser(m_pSettingsMap);
    fpParser.ParseFile(tzPath);
  }
#ifdef LS_CUSTOM_INCLUDEFOLDER
  else if (_wcsicmp(ptzName, _T("includefolder")) == 0)
  {
    TCHAR tzPath[MAX_PATH_LENGTH]; // path+pattern
    TCHAR tzFilter[MAX_PATH_LENGTH]; // path only

    // expands string in ptzValue to tzPath - buffer size defined by MAX_PATH_LENGTH
    VarExpansionEx(tzPath, ptzValue, MAX_PATH_LENGTH);

    PathUnquoteSpaces(tzPath); // strips quotation marks from string

    TRACE(L"Searching IncludeFolder (%s, line %d): \"%s\"",
      m_wzFullPath, m_uLineNumber, tzPath);

    // Hard-coded filter for *.rc files to limit search operation.
    //
    // Create tzFilter as tzPath appended with *.rc
    //  - the API takes care of trailing slash handling thankfully.
    PathCombine(tzFilter, tzPath, _T("*.rc"));

    WIN32_FIND_DATA findData; // defining variable for filename

    HANDLE hSearch = FindFirstFile(tzFilter, &findData); // Looking in tzFilter for data :)

    if (INVALID_HANDLE_VALUE != hSearch)
    {
      BOOL FoundNextFile;

      do
      {
        // stripping out directories, system and hidden files as we're not interested
        // in them and MS throws these kind of files around from time to time....
        const DWORD dwAttrib = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;

        if (0 == (dwAttrib & findData.dwFileAttributes))
        {
          // Processing the valid cFileName data now.
          TCHAR tzFile[MAX_PATH_LENGTH];

          // adding (like above) filename to tzPath to set tzFile for opening.
          if (tzFile == PathCombine(tzFile, tzPath, findData.cFileName))
          {
            TRACE("Found and including: \"%s\"", tzFile);

            FileParser fpParser(m_pSettingsMap);
            fpParser.ParseFile(tzFile);
          }
        }

        FoundNextFile = FindNextFile(hSearch, &findData);
      } while (FoundNextFile);

      FindClose(hSearch);
    }

    TRACE("Done searching IncludeFolder (%s, line %d): \"%s\"",
      m_tzFullPath, m_uLineNumber, tzPath);
  }
#endif // LS_CUSTOM_INCLUDEFOLDER
  else
  {
    m_pSettingsMap->insert(SettingsMap::value_type(pwszName, pwszValue));
  }
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// _ProcessIf
//
void FileParser::_ProcessIf(LPCWSTR pwzExpression)
{
  ASSERT(NULL != m_pSettingsMap);
  ASSERT(NULL != pwzExpression);

  bool result = false;

  if (!MathEvaluateBool(*m_pSettingsMap, pwzExpression, result))
  {
    TRACE(L"Error parsing expression \"%s\" (%s, line %d)",
      pwzExpression, m_wzFullPath, m_uLineNumber);

    // Invalid syntax, so quit processing entire conditional block
    _SkipIf();
    return;
  }

  TRACE(L"Expression (%s, line %d): \"%s\" evaluated to %s",
    m_wzFullPath, m_uLineNumber,
    pwzExpression, result ? "TRUE" : "FALSE");

  wchar_t wzName[MAX_RCCOMMAND] = { 0 };
  wchar_t wzValue[MAX_LINE_LENGTH] = { 0 };

  if (result)
  {
    // When the If expression evaluates true, process lines until we find
    // an ElseIf. Else, or EndIf
    while (_ReadLineFromFile(wzName, wzValue))
    {
      if (_wcsicmp(wzName, L"elseif") == 0 || _wcsicmp(wzName, L"else") == 0)
      {
        // After an ElseIf or Else, skip all lines until EndIf
        _SkipIf();
        break;
      }
      else if (_wcsicmp(wzName, L"endif") == 0)
      {
        // We're done
        break;
      }
      else
      {
        // Just a line, so process it
        _ProcessLine(wzName, wzValue);
      }
    }
  }
  else
  {
    // When the If expression evaluates false, skip lines until we find an
    // ElseIf, Else, or EndIf
    while (_ReadLineFromFile(wzName, wzValue))
    {
      if (_wcsicmp(wzName, L"if") == 0)
      {
        // Nested Ifs are a special case
        _SkipIf();
      }
      else if (_wcsicmp(wzName, L"elseif") == 0)
      {
        // Handle ElseIfs by recursively calling ProcessIf
        _ProcessIf(wzValue);
        break;
      }
      else if (_wcsicmp(wzName, L"else") == 0)
      {
        // Since the If expression was false, when we see Else we
        // start processing lines until EndIf
        while (_ReadLineFromFile(wzName, wzValue))
        {
          if (_wcsicmp(wzName, L"elseif") == 0)
          {
            // Error: ElseIf after Else
            TRACE(L"Syntax Error (%s, %d): \"ElseIf\" directive after \"Else\"",
              m_wzFullPath, m_uLineNumber);

            // Invalid syntax, so quit processing conditional block
            _SkipIf();
            break;
          }
          else if (_wcsicmp(wzName, L"endif") == 0)
          {
            // We're done
            break;
          }
          else
          {
            // Just a line, so process it
            _ProcessLine(wzName, wzValue);
          }
        }
        // We're done
        break;
      }
      else if (_wcsicmp(wzName, L"endif") == 0)
      {
        // We're done
        break;
      }
    }
  }
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// _SkipIf
//
void FileParser::_SkipIf()
{
  wchar_t wzName[MAX_RCCOMMAND];

  while (_ReadLineFromFile(wzName, NULL))
  {
    if (_wcsicmp(wzName, L"if") == 0)
    {
      _SkipIf();
    }
    else if (_wcsicmp(wzName, L"endif") == 0)
    {
      break;
    }
  }
}
