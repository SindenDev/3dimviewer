///////////////////////////////////////////////////////////////////////////////
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


#ifndef CScaledCursor_H_included
#define CScaledCursor_H_included

#include <QCursor>
#include <QColor>

class CScaledCursor
{
public:
    //! Constructor
    CScaledCursor(size_t min_size = 8, size_t max_size = 128);

    //! Destructor
    ~CScaledCursor();

    //! Resize cursor
    void resize(float new_size);

    //! Get current cursor
    QCursor *getCursor() {return m_cursor;}

	//! Set pen width
	void setPenSize(int size);

protected:
    //! Render cursor pixmap
    void render(size_t pixmap_size);

protected:
    //! Cursor
    QCursor *m_cursor;

    //! Minimal cursor size
    size_t m_min_size;

    //! Maximal cursor size
    size_t m_max_size;

    //! Cursor color
    QColor m_color;

	//! Pen width
	int m_pen_size;
};

// CScaledCursor_H_included
#endif
