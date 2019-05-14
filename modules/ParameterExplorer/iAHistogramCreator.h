/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2019  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
*                          Amirkhanov, J. Weissenböck, B. Fröhler, M. Schiwarth       *
* *********************************************************************************** *
* This program is free software: you can redistribute it and/or modify it under the   *
* terms of the GNU General Public License as published by the Free Software           *
* Foundation, either version 3 of the License, or (at your option) any later version. *
*                                                                                     *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY     *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A     *
* PARTICULAR PURPOSE.  See the GNU General Public License for more details.           *
*                                                                                     *
* You should have received a copy of the GNU General Public License along with this   *
* program.  If not, see http://www.gnu.org/licenses/                                  *
* *********************************************************************************** *
* Contact: FH OÖ Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstraße 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#pragma once

#include "charts/iAHistogramData.h"

#include <vtkImageData.h>
#include <vtkSmartPointer.h>

#include <QSharedPointer>
#include <QThread>

class iAHistogramCreator : public QThread
{
	Q_OBJECT
public:
	iAHistogramCreator(vtkSmartPointer<vtkImageData> img, int binCount, int id) :
		m_img(img),
		m_binCount(binCount),
		m_id(id)
	{}
	virtual void run()
	{
		m_histogramData = iAHistogramData::Create(m_img, m_binCount, nullptr);
	}
	QSharedPointer<iAHistogramData> GetData()
	{
		return m_histogramData;
	}
	int GetID() const
	{
		return m_id;
	}
private:
	vtkSmartPointer<vtkImageData> m_img;
	int m_binCount;
	QSharedPointer<iAHistogramData> m_histogramData;
	int m_id;
};