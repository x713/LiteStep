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
#if !defined(MATHEVALUATE_H)
#define MATHEVALUATE_H

#include <string>
#include "SettingsDefines.h"


/**
 * @file MathEvaluate.h
 * @brief Provides functions for evaluating mathematical expressions.
 *
 * This file contains functions for evaluating mathematical expressions
 * and converting the results to various types.
 */

/**
 * @brief Flags for controlling the evaluation of math expressions.
 */
enum
{
    /** Throws an exception if an undefined variable is referenced. */
     MATH_EXCEPTION_ON_UNDEFINED      = (1 << 0)
    /** Evaluates all numbers as integers when converting to a String. */
    ,MATH_VALUE_TO_COMPATIBLE_STRING  = (1 << 2)
};


/**
 * @brief Evaluates a math expression and converts the result to a Boolean.
 *
 * This function parses and evaluates a mathematical expression and returns
 * the result as a boolean value.
 *
 * @param context The context containing variable bindings for the expression.
 * @param expression The mathematical expression to evaluate.
 * @param result A reference to a boolean variable that will hold the result.
 * @param flags Flags that control the parsing and evaluation process.
 * @return `true` if the expression was evaluated successfully, `false` otherwise.
 */
bool MathEvaluateBool(const SettingsMap& context,
    const std::wstring& expression,
    bool& result,
    unsigned int flags = 0);

/**
 * @brief Evaluates a math expression and converts the result to a string.
 *
 * This function parses and evaluates a mathematical expression and returns
 * the result as a string.
 *
 * @param context The context containing variable bindings for the expression.
 * @param expression The mathematical expression to evaluate.
 * @param result A reference to a boolean variable that will hold the result.
 * @param flags Flags that control the parsing and evaluation process.
 * @return `true` if the expression was evaluated successfully, `false` otherwise.
 */
bool MathEvaluateBool(const SettingsMap& context,
    const std::wstring& expression,
    bool& result,
    unsigned int flags = 0);

/**
 * Evaluates a math expression and converts the result to a string.
 *
 * @param  context         map with variable bindings
 * @param  expression      string with expression to evaluate
 * @param  result          variable to hold expression result
 * @param  recursiveVarSet set of variables to check for recursive definitions
 * @param  flags           flags that control parsing and evaluation
 * @return <code>true</code>  if successful or
 *         <code>false</code> if an error occured
 */
bool MathEvaluateString(const SettingsMap& context,
    const std::wstring& expression,
    std::wstring& result,
    const StringSet& recursiveVarSet,
    unsigned int flags = 0);


#endif // MATHEVALUATE_H
