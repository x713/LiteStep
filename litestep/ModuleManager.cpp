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
#include "ModuleManager.h"
#include <algorithm>
#include <vector>
#include "../utility/core.hpp"

ModuleManager::ModuleManager() :
  m_pILiteStep(NULL), m_hLiteStep(NULL)
{
}

ModuleManager::~ModuleManager()
{
}

HRESULT ModuleManager::Start(ILiteStep* pILiteStep)
{
  ASSERT_ISNULL(m_pILiteStep);

  HRESULT hr = E_FAIL;

  if (pILiteStep != NULL)
  {
    m_pILiteStep = pILiteStep;
    m_pILiteStep->AddRef();

    wchar_t* wszAppPath = new wchar_t[MAX_PATH]();

    m_hLiteStep = GetLitestepWnd();

    if (m_hLiteStep && LSGetLitestepPath(wszAppPath, MAX_PATH))
    {
      m_wsAppPath = wszAppPath;

      _LoadModules();

      hr = S_OK;
    }

    delete[] wszAppPath;
  }
  else
  {
    hr = E_INVALIDARG;
  }

  return hr;
}

HRESULT ModuleManager::Stop()
{
  HRESULT hr = S_OK;

  _QuitModules();

  if (m_pILiteStep)
  {
    m_pILiteStep->Release();
    m_pILiteStep = NULL;
  }

  return hr;
}

HRESULT ModuleManager::rStart()
{
  HRESULT hr = S_OK;

  _LoadModules();

  return hr;
}

HRESULT ModuleManager::rStop()
{
  HRESULT hr = S_OK;

  _QuitModules();

  return hr;
}


UINT ModuleManager::_LoadModules()
{
  ASSERT(m_ModuleQueue.empty());

  UINT uReturn = 0;
  wchar_t wszLine[MAX_LINE_LENGTH];

  FILE* f = LCOpen(NULL);

  if (f)
  {
    // need to use a separate queue as modules may load other modules (e.g.
        // mzscript via LoadModule) during the startup process
    ModuleQueue mqModules;

    while (LCReadNextConfig(f, L"LoadModule", wszLine, MAX_LINE_LENGTH))
    {
      wchar_t wszToken1[MAX_LINE_LENGTH] = { 0 };
      wchar_t wszToken2[MAX_LINE_LENGTH] = { 0 };

      // first buffer takes the "LoadModule" token
      LPWSTR lpszBuffers[] = { NULL, wszToken1, wszToken2 };

      if (LCTokenize(wszLine, lpszBuffers, 3, NULL) >= 2)
      {
        DWORD dwFlags = 0;

        if (_wcsicmp(wszToken2, L"threaded") == 0)
        {
          dwFlags |= LS_MODULE_THREADED;
        }

        Module* pModule = _MakeModule(wszToken1, dwFlags);

        if (pModule)
        {
          mqModules.push_back(pModule);
        }
      }
    }
    LCClose(f);

    uReturn = _StartModules(mqModules);
  }

  return uReturn;
}


BOOL ModuleManager::LoadModule(LPCWSTR pwszLocation, DWORD dwFlags)
{
  BOOL bReturn = FALSE;

  Module* pModule = _MakeModule(pwszLocation, dwFlags);

  if (pModule)
  {
    ModuleQueue	mqModule(1, pModule);
    bReturn = (_StartModules(mqModule) == 1);
  }

  return bReturn;
}


Module* ModuleManager::_MakeModule(LPCWSTR pwszLocation, DWORD dwFlags)
{
  Module* pModule = NULL;

#if !defined(LS_NO_EXCEPTION)
  try
  {
#endif /* LS_NO_EXCEPTION */
    pModule = new Module(pwszLocation, dwFlags);
#if !defined(LS_NO_EXCEPTION)
  }
  catch (...)
  {
    delete pModule;
    pModule = NULL;
  }
#endif /* LS_NO_EXCEPTION */

  return pModule;
}


UINT ModuleManager::_StartModules(ModuleQueue& mqModules)
{
  UINT uReturn = 0;

  if (mqModules.size() > 0)
  {
    std::vector<HANDLE> vecInitEvents;
    ModuleQueue::iterator iter = mqModules.begin();

    while (iter != mqModules.end())
    {
      if (*iter)
      {
        if (_FindModule((*iter)->GetLocation()) == m_ModuleQueue.end())
        {
          if ((*iter)->Init(m_hLiteStep, m_wsAppPath))
          {
            if ((*iter)->GetInitEvent())
            {
              /* Note: We are taking ownership of the Event handle
               * here.  We call CloseHandle() below. */
              vecInitEvents.push_back((*iter)->TakeInitEvent());
            }

            m_ModuleQueue.push_back(*iter);
            uReturn++;

            iter++;
            continue;
          }
        }
      }

      /* If we got here, then this is an invalid
       * entry, and needs erased. */
      ModuleQueue::iterator iterOld = iter++;

      delete* iterOld;
      mqModules.erase(iterOld);
    }

    // Are there any "threaded" modules?
    if (!vecInitEvents.empty())
    {
      // Wait for all modules to signal that they have started.
      _WaitForModules(&vecInitEvents[0], vecInitEvents.size());

      // Close the handles we have taken ownership of.
      std::for_each(
        vecInitEvents.begin(), vecInitEvents.end(), CloseHandle);
    }
  }

  return uReturn;
}

void ModuleManager::_QuitModules()
{
  std::vector<HANDLE> vecQuitObjects;
  ModuleQueue::reverse_iterator iter = m_ModuleQueue.rbegin();
  ModuleQueue TempQueue;

  /* Note:
   * Store each module in a temporary queue, so that the module
   * may not be accessed via our main queue while it is being
   * unloaded.  This does -not- protect us from threads, however
   * it does hopefully add some security through obscurity from
   * recursion. */

  while (iter != m_ModuleQueue.rend())
  {
    TempQueue.push_back(*iter);

    if (*iter)
    {
      (*iter)->Quit();

      if ((*iter)->GetThread())
      {
        vecQuitObjects.push_back((*iter)->TakeThread());
      }
    }

    ++iter;
  }

  m_ModuleQueue.clear();

  if (!vecQuitObjects.empty())
  {
    _WaitForModules(&vecQuitObjects[0], vecQuitObjects.size());

    // Close the handles we have taken ownership of.
    std::for_each(
      vecQuitObjects.begin(), vecQuitObjects.end(), CloseHandle);
  }

  // Clean it all up
  iter = TempQueue.rbegin();
  while (iter != TempQueue.rend())
  {
    delete* iter;
    ++iter;
  }
}


BOOL ModuleManager::QuitModule(HINSTANCE hModule)
{
  ModuleQueue::iterator iter = _FindModule(hModule);

  if (iter != m_ModuleQueue.end() && *iter)
  {
    (*iter)->Quit();

    if ((*iter)->GetThread())
    {
      HANDLE hThread = (*iter)->TakeThread();

      _WaitForModules(&hThread, 1);

      CloseHandle(hThread);
    }

    delete* iter;
    m_ModuleQueue.erase(iter);
  }

  return TRUE;
}


BOOL ModuleManager::QuitModule(LPCWSTR pwszLocation)
{
  BOOL bReturn = FALSE;

  ModuleQueue::iterator iter = _FindModule(pwszLocation);

  if (iter != m_ModuleQueue.end() && *iter)
  {
    bReturn = QuitModule((*iter)->GetInstance());
  }

  return bReturn;
}


BOOL ModuleManager::ReloadModule(HINSTANCE hModule)
{
  BOOL bReturn = FALSE;

  ModuleQueue::iterator iter = _FindModule(hModule);

  if (iter != m_ModuleQueue.end())
  {
    std::wstring wsLocation = (*iter)->GetLocation();
    DWORD dwFlags = (*iter)->GetFlags();

    QuitModule(hModule);
    bReturn = LoadModule(wsLocation.c_str(), dwFlags);
  }

  return bReturn;
}


ModuleQueue::iterator ModuleManager::_FindModule(LPCWSTR pwszLocation)
{
  return std::find_if(m_ModuleQueue.begin(), m_ModuleQueue.end(),
    IsLocationEqual(pwszLocation));
}


ModuleQueue::iterator ModuleManager::_FindModule(HINSTANCE hModule)
{
  return std::find_if(m_ModuleQueue.begin(), m_ModuleQueue.end(),
    IsInstanceEqual(hModule));
}


void ModuleManager::_WaitForModules(const HANDLE* pHandles, DWORD dwCount) const
{
  std::vector<HANDLE> vWait(pHandles, pHandles + dwCount);

  /* Loop for as long as we have an object whose state is not signaled. */
  while (vWait.size())
  {
    MSG message;

    /* Handle all window messages for current thread */
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
    {
      m_pILiteStep->MessageHandler(message);
    }

    /* Handle all messages posted to the current thread */
    while (PeekMessage(&message, (HWND)INVALID_HANDLE_VALUE, 0, 0, PM_REMOVE))
    {
      m_pILiteStep->MessageHandler(message);
    }

    /* Wait for a new message to come in, or for one of our objects
     * to become signaled. */
    DWORD dwWaitStatus = MsgWaitForMultipleObjects(vWait.size(), &vWait[0],
      FALSE, INFINITE, QS_ALLINPUT);

    /* Recreate the pObject list, in case any of the objects do not auto
     * reset their signaled state.  Otherwise, we would drop through our
     * outer loop immediately without waiting for all of our objects. */
    if ((dwWaitStatus >= WAIT_OBJECT_0) &&
      (dwWaitStatus < (WAIT_OBJECT_0 + vWait.size())))
    {
      vWait.erase(vWait.begin() + (dwWaitStatus - WAIT_OBJECT_0));
    }
  }
}


HRESULT ModuleManager::EnumModules(LSENUMMODULESPROC pfnCallback, LPARAM lParam) const
{
  HRESULT hr = S_OK;

#if !defined(LS_NO_EXCEPTION)
  try
  {
#endif /* LS_NO_EXCEPTION */
    for (ModuleQueue::const_iterator iter = m_ModuleQueue.begin(); iter != m_ModuleQueue.end(); ++iter)
    {
      if (!pfnCallback((*iter)->GetLocation(), (*iter)->GetFlags(),
        lParam))
      {
        hr = S_FALSE;
        break;
      }
    }
#if !defined(LS_NO_EXCEPTION)
  }
  catch (...)
  {
    hr = E_UNEXPECTED;
  }
#endif /* LS_NO_EXCEPTION */

  return hr;
}
