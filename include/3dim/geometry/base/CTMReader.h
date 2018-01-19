/*===========================================================================*\
 *                                                                           *
 *                               OpenMesh                                    *
 *      Copyright (C) 2001-2014 by Computer Graphics Group, RWTH Aachen      *
 *                           www.openmesh.org                                *
 *                                                                           *
 *---------------------------------------------------------------------------*
 *  This file is part of OpenMesh.                                           *
 *                                                                           *
 *  OpenMesh is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU Lesser General Public License as           *
 *  published by the Free Software Foundation, either version 3 of           *
 *  the License, or (at your option) any later version with the              *
 *  following exceptions:                                                    *
 *                                                                           *
 *  If other files instantiate templates or use macros                       *
 *  or inline functions from this file, or you compile this file and         *
 *  link it with other files to produce an executable, this file does        *
 *  not by itself cause the resulting executable to be covered by the        *
 *  GNU Lesser General Public License. This exception does not however       *
 *  invalidate any other reasons why the executable file might be            *
 *  covered by the GNU Lesser General Public License.                        *
 *                                                                           *
 *  OpenMesh is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU LesserGeneral Public          *
 *  License along with OpenMesh.  If not,                                    *
 *  see <http://www.gnu.org/licenses/>.                                      *
 *                                                                           *
\*===========================================================================*/

/*===========================================================================*\
 *                                                                           *
 *   $Revision: 990 $                                                         *
 *   $Date: 2014-02-05 10:01:07 +0100 (Mi, 05 Feb 2014) $                   *
 *                                                                           *
\*===========================================================================*/


//=============================================================================
//
//  Implements an reader module for STL files
//
//=============================================================================




#ifndef __CTMREADER_HH__
#define __CTMREADER_HH__


//=== INCLUDES ================================================================


//probably something with openMesh
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <string>

#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/SingletonT.hh>
#include <OpenMesh/Core/IO/reader/BaseReader.hh>


#include <openctm.h>

#ifndef WIN32
#include <string.h>
#endif


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {

//== FORWARDS =================================================================

class BaseImporter;

//== IMPLEMENTATION ===========================================================


/**
    Implementation of the CTM format reader. This class is singleton'ed by
    SingletonT to CTMReader.
*/
class OPENMESHDLLEXPORT _CTMReader_ : public BaseReader
{
public:

  // constructor
	_CTMReader_();

  /// Destructor
  virtual ~_CTMReader_() {};


  std::string get_description() const
  { return "Open CTM model format"; }
  std::string get_extensions() const { return "ctm"; }

  bool read(const std::string& _filename,
	    BaseImporter& _bi,
            Options& _opt);
  
  //must be overriden
  bool read(std::istream& _in,
            BaseImporter& _bi,
            Options& _opt);

  
  /** Set the threshold to be used for considering two point to be equal.
      Can be used to merge small gaps */
  void set_epsilon(float _eps) { eps_=_eps; }

  /// Returns the threshold to be used for considering two point to be equal.
  float epsilon() const { return eps_; }



private:

  float eps_;
};


//== TYPE DEFINITION ==========================================================


/// Declare the single entity of the STL reader
extern _CTMReader_  __CTMReaderInstance;
OPENMESHDLLEXPORT _CTMReader_&  CTMReader();


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================
