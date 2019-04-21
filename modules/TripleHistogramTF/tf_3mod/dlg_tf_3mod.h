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

#include <vtkSmartPointer.h>

#include <QDockWidget>

class QStackedLayout;
class QLabel;

class MdiChild;
class iATripleModalityWidget;
class BCoord;

class vtkSmartVolumeMapper;
class vtkVolume;
class vtkRenderer;

//typedef iAQTtoUIConnector<QDockWidget, Ui_dlg_TripleHistogramTF> TripleHistogramTFConnector;

class dlg_tf_3mod : public QDockWidget//public TripleHistogramTFConnector
{
	Q_OBJECT

public:
	dlg_tf_3mod(MdiChild* parent, Qt::WindowFlags f = 0);

public slots:
	void updateTransferFunction();
	void updateModalities();

private slots:

private:
	void updateDisabledLabel();

	// TODO: is it really good to keep the mdiChild as a member variable?
	MdiChild *m_mdiChild;

	// Widgets and stuff
	QStackedLayout *m_stackedLayout;
	QLabel *m_disabledLabel;
	iATripleModalityWidget *m_tripleModalityWidget;

	vtkSmartPointer<vtkSmartVolumeMapper> m_combinedVolMapper;
	vtkSmartPointer<vtkRenderer> m_combinedVolRenderer;
	vtkSmartPointer<vtkVolume> m_combinedVol;
};