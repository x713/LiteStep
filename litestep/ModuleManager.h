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
#ifndef __MODULEMANAGER_H
#define __MODULEMANAGER_H

#include "../utility/common.h"
#include "../utility/IManager.h"
#include "module.h"
#include <list>


typedef std::list<Module*> ModuleQueue;

class ModuleManager: public IManager
{
public:
  ModuleManager();
  virtual ~ModuleManager();
  
  // IManager
  HRESULT Start(ILiteStep *ILiteStep);
  HRESULT Stop();
  
  HRESULT rStart();
  HRESULT rStop();
  
  BOOL LoadModule(LPCWSTR pwszLocation, DWORD dwFlags);
  BOOL QuitModule(HINSTANCE hModule);
    BOOL QuitModule(LPCWSTR pwszLocation);
    BOOL ReloadModule(HINSTANCE hModule);

    HRESULT EnumModules(LSENUMMODULESPROC pfnCallback, LPARAM lParam) const;
  
private:
  UINT _LoadModules();
  UINT _StartModules(ModuleQueue& mqModules);
  void _QuitModules();
  
  ModuleQueue::iterator _FindModule(LPCWSTR pwszLocation);
    ModuleQueue::iterator _FindModule(HINSTANCE hModule);
    Module* _MakeModule(LPCWSTR pwszLocation, DWORD dwFlags);
    
    void _WaitForModules(const HANDLE* pHandles, DWORD dwCount) const;
    
    ModuleQueue m_ModuleQueue;
    ILiteStep *m_pILiteStep;

    HWND m_hLiteStep;
    std::wstring m_wsAppPath;
    
    struct IsLocationEqual
    {
        IsLocationEqual(LPCWSTR pwszLocation) : m_pwszLocation(pwszLocation){}
        
        bool operator() (const Module* pModule) const
        {
            return (_wcsicmp(m_pwszLocation, pModule->GetLocation()) == 0);
        }
        
    private:
        LPCWSTR m_pwszLocation;
    };
    
    struct IsInstanceEqual
    {
        IsInstanceEqual(HINSTANCE hInstance) : m_hInstance(hInstance) {}
        
        bool operator() (const Module* pModule) const
        {
            return (pModule->GetInstance() == m_hInstance);
        }
        
    private:
        HINSTANCE m_hInstance;
    };
};

#endif // __MODULEMANAGER_H
