/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2025 The x7 Dev Team
Copyright (C) 2006 The LiteStep Development Team

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
#if !defined(LSAPIINIT_H)
#define LSAPIINIT_H

#include "../utility/common.h"
#include "BangManager.h"
#include "SettingsManager.h"

enum ErrorType
{
     LSAPI_ERROR_GENERAL
    ,LSAPI_ERROR_NOTINITIALIZED
    ,LSAPI_ERROR_RECURRENT
    ,LSAPI_ERROR_INVALIDTHREAD
    ,LSAPI_ERROR_SETTINGSINIT
    ,LSAPI_ERROR_SETTINGSMANAGER
    ,LSAPI_ERROR_BANGMANAGER
    ,LSAPI_ERROR_BANGINIT
};

class LSAPIException
{
public:
    explicit LSAPIException(ErrorType et) throw() :m_et(et) { }
    ErrorType Type() const throw() { return m_et; }
    
private:
    ErrorType m_et;
};

class LSAPIInit
{
private:
    bool setShellFolderVariable(LPCWSTR pwszVariable, int nFolder);
    void setLitestepVars();
    void setOSVariables(SettingsManager* pSM);
    void getCompileTime(LPWSTR pwszValue, size_t cchValue);
    
public:
    LSAPIInit();
    ~LSAPIInit();
    
    BangManager* GetBangManager() const
    {
        if(!IsInitialized())
        {
            throw LSAPIException(LSAPI_ERROR_NOTINITIALIZED);
        }
        
        return m_bmBangManager;
    }
    
    SettingsManager* GetSettingsManager() const
    {
        if(!IsInitialized())
        {
            throw LSAPIException(LSAPI_ERROR_NOTINITIALIZED);
        }
        
        return m_smSettingsManager;
    }
    
    HWND GetLitestepWnd() const
    {
        return m_hLitestepWnd;
    }
    
    DWORD GetMainThreadID() const
    {
        return m_dwMainThreadID;
    }
    
    void Initialize(LPCWSTR pwszLitestepPath, LPCWSTR pwszRcPath);
    
    bool IsInitialized() const
    {
        return m_bIsInitialized;
    }
    
    void ReloadBangs();
    void ReloadSettings();
    
    void SetLitestepWindow(HWND hLitestepWnd)
    {
        m_hLitestepWnd = hLitestepWnd;
    }
    
private:
    DWORD m_dwMainThreadID;
    
    BangManager* m_bmBangManager;
    SettingsManager* m_smSettingsManager;
    
    HWND m_hLitestepWnd;
    wchar_t m_wszLitestepPath[MAX_PATH];
    wchar_t m_wszRcPath[MAX_PATH];
    
    bool m_bIsInitialized;
};

extern LSAPIInit g_LSAPIManager;

#endif // LSAPIINIT_H
