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

#ifndef CSpinSliderConnection_H_included
#define CSpinSliderConnection_H_included

template<class tpSpin, class tpSlider>
class CDoubleSpinSliderConnection
{
public:
	//! Spin control type
	typedef tpSpin tSpin;

	//! Slider control type
	typedef tpSlider tSlider;

	//! Value type
	typedef double tValue;
public:
	//! Constructor 
	CDoubleSpinSliderConnection(tSpin *spin = NULL, tSlider *slider = NULL) : m_spin(0), m_slider(0) {initConnections();}
	
	//! Set spin and control
	void setSpinSlider(tSpin *spin, tSlider *slider) {removeConnections(); m_spin = spin; m_slider = slider; initConnections(); }

	//! Set spin control
	void setSpin(tSpin *spin) {removeConnections(); m_spin = spin; initConnections(); }
		
	//! Set slider control
	void setSlider(tSlider *slider) {removeConnections(); m_slider = slider; initConnections(); }
	
	//! Get spin control
	tSpin *getSpin() {return m_spin;}
	
	//! Get slider control
	tSlider *getSlider() {return m_slider;}

	//! Set range
	void setRange( tValue min, tValue max) { if(valid()){m_spin->setRange(min, max); m_slider->setRange(min, max);} }

	//! Set minimal value
	void setMinimum(tValue min) { if(valid()){m_spin->setMinimum(min); m_slider->setMinimum(min);} }

	//! Set maximal value
	void setMaximum(tValue max) { if(valid()){m_spin->setMaximum(max); m_slider->setMaximum(max);} }

	//! Set step
	void setSingleStep(tValue step) { if(valid()){m_spin->setSingleStep(step); m_slider->setSingleStep(step);} }
	
	//! Set value
	void setValue(tValue value) {if(valid()){m_spin->setValue(value); m_slider->setValue(value);} }

	//! Get value
	tValue getValue() {if(valid())return m_spin->value(); else return tValue();}

protected:
	//! Initialize connections
	void initConnections();

	//! Remove connections
	void removeConnections();

	//! Both pointers are valid?
	bool valid() {return m_spin != 0 && m_slider != 0;}

protected:
	//! Spin control pointer
	tpSpin *m_spin;
	
	//! Slider control pointer
	tpSlider *m_slider;

}; // template class CSpinSliderConnection

template<class tpSpin, class tpSlider>
void CDoubleSpinSliderConnection<tpSpin, tpSlider>::removeConnections()
{
	if(!valid())
		return;

	QObject::disconnect(m_spin, SIGNAL(valueChanged(double)), m_slider, SLOT(setValue(double)));
	QObject::disconnect(m_slider, SIGNAL(valueChanged(double)), m_spin, SLOT(setValue(double)));

}

template<class tpSpin, class tpSlider>
void CDoubleSpinSliderConnection<tpSpin, tpSlider>::initConnections()
{
	if(!valid())
		return;

	QObject::connect(m_spin, SIGNAL(valueChanged(double)), m_slider, SLOT(setValue(double)));
	QObject::connect(m_slider, SIGNAL(valueChanged(double)), m_spin, SLOT(setValue(double)));
}

template<class tpSpin, class tpSlider>
class CIntSpinSliderConnection
{
public:
	//! Spin control type
	typedef tpSpin tSpin;

	//! Slider control type
	typedef tpSlider tSlider;

	//! Value type
	typedef int tValue;
public:
	//! Constructor 
	CIntSpinSliderConnection(tSpin *spin = NULL, tSlider *slider = NULL) : m_spin(0), m_slider(0) {initConnections();}

	//! Set spin and control
	void setSpinSlider(tSpin *spin, tSlider *slider) {removeConnections(); m_spin = spin; m_slider = slider; initConnections(); }

	//! Set spin control
	void setSpin(tSpin *spin) {removeConnections(); m_spin = spin; initConnections(); }

	//! Set slider control
	void setSlider(tSlider *slider) {removeConnections(); m_slider = slider; initConnections(); }

	//! Get spin control
	tSpin *getSpin() {return m_spin;}

	//! Get slider control
	tSlider *getSlider() {return m_slider;}

	//! Set range
	void setRange( tValue min, tValue max) { if(valid()){m_spin->setRange(min, max); m_slider->setRange(min, max);} }

	//! Set minimal value
	void setMinimum(tValue min) { if(valid()){m_spin->setMinimum(min); m_slider->setMinimum(min);} }

	//! Set maximal value
	void setMaximum(tValue max) { if(valid()){m_spin->setMaximum(max); m_slider->setMaximum(max);} }

	//! Set step
	void setSingleStep(tValue step) { if(valid()){m_spin->setSingleStep(step); m_slider->setSingleStep(step);} }

	//! Set value
	void setValue(tValue value) {if(valid()){m_spin->setValue(value); m_slider->setValue(value);} }

	//! Get value
	tValue getValue() {if(valid())return m_spin->value(); else return tValue();}

protected:
	//! Initialize connections
	void initConnections();

	//! Remove connections
	void removeConnections();

	//! Both pointers are valid?
	bool valid() {return m_spin != 0 && m_slider != 0;}

protected:
	//! Spin control pointer
	tpSpin *m_spin;

	//! Slider control pointer
	tpSlider *m_slider;

}; // template class CSpinSliderConnection

template<class tpSpin, class tpSlider>
void CIntSpinSliderConnection<tpSpin, tpSlider>::removeConnections()
{
	if(!valid())
		return;

	QObject::disconnect(m_spin, SIGNAL(valueChanged(int)), m_slider, SLOT(setValue(int)));
	QObject::disconnect(m_slider, SIGNAL(valueChanged(int)), m_spin, SLOT(setValue(int)));

}

template<class tpSpin, class tpSlider>
void CIntSpinSliderConnection<tpSpin, tpSlider>::initConnections()
{
	if(!valid())
		return;

	QObject::connect(m_spin, SIGNAL(valueChanged(int)), m_slider, SLOT(setValue(int)));
	QObject::connect(m_slider, SIGNAL(valueChanged(int)), m_spin, SLOT(setValue(int)));
}


// CSpinSliderConnection_H_included
#endif
