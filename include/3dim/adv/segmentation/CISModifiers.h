///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CISModifiers_h_included
#define CISModifiers_h_included

#include <segmentation/CISTests.h>
#include <vector>

namespace seg{

    
template
< 
    typename tpStepper 
>
class CSetModifier
{
public:
    //! Volume stepper type
    typedef tpStepper tStepper;

    //! Voxel type
    typedef typename tpStepper::tVoxel tVoxel;

public:
    //! Volume stepper
    tStepper * m_volume;

    //! Set value
    bool set()
    {
        m_volume->set( m_value );
        return true;
    }

    //! Value to set
    tVoxel m_value;

}; // class CSetModifier


template
< 
    typename tpStepper,
    typename tpTest
>
class CTestSetModifier
{
public:
    //! Volume stepper type
    typedef tpStepper tStepper;

    //! Voxel type
    typedef typename tpStepper::tVoxel tVoxel;

    //! Test type
    typedef tpTest tTest;
public:
    //! Volume stepper
    tStepper * m_volume;

    //! Set value
    bool set()
    {
        if( m_test() )
        {
            *m_volume->m_pen = m_value;
            return true;
        }
        return false;
    }

    //! Value to set
    tVoxel m_value;

    //! Used test
    tTest m_test;

}; // class CSetModifier

template
<
    typename tpStepper
>
class CMultiTestSetModifier
{
public:
    //! Volume type
    typedef tpStepper tStepper;

    //! Voxel type
    typedef typename tpStepper::tVoxel tVoxel;

    //! Maximal number of tests
    enum{
        MAX_TESTS = 10
    };

    //! Tests vector type
    typedef seg::CTest * tVecTests[ MAX_TESTS ];
public:
    //! Constructor
    CMultiTestSetModifier() : m_volume( NULL ), m_testsCount( 0 ) {}

    //! Volume stepper
    tStepper * m_volume;

    //! Set value
    bool set()
    {
        if (!m_volume->isIn())
            return false;
        if( doTests() )
        {
            m_volume->set( m_value );
            return true;
        }
        return false;
    }

    //! Add test
    bool addTest( CTest * test )
    {
        if( test == NULL || m_testsCount == MAX_TESTS )
            return false;

        m_tests[ m_testsCount ] = test;
        ++m_testsCount;

        return true;
    }

    //! Clear tests
    void clearTests() { m_testsCount = 0; }

public:
    //! Writed value
    tVoxel m_value;

    //! Do testing
    bool doTests() 
    {
        for( int i = 0; i < m_testsCount; ++i )
        {
            if( ! m_tests[i]->operator ()( ) )
                return false;
        }
        return true;
    }

protected:
    //! Current test number
    int m_testsCount;

    //! Tests vector
    tVecTests m_tests;

}; // class CMultiTestSetModifier

} // namespace seg

// CISModifiers_h_included
#endif

