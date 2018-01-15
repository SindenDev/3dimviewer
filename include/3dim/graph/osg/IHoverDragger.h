#pragma once

namespace osgManipulator
{
    ////////////////////////////////////////////////////////////
    //! Dragger interface used mostly for highlighting actual dragger
    //
    class IHoverDragger
    {
    public:
        virtual void onMouseEnter() = 0;
        virtual void onMouseLeave() = 0;
    };
}