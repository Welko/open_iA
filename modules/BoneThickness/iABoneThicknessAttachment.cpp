/*********************************  open_iA 2016 06  ******************************** *
* **********  A tool for scientific visualisation and 3D image processing  ********** *
* *********************************************************************************** *
* Copyright (C) 2016  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, J. Weissenb�ck, *
*                     Artem & Alexander Amirkhanov, B. Fr�hler                        *
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
* Contact: FH O� Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstra�e 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/

#include "iABoneThicknessAttachment.h"

#include "mdichild.h"
#include "mainwindow.h"

#include<vtkCellLocator.h>
#include<vtkLineSource.h>
#include<vtkMath.h>
#include<vtkPolyData.h>
#include<vtkPolyDataMapper.h>
#include<vtkPoints.h>
#include<vtkProperty.h>

#include "iADockWidgetWrapper.h"

#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QSlider>

iABoneThicknessAttachment::iABoneThicknessAttachment(MainWindow* _pMainWnd, iAChildData _iaChildData):
	iAModuleAttachmentToChild(_pMainWnd, _iaChildData)
{
	QWidget* pBoneThicknessWidget (new QWidget());

	QPushButton* pPushButtonBoneThicknessOpen(new QPushButton("Open point file...", pBoneThicknessWidget));
	pPushButtonBoneThicknessOpen->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogOpenButton));
	pPushButtonBoneThicknessOpen->setFixedWidth(15 * pBoneThicknessWidget->logicalDpiX() / 10);
	connect(pPushButtonBoneThicknessOpen, SIGNAL(clicked()), this, SLOT(slotPushButtonBoneThicknessOpen()));

	m_pBoneThicknessTable = new iABoneThicknessTable(m_childData.child->getRaycaster(), pBoneThicknessWidget);

	QLabel* pLabelSphereRadius(new QLabel("Sphere radius:", pBoneThicknessWidget));
	QDoubleSpinBox* pDoubleSpinBoxSphereRadius(new QDoubleSpinBox(pBoneThicknessWidget));
	pDoubleSpinBoxSphereRadius->setMinimum(0.1);
	pDoubleSpinBoxSphereRadius->setSingleStep(0.1);
	pDoubleSpinBoxSphereRadius->setValue(m_pBoneThicknessTable->sphereRadius());
	connect(pDoubleSpinBoxSphereRadius, SIGNAL(valueChanged(const double&)), this, SLOT(slotDoubleSpinBoxSphereRadius(const double&)));

	QGridLayout* pBoneThicknessLayout (new QGridLayout(pBoneThicknessWidget));
	pBoneThicknessLayout->addWidget(pPushButtonBoneThicknessOpen, 0, 0);
	pBoneThicknessLayout->addWidget(m_pBoneThicknessTable, 1, 0, 1, 4);
	pBoneThicknessLayout->addWidget(pLabelSphereRadius, 2, 0);
	pBoneThicknessLayout->addWidget(pDoubleSpinBoxSphereRadius, 2, 1);

	_iaChildData.child->tabifyDockWidget(_iaChildData.logs, new iADockWidgetWrapper(pBoneThicknessWidget, tr("Bone thickness"), "BoneThickness"));
}

void iABoneThicknessAttachment::calculate()
{
	qApp->setOverrideCursor(Qt::WaitCursor);

	vtkSmartPointer<vtkCellLocator> CellLocator(vtkCellLocator::New());
	CellLocator->SetDataSet(m_childData.polyData);
	CellLocator->BuildLocator();
	CellLocator->Update();

	vtkSmartPointer<vtkPoints> Points(m_pBoneThicknessTable->point());

	const vtkIdType PointsTable (Points->GetNumberOfPoints());

	QVector<double>* pvDistance(m_pBoneThicknessTable->distance());
	QVector<double>* pvThickness(m_pBoneThicknessTable->thickness());
	
	vtkSmartPointer<vtkPoints> PointNormals(vtkPoints::New());
	addPointNormalsIn(PointNormals);

	m_pBoneThicknessTable->clearThicknessLines();

	for (int i(0); i < PointsTable ; ++i)
	{
		double Point1[3];
		Points->GetPoint(i, Point1);

		vtkIdType cellId;
		int subId;
		double closestPointDist2 (0.0);
		double PointClosest1[3];
		CellLocator->FindClosestPoint(Point1, PointClosest1, cellId, subId, closestPointDist2);

		(*pvDistance)[i] = sqrt(closestPointDist2);

		double pNormal[3];
		PointNormals->GetPoint(i, pNormal);

		const double dLength (5.0);
		double pPoint21[3] = { PointClosest1[0] + dLength * pNormal[0], PointClosest1[1] + dLength * pNormal[1], PointClosest1[2] + dLength * pNormal[2] };
		double pPoint22[3] = { PointClosest1[0] - dLength * pNormal[0], PointClosest1[1] - dLength * pNormal[1], PointClosest1[2] - dLength * pNormal[2] };

		double tol (0.0);
		double t (0.0);
		double x1[3], x2[3];
		double pcoords[3];

		vtkSmartPointer<vtkLineSource> pLine(vtkSmartPointer<vtkLineSource>::New());
		vtkSmartPointer<vtkPolyDataMapper> pMapper(vtkSmartPointer<vtkPolyDataMapper>::New());
		vtkSmartPointer<vtkActor> pActor(vtkSmartPointer<vtkActor>::New());
		pActor->GetProperty()->SetColor(0.0, 0.0, 1.0);

		CellLocator->IntersectWithLine(pPoint21, PointClosest1, tol, t, x1, pcoords, subId);
		(*pvThickness)[i] = sqrt(vtkMath::Distance2BetweenPoints(PointClosest1, x1));

		pLine->SetPoint1(PointClosest1);
		pLine->SetPoint2(x1);
		pMapper->SetInputConnection(pLine->GetOutputPort());
		pActor->SetMapper(pMapper);

		CellLocator->IntersectWithLine(pPoint22, PointClosest1, tol, t, x2, pcoords, subId);

		const double dThickness (sqrt(vtkMath::Distance2BetweenPoints(PointClosest1, x2)));

		if (dThickness > (*pvThickness)[i])
		{
			(*pvThickness)[i] = dThickness;

			pLine->SetPoint1(PointClosest1);
			pLine->SetPoint2(x2);
			pMapper->SetInputConnection(pLine->GetOutputPort());
			pActor->SetMapper(pMapper);
		}

		m_pBoneThicknessTable->addThicknessLine(pActor);
	}

	m_pBoneThicknessTable->setTable();
	m_pBoneThicknessTable->setWindow();

	qApp->restoreOverrideCursor();
}

void iABoneThicknessAttachment::addPointNormalsIn(vtkPoints* _pPointNormals)
{
	vtkPolyData* pPolyData(m_childData.polyData);
	const vtkIdType idPointsData (pPolyData->GetNumberOfPoints());

	vtkSmartPointer<vtkPoints> pPoints(m_pBoneThicknessTable->point());
	const vtkIdType idPointsTable(pPoints->GetNumberOfPoints());

	const double dPointRadius(m_pBoneThicknessTable->sphereRadius());

	QVector<vtkSmartPointer<vtkPoints>> vPoints;
	vPoints.resize(idPointsTable);

	for (vtkIdType i(0); i < idPointsTable; ++i)
	{
		vPoints[i] = vtkPoints::New();
	}

	for (vtkIdType j (0); j < idPointsData; ++j)
	{
		double pPointData[3];
		pPolyData->GetPoint(j, pPointData);

		for (vtkIdType i(0); i < idPointsTable; ++i)
		{
			double pPoint1[3];
			pPoints->GetPoint(i, pPoint1);

			const double Distance(sqrt(vtkMath::Distance2BetweenPoints(pPoint1, pPointData)));

			if (Distance < dPointRadius)
			{
				vPoints[i]->InsertNextPoint(pPointData);
			}
		}
	}

	double pNormal[3];
		
	for (vtkIdType i(0); i < idPointsTable; ++i)
	{
		getNormal(vPoints[i], pNormal);
		vtkMath::Normalize(pNormal);

		_pPointNormals->InsertNextPoint(pNormal);
	}
}

void iABoneThicknessAttachment::getNormal(vtkPoints* _pPoints, double* _pNormal)
{
	const vtkIdType idPointsArea(_pPoints->GetNumberOfPoints());

	if (idPointsArea > 0)
	{
		double pPoint1[3];
		_pPoints->GetPoint(0, pPoint1);

		double dSumX(pPoint1[0]);
		double dSumXX(pPoint1[0] * pPoint1[0]);
		double dSumXY(pPoint1[0] * pPoint1[1]);

		double dSumY(pPoint1[1]);
		double dSumYY(pPoint1[1] * pPoint1[1]);

		double dSumXZ(pPoint1[0] * pPoint1[2]);
		double dSumYZ(pPoint1[1] * pPoint1[2]);
		double dSumZ(pPoint1[2]);

		for (vtkIdType i(1); i < idPointsArea; ++i)
		{
			_pPoints->GetPoint(i, pPoint1);

			dSumX += pPoint1[0];
			dSumXX += pPoint1[0] * pPoint1[0];
			dSumXY += pPoint1[0] * pPoint1[1];
			dSumXZ += pPoint1[0] * pPoint1[2];

			dSumY += pPoint1[1];
			dSumYY += pPoint1[1] * pPoint1[1];
			dSumYZ += pPoint1[1] * pPoint1[2];

			dSumZ += pPoint1[2];
		}

		const double AB(dSumXX * dSumYY - dSumXY * dSumXY);

		_pNormal[2] = (dSumXZ * dSumYY - dSumYZ * dSumXY) / AB; // A
		_pNormal[0] = (dSumXX * dSumXZ - dSumXZ * dSumXY) / AB; // B
		_pNormal[1] = (dSumZ - _pNormal[0] * dSumX - _pNormal[1] * dSumY) / ((double)idPointsArea); // C
	}
	else
	{
		_pNormal[0] = _pNormal[1] = _pNormal[2] = 0.0;
	}
}

void iABoneThicknessAttachment::slotDoubleSpinBoxSphereRadius(const double& _dValue)
{
	m_pBoneThicknessTable->setSphereRadius(_dValue);
	calculate();
}

void iABoneThicknessAttachment::slotPushButtonBoneThicknessOpen()
{
	QPushButton* pPushButtonOpen ((QPushButton*) sender());

	QFileDialog* pFileDialog (new QFileDialog());
	pFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
	pFileDialog->setDefaultSuffix("txt");
	pFileDialog->setFileMode(QFileDialog::ExistingFile);
	pFileDialog->setNameFilter("Point file (*.txt)");
	pFileDialog->setWindowTitle(pPushButtonOpen->text());

	if (pFileDialog->exec())
	{
		m_pBoneThicknessTable->open(pFileDialog->selectedFiles().first());
		calculate();
	}

	delete pFileDialog;
}
