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

#include <geometry/base/OBJReader.h>

namespace geometry
{
    std::string OBJReader::getLine(std::istream &stream)
    {
        std::string line = "";

        if (stream.eof())
        {
            return line;
        }

        char buffer[8];
        int read = 0;
        do
        {
            stream.clear();
            std::memset(buffer, '\0', 8);
            stream.getline(buffer, 8);
            line += buffer;
            read = stream.gcount();
        } while (!stream.eof() && (read == 8 - 1) && (buffer[read - 1] != '\0'));

        return line;
    }

    std::vector<std::string> OBJReader::getItems(std::string source)
    {
        std::vector<std::string> items;
        while (source.length() > 0)
        {
            std::size_t index0;
            index0 = source.find_first_of(' ');
            index0 = index0 == std::string::npos ? source.length() : index0;

            std::size_t index1;
            index1 = source.find_first_of('\t');
            index1 = index1 == std::string::npos ? source.length() : index1;

            std::size_t index = source.length();
            index = std::min(index0, index);
            index = std::min(index1, index);

            std::string item = source.substr(0, index);
            source = source.substr(std::min(source.length(), index + 1));

            if (!item.empty())
            {
                items.push_back(item);
            }
        }
        return items;
    }

    OBJReader::OBJReader()
    { }

    OBJReader::~OBJReader()
    { }

    bool OBJReader::read(std::ifstream &input, geometry::CMesh &mesh, std::vector<std::string> &materialFiles)
    {
        materialFiles.clear();

        while (!input.eof())
        {
            std::string line = getLine(input);
            if (line.empty())
            {
                continue;
            }

            std::vector<std::string> items = getItems(line);

            if (items[0] == "mtllib")
            {
                if (items.size() >= 2)
                {
                    materialFiles.push_back(items[1]);
                }
            }
        }

        return true;
    }

    bool OBJReader::read(const std::string &filename, geometry::CMesh &mesh, std::vector<std::string> &materialFiles)
    {
        std::ifstream input;
        input.open(filename, std::ios_base::in);
        read(input, mesh, materialFiles);
        input.close();

        std::string path = filename;
        size_t index0 = path.find_last_of('\\');
        index0 = index0 == std::string::npos ? 0 : index0;
        size_t index1 = path.find_last_of('/');
        index1 = index1 == std::string::npos ? 0 : index1;

        std::size_t index = 0;
        index = std::max(index0, index);
        index = std::max(index1, index);

        path = path.substr(0, index);

        for (int i = 0; i < materialFiles.size(); ++i)
        {
            materialFiles[i] = path + "/" + materialFiles[i];
        }

        OpenMesh::IO::Options ropt;
        ropt += OpenMesh::IO::Options::Binary;
        ropt += OpenMesh::IO::Options::VertexTexCoord;
        bool result = true;
        if (!OpenMesh::IO::read_mesh(mesh, filename, ropt))
        {
            result = false;
        }

        return result;
    }
}
