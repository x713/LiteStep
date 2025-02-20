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
#ifndef  __LSAPI_H
#define  __LSAPI_H

/*
 * IMPORTANT:
 *   The exports and definitions included
 *   with LSAPI_PRIVATE are for INTERNAL USE ONLY!
 *
 *   They MUST NOT be used by any module.
 *   Their names and/or functionality may be changed at ANY time!
 *
 */
#include "../litestep/buildoptions.h"

#include <windows.h>
#include <stdio.h>
#include "lsmultimon.h"

#include "lsapidefines.h"

#ifdef LSAPI_INTERNAL /* only defined when compiling lsapi */
#  define LSAPI __declspec(dllexport)
#else
#  define LSAPI __declspec(dllimport)
#endif	/* LSAPI_INTERNAL */

#define MAX_TOKEN_SIZE MAX_PATH_LENGTH

#ifdef  __cplusplus
extern "C"
{
#endif  // __cplusplus

  LSAPI FILE* LCOpen(LPCWSTR wszPath);
  LSAPI BOOL LCClose(FILE* f);
  LSAPI BOOL LCReadNextCommand(FILE* pFile, LPWSTR pwszValue, size_t cchValue);
  LSAPI BOOL LCReadNextConfig(FILE* pFile, LPCWSTR pwszConfig, LPWSTR pwszValue, size_t cchValue);
  LSAPI BOOL LCReadNextLine(FILE* pFile, LPWSTR pwszValue, size_t cchValue);
  LSAPI int LCTokenize(LPCWSTR wszString, LPWSTR* lpwszBuffers, DWORD dwNumBuffers, LPWSTR wszExtraParameters);

  LSAPI int GetRCInt(LPCWSTR lpKeyName, int nDefault);
  LSAPI BOOL GetRCString(LPCWSTR lpKeyName, LPWSTR value, LPCWSTR defStr, int maxLen);
  LSAPI BOOL GetRCBool(LPCWSTR lpKeyName, BOOL ifFound);
  LSAPI BOOL GetRCBoolDef(LPCWSTR lpKeyName, BOOL bDefault);
  LSAPI BOOL GetRCLine(LPCWSTR, LPWSTR, UINT, LPCWSTR);
  LSAPI COLORREF GetRCColor(LPCWSTR lpKeyName, COLORREF colDef);

  LSAPI BOOL LSGetVariable(LPCWSTR pwszKeyName, LPWSTR pwszValue);
  LSAPI BOOL LSGetVariableEx(LPCWSTR pwszKeyName, LPWSTR pwszValue, DWORD dwLength);
  LSAPI void LSSetVariable(LPCWSTR pwszKeyName, LPCWSTR pwszValue);

  LSAPI BOOL AddBangCommand(LPCWSTR pwszCommand, BangCommand pfnBangCommand);
  LSAPI BOOL AddBangCommandEx(LPCWSTR pwszCommand, BangCommandEx pfnBangCommand);
  LSAPI BOOL RemoveBangCommand(LPCWSTR pwszCommand);
  LSAPI BOOL ParseBangCommand(HWND hCaller, LPCWSTR pwszCommand, LPCWSTR pwszArgs);

  LSAPI HRGN BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance, int xoffset, int yoffset);
  LSAPI HBITMAP BitmapFromIcon(HICON hIcon);
  LSAPI HBITMAP LoadLSImage(LPCWSTR wszFile, LPCWSTR wszImage);
  LSAPI HICON LoadLSIcon(LPCWSTR wszImage, LPCWSTR wszFile);
  LSAPI void GetLSBitmapSize(HBITMAP hBitmap, LPINT nWidth, LPINT nHeight);
  LSAPI void TransparentBltLS(HDC dc, int nXDest, int nYDest, int nWidth, int nHeight, HDC tempDC, int nXSrc, int nYSrc, COLORREF colorTransparent);

  LSAPI int CommandTokenize(LPCWSTR wszString, LPWSTR* lpwszBuffers, DWORD dwNumBuffers, LPWSTR wszExtraParameters);
  LSAPI void CommandParse(LPCWSTR pwszCommand, LPWSTR pwszOutCommand, LPWSTR pwszOutArgs, size_t cchOutCommand, size_t cchOutArgs);

  LSAPI HINSTANCE LSExecute(HWND Owner, LPCWSTR wszCommand, int nShowCmd);
  LSAPI HINSTANCE LSExecuteEx(HWND Owner, LPCWSTR wszOperation, LPCWSTR wszCommand, LPCWSTR wszArgs, LPCWSTR wszDirectory, int nShowCmd);

  LSAPI HWND GetLitestepWnd(void);
  LSAPI BOOL WINAPI LSGetLitestepPath(LPWSTR pwszPath, size_t cchPath);
  LSAPI BOOL WINAPI LSGetImagePath(LPWSTR pwszPath, size_t cchPath);

  LSAPI void VarExpansion(LPWSTR pwszExpandedString, LPCWSTR pwszTemplate);
  LSAPI void VarExpansionEx(LPWSTR pwszExpandedString, LPCWSTR pwszTemplate, size_t cchExpandedString);

  LSAPI BOOL GetToken(LPCWSTR wszString, LPWSTR wszToken, LPCWSTR* wszNextToken, BOOL useBrackets);
  LSAPI void Frame3D(HDC dc, RECT rect, COLORREF TopColor, COLORREF BottomColor, int Width);
  LSAPI void SetDesktopArea(int left, int top, int right, int bottom);

  LSAPI BOOL match(LPCWSTR pattern, LPCWSTR text);
  LSAPI int matche(LPCWSTR pattern, LPCWSTR text);
  LSAPI BOOL is_valid_pattern(LPCWSTR p, LPINT error_type);

  LSAPI void GetResStr(HINSTANCE hInstance, UINT uIDText, LPWSTR pwszText, size_t cchText, LPCWSTR pwszDefText);
  LSAPI void GetResStrEx(HINSTANCE hInstance, UINT uIDText, LPWSTR pwszText, size_t cchText, LPCWSTR pwszDefText, ...);

  LSAPI int LSGetSystemMetrics(int);
  LSAPI HMONITOR LSMonitorFromWindow(HWND, DWORD);
  LSAPI HMONITOR LSMonitorFromRect(LPCRECT, DWORD);
  LSAPI HMONITOR LSMonitorFromPoint(POINT, DWORD);
  LSAPI BOOL LSGetMonitorInfo(HMONITOR, LPMONITORINFO);
  LSAPI BOOL LSEnumDisplayMonitors(HDC, LPCRECT, MONITORENUMPROC, LPARAM);
  LSAPI BOOL LSEnumDisplayDevices(PVOID, DWORD, PDISPLAY_DEVICE, DWORD);

  LSAPI BOOL WINAPI LSLog(int nLevel, LPCWSTR pwszModule, LPCWSTR pwszMessage);
  LSAPI BOOL WINAPIV LSLogPrintf(int nLevel, LPCWSTR pwszModule, LPCWSTR pwszFormat, ...);
  LSAPI int GetRCCoordinate(LPCWSTR pszKeyName, int nDefault, int nMaxVal);
  LSAPI int ParseCoordinate(LPCWSTR wszString, int nDefault, int nMaxVal);

  LSAPI HRESULT EnumLSData(UINT uInfo, FARPROC pfnCallback, LPARAM lParam);

#ifdef LSAPI_PRIVATE
  LSAPI BOOL LSAPIInitialize(LPCWSTR pwszLitestepPath, LPCWSTR pwszRcPath);
  LSAPI void LSAPIReloadBangs(void);
  LSAPI void LSAPIReloadSettings(void);
  LSAPI void LSAPISetLitestepWindow(HWND hLitestepWnd);
  LSAPI BOOL InternalExecuteBangCommand(HWND hCaller, LPCWSTR pwszCommand, LPCWSTR pwszArgs);
#endif /* LSAPI_PRIVATE */

#ifdef  __cplusplus
};
#endif  // __cplusplus

#endif  // __LSAPI_H
