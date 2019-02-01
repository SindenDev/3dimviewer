///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2018 TESCAN 3DIM, s.r.o.
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

//! \brief Displaying QT information window.

#pragma once
#include <QMessageBox>

class MessageWindow
{
public:
    //! \brief Constructor - Open error window
    MessageWindow(const QString& message)
    {
        m_icon = QMessageBox::Critical;
        m_message = message;
    }

    //! \brief Constructor
    //! \param icon Icon type (warning, error, information ...)
    //! \param message strings
    MessageWindow(const QMessageBox::Icon icon, const QString& message)
    {
        m_icon = icon;
        m_message = message;
    }

    virtual ~MessageWindow() = default;

    //! \brief Enable message box and display it.
    void show() const
    {
        QMessageBox msgBox;
        msgBox.setIcon(m_icon);
        msgBox.setText(m_message);
        msgBox.exec();
    }

private:
    QMessageBox::Icon m_icon;
    QString m_message;
};
