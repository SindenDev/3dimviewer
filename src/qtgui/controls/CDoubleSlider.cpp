///////////////////////////////////////////////////////////////////////////////
// $Id:$
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

#include <controls/CDoubleSlider.h>
#include <assert.h>
#include <cmath>
#include <limits>

/**
 * Constructor.
 *
 * \param [in,out]	slider	If non-null, the slider.
**/
CDoubleSlider::CDoubleSlider( QSlider *slider /*= NULL*/ )
	: m_slider(0)
	, m_min(0.0)
	, m_max(100.0)
	, m_step(1.0)
{
	setSlider(slider);
}

/**
 * Sets a slider.
 *
 * \param [in,out]	slider	If non-null, the slider.
**/
void CDoubleSlider::setSlider( QSlider *slider )
{
	// Disconnect previous slider if need to
	if(m_slider != 0)
	{
		QObject::disconnect(this, SLOT(valueChanged(int)));
	}

	m_slider = slider;

	// Connect slot
	if(m_slider != 0)
		QObject::connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));

	updateSlider();
}

/**
 * Sets a maximum.
 *
 * \param	parameter1	The first parameter.
**/
void CDoubleSlider::setMaximum( double maximum )
{
	m_max = maximum;
	updateSlider();
}

/**
 * Sets a minimum.
 *
 * \param	minimum	The minimum.
**/
void CDoubleSlider::setMinimum( double minimum)
{
	m_min = minimum;
	updateSlider();
}

/**
 * Sets a f range.
 *
 * \param	min	The minimum.
 * \param	max	The maximum.
**/
void CDoubleSlider::setRange( double min, double max )
{
	m_max = max; m_min = min;
	updateSlider();
}

/**
 * Sets a f single step.
 *
 * \param	step	Amount to increment by.
**/
void CDoubleSlider::setSingleStep( double step )
{
	assert(fabs(step) > std::numeric_limits<double>::min() );
	m_step = fabs(step);

	updateSlider();
}

/**
 * Updates a slider.
**/
void CDoubleSlider::updateSlider()
{
	if(m_slider == 0)
		return;

	// Compute number of steps
	float divided_range((m_max-m_min)/m_step);
	int num_steps(fabs(divided_range));

	// Set slider range to cover interval
	m_slider->setRange(0, num_steps);
	m_slider->setSingleStep(1);
}

/**
 * Sets a value.
 *
 * \param	value	The value.
**/
void CDoubleSlider::setValue( double value )
{
	assert(m_slider);

	if(fabs(getValue()-value) < m_step)
		return;

	double v(std::min(std::max(value, m_min), m_max));

	int iv((v-m_min)/m_step);
	m_slider->setValue(iv);
}

/**
 * Gets the value.
 *
 * \return	The value.
**/
double CDoubleSlider::getValue()
{
	assert(m_slider);
	int iv(m_slider->value());
	return double(iv)*m_step + m_min;
}

/**
 * Value changed.
 *
 * \param	parameter1	The first parameter.
**/
void CDoubleSlider::valueChanged( int )
{
	// Get current value
	double value(getValue());

	// Emit signals
	emit valueChanged(value);
	emit valueChanged(QString::number(value));
}

