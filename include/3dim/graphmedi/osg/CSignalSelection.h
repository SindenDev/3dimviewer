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

#ifndef CSignalSelection_H
#define CSignalSelection_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Module/Signal.h>

#include <osg/Matrix>
#include <osgManipulator/Selection>
#include <osgManipulator/Command>


namespace osgManipulator
{

///////////////////////////////////////////////////////////////////////////////
//! class description
//
template <class tpSelectionType, class tpParameter>
class CSignalMatrixSelection : public tpSelectionType
{

public:
	//! Invoking selection matrix signal
	typedef vpl::mod::CSignal<void, const osg::Matrix &, const osgManipulator::MotionCommand &, tpParameter> tMatrixSignal;

	virtual bool receive (const MotionCommand & command)
	{
		bool bResult = tpSelectionType::receive(command);
		if(m_invokeSignal)
			m_signal.invoke(tpSelectionType::getMatrix(), command, m_parameter);

		return bResult;
	}
		
	virtual bool receive (const TranslateInLineCommand &command)
	{
		return receive((MotionCommand &) command);
	}
	
	virtual bool receive (const TranslateInPlaneCommand &command)
	{
		return receive((MotionCommand &) command);
	}
		
	virtual bool receive (const Scale1DCommand &command)
	{
		return receive((MotionCommand &) command);
	}
		
	virtual bool receive (const Scale2DCommand &command)
	{
		return receive((MotionCommand &) command);
	}

	virtual bool receive (const ScaleUniformCommand &command)
	{
		return receive((MotionCommand &) command);
	}
	
	virtual bool receive (const Rotate3DCommand &command)
	{
		return receive((MotionCommand &) command);
	}

	tMatrixSignal m_signal;

	tpParameter m_parameter;

	bool m_invokeSignal;

}; // template CSignalMatrixSelection


///////////////////////////////////////////////////////////////////////////////
//

template <class tpSelectionType, class tpParameter>
class CSignalCommandSelection : public tpSelectionType
{
public:
	//! Invoking movement command signal
	typedef vpl::mod::CSignal<void, const osgManipulator::MotionCommand &, tpParameter> tCommandSignal;

	virtual bool receive (const MotionCommand & command)
	{
		//------- TESTING

		if(command.getStage() == MotionCommand::MOVE){
			// Get the LocalToWorld and WorldToLocal matrix for this node.
			osg::NodePath nodePathToRoot;
			computeNodePathToRoot(*this,nodePathToRoot);
			this->_worldToLocal = osg::Matrix::inverse(this->_localToWorld);

			osg::Matrix csLW, csGlobal, csLocal, curLW, curGlobal, curLocal;
			csLW = command.getLocalToWorld();
			csGlobal = command.getMotionMatrix() * csLW;
			csLocal = command.getMotionMatrix();

			curLW = osg::computeLocalToWorld(nodePathToRoot);
			curGlobal = command.getMotionMatrix() * curLW;

			osg::Matrix e = csLW*command.getWorldToLocal();
			e = curLW * this->_worldToLocal;
			// Transform the command's motion matrix into local motion matrix.
			osg::Matrix localMotionMatrix = curLW * command.getWorldToLocal()
			* command.getMotionMatrix()
			* command.getLocalToWorld() * this->_worldToLocal;
		}
		//------- TESTING

		bool bResult = tpSelectionType::receive(command);
		if(m_invokeSignal)
			m_signal.invoke(command, m_parameter);

		return bResult;
	}

	virtual bool receive (const TranslateInLineCommand &command)
	{
		return receive((MotionCommand &) command);
	}

	virtual bool receive (const TranslateInPlaneCommand &command)
	{
		return receive((MotionCommand &) command);
	}

	virtual bool receive (const Scale1DCommand &command)
	{
		return receive((MotionCommand &) command);
	}

	virtual bool receive (const Scale2DCommand &command)
	{
		return receive((MotionCommand &) command);
	}

	virtual bool receive (const ScaleUniformCommand &command)
	{
		return receive((MotionCommand &) command);
	}

	virtual bool receive (const Rotate3DCommand &command)
	{
		return receive((MotionCommand &) command);
	}

	tCommandSignal m_signal;

	tpParameter m_parameter;

	bool m_invokeSignal;
};


} // namespace osgManipulator

#endif // CSignalSelection_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
