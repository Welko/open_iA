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

#include <QWidget>
#include <QListWidget>
#include <QMap>
#include <QSharedPointer>

struct iANModalLabelAbstract;

class QGridLayout;
class QLabel;
class QSlider;

class iANModalLabelControls : public QWidget {
	Q_OBJECT

public:
	iANModalLabelControls(QWidget *parent = nullptr);

	void updateTable(QList<QSharedPointer<iANModalLabelAbstract>>);
	void insertLabel(int row, QSharedPointer<iANModalLabelAbstract>, float opacity);
	void removeLabel(int row);
	bool containsLabel(int row);

	float opacity(int labelId);

private:

	enum Column {
		NAME = 0,
		COLOR = 1,
		NUMBER = 2,
		OPACITY = 3
	};

	struct Row {
		Row() {}
		Row(int _row, QLabel *_name, QLabel *_color, QLabel *_number, QSlider *_opacity) : 
			row(_row), name(_name), color(_color), number(_number), opacity(_opacity)
		{}
		int row = -1;
		QLabel *name = nullptr;
		QLabel *color = nullptr;
		QLabel *number = nullptr;
		QSlider *opacity = nullptr;
	};

	QGridLayout *m_layout;
	QVector<QSharedPointer<iANModalLabelAbstract>> m_labels;
	QVector<Row> m_rows;

	int m_nextId = 0;

	void addRow(int row, QSharedPointer<iANModalLabelAbstract>, float opacity);
	void updateRow(int row, QSharedPointer<iANModalLabelAbstract>);

signals:
	void labelOpacityChanged(int labelId);

};