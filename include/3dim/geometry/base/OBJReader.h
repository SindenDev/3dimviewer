///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2015 3Dim Laboratory s.r.o.
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

#ifndef OBJREADER_H
#define OBJREADER_H

#include <geometry/base/MTLReader.h>
#include <geometry/base/CMesh.h>

namespace geometry
{
    class OBJReader
    {
    private:
        static std::string getLine(std::istream &stream);
        static std::vector<std::string> getItems(std::string source);

    public:
        OBJReader();
        ~OBJReader();
        bool read(const std::string &filename, geometry::CMesh &mesh, std::vector<std::string> &materialFiles);

    private:
        bool read(std::ifstream &input, geometry::CMesh &mesh, std::vector<std::string> &materialFiles);
    };
}
#endif
