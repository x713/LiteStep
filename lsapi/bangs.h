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
#ifndef __BANGS_H
#define __BANGS_H

#include "../utility/common.h"

void SetupBangs();

void BangAbout(HWND hCaller, LPCWSTR pwszArgs);
void BangAlert(HWND hCaller, LPCWSTR pwszArgs);
void BangCascadeWindows(HWND hCaller, LPCWSTR pwszArgs);
void BangConfirm(HWND hCaller, LPCWSTR pwszArgs);
void BangExecute(HWND hCaller, LPCWSTR pwszArgs);
void BangHideModules (HWND hCaller, LPCWSTR pwszArgs);
void BangLogoff(HWND hCaller, LPCWSTR pwszArgs);
void BangMinimizeWindows(HWND hCaller, LPCWSTR pwszArgs);
void BangQuit(HWND caller, LPCWSTR param);
void BangRecycle (HWND hCaller, LPCWSTR pwszArgs);
void BangRefresh(HWND hCaller, LPCWSTR pwszArgs);
void BangReload(HWND hCaller, LPCWSTR pwszArgs);
void BangReloadModule (HWND hCaller, LPCWSTR pwszArgs);
void BangRestoreWindows(HWND hCaller, LPCWSTR pwszArgs);
void BangRun (HWND hCaller, LPCWSTR pwszArgs);
void BangShowModules (HWND hCaller, LPCWSTR pwszArgs);
void BangShutdown(HWND hCaller, LPCWSTR pwszArgs);
void BangSwitchUser(HWND hCaller, LPCWSTR pwszArgs);
void BangTileWindowsH(HWND hCaller, LPCWSTR pwszArgs);
void BangTileWindowsV(HWND hCaller, LPCWSTR pwszArgs);
void BangToggleModules (HWND hCaller, LPCWSTR pwszArgs);
void BangUnloadModule (HWND hCaller, LPCWSTR pwszArgs);

#endif // __BANGS_H

