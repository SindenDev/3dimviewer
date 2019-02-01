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

#include <geometry/base/MTLReader.h>
#include <istream>
#include <vector>
#include <fstream>
#include <algorithm>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace geometry
{
    std::string MTLReader::getLine(std::istream &stream)
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

    std::vector<std::string> MTLReader::getItems(std::string source)
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

    Material::Material()
        : name("")
        , map_Kd("")
    { }

    Material::~Material()
    { }

    MTLReader::MTLReader()
    { }

    MTLReader::~MTLReader()
    { }

    bool MTLReader::read(std::ifstream &input, std::vector<Material> &materials)
    {
        materials.clear();

        while (!input.eof())
        {
            std::string line = getLine(input);
            if (line.empty())
            {
                continue;
            }

            std::vector<std::string> items = getItems(line);

            if (items[0] == "newmtl")
            {
                if (items.size() >= 2)
                {
                    Material material;
                    material.name = items[1];
                    materials.push_back(material);
                }
            }
            else if (items[0] == "map_Kd")
            {
                if (!materials.empty())
                {
                    if (items.size() >= 2)
                    {
                        materials.back().map_Kd = items[1];
                    }
                }
            }
        }

        return true;
    }

    bool MTLReader::read(std::string &filename, std::vector<Material> &materials)
    {
        std::ifstream input;
        input.open(filename, std::ios_base::in);
        read(input, materials);
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

        for (int i = 0; i < materials.size(); ++i)
        {
            materials[i].map_Kd = path + "/" + materials[i].map_Kd;
        }

        return true;
    }
}
