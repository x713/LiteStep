/*
This is a part of the LiteStep Shell Source code.

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
/****************************************************************************
****************************************************************************/ 
/*
  Provides routines useful for localizing code
*/
#include "localization.h"
#include <stdio.h>
#include "safeptr.h"
#include "safestr.h"


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// GetString
//
// Obtains strings from litestep.exe's resources.
//
static void GetString(UINT uId, wchar_t* pwzBuffer, size_t cchBuffer,
                      const wchar_t* pwzDefault)
{
  ASSERT_ISVALIDBUF(pwzBuffer, cchBuffer);
  ASSERT_ISNOTNULL(pwzDefault);

    if (LoadString(GetModuleHandle(NULL), uId, pwzBuffer, static_cast<int>(cchBuffer)) == 0)
    {
        HRESULT hr = StringCchCopy(pwzBuffer, cchBuffer, pwzDefault);
        
        if (FAILED(hr) && hr != STRSAFE_E_INSUFFICIENT_BUFFER)
        {
            *pwzBuffer = '\0';
        }
    }
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// CheckedFormat
//
static void CheckedFormat(wchar_t* pwzBuffer, size_t cchBuffer,
                          const wchar_t* pwzFormat, const va_list& vargs)
{
    ASSERT_ISVALIDBUF(pwzBuffer, cchBuffer);
    ASSERT_ISNOTNULL(pwzFormat);
    
    HRESULT hr = E_FAIL;
    
#if !defined(LS_NO_EXCEPTION)
    try
    {
#endif /* LS_NO_EXCEPTION */
        hr = StringCchVPrintf(pwzBuffer, cchBuffer, pwzFormat, vargs);
#if !defined(LS_NO_EXCEPTION)
    }
    catch (...)
    {
        // Don't want exceptions during error reporting
        ASSERT(false);
    }
#endif /* LS_NO_EXCEPTION */
    
    if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER))
    {
        StringCchCopy(pwzBuffer, cchBuffer, pwzFormat);
    }
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// DoError
//
static int DoError(const wchar_t* pwzText, const wchar_t* pwzCaption = NULL)
{
    ASSERT_ISNOTNULL(pwzText);

    wchar_t wzCaption[MAX_LINE_LENGTH] = { L"LiteStep :: Error" };
    
    if (pwzCaption != NULL)
    {
        StringCchPrintfEx(wzCaption, MAX_LINE_LENGTH,
            NULL, NULL, STRSAFE_NULL_ON_FAILURE,
            L"LiteStep :: %s :: Error", pwzCaption);
    }

    return MessageBox(NULL, pwzText, wzCaption,
        MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
}


void Error(UINT uMessageId, LPCWSTR pwzDefault, ...)
{
    ASSERT_ISNOTNULL(pwzDefault);

    // Allocate memory on the heap instead of the stack
    wchar_t* wzMessage = new wchar_t[MAX_LINE_LENGTH];
    wchar_t* wzFormat = new wchar_t[MAX_LINE_LENGTH];

    if (wzMessage && wzFormat)
    {
        GetString(uMessageId, wzFormat, MAX_LINE_LENGTH, pwzDefault);

        va_list vargs;
        va_start(vargs, pwzDefault);

        CheckedFormat(wzMessage, MAX_LINE_LENGTH, wzFormat, vargs);

        va_end(vargs);

        DoError(wzMessage);
    }

    // Free the allocated memory
    delete[] wzMessage;
    delete[] wzFormat;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ErrorEx
//
void ErrorEx(LPCWSTR pwzCaption, UINT uMessageId, LPCWSTR pwzDefault) {
  ErrorEx(pwzCaption, uMessageId, pwzDefault, nullptr);
}

void ErrorEx(LPCWSTR pwzCaption, UINT uMessageId, LPCWSTR pwzDefault, ...)
{
    ASSERT_ISNOTNULL(pwzDefault);
    ASSERT_ISNOTNULL(pwzCaption);

    // Allocate memory on the heap instead of the stack
    wchar_t* wzFormat = new wchar_t[MAX_LINE_LENGTH];
    wchar_t* wzMessage = new wchar_t[MAX_LINE_LENGTH];

    if (wzFormat && wzMessage)
    {
        GetString(uMessageId, wzFormat, MAX_LINE_LENGTH, pwzDefault);

        va_list vargs;
        va_start(vargs, pwzDefault);

        CheckedFormat(wzMessage, MAX_LINE_LENGTH, wzFormat, vargs);

        va_end(vargs);

        DoError(wzMessage, pwzCaption);
    }

    // Free the allocated memory
    delete[] wzFormat;
    delete[] wzMessage;
}
