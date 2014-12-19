///////////////////////////////////////////////////////////////////////////////
// $Id:$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2014 3Dim Laboratory s.r.o.
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

#ifndef CDoubleSlider_H_included
#define CDoubleSlider_H_included

#include <QSlider>

class CDoubleSlider : public QObject
{
	Q_OBJECT

public:
	//! Constructor
	explicit CDoubleSlider(QSlider *slider = NULL);
	//! Set currently used slider
	void setSlider(QSlider *slider);
	//! Get currently used slider
	QSlider *getSlider() {return m_slider;}
	//! Set maximal value
	void	setMaximum ( double );
	//! Set minimal value
	void	setMinimum ( double );
	//! Get maximal value
	double	maximum () const {return m_min;}
	//! Get minimal value
	double	minimum () const {return m_max;}
	//! Set range
	void	setRange ( double min, double max );
	//! Set single step value
	void	setSingleStep ( double step );
	//! Get current value
	double	getValue();

signals: 
	void valueChanged(double);
	void valueChanged(const QString &);

public slots:
	//! Set current value
	void setValue(double value);

protected slots:
	//! Value has changed slot - used to connect to the slider
	void valueChanged(int);
	

protected:
	void updateSlider();

protected:
	//! Nested slider
	QSlider *m_slider;
	//! Minimal and maximal float value
	double m_min, m_max;
	//! Step value
	double m_step;
};


// QFloatSlider_H_included
#endif
