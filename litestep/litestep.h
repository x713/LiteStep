/*
This is a part of the LiteStep Shell Source code.

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
#ifndef __LITESTEP_H
#define __LITESTEP_H

#include "../utility/common.h"
#include "../utility/ILiteStep.h"
#include <string>
#include <vector>


// forward declarations
class IService;
class TrayService;
class DataStore;
class MessageManager;
class ModuleManager;


// RegisterShellHook flags
#define RSH_UNREGISTER	0
#define RSH_REGISTER	1
#define RSH_PROGMAN		2
#define RSH_TASKMAN		3

#define LT_RUDEAPP   0xBEAF

// Program Options
const wchar_t wszMainWindowClass[] = L"TApplication";
const wchar_t wszMainWindowTitle[] = L"LiteStep";

#define GWL_CLASSPOINTER	0


class CLiteStep: public ILiteStep
{
public:
  CLiteStep();
  ~CLiteStep();

  HRESULT Start(HINSTANCE hInstance, int nStartupMode);

  static LRESULT CALLBACK ExternalWndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  // ILiteStep
  int MessageHandler(MSG &message);

private:

  LRESULT InternalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  void _Recycle();
    HRESULT _EnumRevIDs(LSENUMREVIDSPROC pfnCallback, LPARAM lParam) const;

  // Application instance
  HINSTANCE m_hInstance;

  // LSAutoHideModules helpers
  bool m_bRudeAppBit;
  bool m_bAutoHideModules;
  bool m_bAppIsFullScreen; // = false;
  static bool _IsWindowFullScreen(HWND hWnd);
  static BOOL CALLBACK _EnumThreadFSWnd(HWND hWnd, LPARAM lParam);
  bool _IsFullScreenActive(HWND hWnd);
  void _HandleFullScreenApp(bool isFullscreen);

  // Windows
  HWND m_hMainWindow; // = NULL;

  // Shell hooks
  UINT WM_ShellHook; // = 0;

  // Function blocks
  UINT m_BlockRecycle; // = 0

  //
  // Manager Related
  //
  ModuleManager* m_pModuleManager; // = NULL;
  DataStore* m_pDataStoreManager; // = NULL;
  MessageManager* m_pMessageManager; // = NULL;

  HRESULT _InitManagers();
  HRESULT _StartManagers();
  HRESULT _StopManagers();
  void _CleanupManagers();

  bool m_bHookManagerStarted; // = false
  bool m_bSignalExit; // = false

  //
  // Service Related
  //
  TrayService* m_pTrayService; // = NULL;
    std::vector<IService*> m_Services;
    
  HRESULT _InitServices();
  HRESULT _StartServices();
  HRESULT _StopServices();
  void _CleanupServices();
};

#endif // __LITESTEP_H
