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
#include "pch.h"
#include "iAAstraReconstructionModuleInterface.h"

#include "iAAstraAlgorithm.h"
#include "iAConsole.h"
#include "mainwindow.h"
#include "mdichild.h"
#include "dlg_ProjectionParameters.h"

#include <vtkImageData.h>

#include <QSettings>

#include <cassert>


void iAAstraReconstructionModuleInterface::Initialize( )
{
	QMenu* toolsMenu = m_mainWnd->getToolsMenu( );
	QMenu * astraReconMenu = getMenuWithTitle( toolsMenu, QString( "Astra Reconstruction" ), false );
	
	QAction * actionForwardProject = new QAction( m_mainWnd );
	actionForwardProject->setText( QApplication::translate( "MainWindow", "Forward Projection", 0 ) );
	AddActionToMenuAlphabeticallySorted(astraReconMenu, actionForwardProject, true);
	connect(actionForwardProject, SIGNAL(triggered()), this, SLOT(ForwardProject()));

	QAction * actionBackProject = new QAction(m_mainWnd);
	actionBackProject->setText(QApplication::translate("MainWindow", "Back Projection", 0));
	AddActionToMenuAlphabeticallySorted(astraReconMenu, actionBackProject, true);
	connect(actionBackProject, SIGNAL(triggered()), this, SLOT(BackProject()));
}


void iAAstraReconstructionModuleInterface::ForwardProject()
{
	// ask for and store settings:
	QSettings settings;
	QString SettingsKeyBase("Tools/AstraReconstruction/ForwardProjection/");
	projGeomType = settings.value(SettingsKeyBase + "projGeomType").toString();
	detSpacingX = settings.value(SettingsKeyBase + "detSpacingX").toDouble();
	detSpacingY = settings.value(SettingsKeyBase + "detSpacingY").toDouble();
	detRowCnt = settings.value(SettingsKeyBase + "detRowCnt").toInt();
	detColCnt = settings.value(SettingsKeyBase + "detColCnt").toInt();
	projAngleStart = settings.value(SettingsKeyBase + "projAngleStart").toDouble();
	projAngleEnd = settings.value(SettingsKeyBase + "projAngleEnd").toDouble();
	projAnglesCount = settings.value(SettingsKeyBase + "projAnglesCount").toInt();
	distOrigDet = settings.value(SettingsKeyBase + "distOrigDet").toDouble();
	distOrigSource = settings.value(SettingsKeyBase + "distOrigSource").toDouble();
	dlg_ProjectionParameters dlg;
	dlg.fillProjectionGeometryValues(projGeomType, detSpacingX, detSpacingY, detRowCnt, detColCnt, projAngleStart, projAngleEnd, projAnglesCount, distOrigDet, distOrigSource);
	if (dlg.exec() != QDialog::Accepted)
		return;
	projGeomType = dlg.ProjGeomType->currentText();
	detSpacingX = dlg.ProjGeomDetectorSpacingX->value();
	detSpacingY = dlg.ProjGeomDetectorSpacingY->value();
	detColCnt = dlg.ProjGeomDetectorPixelsX->value();
	detRowCnt = dlg.ProjGeomDetectorPixelsY->value();
	projAngleStart = dlg.ProjGeomProjAngleStart->value();
	projAngleEnd = dlg.ProjGeomProjAngleEnd->value();
	projAnglesCount = dlg.ProjGeomProjCount->value();
	distOrigDet = dlg.ProjGeomDistOriginDetector->value();
	distOrigSource = dlg.ProjGeomDistOriginSource->value();
	settings.setValue(SettingsKeyBase + "projGeomType", projGeomType);
	settings.setValue(SettingsKeyBase + "detSpacingX", detSpacingX);
	settings.setValue(SettingsKeyBase + "detSpacingY", detSpacingY);
	settings.setValue(SettingsKeyBase + "detRowCnt", detRowCnt);
	settings.setValue(SettingsKeyBase + "detColCnt", detColCnt);
	settings.setValue(SettingsKeyBase + "projAngleStart", projAngleStart);
	settings.setValue(SettingsKeyBase + "projAngleEnd", projAngleEnd);
	settings.setValue(SettingsKeyBase + "projAnglesCount", projAnglesCount);
	settings.setValue(SettingsKeyBase + "distOrigDet", distOrigDet);
	settings.setValue(SettingsKeyBase + "distOrigSource", distOrigSource);
	
	// start forward projection filter:
	QString filterName = "Forward Projection";
	PrepareResultChild(filterName);
	iAAstraAlgorithm* fwdProjection = new iAAstraAlgorithm(iAAstraAlgorithm::FP3D, filterName,
		m_childData.imgData, m_childData.polyData, m_mdiChild->getLogger(), m_mdiChild);
	m_mdiChild->connectThreadSignalsToChildSlots(fwdProjection);
	fwdProjection->SetFwdProjectParams(projGeomType, detSpacingX, detSpacingY, detRowCnt, detColCnt, projAngleStart, projAngleEnd, projAnglesCount, distOrigDet, distOrigSource);
	fwdProjection->start();
	m_mdiChild->addStatusMsg(filterName);
	m_mainWnd->statusBar()->showMessage(filterName, 10000);
}


void iAAstraReconstructionModuleInterface::BackProject()
{
	// ask for and store settings:
	MdiChild* child = m_mainWnd->activeMdiChild();
	vtkSmartPointer<vtkImageData> img = child->getImageData();
	int const * const dim = img->GetDimensions();
	QSettings settings;
	QString SettingsKeyBase("Tools/AstraReconstruction/BackProjection/");
	projGeomType   = settings.value(SettingsKeyBase + "projGeomType").toString();
	detSpacingX    = settings.value(SettingsKeyBase + "detSpacingX").toDouble();
	detSpacingY    = settings.value(SettingsKeyBase + "detSpacingY").toDouble();
	detRowDim      = settings.value(SettingsKeyBase + "detRowDim", 1).toInt();
	detColDim      = settings.value(SettingsKeyBase + "detColDim", 0).toInt();
	projAngleDim   = settings.value(SettingsKeyBase + "projAngleDim", 2).toInt();
	projAngleStart = settings.value(SettingsKeyBase + "projAngleStart").toDouble();
	projAngleEnd   = settings.value(SettingsKeyBase + "projAngleEnd").toDouble();
	distOrigDet    = settings.value(SettingsKeyBase + "distOrigDet").toDouble();
	distOrigSource = settings.value(SettingsKeyBase + "distOrigSource").toDouble();
	volDim[0]      = settings.value(SettingsKeyBase + "volumeDimX").toInt();
	volDim[1]      = settings.value(SettingsKeyBase + "volumeDimY").toInt();
	volDim[2]      = settings.value(SettingsKeyBase + "volumeDimZ").toInt();
	volSpacing[0]  = settings.value(SettingsKeyBase + "volumeSpacingX", 1).toDouble();
	volSpacing[1]  = settings.value(SettingsKeyBase + "volumeSpacingY", 1).toDouble();
	volSpacing[2]  = settings.value(SettingsKeyBase + "volumeSpacingZ", 1).toDouble();
	algorithmType  = settings.value(SettingsKeyBase + "algorithmType").toInt();
	numberOfIterations = settings.value(SettingsKeyBase + "numberOfIterations", 100).toInt();
	dlg_ProjectionParameters dlg;
	dlg.fillProjectionGeometryValues(projGeomType, detSpacingX, detSpacingY, projAngleStart, projAngleEnd, distOrigDet, distOrigSource);
	dlg.fillVolumeGeometryValues(volDim, volSpacing);
	dlg.fillProjInputMapping(detRowDim, detColDim, projAngleDim, volDim);
	dlg.fillAlgorithmValues(algorithmType, numberOfIterations);
	if (dlg.exec() != QDialog::Accepted)
		return;
	projGeomType = dlg.ProjGeomType->currentText();
	detSpacingX = dlg.ProjGeomDetectorSpacingX->value();
	detSpacingY = dlg.ProjGeomDetectorSpacingY->value();
	detRowDim = dlg.ProjInputDetectorRowDim->currentIndex();
	detColDim = dlg.ProjInputDetectorColDim->currentIndex();
	projAngleDim = dlg.ProjInputProjAngleDim->currentIndex();
	projAngleStart = dlg.ProjGeomProjAngleStart->value();
	projAngleEnd = dlg.ProjGeomProjAngleEnd->value();
	distOrigDet = dlg.ProjGeomDistOriginDetector->value();
	distOrigSource = dlg.ProjGeomDistOriginSource->value();
	volDim[0] =   dlg.VolGeomDimensionX->text().toInt();
	volDim[1] =   dlg.VolGeomDimensionY->text().toInt();
	volDim[2] =   dlg.VolGeomDimensionZ->text().toInt();
	volSpacing[0] = dlg.VolGeomSpacingX->text().toDouble();
	volSpacing[1] = dlg.VolGeomSpacingY->text().toDouble();
	volSpacing[2] = dlg.VolGeomSpacingZ->text().toDouble();
	switch (dlg.AlgorithmType->currentIndex())
	{
		case 0: algorithmType = iAAstraAlgorithm::BP3D;	  break;
		case 1: algorithmType = iAAstraAlgorithm::FDK3D;  break;
		case 2: algorithmType = iAAstraAlgorithm::SIRT3D; break;
		case 3: algorithmType = iAAstraAlgorithm::CGLS3D; break;
		default: DEBUG_LOG("Invalid Algorithm Type selection!"); return;
	}
	numberOfIterations = dlg.AlgorithmIterations->value();
	if ((detColDim % 3) == (detRowDim % 3) || (detColDim % 3) == (projAngleDim %3) || (detRowDim % 3) == (projAngleDim % 3))
	{
		child->addMsg("One of the axes (x, y, z) has been specified for more than one usage out of (detector row / detector column / projection angle) dimensions. "
			"Make sure each axis is used exactly for one dimension!");
		return;
	}
	detRowCnt = dim[detRowDim % 3];
	detColCnt = dim[detColDim % 3];
	projAnglesCount = dim[projAngleDim % 3];
	settings.setValue(SettingsKeyBase + "projGeomType", projGeomType);
	settings.setValue(SettingsKeyBase + "detSpacingX", detSpacingX);
	settings.setValue(SettingsKeyBase + "detSpacingY", detSpacingY);
	settings.setValue(SettingsKeyBase + "detRowDim", detRowDim);
	settings.setValue(SettingsKeyBase + "detColDim", detColDim);
	settings.setValue(SettingsKeyBase + "projAngleDim", projAngleDim);
	settings.setValue(SettingsKeyBase + "projAngleStart", projAngleStart);
	settings.setValue(SettingsKeyBase + "projAngleEnd", projAngleEnd);
	settings.setValue(SettingsKeyBase + "distOrigDet", distOrigDet);
	settings.setValue(SettingsKeyBase + "distOrigSource", distOrigSource);
	settings.setValue(SettingsKeyBase + "volumeDimX", volDim[0]);
	settings.setValue(SettingsKeyBase + "volumeDimY", volDim[1]);
	settings.setValue(SettingsKeyBase + "volumeDimZ", volDim[2]);
	settings.setValue(SettingsKeyBase + "volumeSpacingX", volSpacing[0]);
	settings.setValue(SettingsKeyBase + "volumeSpacingY", volSpacing[1]);
	settings.setValue(SettingsKeyBase + "volumeSpacingZ", volSpacing[2]);
	settings.setValue(SettingsKeyBase + "algorithmType", algorithmType);
	settings.setValue(SettingsKeyBase + "numberOfIterations", numberOfIterations);
	
	// start back projection filter:
	QString filterName = dlg.AlgorithmType->currentText();
	PrepareResultChild(filterName);
	iAAstraAlgorithm* backProjection = new iAAstraAlgorithm(static_cast<iAAstraAlgorithm::AlgorithmType>(algorithmType), filterName,
		m_childData.imgData, m_childData.polyData, m_mdiChild->getLogger(), m_mdiChild);
	m_mdiChild->connectThreadSignalsToChildSlots(backProjection);
	backProjection->SetBckProjectParams(projGeomType, detSpacingX, detSpacingY, detRowCnt, detColCnt, projAngleStart, projAngleEnd, projAnglesCount, distOrigDet, distOrigSource,
		detRowDim, detColDim, projAngleDim, volDim, volSpacing, numberOfIterations);
	backProjection->start();
	m_mdiChild->addStatusMsg(filterName);
	m_mainWnd->statusBar()->showMessage(filterName, 10000);
}