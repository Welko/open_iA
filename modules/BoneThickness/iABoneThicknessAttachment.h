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

#include "iABoneThickness.h"

#include <iAModuleAttachmentToChild.h>

#include <QDoubleSpinBox>
#include <QScopedPointer>

class iABoneThicknessChartBar;
class iABoneThicknessTable;

class iABoneThicknessAttachment : public iAModuleAttachmentToChild
{
	Q_OBJECT

	public:
		iABoneThicknessAttachment(MainWindow* _pMainWnd, iAChildData _iaChildData);

	private:
		iABoneThicknessTable* m_pBoneThicknessTable = nullptr;
		iABoneThicknessChartBar* m_pBoneThicknessChartBar = nullptr;

		QDoubleSpinBox* m_pDoubleSpinBoxSphereRadius = nullptr;
		QDoubleSpinBox* m_pDoubleSpinBoxThicknessMaximum = nullptr;

		QScopedPointer<iABoneThickness> m_pBoneThickness;

	private slots:
	    void slotDoubleSpinBoxSphereRadius();
		void slotDoubleSpinBoxThicknessMaximum();
		void slotPushButtonOpen();
		void slotPushButtonSave();
		void slotCheckBoxShowThickness(const bool& _bChecked);
		void slotCheckBoxTransparency(const bool& _bChecked);
};
