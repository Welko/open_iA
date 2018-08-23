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
#pragma once

#include "iA3DObjectVis.h"

class vtkPiecewiseFunction;
class vtkColorTransferFunction;

class iA3DLabelledVolumeVis: public iA3DObjectVis
{
public:
	iA3DLabelledVolumeVis( iAVtkWidgetClass* widget, vtkColorTransferFunction* color, vtkPiecewiseFunction* opac, vtkTable* objectTable, QSharedPointer<QMap<uint, uint> > columnMapping );
	void renderSelection( std::vector<size_t> const & sortedSelInds, int classID, QColor const & classColor, QStandardItem* activeClassItem ) override;
	void renderSingle( int labelID, int classID, QColor const & classColor, QStandardItem* activeClassItem ) override;
	void multiClassRendering( QList<QColor> const & classColors, QStandardItem* rootItem, double alpha ) override;
	void renderOrientationDistribution( vtkImageData* oi ) override;
	void renderLengthDistribution( vtkColorTransferFunction* ctFun, vtkFloatArray* extents, double halfInc, int filterID, double const * range ) override;
private:
	vtkPiecewiseFunction     *oTF;
	vtkColorTransferFunction *cTF;
};
