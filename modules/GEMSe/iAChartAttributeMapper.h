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

#include <QMap>

#include <utility>

class iAChartAttributeMapper
{
public:
	int GetChartID(int datasetID, int attributeID) const;
	QList<int> GetDatasetIDs(int chartID) const;
	int GetAttributeID(int chartID, int datasetID) const;
	int GetAttribCount(int datasetID) const;

	void Add(int datasetID, int attributeID, int chartID);
	void Clear();
private:
	//! map from chartID to (map from datasetID to attributeID)
	QMap<int, QMap<int, int> > m_chartAttributeMap;

	//! map from (datasetID, attributeID) to chartID
	QMap<std::pair<int, int>, int>  m_attributeChartMap;
	//! map from datasetID to attribute count
	QMap<int, int> m_datasetAttribCount;
};