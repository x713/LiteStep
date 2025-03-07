/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2006-2007 The LiteStep Development Team

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
#include <sstream>
#include <string>
#include "../utility/localization.h"
#include "MathEvaluate.h"
#include "MathException.h"
#include "MathParser.h"
#include "SettingsDefines.h"

using namespace std;


bool MathEvaluateBool(const SettingsMap& context, const wstring& expression,
    bool& result, unsigned int flags)
{
    try
    {
        const StringSet recursiveVarSet; // dummy set
        MathParser mathParser(context, expression, recursiveVarSet, flags);
        result = mathParser.Evaluate().ToBoolean();
    }
    catch (const MathException& e)
    {
        // FIXME: Localize this
        wostringstream message;
        
        message << L"Error in Expression:\n  "
                << expression << L"\n"
                << L"\n"
                << L"Description:\n  "
                << e.what();
        
        // TODO : fix
        //Error(LOCALIZE_THIS, message.str().c_str());
        return false;
    }
    
    return true;
}



bool MathEvaluateString(const SettingsMap& context, const wstring& expression,
    wstring& result, const StringSet& recursiveVarSet, unsigned int flags)
{
    try
    {
        MathParser mathParser(context, expression, recursiveVarSet, flags);
        
        if (MATH_VALUE_TO_COMPATIBLE_STRING & flags)
        {
            result = mathParser.Evaluate().ToCompatibleString();
        }
        else
        {
            result = mathParser.Evaluate().ToString();
        }
    }
    catch (const MathException& e)
    {
        // FIXME: Localize this
        wostringstream message;
        
        message << L"Error in Expression:\n  "
                << expression << L"\n"
                << L"\n"
                << L"Description:\n  "
                << e.what();
        
        // TODO : fix
        //Error(LOCALIZE_THIS, message.str().c_str());
        return false;
    }
    
    return true;
}
