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

#ifndef CSignalDraggerCallback_H_included
#define CSignalDraggerCallback_H_included

#include <osgManipulator/Dragger>
#include <VPL/Module/Signal.h>
//#include <osg/dbout.h>

namespace osgManipulator
{
////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Signal dragger callback. 
////////////////////////////////////////////////////////////////////////////////////////////////////
template
<
typename tpParameter
>
class CSignalDraggerCallback : public DraggerTransformCallback
{

public:
	//! Invoking selection matrix signal
	typedef vpl::mod::CSignal<bool, const MotionCommand &, int, tpParameter> tCommandSignal;

	//! Invoking new matrix signal
	typedef vpl::mod::CSignal< bool, const osg::Matrix &, const MotionCommand &, int, tpParameter > tMatrixSignal;

public:
	//! Constructor
	CSignalDraggerCallback( osg::MatrixTransform * transform, int handleCommandMask = HANDLE_ALL ) 
        : DraggerTransformCallback(transform, handleCommandMask )
		, m_invoke( true )
        , m_scaleMT(transform)
        , m_transMT(transform)
        , m_rotMT(transform)
        , m_finishSignalEnabled(false)
	{}

    //! Constructor used to apply different commands on different transforms
    CSignalDraggerCallback( osg::MatrixTransform * traslate_mt, osg::MatrixTransform * rotate_mt, osg::MatrixTransform * scale_mt, int handleCommandMask = HANDLE_ALL)
        : DraggerTransformCallback(traslate_mt, handleCommandMask )
        , m_invoke( true )
        , m_finishSignalEnabled(false)
        , m_scaleMT(scale_mt)
        , m_transMT(traslate_mt)
        , m_rotMT(rotate_mt)
    {}

	/**
    * Receive motion commands. Returns true on success.
    */
    virtual bool receive(const MotionCommand&command)
    {
        m_commandType = HANDLE_ALL;
        _transform = m_transMT; 
        return receiveInternal(command); 
    }
    virtual bool receive(const TranslateInLineCommand& command)
    {
        if ((_handleCommandMask&HANDLE_TRANSLATE_IN_LINE)!=0) 
        {
            m_commandType = HANDLE_TRANSLATE_IN_LINE;
            _transform = m_transMT;
            return receiveInternal(static_cast<const MotionCommand&>(command));
        }
        return false;
    }
    virtual bool receive(const TranslateInPlaneCommand& command)
    {
        if ((_handleCommandMask&HANDLE_TRANSLATE_IN_PLANE)!=0) 
        {
            m_commandType = HANDLE_TRANSLATE_IN_PLANE;
            _transform = m_transMT;
            return receiveInternal(static_cast<const MotionCommand&>(command));
        }
        return false;
    }
    virtual bool receive(const Scale1DCommand& command)
    {
        if ((_handleCommandMask&HANDLE_SCALED_1D)!=0) 
        {
            m_commandType = HANDLE_SCALED_1D;
            _transform = m_scaleMT;
            return receiveInternal(static_cast<const MotionCommand&>(command));
        }
        return false;
    }
    virtual bool receive(const Scale2DCommand& command)
    {
        if ((_handleCommandMask&HANDLE_SCALED_2D)!=0) 
        {
            m_commandType = HANDLE_SCALED_2D;
            _transform = m_scaleMT;
            return receiveInternal(static_cast<const MotionCommand&>(command));
        }
        return false;
    }
    virtual bool receive(const ScaleUniformCommand& command)
    {
        if ((_handleCommandMask&HANDLE_SCALED_UNIFORM)!=0) 
        {
            m_commandType = HANDLE_SCALED_UNIFORM;
            _transform = m_scaleMT;
            return receiveInternal(static_cast<const MotionCommand&>(command));
        }
        return false;
    }
    virtual bool receive(const Rotate3DCommand& command)
    {
        if ((_handleCommandMask&HANDLE_ROTATE_3D)!=0) 
        {
            m_commandType = HANDLE_ROTATE_3D;
            _transform = m_rotMT;
            return receiveInternal(static_cast<const MotionCommand&>(command));
        }
        return false;
    }

	//! Get command signal
	tCommandSignal & getCommandSignal() { return m_commandSignal; }

	//! Get matrix signal
	tMatrixSignal & getMatrixSignal() { return m_matrixSignal; }

	//! Get/set parameter
	tpParameter & parameter() { return m_parameter; }

	//! Invoke signal?
	void shouldInvoke( bool invoke ) { m_invoke = invoke; }

	//! Get flag if FINISH command signalling is signalled
	void isFinishSignalEnabled() { return m_finishSignalEnabled; }

    //! Set flag if FINISH command signalling is signalled
    void setFinishSignalEnabled(bool value) { m_finishSignalEnabled = value; }

protected:
    virtual bool receiveInternal(const MotionCommand&);

protected:
	//! Invoked signal	
	tMatrixSignal m_matrixSignal;

	//! Invoked command signal
	tCommandSignal m_commandSignal;

	//! Used parameter
	tpParameter m_parameter;

	//! Use signal?
	bool m_invoke;

    //! Flag ig FINISH command signalling is enabled
    bool m_finishSignalEnabled;

    //! Handling trasforms for different motions
    osg::observer_ptr<osg::MatrixTransform> m_scaleMT, m_transMT, m_rotMT;

    //! Command type
    int m_commandType;

};

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Use command. 
//!
//!\typeparam	tpParameter	. 
//!\param	command	The command. 
//!
//!\return	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
template
	<
	typename tpParameter
	>
	bool osgManipulator::CSignalDraggerCallback<tpParameter>::receiveInternal( const MotionCommand& command )
	{ 
		bool rv( false );
 //       DBOUT(m_commandType);
		if( m_invoke )
		{
			// Command signal is invoked before matrix transformation
			rv = m_commandSignal.invoke2( command, m_commandType, m_parameter );

			if( !rv )
			{
				if (!_transform) return false;

				// Do transformation, but just use only wanted part of the motion matrix
				switch (command.getStage())
				{
				case MotionCommand::START:
					{
						// Save the current matrix
						_startMotionMatrix = _transform->getMatrix();

						// Get the LocalToWorld and WorldToLocal matrix for this node.
						osg::NodePath nodePathToRoot;
						computeNodePathToRoot(*_transform,nodePathToRoot);
						_localToWorld = osg::computeLocalToWorld(nodePathToRoot);
						_worldToLocal = osg::Matrix::inverse(_localToWorld);

                        // Matrix signal is invoked after the matrix transformation
                        m_matrixSignal.invoke2( getTransform()->getMatrix(), command, m_commandType, m_parameter );

						return true;
					}

				case MotionCommand::MOVE:
					{
                        // Transform the command's motion matrix into local motion matrix.
                        osg::Matrix localMotionMatrix = _localToWorld * command.getWorldToLocal()
                            * command.getMotionMatrix()
                            * command.getLocalToWorld() * _worldToLocal;

						// Transform by the localMotionMatrix
						_transform->setMatrix(localMotionMatrix * _startMotionMatrix);

						// Matrix signal is invoked after the matrix transformation
						m_matrixSignal.invoke2( getTransform()->getMatrix(), command, m_commandType, m_parameter );
						
						return true;
					}

				case MotionCommand::FINISH:
					{
                        if (m_finishSignalEnabled)
                        {
						    // Matrix signal is invoked after the matrix transformation
						    m_matrixSignal.invoke2( getTransform()->getMatrix(), command, m_commandType, m_parameter );
                        }
						return true;
					}

				case MotionCommand::NONE:
				default:
					return false;
				}
			}

			// Matrix signal is invoked after the matrix transformation
			//m_matrixSignal.invoke2( getTransform()->getMatrix(), command, m_parameter );
		}

		return rv; 
	}




} // namespace osgGA

// CSignalDraggerCallback_H_included
#endif	