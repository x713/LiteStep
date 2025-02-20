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
#ifndef __LSMACROS_H
#define __LSMACROS_H

#include "common.h"
#include <string>
#include "..\lsapi\lsapi.h"

// Macros for Resource strings and localization
#ifndef _LSRESOURCEBUFFER
#define _LSRESOURCEBUFFER

#ifdef _UNICODE
//static std::wstring resourceTextBuffer;// [MAX_LINE_LENGTH + 1] ;
//static std::wstring resourceTitleBuffer;// [MAX_LINE_LENGTH + 1] ;
static wchar_t resourceTextBuffer[MAX_LINE_LENGTH + 1];
static wchar_t resourceTitleBuffer[MAX_LINE_LENGTH + 1];
#else

static char resourceTextBuffer[MAX_LINE_LENGTH + 1];
static char resourceTitleBuffer[MAX_LINE_LENGTH + 1];

#endif // _UNICODE
#endif // _LSRESOURCEBUFFER


#ifdef RESOURCE_MSGBOX
#undef RESOURCE_MSGBOX
#endif
#ifdef _UNICODE
#define RESOURCE_MSGBOX RESOURCE_MSGBOXW
#else
#define RESOURCE_MSGBOX RESOURCE_MSGBOXA
#endif // _UNICODE



#define RESOURCE_MSGBOXW(instance, id, deftext, title) \
    GetResStr( (instance), (id)                       \
        ,resourceTextBuffer, MAX_LINE_LENGTH          \
        ,deftext);                                    \
    MessageBox( NULL                                  \
        ,resourceTextBuffer, (title)                  \
        ,MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION )


#define RESOURCE_MSGBOX_T(instance, id, deftext, titleid, deftitle) \
    GetResStr( (instance), (id)                                     \
        ,resourceTextBuffer, MAX_LINE_LENGTH                        \
        ,deftext );                                                 \
    GetResStr( (instance), (titleid)                                \
        ,resourceTitleBuffer, MAX_LINE_LENGTH                       \
        ,deftitle );                                                \
    MessageBox( NULL                                                \
        ,resourceTextBuffer, resourceTitleBuffer                    \
        ,MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION )

#ifdef _UNICODE
#define RESOURCE_STR(instance, id, deftext)  \
    GetResStr( (instance), (id)              \
        ,resourceTextBuffer, MAX_LINE_LENGTH \
        ,deftext )
#else
#define RESOURCE_STR(instance, id, deftext)  \
    GetResStr( (instance), (id)              \
        ,resourceTextBuffer, MAX_LINE_LENGTH \
        ,deftext )
#endif
#define RESOURCE_STREX GetResStrEx

#define RESOURCE_DISPLAY_MSGBOX                   \
    MessageBox( NULL                              \
        ,resourceTextBuffer, resourceTitleBuffer  \
        ,MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION )

#define RESOURCE_YESNO_MSGBOX                    \
    MessageBox( NULL                             \
        ,resourceTextBuffer, resourceTitleBuffer \
        ,MB_YESNO | MB_TOPMOST )

#define RESOURCE_TITLE(instance, id, deftext) \
    GetResStr( (instance), (id)               \
        ,resourceTitleBuffer, MAX_LINE_LENGTH \
        ,deftext )

// Code for multimonitor systems
#define SCREEN_LEFT GetSystemMetrics(SM_XVIRTUALSCREEN)
#define SCREEN_TOP GetSystemMetrics(SM_YVIRTUALSCREEN)
#define IS_MULTI_MON (GetSystemMetrics(SM_CMONITORS) > 1)
#define SCREEN_WIDTH (IS_MULTI_MON ? GetSystemMetrics(SM_CXVIRTUALSCREEN) : GetSystemMetrics(SM_CXSCREEN))
#define SCREEN_HEIGHT (IS_MULTI_MON ? GetSystemMetrics(SM_CYVIRTUALSCREEN) : GetSystemMetrics(SM_CYSCREEN))

// Converts coordinates properly, taking into account for negative values
// that are relative to the right/bottom of the screen
#define CONVERT_COORDINATE_WIDTH(x) ((x) + ((x) < 0 ? SCREEN_WIDTH : 0))
#define CONVERT_COORDINATE_HEIGHT(y) ((y) + ((y) < 0 ? SCREEN_HEIGHT : 0))
#define CONVERT_COORDINATE_X(x) (SCREEN_LEFT + CONVERT_COORDINATE_WIDTH((x)))
#define CONVERT_COORDINATE_Y(y) (SCREEN_TOP + CONVERT_COORDINATE_HEIGHT((y)))

#endif // __LSMACROS_H
