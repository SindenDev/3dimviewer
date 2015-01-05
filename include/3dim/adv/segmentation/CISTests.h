#ifndef CISTests_H_included
#define CISTests_H_included

#include <segmentation/CISVolumeStepper.h>

namespace seg
{
class CTest
{

public:
    //! Virtual destructor
    virtual ~CTest() {}

    //! Test data
    virtual bool operator()( ) = 0;

}; // class CTest 

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class  CFillTest
///
/// \brief  Fill test. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
template
< 
    typename tpStepper
>
class CFillTest : public CTest
{
public:
    //! Volume stepper type
    typedef tpStepper tStepper;

    //! Voxel type
    typedef typename tpStepper::tVoxel tVoxel;

    //! Used stepper
    tStepper * m_volume;

public:
    //! Test data
    virtual bool operator()( )
    {
        return m_volume->isIn();
    }
}; // class CFillTest

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class  CNotDoneTest
///
/// \brief  Not done test. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
template
< 
    typename tpStepper
>
class CNotDoneTest : public CTest
{
public:
    //! Volume stepper type
    typedef tpStepper tStepper;

    //! Voxel type
    typedef typename tpStepper::tVoxel tVoxel;

    //! Used stepper
    tStepper * m_volume;

public:
    //! Test data
    virtual bool operator()( )
    {
        return m_volume->get() != m_color;
    }

    //! Mask
    tVoxel m_color;

}; // class CNotDoneTest

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class  CMaskTest
///
/// \brief  Mask test. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
template
< 
    typename tpStepper
>
class CMaskTest : public CTest
{

public:
    //! Volume stepper type
    typedef tpStepper tStepper;

    //! Voxel type
    typedef typename tpStepper::tVoxel tVoxel;

    //! Used stepper
    tStepper * m_volume;

public:
    //! Test data
    virtual bool operator()( )
    {
        return m_volume->get() == m_mask;
    }

    //! Mask
    tVoxel m_mask;

}; // class CMaskTest 


////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class  CMaskTest
///
/// \brief  Mask and zero test. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
template
< 
    typename tpStepper
>
class CMaskZeroTest : public CTest
{

public:
    //! Volume stepper type
    typedef tpStepper tStepper;

    //! Voxel type
    typedef typename tpStepper::tVoxel tVoxel;

    //! Used stepper
    tStepper * m_volume;

public:
    //! Test data
    virtual bool operator()( )
    {
        tVoxel v( m_volume->get() );
        return !(v != m_mask && v != 0);
    }

    //! Mask
    tVoxel m_mask;

}; // class CMaskTest 
////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class  CRangeTest
///
/// \brief  Range test. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
template
< 
    typename tpStepper
>
class CRangeTest : public CTest
{
public:
    //! Volume stepper type
    typedef tpStepper tStepper;

    //! Voxel type
    typedef typename tpStepper::tVoxel tVoxel;

    //! Used stepper
    tStepper * m_volume;

public:
    //! Test data
    virtual bool operator() ( )
    {
        tVoxel & voxel( m_volume->get() );

        return voxel <= m_max && voxel >= m_min;
    }

    //! Range 
    tVoxel m_min, m_max;
}; // class CRangeTest

} // namespace seg

// CISTests_H_included
#endif

