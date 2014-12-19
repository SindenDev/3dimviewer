///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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


//////////////////////////////////////////////////////////////////////////
//

template <class T>
CAnyModelVisualizer<T>::CAnyModelVisualizer(int ModelId) 
	: COnOffNode(), m_ModelId(ModelId), m_bManualUpdate(false), m_bUseKDTree(true)
{
    // Create a new matrix transform
    m_pTransform = new osg::MatrixTransform;
    this->addChild(m_pTransform.get());

    // Create a new surface mesh
    m_pMesh = new CTriMesh;
    m_pTransform->addChild(m_pMesh.get());

    // Retrieve the current mesh from the storage
    data::CObjectPtr< tModel > spModel( APP_STORAGE.getEntry(m_ModelId) );
//	m_pMesh->createMesh( spModel->getMesh() );
    m_pMesh->createMesh( spModel->getMesh(), true, true );

    // Get transformation matrix from the storage
    m_pTransform->setMatrix(spModel->getTransformationMatrix());

    // Setup OSG state set
    ModelVisualizer::setupModelStateSet( m_pMesh.get() );

    // Set the update callback
    APP_STORAGE.connect( m_ModelId, this );
    this->setupObserver( this );
}


//////////////////////////////////////////////////////////////////////////
//

template <class T>
void CAnyModelVisualizer<T>::updateFromStorage()
{
	if(m_bManualUpdate)
		return;

    data::CObjectPtr< tModel > spModel( APP_STORAGE.getEntry(m_ModelId) );

    // Update the triangular mesh
    data::CChangedEntries Changes( this );
    if( !Changes.checkFlagAll(data::CModel::MESH_NOT_CHANGED) )
    {
//        m_pMesh->createMesh( spModel->getMesh() );
        m_pMesh->createMesh( spModel->getMesh(), true, true );

        // Setup OSG state set
        ModelVisualizer::setupModelStateSet( m_pMesh.get() );
    }

	if(m_bUseKDTree)
		buildKDTree();

    // Update model matrix
    m_pTransform->setMatrix(spModel->getTransformationMatrix());

    // Get model color
    float r, g, b, a;
    spModel->getColor(r, g, b, a);

    // Set model color
    m_pMesh->setColor( r, g, b, a );

    // Set visibility
    setOnOffState( spModel->isShown() );    
}

template <class T>
void osg::CAnyModelVisualizer<T>::updatePartOfMesh( const CTriMesh::tIdPosVec &handles )
{
	if(!m_bManualUpdate)
		return;

	// Get model
	data::CObjectPtr< tModel > spModel( APP_STORAGE.getEntry(m_ModelId) );
	m_pMesh->updatePartOfMesh(spModel->getMesh(), handles, true, true);

	if(m_bUseKDTree)
		buildKDTree();
}

template <class T>
void osg::CAnyModelVisualizer<T>::showWireframe( bool bShow )
{
	if(m_pMesh.get() == 0)
		return;

	osg::StateSet *state = m_pMesh->getOrCreateStateSet();

	//   We need to retireve the Poylgon mode of the  state set, and create one if does not have one
	osg::PolygonMode *polyModeObj;

	polyModeObj = dynamic_cast< osg::PolygonMode* >
		( state->getAttribute( osg::StateAttribute::POLYGONMODE ));

	if ( !polyModeObj ) {

		polyModeObj = new osg::PolygonMode;

		state->setAttribute( polyModeObj );    
	}

	//  Now we can set or unset the state to WIREFRAME

	if(bShow)
		polyModeObj->setMode(  osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );
	else
		polyModeObj->setMode(  osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL );
}


template <class T>
void osg::CAnyModelVisualizer<T>::buildKDTree()
{
	m_pMesh->buildKDTree();
}
