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

#include "open_iA_Core_export.h"

#include "iAAlgorithm.h"

#include <QMap>
#include <QSharedPointer>

class iAFilter;
class MainWindow;
class MdiChild;

//! GUI Runner for descendants of iAFilter
//!
//! Used in RunFilter (see below) to run a descendant of iAFilter inside its
//! own thread
class open_iA_Core_API iAFilterRunnerGUI : public iAAlgorithm
{
	Q_OBJECT
public:
	iAFilterRunnerGUI(QSharedPointer<iAFilter> filter, QMap<QString, QVariant> paramValues, MdiChild* mdiChild);
	void performWork();
	QSharedPointer<iAFilter> Filter();
private:
	QSharedPointer<iAFilter> m_filter;
	QMap<QString, QVariant> m_paramValues;
signals:
	void workDone();
};

//! For the given descendant of iAFilter, this method loads its settings from
//! the platform-specific settings store (Registry under Windows, .config
//! folder under Unix, ...).
//! Then it shows a dialog to the user to change these parameters.
//! Afterwards it checks the parameters with the given filter.
//! If they are ok, it stores them back to the settings store.
//! Subsequently it creates a thread for the given filter, assigns the slots
//! required for progress indication, final display and cleanup, and finally
//! it runs the filter with the parameters.
open_iA_Core_API iAFilterRunnerGUI* RunFilter(QSharedPointer<iAFilter> filter, MainWindow* mainWnd);
