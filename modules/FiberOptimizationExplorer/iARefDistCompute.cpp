/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2018  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan,            *
*                          J. Weissenböck, Artem & Alexander Amirkhanov, B. Fröhler   *
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
#include "iARefDistCompute.h"

#include "iAFiberCharData.h"
#include "iAFiberData.h"

#include "iACsvConfig.h"
#include "iAPerformanceHelper.h"

#include "charts/iASPLOMData.h"
#include "iAConsole.h"
#include "iAStringHelper.h"
#include "iAvec3.h"

#include <vtkTable.h>
#include <vtkVariant.h>

#include <array>

namespace
{

	void getBestMatches(iAFiberData const & fiber, QMap<uint, uint> const & mapping, vtkTable* refTable,
		std::vector<std::vector<iAFiberDistance> > & bestMatches, double diagonalLength, double maxLength)
	{
		size_t refFiberCount = refTable->GetNumberOfRows();
		bestMatches.resize(iARefDistCompute::DistanceMetricCount);
		for (int d = 0; d<iARefDistCompute::DistanceMetricCount; ++d)
		{
			std::vector<iAFiberDistance> distances;
			if (d < 3)
			{
				distances.resize(refFiberCount);
				for (size_t refFiberID = 0; refFiberID < refFiberCount; ++refFiberID)
				{
					iAFiberData refFiber(refTable, refFiberID, mapping);
					distances[refFiberID].index = refFiberID;
					double curDistance = getDistance(fiber, refFiber, d, diagonalLength, maxLength);
					distances[refFiberID].distance = curDistance;
				}
			}
			else
			{ // compute overlap measures only for the best-matching fibers according to metric 2:
				distances.resize(bestMatches[1].size());
				for (size_t bestMatchID = 0; bestMatchID < bestMatches[1].size(); ++bestMatchID)
				{

					iAFiberData refFiber(refTable, distances[bestMatchID].index, mapping);
					distances[bestMatchID].index = bestMatches[1][bestMatchID].index;
					double curDistance = getDistance(fiber, refFiber, d, diagonalLength, maxLength);
					distances[bestMatchID].distance = curDistance;
				}
			}
			std::sort(distances.begin(), distances.end());
			std::copy(distances.begin(), distances.begin() + iARefDistCompute::MaxNumberOfCloseFibers, std::back_inserter(bestMatches[d]));
		}
	}
}

int iARefDistCompute::MaxNumberOfCloseFibers = 25;

iARefDistCompute::iARefDistCompute(std::vector<iAFiberCharData> & results, iASPLOMData & splomData, int referenceID):
	m_splomData(splomData),
	m_resultData(results),
	m_referenceID(referenceID)
{}

void iARefDistCompute::run()
{
	iAPerformanceHelper perfRefComp;
	perfRefComp.start("Reference Distance computation");
	// "register" other datasets to reference:
	auto & ref = m_resultData[m_referenceID];
	auto const & mapping = *ref.mapping.data();
	double const * cxr = m_splomData.paramRange(mapping[iACsvConfig::CenterX]),
		*cyr = m_splomData.paramRange(mapping[iACsvConfig::CenterY]),
		*czr = m_splomData.paramRange(mapping[iACsvConfig::CenterZ]);
	double a = cxr[1] - cxr[0], b = cyr[1] - cyr[0], c = czr[1] - czr[0];
	double diagLength = std::sqrt(std::pow(a, 2) + std::pow(b, 2) + std::pow(c, 2));
	double const * lengthRange = m_splomData.paramRange(mapping[iACsvConfig::Length]);
	double maxLength = lengthRange[1] - lengthRange[0];

	for (size_t resultID = 0; resultID < m_resultData.size(); ++resultID)
	{
		m_progress.EmitProgress(static_cast<int>(100.0 * resultID / m_resultData.size()));
		auto & d = m_resultData[resultID];
		if (resultID == m_referenceID)
			continue;
		size_t fiberCount = d.table->GetNumberOfRows();
		d.refDiffFiber.resize(fiberCount);
		for (size_t fiberID = 0; fiberID < fiberCount; ++fiberID)
		{
			// find the best-matching fibers in reference & compute difference:
			iAFiberData fiber(d.table, fiberID, mapping);
			getBestMatches(fiber, mapping, ref.table,
				d.refDiffFiber[fiberID].dist, diagLength, maxLength);
		}
	}
	perfRefComp.stop();
	std::array<size_t, iAFiberCharData::FiberValueCount> diffCols = {
		mapping[iACsvConfig::StartX],  mapping[iACsvConfig::StartY],  mapping[iACsvConfig::StartZ],
		mapping[iACsvConfig::EndX],    mapping[iACsvConfig::EndY],    mapping[iACsvConfig::EndZ],
		mapping[iACsvConfig::CenterX], mapping[iACsvConfig::CenterY], mapping[iACsvConfig::CenterZ],
		mapping[iACsvConfig::Phi], mapping[iACsvConfig::Theta],
		mapping[iACsvConfig::Length],
		mapping[iACsvConfig::Diameter]
	};
	iAPerformanceHelper perfDistComp;
	perfDistComp.start("Distance computation");
	for (size_t resultID = 0; resultID < m_resultData.size(); ++resultID)
	{
		auto& d = m_resultData[resultID];
		if (resultID == m_referenceID)
			continue;
		size_t fiberCount = d.table->GetNumberOfRows();
		d.refDiffFiber.resize(fiberCount);
		for (size_t fiberID = 0; fiberID < fiberCount; ++fiberID)
		{
			size_t timeStepCount = d.timeValues.size();
			auto & timeSteps = d.refDiffFiber[fiberID].timeStep;
			timeSteps.resize(timeStepCount);
			for (size_t timeStep = 0; timeStep < timeStepCount; ++timeStep)
			{
				// compute error (=difference - startx, starty, startz, endx, endy, endz, shiftx, shifty, shiftz, phi, theta, length, diameter)
				auto & diffs = timeSteps[timeStep].diff;
				diffs.resize(iAFiberCharData::FiberValueCount+DistanceMetricCount);
				for (size_t diffID = 0; diffID < iAFiberCharData::FiberValueCount; ++diffID)
				{
					diffs[diffID] = d.timeValues[timeStep][fiberID][diffID]
						- ref.table->GetValue(d.refDiffFiber[fiberID].dist[0][0].index, diffCols[diffID]).ToDouble();
				}
				for (size_t distID = 0; distID < DistanceMetricCount; ++distID)
				{
					iAFiberData refFiber(ref.table, d.refDiffFiber[fiberID].dist[0][0].index, mapping);
					iAFiberData fiber(d.timeValues[timeStep][fiberID]);
					double dist = getDistance(fiber, refFiber, distID, diagLength, maxLength);
					diffs[iAFiberCharData::FiberValueCount + distID] = dist;
				}
			}
		}
	}
	perfDistComp.stop();
	size_t splomID = 0;
	iAPerformanceHelper perfSPLOMUpdate;
	perfSPLOMUpdate.start("SPLOM Update");
	for (size_t resultID = 0; resultID < m_resultData.size(); ++resultID)
	{
		iAFiberCharData& d = m_resultData[resultID];
		if (resultID == m_referenceID)
		{
			splomID += d.fiberCount;
			continue;
		}
		for (size_t fiberID = 0; fiberID < d.fiberCount; ++fiberID)
		{
			auto & diffData = d.refDiffFiber[fiberID];
			for (size_t diffID = 0; diffID < iAFiberCharData::FiberValueCount; ++diffID)
			{
				size_t tableColumnID = m_splomData.numParams() - (iAFiberCharData::FiberValueCount + DistanceMetricCount + EndColumns) + diffID;
				m_splomData.data()[tableColumnID][splomID] = diffData.timeStep[d.timeValues.size()-1].diff[diffID];
				//d.table->SetValue(fiberID, tableColumnID, d.timeRefDiff[fiberID][d.m_timeValues.size()-1][diffID]);
			}
			for (size_t distID = 0; distID < DistanceMetricCount; ++distID)
			{
				double dist = diffData.dist[distID][0].distance;
				size_t tableColumnID = m_splomData.numParams() - (DistanceMetricCount + EndColumns) + distID;
				m_splomData.data()[tableColumnID][splomID] = dist;
				//d.table->SetValue(fiberID, tableColumnID, dist);
			}
			++splomID;
		}
	}
	perfSPLOMUpdate.stop();
}

iAProgress* iARefDistCompute::progress()
{
	return &m_progress;
}
