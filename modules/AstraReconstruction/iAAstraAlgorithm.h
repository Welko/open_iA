/*************************************  open_iA  ************************************ *
* **********  A tool for scientific visualisation and 3D image processing  ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2017  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan,            *
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
#pragma once

#include "iAAlgorithm.h"

class iAAstraAlgorithm : public iAAlgorithm
{
public:
	enum AlgorithmType
	{
		ForwardProjection,
		FilteredBackProjection,
		SIRTBackProjection
	};
	iAAstraAlgorithm(AlgorithmType type, QString const & filterName, vtkImageData* i, vtkPolyData* p, iALogger* logger, QObject *parent);
	void SetFwdProjectionParams(QString const & projGeomType, double detSpacingX, double detSpacingY, int detRowCnt, int detColCnt,
		double projAngleStart, double projAngleEnd, int projAnglesCount, double distOrigDet, double distOrigSource);
	void SetFBPParams(QString const & projGeomType, double detSpacingX, double detSpacingY, int detRowCnt, int detColCnt,
		double projAngleStart, double projAngleEnd, int projAnglesCount, double distOrigDet, double distOrigSource,
		int detRowDim, int detColDim, int projAngleDim, int volDim[3], double volSpacing[3]);
private:
	virtual void performWork();
	void ForwardProject();
	void BackProject();

	int m_type;
	QString m_projGeomType;
	double m_detSpacingX, m_detSpacingY, m_distOrigDet,
		m_distOrigSource, m_projAngleStart, m_projAngleEnd;
	int m_detRowCnt, m_detColCnt, m_projAnglesCount;

	int m_detRowDim, m_detColDim, m_projAngleDim;
	int m_volDim[3];
	double m_volSpacing[3];
};
