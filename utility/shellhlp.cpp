/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2025 The x7 Dev Team
Copyright (C) 1997-2003,2005 The LiteStep Development Team

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
#include "shellhlp.h"
#include "core.hpp"
#include "windows.h"

//
// GetShellFolderPath
//
// Given a CLSID, returns the given shell folder.
// cchPath must be atleast equal to MAX_PATH.
// Does NOT quote spaces, because often times the first thing that is done with
// the returned path is to append another string to it, which doesn't work with
// quotes.
//
bool GetShellFolderPath(int nFolder, LPWSTR pwzPath, size_t cchPath)
{
    ASSERT(cchPath >= MAX_PATH);
    ASSERT_ISVALIDBUF(pwzPath, cchPath);
    UNREFERENCED_PARAMETER(cchPath);

    IMalloc* pMalloc;
    bool bReturn = false;

    // SHGetSpecialFolderPath is not available on Win95
    // use SHGetSpecialFolderLocation and SHGetPathFromIDList instead
    if (SUCCEEDED(SHGetMalloc(&pMalloc)))
    {
        LPITEMIDLIST pidl;

        if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, nFolder, &pidl)))
        {
            bReturn = SHGetPathFromIDList(pidl, pwzPath) ? true : false;
            
            pMalloc->Free(pidl);
        }

        pMalloc->Release();
    }
    
    return bReturn;
}


//
// PathAddBackslashEx
//
// Checked version of PathAddBackslash which also handles quoted paths
//
// Return values:  S_OK          - backslash appended
//                 S_FALSE       - path already ended with a backslash
//                 E_OUTOFMEMORY - buffer too small
//                 E_FAIL        - other failure (invalid input string)
//
HRESULT PathAddBackslashEx(LPWSTR pwzPath, size_t cchPath)
{
    ASSERT(cchPath <= STRSAFE_MAX_CCH);
    ASSERT_ISVALIDBUF(pwzPath, cchPath);

    HRESULT hr = E_FAIL;
    size_t cchCurrentLength = 0;

    if (SUCCEEDED(StringCchLength(pwzPath, cchPath, &cchCurrentLength)))
    {
        bool bHasQuote = false;
        LPWSTR pwzEnd = pwzPath + cchCurrentLength;
        
        if ((pwzEnd > pwzPath) && (*(pwzEnd-1) == '\"'))
        {
            --pwzEnd;
            bHasQuote = true;
        }

        if (pwzEnd > pwzPath)
        {
            if (*(pwzEnd-1) != '\\')
            {
                if (cchPath - cchCurrentLength > 1)
                {
                    if (bHasQuote)
                    {
                        *(pwzEnd+1) = *pwzEnd;
                    }
                    
                    *pwzEnd = '\\';
                    
                    if (bHasQuote)
                    {
                        ++pwzEnd;
                    }
                    
                    ASSERT(static_cast<long long>(pwzEnd - pwzPath) < static_cast<long long>(cchPath));
                    *(pwzEnd+1) = '\0';
                    
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = S_FALSE;
            }
        }
    }

    return hr;
}


//
// GetSystemString
//
bool GetSystemString(DWORD dwCode, LPWSTR pwzBuffer, size_t cchBuffer)
{
    ASSERT_ISVALIDBUF(pwzBuffer, cchBuffer);

    return (0 != FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwCode,
        0,
        pwzBuffer,
        static_cast<DWORD>(cchBuffer),
        NULL
        ));
}


//
// LSGetModuleFileName
//
// Wrapper around GetModuleFileName that takes care of truncated buffers. If
// people are interested in the number of bytes written we could add another
// parameter (DWORD* pcchWritten)
//
bool LSGetModuleFileName(HINSTANCE hInst, LPWSTR pwszBuffer, DWORD cchBuffer)
{
    bool bSuccess = false;

    DWORD cchCopied = GetModuleFileName(hInst, pwszBuffer, cchBuffer);

    if (cchCopied == cchBuffer)
    {
        ASSERT(GetLastError() == ERROR_INSUFFICIENT_BUFFER);

        // GetModuleFileName doesn't null-terminate the buffer if it is too
        // small. Make sure that even in this error case the buffer is properly
        // terminated - some people don't check return values.
        pwszBuffer[cchBuffer-1] = '\0';
    }
    else if (cchCopied > 0 && cchCopied < cchBuffer)
    {
        bSuccess = true;
    }

    return bSuccess;
}


HRESULT TryAllowSetForegroundWindow(HWND hWnd)
{
    ASSERT(hWnd != NULL);
    HRESULT hr = E_FAIL;

    typedef BOOL (WINAPI* ASFWPROC)(DWORD);

    HMODULE hUser32 = GetModuleHandle(L"user32.dll");
    if (hUser32 == NULL)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    ASFWPROC pAllowSetForegroundWindow = (ASFWPROC)GetProcAddress(hUser32, "AllowSetForegroundWindow");

    if (pAllowSetForegroundWindow)
    {
        DWORD dwProcessId = 0;
        GetWindowThreadProcessId(hWnd, &dwProcessId);

        if (pAllowSetForegroundWindow(dwProcessId))
        {
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        // this platform doesn't have ASFW (Win95, NT4), so the
        // target process is allowed to set the foreground window anyway
        hr = S_FALSE;
    }

    return hr;
}
