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

#include "iAPlotData.h"
#include "open_iA_Core_export.h"

class open_iA_Core_API iAMappingDiagramData: public iAPlotData
{
private:
	size_t m_numBin;
	DataType * m_data;
	double m_spacing;
	double m_xBounds[2];
	double m_yBounds[2];
public:
	iAMappingDiagramData(DataType const * data,
		int srcNumBin, double srcMinX, double srcMaxX,
		size_t targetNumBin, double targetMinX, double targetMaxX,
		DataType const maxValue);
	~iAMappingDiagramData();
	size_t GetNumBin() const override;
	DataType const * GetRawData() const override;
	double GetSpacing() const override;
	double const * XBounds() const override;
	DataType const * YBounds() const override;
};
