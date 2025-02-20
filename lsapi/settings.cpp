#pragma once
/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2025 The x7 Dev Team
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
#include "settingsmanager.h"
#include "../utility/core.hpp"
#include "lsapiInit.h"

FILE* LCOpen(LPCWSTR pwszPath)
{
	FILE * pFile = NULL;

	if (g_LSAPIManager.IsInitialized())
	{
		if (pwszPath)
		{
			if (pwszPath[0] != '\0')
			{
				pFile = g_LSAPIManager.GetSettingsManager()->LCOpen(pwszPath);
			}
		}
		if (pFile == NULL)
		{
			pFile = g_LSAPIManager.GetSettingsManager()->LCOpen(NULL);
		}
	}

	return pFile;
}


BOOL LCClose(FILE *pFile)
{
	BOOL bReturn = FALSE;

	if (g_LSAPIManager.IsInitialized())
	{
		if (NULL != pFile)
		{
			bReturn = g_LSAPIManager.GetSettingsManager()->LCClose(pFile);
		}
	}

	return bReturn;
}


BOOL LCReadNextCommand(FILE *pFile, LPWSTR pwszValue, size_t cchValue)
{
	BOOL bReturn = FALSE;

	if (g_LSAPIManager.IsInitialized())
	{
		if (NULL != pFile)
		{
			if (IsValidStringPtr(pwszValue, cchValue))
			{
				bReturn = g_LSAPIManager.GetSettingsManager()->LCReadNextLineOrCommand(pFile, pwszValue, cchValue);
			}
		}
	}

	return bReturn;
}


BOOL LCReadNextConfig(FILE *pFile, LPCWSTR pwszConfig, LPWSTR pwszValue, size_t cchValue)
{
	BOOL bReturn = FALSE;

	if (g_LSAPIManager.IsInitialized())
	{
		if ((pFile != NULL) && IsValidStringPtr(pwszConfig) &&
		    IsValidStringPtr(pwszValue, cchValue))
		{
			bReturn = g_LSAPIManager.GetSettingsManager()->LCReadNextConfig(pFile, pwszConfig, pwszValue, cchValue);
		}
	}

	return bReturn;
}


BOOL LCReadNextLine(FILE *pFile, LPWSTR pwszValue, size_t cchValue)
{
	BOOL bReturn = FALSE;

	if (g_LSAPIManager.IsInitialized())
	{
		if ((pFile != NULL) && IsValidStringPtr(pwszValue, cchValue))
		{
			bReturn = g_LSAPIManager.GetSettingsManager()->LCReadNextLineOrCommand(pFile, pwszValue, cchValue);
		}
	}

	return bReturn;
}

int GetRCInt(LPCWSTR wszKeyName, int nDefault)
{
	if (g_LSAPIManager.IsInitialized())
	{
		return g_LSAPIManager.GetSettingsManager()->GetRCInt(wszKeyName, nDefault);
	}

	return nDefault;
}


BOOL GetRCBool(LPCWSTR wszKeyName, BOOL ifFound)
{
	if (g_LSAPIManager.IsInitialized())
	{
		return g_LSAPIManager.GetSettingsManager()->GetRCBool(wszKeyName, ifFound);
	}

	return !ifFound;
}


BOOL GetRCBoolDef(LPCWSTR wszKeyName, BOOL bDefault)
{
	if (g_LSAPIManager.IsInitialized())
	{
		return g_LSAPIManager.GetSettingsManager()->GetRCBoolDef(wszKeyName, bDefault);
	}

	return bDefault;
}


BOOL GetRCString(LPCWSTR wszKeyName, LPWSTR wszValue, LPCWSTR defStr, int maxLen)
{
	if (g_LSAPIManager.IsInitialized())
	{
		return g_LSAPIManager.GetSettingsManager()->GetRCString(wszKeyName, wszValue, defStr, maxLen);
	}
	else if (wszValue && defStr)
	{
		size_t defStrLen = wcslen(defStr);
		// Calculate the length to copy
		size_t copyLen = (defStrLen < maxLen - 1) ? defStrLen : maxLen - 1; 

		errno_t error = wcsncpy_s(wszValue, maxLen, defStr, copyLen);

		if (error != 0) {
			// Handle the error appropriately (e.g., log it, return an error code)
			// Example:
			// std::wcerr << L"Error copying string: " << error << std::endl;
			return FALSE; // Or return a specific error code
		}
		wszValue[maxLen - 1] = L'\0'; // Ensure null termination
	}
	return FALSE;
}


COLORREF GetRCColor(LPCWSTR wszKeyName, COLORREF colDef)
{
	if (g_LSAPIManager.IsInitialized())
	{
		return g_LSAPIManager.GetSettingsManager()->GetRCColor(wszKeyName, colDef);
	}

	return colDef;
}



/*
BOOL GetRCLine(LPCWSTR wszKeyName, LPWSTR wszBuffer, UINT nBufLen, LPCWSTR wszDefault)
{
	if (g_LSAPIManager.IsInitialized())
	{
		return g_LSAPIManager.GetSettingsManager()->GetRCLine(wszKeyName, wszBuffer, nBufLen, wszDefault);
	}
	else if(wszBuffer && wszDefault)
	{
		wcsncpy(wszBuffer, wszDefault, nBufLen);
	}
	return FALSE;
}
*/
BOOL GetRCLine(LPCWSTR wszKeyName, LPWSTR wszBuffer, UINT nBufLen, LPCWSTR wszDefault) {
	if (g_LSAPIManager.IsInitialized()) 
	{
		return g_LSAPIManager.GetSettingsManager()->GetRCLine(wszKeyName, wszBuffer, nBufLen, wszDefault);
	}
	else if (wszBuffer && wszDefault) 
	{
		if (nBufLen > 0) { // Check if nBufLen is valid
			size_t charsToCopy = wcslen(wszDefault) < nBufLen - 1 ? wcslen(wszDefault) : nBufLen - 1; // Calculate safe copy length
			wcsncpy_s(wszBuffer, nBufLen, wszDefault, charsToCopy);
			wszBuffer[charsToCopy] = '\0'; // Ensure null termination
		}
		else {
			// Handle invalid buffer size (nBufLen <= 0)
			return FALSE; // Or other appropriate error handling
		}
	}
	return FALSE;
}


BOOL LSGetVariableEx(LPCWSTR pwszKeyName, LPWSTR pwszValue, DWORD dwLength)
{
	if (g_LSAPIManager.IsInitialized())
	{
		return g_LSAPIManager.GetSettingsManager()->GetVariable(pwszKeyName, pwszValue, dwLength);
	}

	return FALSE;
}




BOOL LSGetVariable(LPCWSTR pwszKeyName, LPWSTR pwszValue)
{
	BOOL bReturn = FALSE;
	wchar_t wszTempValue[MAX_LINE_LENGTH];

	if (pwszValue)
	{
		pwszValue[0] = '\0';
	}

	if (g_LSAPIManager.IsInitialized())
	{
		bReturn = g_LSAPIManager.GetSettingsManager()->GetVariable(pwszKeyName, wszTempValue, MAX_LINE_LENGTH);

		if (bReturn && pwszValue)
		{
			StringCchCopy(pwszValue, wcslen(wszTempValue) + 1, wszTempValue);
		}
	}

	return bReturn;
}





void LSSetVariable(LPCWSTR pwszKeyName, LPCWSTR pwszValue)
{
	if (g_LSAPIManager.IsInitialized())
	{
		g_LSAPIManager.GetSettingsManager()->SetVariable(pwszKeyName, pwszValue);
	}
}
