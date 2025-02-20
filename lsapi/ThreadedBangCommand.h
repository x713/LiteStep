/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2025 The x7 Dev Team
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
#ifndef __THREADEDBANGCOMMAND_H
#define __THREADEDBANGCOMMAND_H

#include "../utility/common.h"
#include "../utility/base.h"
#include "../utility/safestr.h"
#include "lsapi.h"

class ThreadedBangCommand : public CountedBase
{
public:
    ThreadedBangCommand(HWND hCaller, LPCWSTR pwszName, LPCWSTR pwszParams)
    :m_hCaller(hCaller)
    {
        ASSERT_ISNOTNULL(pwszName);

        // pwszName is guaranteed to be non-NULL
        StringCchCopy(m_wszName, MAX_BANGCOMMAND, pwszName);

        if (pwszParams)
        {
            StringCchCopy(m_wszParams, MAX_BANGARGS, pwszParams);
        }
        else
        {
            m_wszParams[0] = '\0';
        }
    }

    void Execute()
    {
        // Cannot use ParseBangCommand here because that would expand variables
        // again - and some themes rely on the fact that they are expanded only
        // once. Besides, it would create inconsistent behavior.
        InternalExecuteBangCommand(m_hCaller, m_wszName, m_wszParams);
    }

private:
    wchar_t m_wszName[MAX_BANGCOMMAND];
    wchar_t m_wszParams[MAX_BANGARGS];
    HWND m_hCaller;
};

#endif // __THREADEDBANGCOMMAND_H
