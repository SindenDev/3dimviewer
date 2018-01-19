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

#include <drawing/CLineOptimizer.h>

///////////////////////////////////////////////////////////////////////////////
// Optimize 2D line
void draw::CLineOptimizer::Optimize(const osg::Vec2Array *input, osg::Vec2Array *output)
{
	// Test arguments
	if( input == NULL || output == NULL )
		return;

	osg::ref_ptr< osg::Vec2Array > buffer = new osg::Vec2Array;

	RemoveDuplicities( input, buffer );
	RemoveCollinear( buffer, output );
}

///////////////////////////////////////////////////////////////////////////////
// Optimize 3D line
void draw::CLineOptimizer::Optimize(const osg::Vec3Array *input, osg::Vec3Array *output)
{
	// Test arguments
	if( input == NULL || output == NULL )
		return;

	osg::ref_ptr< osg::Vec3Array > buffer = new osg::Vec3Array;

	RemoveDuplicities( input, output );
	//RemoveCollinear( buffer, output );
}

///////////////////////////////////////////////////////////////////////////////
// Remove duplicities 2D
void draw::CLineOptimizer::RemoveDuplicities(const osg::Vec2Array *input, osg::Vec2Array *output)
{
	osg::Vec2Array::const_iterator itInput, itLast;

	// Nothing to test
	if( input->size() < 2 )
	{
		for( itInput = input->begin(); itInput != input->end(); ++itInput )
			output->push_back( *itInput );

		return;
	}

	// First point
	itLast = input->begin();
	// Store first
	output->push_back( *itLast );

	// For points 1->(N-1)
	for( itInput = input->begin(), ++itInput; itInput != input->end(); ++itInput )
	{
		// If points are not the same
		if( *itLast != *itInput )
			output->push_back( *itInput );

		itLast = itInput;
	}

}

///////////////////////////////////////////////////////////////////////////////
// Remove collinear segments 3D
void draw::CLineOptimizer::RemoveCollinear(const osg::Vec2Array *input, osg::Vec2Array *output)
{
	osg::Vec2Array::const_iterator itInput;

	// Nothing to do
	if( input->size() < 3 )
	{
		for( itInput = input->begin(); itInput != input->end(); ++itInput )
			output->push_back( *itInput );

		return;
	}

	osg::Vec2 first, second, direction1, direction2;

	// Get first point
	itInput = input->begin();
	first = *itInput;

	// Get second point
	++itInput;
	second = *itInput;

	// Compute direction of the first segment
	direction1 = second - first;

	// Store first point
	output->push_back( first );
	
	// For points 2->(N-1)
	for( ++itInput; itInput != input->end(); ++itInput )
	{
		direction2 = *itInput - first;

		if( direction2 != direction1 )
		{
			// Store segment
			output->push_back( second );

			// Start new segment
			first = second;
			direction1 = *itInput - second;
		}

	}
}

///////////////////////////////////////////////////////////////////////////////
// Remove duplicities 3D
void draw::CLineOptimizer::RemoveDuplicities(const osg::Vec3Array *input, osg::Vec3Array *output)
{
	osg::Vec3Array::const_iterator itInput, itLast;

	// Nothing to test
	if( input->size() < 2 )
	{
		for( itInput = input->begin(); itInput != input->end(); ++itInput )
			output->push_back( *itInput );

		return;
	}

	// First point
	itLast = input->begin();
	// Store first
	output->push_back( *itLast );

	// For points 1->(N-1)
	for( itInput = input->begin(), ++itInput; itInput != input->end(); ++itInput )
	{
		// If points are not the same
		if( *itLast != *itInput )
			output->push_back( *itInput );

		itLast = itInput;
	}

}

///////////////////////////////////////////////////////////////////////////////
// Remove collinear segments 3D
void draw::CLineOptimizer::RemoveCollinear(const osg::Vec3Array *input, osg::Vec3Array *output)
{
	osg::Vec3Array::const_iterator itInput;

	// Nothing to do
	if( input->size() < 3 )
	{
		for( itInput = input->begin(); itInput != input->end(); ++itInput )
			output->push_back( *itInput );

		return;
	}

	osg::Vec3 first, second, direction1, direction2;

	// Get first point
	itInput = input->begin();
	first = *itInput;

	// Get second point
	++itInput;
	second = *itInput;

	// Compute direction of the first segment
	direction1 = second - first;
	direction1.normalize();

	// Store first point
	output->push_back( first );
	
	// For points 2->(N-1)
	for( ++itInput; itInput != input->end(); ++itInput )
	{
		direction2 = *itInput - first;
		direction2.normalize();

		if( direction2 != direction1 )
		{
			// Store segment
			output->push_back( second );

			// Start new segment
			first = second;
			direction1 = *itInput - second;
		}

	}
}

