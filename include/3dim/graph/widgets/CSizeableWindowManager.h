///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SizeableWindowManager_H_included
#define SizeableWindowManager_H_included

#include <osgWidget/WindowManager>
#include <osg/NodeCallback>
#include <VPL/Module/Signal.h>

namespace scene
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief Cvm size changed callback. 

class CVMSizeChangedCallback : public osg::NodeCallback
{
public:
    //! Constructor
    CVMSizeChangedCallback(  ){}

    //! Update operator
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
};


////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief Manager for sizeable windows. 

class CSizeableWindowManager : public osgWidget::WindowManager
{
public:
    //! Size changed signal
    typedef vpl::mod::CSignal< void, int, int > tSigSizeChanged;

public:
    //! Constructor
    CSizeableWindowManager( osgViewer::Viewer * View, int initial_width = 0, int initial_height = 0);

    //! Show/hide widgets
    void showWidgets( bool bShow );

    //! Get size changed signal
    tSigSizeChanged & getSigSizeChanged() { return m_sigSizeChanged; }

    //! Called when size changed
    virtual void onSizeChanged();

    //! Returns true if the size has benn changed
    bool sizeChanged( int w, int h ) { return w != m_width || h != m_height; }

    //! Restore all windows positions acording to their anchoring
    void repositAllWindows();

protected:
    //! Callback object
    osg::ref_ptr< CVMSizeChangedCallback > m_size_callback;

    //! Viewer 
    osg::ref_ptr< osgViewer::Viewer > m_view;

    //! Size changed signal
    tSigSizeChanged m_sigSizeChanged;

    //! Update callback is my friend
    friend class CVMSizeChangedCallback;

    //! Window sizes
    int m_width, m_height;
};


} // namespace scene

#endif // SizeableWindowManager_H_included
