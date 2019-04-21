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

#include "tf_3mod/BCoord.h"

#include "iASimpleSlicerWidget.h"

#include <charts/iADiagramFctWidget.h>
#include <iATransferFunction.h>

#include <vtkSmartPointer.h>

#include <QSlider>
#include <QComboBox>
#include <QWidget>
#include <QVector>
#include <QSharedPointer>

class MdiChild;

class vtkColorTransferFunction;
class vtkPiecewiseFunction;

class QLabel;

class iAMultimodalWidget : public QWidget {
	Q_OBJECT

// Private methods used by public/protected
private:
	double bCoord_to_t(BCoord bCoord) { return bCoord[1]; }
	BCoord t_to_bCoord(double t) { return BCoord(0, t); }
	void setWeights(BCoord bCoord, double t);

public:
	enum NumberOfModalities {
		UNDEFINED = -1,
		TWO = 2,
		THREE = 3
	};

	iAMultimodalWidget(QWidget *parent, MdiChild* mdiChild, NumberOfModalities num);

	QSharedPointer<iADiagramFctWidget> w_histogram(int i) {
		return m_histograms[i];
	}

	QSharedPointer<iASimpleSlicerWidget> w_slicer(int i) {
		return m_slicerWidgets[i];
	}

	QComboBox* w_slicerModeComboBox() {
		return m_slicerModeComboBox;
	}

	QSlider* w_sliceNumberSlider() {
		return m_sliceSlider;
	}

	void setWeights(BCoord bCoord) {
		setWeights(bCoord, bCoord_to_t(bCoord));
	}
	void setWeights(double t) {
		setWeights(t_to_bCoord(t), t);
	}
	BCoord getWeights();
	double getWeight(int i);

	bool setSlicerMode(iASlicerMode slicerMode);
	iASlicerMode getSlicerMode();
	iASlicerMode getSlicerModeAt(int comboBoxIndex);

	bool setSliceNumber(int sliceNumber);
	int getSliceNumber();

	QSharedPointer<iAModality> getModality(int index);
	int getModalitiesCount();
	bool containsModality(QSharedPointer<iAModality> modality);
	void updateModalities();

	bool isReady();

	void updateTransferFunction(int index);

protected:
	void setWeightsProtected(BCoord bCoord, double t);
	void setSlicerModeProtected(iASlicerMode slicerMode);
	void setSliceNumberProtected(int sliceNumber);

	void resetSlicers();
	void resetSlicer(int i);

	void setWeightsProtected(double t) {
		setWeightsProtected(t_to_bCoord(t), t);
	}

	void setWeightsProtected(BCoord bCoord) {
		setWeightsProtected(bCoord, bCoord_to_t(bCoord));
	}

	// TODO: another pointer to MdiChild... is this really optimal?
	MdiChild *m_mdiChild;

private:
	// User interface {
	QVector<QSharedPointer<iADiagramFctWidget>> m_histograms;
	QVector<QSharedPointer<iASimpleSlicerWidget>> m_slicerWidgets;
	QComboBox *m_slicerModeComboBox;
	QSlider *m_sliceSlider;

	QLabel *m_disabledLabel;
	// }

	void updateScrollBars(int newValue);
	void updateCopyTransferFunction(int index);
	void updateOriginalTransferFunction(int index);
	void applyWeights();

	BCoord m_weights;

	NumberOfModalities m_numOfMod = UNDEFINED;
	QVector<QSharedPointer<iAModality>> m_modalitiesActive;

	// Background stuff
	QVector<QSharedPointer<iATransferFunction>> m_copyTFs;
	QSharedPointer<iATransferFunction> createCopyTf(int index, vtkSmartPointer<vtkColorTransferFunction> colorTf, vtkSmartPointer<vtkPiecewiseFunction> opacity);

signals:
	void transferFunctionChanged();

	void weightsChanged3(BCoord weights);
	void weightsChanged2(double t);
	void slicerModeChanged(iASlicerMode slicerMode);
	void sliceNumberChanged(int sliceNumber);

	void slicerModeChangedExternally(iASlicerMode slicerMode);
	void sliceNumberChangedExternally(int sliceNumber);

	void modalitiesLoaded_beforeUpdate();

private slots:
	void originalHistogramChanged();

	void slicerModeComboBoxIndexChanged(int newIndex);
	void sliderValueChanged(int newValue);

	void setSliceXYScrollBar();
	void setSliceXZScrollBar();
	void setSliceYZScrollBar();
	void setSliceXYScrollBar(int sliceNumberXY);
	void setSliceXZScrollBar(int sliceNumberXZ);
	void setSliceYZScrollBar(int sliceNumberYZ);

};