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
/****************************************************************************
****************************************************************************/
#include "lsapiinit.h"
#include "lsapi.h"
#include "bangs.h"
#include "../utility/shellhlp.h"
#include "../utility/core.hpp"
#include <time.h>

#include <VersionHelpers.h> // For IsWindows...OrGreater functions
#include <windows.h>

#ifdef __GNUC__
using std::max;
#endif

LSAPIInit g_LSAPIManager;


LSAPIInit::LSAPIInit()
  :m_smSettingsManager(NULL)
  , m_bmBangManager(NULL)
  , m_hLitestepWnd(NULL)
  , m_bIsInitialized(false)
{
  m_dwMainThreadID = GetCurrentThreadId();

  m_wszLitestepPath[0] = '\0';
  m_wszRcPath[0] = '\0';
}

LSAPIInit::~LSAPIInit()
{
  m_bIsInitialized = false;
  delete m_bmBangManager;
  delete m_smSettingsManager;
}

void LSAPIInit::Initialize(LPCWSTR pwszLitestepPath, LPCWSTR pwszRcPath)
{
  try
  {
    // Error if called again
    if (IsInitialized())
    {
      throw LSAPIException(LSAPI_ERROR_RECURRENT);
    }

    // Do not allow any thread but the original to load us
    if (GetCurrentThreadId() != GetMainThreadID())
    {
      throw LSAPIException(LSAPI_ERROR_INVALIDTHREAD);
    }

    // Copy over the strings
    if (FAILED(StringCchCopy(m_wszLitestepPath, MAX_PATH, pwszLitestepPath)))
    {
      throw LSAPIException(LSAPI_ERROR_GENERAL);
    }

    if (FAILED(StringCchCopy(m_wszRcPath, MAX_PATH, pwszRcPath)))
    {
      throw LSAPIException(LSAPI_ERROR_GENERAL);
    }

    // Create the Bang Manager
    m_bmBangManager = new BangManager;

    if (!m_bmBangManager)
    {
      throw LSAPIException(LSAPI_ERROR_GENERAL);
    }

    // Create the Settings Manager
    m_smSettingsManager = new SettingsManager();

    if (!m_smSettingsManager)
    {
      throw LSAPIException(LSAPI_ERROR_GENERAL);
    }

    // Signal that we are initialized before continuing, so that
    // the LSAPI is available during setup
    m_bIsInitialized = true;

    // Initialize default variables
    setLitestepVars();

    // Load the default RC config file
    m_smSettingsManager->ParseFile(m_wszRcPath);

    // Add our internal bang commands to the Bang Manager.
    SetupBangs();
  }
  catch (LSAPIException& lse)
  {
    if (LSAPI_ERROR_RECURRENT != lse.Type())
    {
      m_bIsInitialized = false;

      delete m_smSettingsManager;
      m_smSettingsManager = NULL;
      delete m_bmBangManager;
      m_bmBangManager = NULL;
    }

    throw; //rethrow
  }
}

void LSAPIInit::ReloadBangs()
{
  if (!IsInitialized())
  {
    throw LSAPIException(LSAPI_ERROR_NOTINITIALIZED);
  }

  m_bmBangManager->ClearBangCommands();
  SetupBangs();
}

void LSAPIInit::ReloadSettings()
{
  if (!IsInitialized())
  {
    throw LSAPIException(LSAPI_ERROR_NOTINITIALIZED);
  }

  m_bIsInitialized = false;

  delete m_smSettingsManager;
  m_smSettingsManager = NULL;

  m_smSettingsManager = new SettingsManager();

  if (!m_smSettingsManager)
  {
    throw LSAPIException(LSAPI_ERROR_GENERAL);
  }

  m_bIsInitialized = true;

  // Reinitialize default variables
  setLitestepVars();

  // Reload the default RC config file
  m_smSettingsManager->ParseFile(m_wszRcPath);
}

void LSAPIInit::setLitestepVars()
{
  wchar_t wszTemp[MAX_PATH];
  DWORD dwLength = MAX_PATH;

  // just using a shorter name, no real reason to re-assign.
  SettingsManager* pSM = m_smSettingsManager;

  // Set the variable "litestepdir" since it was never set
  if (SUCCEEDED(StringCchCopy(wszTemp, MAX_PATH, m_wszLitestepPath)))
  {
    PathAddBackslashEx(wszTemp, MAX_PATH);
    PathQuoteSpaces(wszTemp);
    pSM->SetVariable(L"litestepdir", wszTemp);
  }

  if (GetWindowsDirectory(wszTemp, MAX_PATH))
  {
    PathAddBackslashEx(wszTemp, MAX_PATH);
    pSM->SetVariable(L"windir", wszTemp);
  }

  if (GetUserName(wszTemp, &dwLength))
  {
    PathQuoteSpaces(wszTemp);
    pSM->SetVariable(L"username", wszTemp);
  }

  if (GetShellFolderPath(CSIDL_APPDATA, wszTemp, MAX_PATH))
  {
    PathAppend(wszTemp, L"Microsoft\\Internet Explorer\\Quick Launch\\");
    PathQuoteSpaces(wszTemp);

    pSM->SetVariable(L"quicklaunch", wszTemp);
  }

  pSM->SetVariable(L"bitbucket", L"::{645FF040-5081-101B-9F08-00AA002F954E}");
  pSM->SetVariable(L"documents", L"::{450D8FBA-AD25-11D0-98A8-0800361B1103}");
  pSM->SetVariable(L"drives", L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}");
  pSM->SetVariable(L"network", L"::{208D2C60-3AEA-1069-A2D7-08002B30309D}");
  pSM->SetVariable(L"controls", L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}");
  pSM->SetVariable(L"dialup", L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{992CFFA0-F557-101A-88EC-00DD010CCC48}");
  pSM->SetVariable(L"networkanddialup", L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{7007ACC7-3202-11D1-AAD2-00805FC1270E}");
  pSM->SetVariable(L"printers", L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{2227A280-3AEA-1069-A2DE-08002B30309D}");
  pSM->SetVariable(L"scheduled", L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{D6277990-4C6A-11CF-8D87-00AA0060F5BF}");
  pSM->SetVariable(L"admintools", L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{D20EA4E1-3957-11d2-A40B-0C5020524153}");

  setShellFolderVariable(L"commondesktopdir", CSIDL_COMMON_DESKTOPDIRECTORY);
  setShellFolderVariable(L"commonfavorites", CSIDL_COMMON_FAVORITES);
  setShellFolderVariable(L"commonprograms", CSIDL_COMMON_PROGRAMS);
  setShellFolderVariable(L"commonstartmenu", CSIDL_COMMON_STARTMENU);
  setShellFolderVariable(L"commonstartup", CSIDL_COMMON_STARTUP);
  setShellFolderVariable(L"cookies", CSIDL_COOKIES);
  setShellFolderVariable(L"desktop", CSIDL_DESKTOP);
  setShellFolderVariable(L"desktopdir", CSIDL_DESKTOPDIRECTORY);
  setShellFolderVariable(L"favorites", CSIDL_FAVORITES);
  setShellFolderVariable(L"fonts", CSIDL_FONTS);
  setShellFolderVariable(L"history", CSIDL_HISTORY);
  setShellFolderVariable(L"internet", CSIDL_INTERNET);
  setShellFolderVariable(L"internetcache", CSIDL_INTERNET_CACHE);
  setShellFolderVariable(L"nethood", CSIDL_NETHOOD);
  setShellFolderVariable(L"documentsdir", CSIDL_PERSONAL);
  setShellFolderVariable(L"printhood", CSIDL_PRINTHOOD);
  setShellFolderVariable(L"programs", CSIDL_PROGRAMS);
  setShellFolderVariable(L"recent", CSIDL_RECENT);
  setShellFolderVariable(L"sendto", CSIDL_SENDTO);
  setShellFolderVariable(L"startmenu", CSIDL_STARTMENU);
  setShellFolderVariable(L"startup", CSIDL_STARTUP);
  setShellFolderVariable(L"templates", CSIDL_TEMPLATES);
  setShellFolderVariable(L"commonadmintoolsdir", CSIDL_COMMON_ADMINTOOLS);
  setShellFolderVariable(L"admintoolsdir", CSIDL_ADMINTOOLS);


  // Default platform conditionals to FALSE
  // and set the correct platform conditional to TRUE
  setOSVariables(pSM);

  // screen resolution
  StringCchPrintf(wszTemp, MAX_PATH, L"%d", GetSystemMetrics(SM_CXSCREEN));
  pSM->SetVariable(L"ResolutionX", wszTemp);

  StringCchPrintf(wszTemp, MAX_PATH, L"%d", GetSystemMetrics(SM_CYSCREEN));
  pSM->SetVariable(L"ResolutionY", wszTemp);

  // build date/time from PE headers
  getCompileTime(wszTemp, MAX_PATH);
  pSM->SetVariable(L"CompileDate", wszTemp);

#ifdef LS_CUSTOM_INCLUDEFOLDER
  pSM->SetVariable("IncludeFolder", "1");
#endif // LS_CUSTOM_INCLUDEFOLDER
}



void LSAPIInit::setOSVariables(SettingsManager* pSM) {
  if (!pSM) {
    return; // Or throw an exception
  }

  // Initialize all variables to false
  std::initializer_list<LPCWSTR> osVars = {
      L"Win9x", L"WinME", L"Win98", L"Win95",
      L"WinNT", L"Win2003", L"WinXP", L"Win2000", L"WinNT4"
  };
  for (LPCWSTR var : osVars) {
    pSM->SetVariable(var, L"false");
  }

  pSM->SetVariable(L"WinNT", L"true"); // All modern Windows are NT-based

  if (IsWindows10OrGreater()) {
    pSM->SetVariable(L"WinNT", L"true");
  }
  else if (IsWindows8Point1OrGreater()) {
    pSM->SetVariable(L"WinNT", L"true");
  }
  else if (IsWindows8OrGreater()) {
    pSM->SetVariable(L"WinNT", L"true");
  }
  else if (IsWindows7OrGreater()) {
    pSM->SetVariable(L"WinNT", L"true");
  }
  else if (IsWindowsVistaOrGreater()) {
    pSM->SetVariable(L"WinNT", L"true");
  }
  else if (IsWindowsXPOrGreater()) {
    pSM->SetVariable(L"WinNT", L"true");
    pSM->SetVariable(L"WinXP", L"true");
  } //  NO CHECKS for versions older than XP (2000, NT4, Win9x, WinME, Win98, Win95) - Use feature detection if needed.
}



bool LSAPIInit::setShellFolderVariable(LPCWSTR pszVariable, int nFolder)
{
  wchar_t wszPath[MAX_PATH] = { 0 };

  bool bReturn = GetShellFolderPath(nFolder, wszPath, MAX_PATH);

  if (bReturn)
  {
    PathAddBackslashEx(wszPath, MAX_PATH);
    PathQuoteSpaces(wszPath);

    m_smSettingsManager->SetVariable(pszVariable, wszPath);
  }

  return bReturn;
}



// Gets the compiletime/date from the PE header
//
#define MakePtr(cast, ptr, addValue) (cast)((DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))
void LSAPIInit::getCompileTime(LPWSTR pszValue, size_t cchValue)
{
  IMAGE_DOS_HEADER* dosheader;
  IMAGE_NT_HEADERS* ntheader;
  time_t lsexetime;
  time_t lsapitime;
  time_t hooktime = 0;
  time_t compiletime;

  // Get the litestep.exe build time.
  dosheader = (IMAGE_DOS_HEADER*)GetModuleHandle(NULL);
  ASSERT(dosheader);
  ASSERT(dosheader->e_magic == IMAGE_DOS_SIGNATURE);
  ntheader = MakePtr(IMAGE_NT_HEADERS*, dosheader, dosheader->e_lfanew);
  ASSERT(ntheader);
  lsexetime = (time_t)(ntheader->FileHeader.TimeDateStamp);

  // Get the lsapi.dll build time (TODO: don't hardcode "lsapi.dll")
  dosheader = (IMAGE_DOS_HEADER*)GetModuleHandle(_T("lsapi.dll"));
  ASSERT(dosheader);
  ASSERT(dosheader->e_magic == IMAGE_DOS_SIGNATURE);
  ntheader = MakePtr(IMAGE_NT_HEADERS*, dosheader, dosheader->e_lfanew);
  ASSERT(ntheader);
  lsapitime = (time_t)(ntheader->FileHeader.TimeDateStamp);

  // Get the hook.dll build time (TODO: don't hardcode "hook.dll")
  dosheader = (IMAGE_DOS_HEADER*)GetModuleHandle(_T("hook.dll"));
  if (dosheader)
  {
    ASSERT(dosheader->e_magic == IMAGE_DOS_SIGNATURE);
    ntheader = MakePtr(IMAGE_NT_HEADERS*, dosheader, dosheader->e_lfanew);
    ASSERT(ntheader);
    hooktime = (time_t)(ntheader->FileHeader.TimeDateStamp);
  }

  compiletime = max(lsexetime, max(lsapitime, hooktime));
  tm timeStruct;
  errno_t error = gmtime_s(&timeStruct, &compiletime);

  if (error != 0) {
    StringCchPrintf(pszValue, cchValue, L"\"Compiled at an unknown time\"");
  }
  else {
    // Calculate required buffer size (including null terminator)
    size_t requiredSize = _tcsftime(nullptr, 0, _T("\"Compiled on %b %d %Y at %H:%M:%S UTC\""), &timeStruct) + 1;

    if (cchValue < requiredSize) {
      // Handle insufficient buffer size (e.g., truncate, return an error)
      StringCchPrintf(pszValue, cchValue, L"\"Compiled at an unknown time\""); // Or a truncated message
      // ... Log or handle the error as needed.
    }
    else {
      _tcsftime(pszValue, cchValue, _T("\"Compiled on %b %d %Y at %H:%M:%S UTC\""), &timeStruct);
    }
  }
}
