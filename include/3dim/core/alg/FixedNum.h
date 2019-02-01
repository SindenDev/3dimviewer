//==============================================================================
/* This file is part of
 * 
 * VPL - Voxel Processing Library
 * Changes are Copyright 2018 TESCAN 3DIM, s.r.o.
 * All rights reserved.
 * 
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 * 
 * Description:
 * - Fixed point arithmetic.
 */

#pragma once
#ifndef VPL_FixedNum_H
#define VPL_FixedNum_H

#include <VPL/Base/TypeTraits.h>

#include <cmath>
#include <cstddef>

namespace vpl
{
namespace math
{

//==============================================================================
/*!
 * Class representing a number in fixed point arithmetic.
 */
template <typename IntegralType, std::size_t numOfFracBits = 16>
class FixedNum
{
public:
    //! Type of the number.
    typedef IntegralType tValue;
    
    //! Total number of bits used to represent the fixed point number
    static const std::size_t totalBits = sizeof(tValue);
    
    //! Number of bits used to represent the fractional parts - the precision.
    static const std::size_t fracBits = numOfFracBits;
    
    //! One in fixed point format
    static const tValue one = tValue(1) << fracBits;
    
public:
    //! Value stored using the fixed point arithmetic.
    tValue value;
    
public:
    //! Default constructor does not initializes the value!
    FixedNum() {}

    //! Constructor creates a fixed point number from a float.
    FixedNum(float v) { value = tValue(v * static_cast<float>(one) + 0.5f); }

    //! Constructor creates a fixed point number from a double.
    FixedNum(double v) { value = tValue(v * static_cast<double>(one) + 0.5); }

    //! Copy constructor.
    FixedNum(const FixedNum& n) : value(n.value) {}

    //! Empty destructor.
    ~FixedNum() {}


    //! Assignment operator.
    FixedNum& operator =(const FixedNum& n)
    {
        value = n.value;
        return *this;
    }

    //! Converts a float to fixed point.
    FixedNum& operator =(float v)
    {
        value = FixedNum(v);
        return *this;
    }

    //! Converts a double to fixed point.
    FixedNum& operator =(double v)
    {
        value = FixedNum(v);
        return *this;
    }


    //! Converts the fixed point value back to float
    float toFloat() const
    {
        static const float invOne = 1.0f / static_cast<float>(one);
        return static_cast<float>(value) * invOne;
    }
    
    //! Converts the fixed point value back to double
    double toDouble() const
    {
        static const double invOne = 1.0 / static_cast<double>(one);
        return static_cast<double>(value) * invOne;
    }
    
    //! This cast operator converts the fixed point value back to float
    operator float() const { return toFloat(); }
    
    //! This cast operator converts the fixed point value back to double
    operator double() const { return toDouble(); }
    
    //! Returns the underlying fixed point value
    tValue raw() const { return value; }


    // Comparison operators
    inline bool operator ==(const FixedNum &rhs) const { return value == rhs.value; }
    inline bool operator !=(const FixedNum &rhs) const { return value != rhs.value; }
    inline bool operator <(const FixedNum &rhs) const { return value < rhs.value; }
    inline bool operator >(const FixedNum &rhs) const { return value > rhs.value; }
    inline bool operator <=(const FixedNum &rhs) const { return value <= rhs.value; }
    inline bool operator >=(const FixedNum &rhs) const { return value >= rhs.value; }

    // Unary operators
    inline FixedNum &operator ++() { value += one; return *this; }
    inline FixedNum &operator --() { value += one; return *this; }

    // Combined asignment operators
    inline FixedNum &operator +=(const FixedNum &rhs) { value += rhs.value; return *this; }
    inline FixedNum &operator -=(const FixedNum &rhs) { value -= rhs.value; return *this; }
    
    inline FixedNum &operator *=(const FixedNum &rhs)
    {
        value *= rhs.value;
        value >>= fracBits;
        return *this;
    }
    
    inline FixedNum &operator /=(const FixedNum &rhs)
    {
        value <<= fracBits;
        value /= rhs.value;
        return *this;
    }
    
    // Binary operators
    inline FixedNum operator +(const FixedNum &rhs) const { FixedNum aux(*this); aux += rhs; return aux; }
    inline FixedNum operator -(const FixedNum &rhs) const { FixedNum aux(*this); aux -= rhs; return aux; }
    inline FixedNum operator *(const FixedNum &rhs) const { FixedNum aux(*this); aux *= rhs; return aux; }
    inline FixedNum operator /(const FixedNum &rhs) const { FixedNum aux(*this); aux /= rhs; return aux; }
};


//=============================================================================
/*
 * Basic template instances and type definitions.
 */

//! Default type used to represent FP numbers in fixed point arithmetic
typedef FixedNum<vpl::sys::tInt64, 16> tFixedNum;


} // namespace math
} // namespace vpl

#endif // VPL_FixedNum_H
