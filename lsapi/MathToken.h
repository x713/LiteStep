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
#if !defined(MATHTOKEN_H)
#define MATHTOKEN_H

#include <string>


/**
 * @file MathToken.h
 * @brief Provides a token class for the math parser.
 */

/**
 * @brief Token types for the math parser.
 */
enum
{
    TT_INVALID,
    TT_ID,
    TT_FALSE,
    TT_TRUE,
    TT_NUMBER,
    TT_INFINITY,
    TT_NAN,
    TT_STRING,
    TT_LPAREN,
    TT_RPAREN,
    TT_DEFINED,
    TT_COMMA,
    TT_PLUS,
    TT_MINUS,
    TT_STAR,
    TT_SLASH,
    TT_DIV,
    TT_MOD,
    TT_AMPERSAND,
    TT_AND,
    TT_OR,
    TT_NOT,
    TT_EQUAL,
    TT_GREATER,
    TT_GREATEREQ,
    TT_LESS,
    TT_LESSEQ,
    TT_NOTEQUAL,
    TT_END
};


/**
 * @brief Represents a token in a math expression.
 *
 * A token has a type and a lexical value.
 */
class MathToken
{
public:

    /**
     * @brief Constructs a token with type <code>TT_INVALID</code>.
     */
    MathToken();

    /**
     * @brief Constructs a token with the specified type.
     * @param type The type of the token.
     */
    MathToken(int type);

    /**
     * @brief Constructs a token with the specified type and lexical value.
     * @param type The type of the token.
     * @param value The lexical value of the token.
     */
    MathToken(int type, const std::wstring& value);

    /**
     * @brief Returns the type of this token.
     * @return The type of the token.
     */
    int GetType() const { return mType; }

    /**
     * @brief Returns a string description of this token's type.
     * @return A string description of the token's type.
     */
    std::wstring GetTypeName() const;

    /**
     * @brief Sets the type of this token.
     * @param type The new type of the token.
     */
    void SetType(int type);

    /**
     * @brief Returns the lexical value of this token.
     * @return The lexical value of the token.
     */
    std::wstring GetValue() const { return mValue; }

    /**
     * @brief Sets the lexical value of this token.
     * @param value The new lexical value of the token.
     */
    void SetValue(const std::wstring& value);

private:

    /** Token type */
    int mType;

    /** Lexical value */
    std::wstring mValue;
};


#endif
