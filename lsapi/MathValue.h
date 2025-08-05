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
#if !defined(MATHVALUE_H)
#define MATHVALUE_H

#include <string>
#include <vector>


/**
 * @file MathValue.h
 * @brief Provides a class for representing values in the math parser.
 */

/**
 * @brief Represents the result of evaluating a math expression.
 *
 * A MathValue can be one of four types: UNDEFINED, BOOLEAN, NUMBER, or STRING.
 */
class MathValue
{
public:

    /**
     * @brief The types of values that a MathValue can hold.
     */
    enum
    {
        UNDEFINED,
        BOOLEAN,
        NUMBER,
        STRING
    };

public:

    /**
     * @brief Constructs an undefined value.
     */
    MathValue();

    /**
     * @brief Constructs a Boolean value.
     * @param value The boolean value.
     */
    MathValue(bool value);

    /**
     * @brief Constructs a numeric value from an integer.
     * @param value The integer value.
     */
    MathValue(int value);

    /**
     * @brief Constructs a numeric value from a floating point number.
     * @param value The floating point value.
     */
    MathValue(double value);

    /**
     * @brief Constructs a string value.
     * @param value The string value.
     */
    MathValue(const std::wstring& value);

    /**
     * @brief Constructs a string value.
     * @param value The string value.
     */
    MathValue(const wchar_t *value);

    /**
     * @brief Assigns a Boolean to this value.
     * @param value The boolean value to assign.
     * @return A reference to this MathValue.
     */
    MathValue& operator=(bool value);

    /**
     * @brief Assigns an integer to this value.
     * @param value The integer value to assign.
     * @return A reference to this MathValue.
     */
    MathValue& operator=(int value);

    /**
     * @brief Assigns a floating point number to this value.
     * @param value The floating point value to assign.
     * @return A reference to this MathValue.
     */
    MathValue& operator=(double value);

    /**
     * @brief Assigns a string to this value.
     * @param value The string value to assign.
     * @return A reference to this MathValue.
     */
    MathValue& operator=(const std::wstring& value);

    /**
     * @brief Assigns a string to this value.
     * @param value The string value to assign.
     * @return A reference to this MathValue.
     */
    MathValue& operator=(const wchar_t *value);

    /**
     * @brief Returns a string description of this value's type.
     * @return A string description of the value's type.
     */
    std::wstring GetTypeName() const;

    /**
     * @brief Returns <code>true</code> if this value is undefined.
     * @return <code>true</code> if the value is undefined, <code>false</code> otherwise.
     */
    bool IsUndefined() const { return (mType == UNDEFINED); }

    /**
     * @brief Returns <code>true</code> if this value is a Boolean.
     * @return <code>true</code> if the value is a Boolean, <code>false</code> otherwise.
     */
    bool IsBoolean() const { return (mType == BOOLEAN); }

    /**
     * @brief Returns <code>true</code> if this value is a number.
     * @return <code>true</code> if the value is a number, <code>false</code> otherwise.
     */
    bool IsNumber() const { return (mType == NUMBER); }

    /**
     * @brief Returns <code>true</code> if this value is a string.
     * @return <code>true</code> if the value is a string, <code>false</code> otherwise.
     */
    bool IsString() const { return (mType == STRING); }

    /**
     * @brief Converts this value to a Boolean.
     * @return The value as a Boolean.
     */
    bool ToBoolean() const;

    /**
     * @brief Converts this value to an integer.
     * @return The value as an integer.
     */
    int ToInteger() const;

    /**
     * @brief Converts this value to a number.
     * @return The value as a number.
     */
    double ToNumber() const;

    /**
     * @brief Converts this value to a string.
     * @return The value as a string.
     */
    std::wstring ToString() const;

    /**
     * @brief Converts this value to a string using integer representation
     * for any NUMBER type.
     * @return The value as a string.
     */
    std::wstring ToCompatibleString() const;

    /** Operators */
    friend MathValue operator+ (const MathValue& a, const MathValue& b);
    friend MathValue operator+ (const MathValue& a);
    friend MathValue operator- (const MathValue& a, const MathValue& b);
    friend MathValue operator- (const MathValue& a);
    friend MathValue operator* (const MathValue& a, const MathValue& b);
    friend MathValue operator/ (const MathValue& a, const MathValue& b);
    friend MathValue operator% (const MathValue& a, const MathValue& b);
    friend MathValue operator&&(const MathValue& a, const MathValue& b);
    friend MathValue operator||(const MathValue& a, const MathValue& b);
    friend MathValue operator! (const MathValue& a);
    friend MathValue operator==(const MathValue& a, const MathValue& b);
    friend MathValue operator!=(const MathValue& a, const MathValue& b);
    friend MathValue operator< (const MathValue& a, const MathValue& b);
    friend MathValue operator<=(const MathValue& a, const MathValue& b);
    friend MathValue operator> (const MathValue& a, const MathValue& b);
    friend MathValue operator>=(const MathValue& a, const MathValue& b);

private:

    /** Type */
    int mType;

    /** Boolean value */
    bool mBoolean;

    /** Numeric value */
    double mNumber;

    /** String value */
    std::wstring mString;
};


/** Convert values to strings and concatenate them. */
MathValue MathConcatenate(const MathValue& a, const MathValue& b);

/** Convert values to integers and divide them. */
MathValue MathIntDivide(const MathValue& a, const MathValue& b);

/** Convert a number to a string. */
std::wstring MathNumberToString(double number);

/** Convert a string to a number. */
double MathStringToNumber(const std::wstring& wstr);


#endif
