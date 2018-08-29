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
 
#include "dlg_TripleHistogramTF.h"

#include <qsplitter.h>

#include "dlg_modalities.h"
#include "iAModalityList.h"
#include "iARenderer.h"
#include "iASlicerData.h"
#include "iABarycentricHelperRenderer.h"

#include <vtkImageData.h>

// Debug
#include "qdebug.h"

const static QString DEFAULT_LABELS[3] = { "A", "B", "C" };

dlg_TripleHistogramTF::dlg_TripleHistogramTF(MdiChild * mdiChild /*= 0*/, Qt::WindowFlags f /*= 0 */) :
	//TripleHistogramTFConnector(mdiChild, f), m_mdiChild(mdiChild)
	QDockWidget("Triple Histogram Transfer Function", mdiChild, f),
	m_mdiChild(mdiChild)
{

	// Initialize dock widget
	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetVerticalTitleBar);

	// Set up connects
	// ...

	//-----------------------------------------------
	// Test vvv // TODO: remove comments
	resize(779, 501);

	//QWidget *dockWidgetContents = new QWidget();
	QSplitter *dockWidgetContents = new QSplitter(Qt::Horizontal);
	setWidget(dockWidgetContents);
	dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));

	//RightBorderLayout *mainLayout = new RightBorderLayout(dockWidgetContents, RightBorderLayout::Right);
	//m_gridLayout = new QHBoxLayout(dockWidgetContents);
	//m_gridLayout->setSpacing(0);
	//m_gridLayout->setObjectName(QStringLiteral("horizontalLayout_2"));
	//m_gridLayout->setContentsMargins(0, 0, 0, 0);

	QWidget *optionsContainer = new QWidget();
	optionsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	// Test ^^^
	//-----------------------------------------------

	m_triangleRenderer = new iABarycentricHelperRenderer();
	m_triangleWidget = new iABarycentricTriangleWidget(dockWidgetContents);
	m_triangleWidget->setModality1label(DEFAULT_LABELS[0]);
	m_triangleWidget->setModality2label(DEFAULT_LABELS[1]);
	m_triangleWidget->setModality3label(DEFAULT_LABELS[2]);
	m_triangleWidget->setTriangleRenderer(m_triangleRenderer);

	m_slicerModeComboBox = new QComboBox(optionsContainer);
	m_slicerModeComboBox->addItem("YZ", QVariant(iASlicerMode::YZ));
	m_slicerModeComboBox->addItem("XY", QVariant(iASlicerMode::XY));
	m_slicerModeComboBox->addItem("XZ", QVariant(iASlicerMode::XZ));

	m_sliceSlider = new QSlider(Qt::Horizontal, optionsContainer);
	m_sliceSlider->setMinimum(0);

	QHBoxLayout *optionsContainerLayout = new QHBoxLayout(optionsContainer);
	optionsContainerLayout->addWidget(m_slicerModeComboBox);
	optionsContainerLayout->addWidget(m_sliceSlider);

	m_histogramStack = new iAHistogramStack(dockWidgetContents, mdiChild);
	m_histogramStack->setModalityLabel(DEFAULT_LABELS[0], 0);
	m_histogramStack->setModalityLabel(DEFAULT_LABELS[1], 1);
	m_histogramStack->setModalityLabel(DEFAULT_LABELS[2], 2);

	QWidget *leftWidget = new QWidget();
	QVBoxLayout *leftWidgetLayout = new QVBoxLayout(leftWidget);
	leftWidgetLayout->addWidget(optionsContainer);
	leftWidgetLayout->addWidget(m_histogramStack);

	dockWidgetContents->addWidget(leftWidget);
	dockWidgetContents->addWidget(m_triangleWidget);

	//m_mdiChild->getRaycaster()->setTransferFunction(m_transferFunction);
	//m_mdiChild->getSlicerXY()->setTransferFunction(m_transferFunction);
	//...

	updateSlicerMode();
	setWeight(m_triangleWidget->getControlPointCoordinates());

	// Does not work. TODO: fix
	/*mdiChild->getSlicerXY()->reInitialize(
		mdiChild->getImageData(),
		vtkTransform::New(), // no way of getting current transform, so create a new one // TODO: fix?
		m_histogramStack->getTransferFunction()); // here is where the magic happens ;)*/

	//Connect
	connect(m_triangleWidget, SIGNAL(weightChanged(BCoord)), this, SLOT(setWeight(BCoord)));
	connect(m_slicerModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSlicerMode()));
	connect(m_sliceSlider, SIGNAL(valueChanged(int)), this, SLOT(setSliceNumber(int)));
	// TODO: move into for-loop?
	// {
	// TODO: necessary?
	//     {
	connect(m_histogramStack, SIGNAL(transferFunctionChanged()), this, SLOT(updateTransferFunction()));
	connect(m_histogramStack, SIGNAL(modalitiesChanged(QSharedPointer<iAModality> modality1, QSharedPointer<iAModality> modality2, QSharedPointer<iAModality> modality3)), this, SLOT(modalityAddedToStack(QSharedPointer<iAModality> modality1, QSharedPointer<iAModality> modality2, QSharedPointer<iAModality> modality3)));
	//     }
	//connect(this, SIGNAL(transferFunctionUpdated()), m_mdiChild, SLOT(ModalityTFChanged()));

	connect(mdiChild->GetModalitiesDlg(), SIGNAL(ModalityAvailable(int)), this, SLOT(modalityAvailable(int)));
	connect(mdiChild->GetModalitiesDlg(), SIGNAL(ModalitySelected(int)), this, SLOT(modalitySelected(int)));
	connect(mdiChild->GetModalitiesDlg(), SIGNAL(ModalitiesChanged()), this, SLOT(modalitiesChanged()));
	// }
}

dlg_TripleHistogramTF::~dlg_TripleHistogramTF()
{
	delete m_triangleRenderer;
}

void dlg_TripleHistogramTF::updateSlicerMode()
{
	setSlicerMode((iASlicerMode)m_slicerModeComboBox->currentData().toInt());
}

void dlg_TripleHistogramTF::setWeight(BCoord bCoord)
{
	m_histogramStack->setWeight(bCoord);
}

void dlg_TripleHistogramTF::setSlicerMode(iASlicerMode slicerMode)
{
	int dimensionIndex;

	switch (slicerMode)
	{
	case iASlicerMode::YZ:
		dimensionIndex = 0; // X length is in position 0 in the dimensions array
		break;
	case iASlicerMode::XZ:
		dimensionIndex = 1; // Y length is in position 1 in the dimensions array
		break;
	case iASlicerMode::XY:
		dimensionIndex = 2; // Z length is in position 2 in the dimensions array
		break;
	default:
		// TODO?
		return;
	}

	int dimensionLength = m_mdiChild->getImageData()->GetDimensions()[dimensionIndex];
	m_sliceSlider->setMaximum(dimensionLength - 1);
	m_sliceSlider->setValue(dimensionLength / 2);

	m_histogramStack->setSlicerMode(slicerMode, dimensionLength);
}

void dlg_TripleHistogramTF::setSliceNumber(int sliceNumber)
{
	m_histogramStack->setSliceNumber(sliceNumber);
}

void dlg_TripleHistogramTF::updateTransferFunction()
{
	/*for (int i = 0; i < m_histogramStack->modalitiesCount(); ++i) {
		iAChannelID id = static_cast<iAChannelID>(ch_Meta0 + i);
		m_mdiChild->UpdateChannelSlicerOpacity(id, m_histogramStack->getWeight(i));
		m_mdiChild->getSlicerDataXY()->updateChannelMappers();
		m_mdiChild->getSlicerDataXZ()->updateChannelMappers();
		m_mdiChild->getSlicerDataYZ()->updateChannelMappers();
		m_mdiChild->updateSlicers();
	}*/

	m_mdiChild->redrawHistogram();
	m_mdiChild->getRenderer()->update();
	//m_mdiChild->renderer->update();
}

void dlg_TripleHistogramTF::modalityAvailable(int modalityIdx)
{

}

void dlg_TripleHistogramTF::modalitySelected(int modalityIdx)
{

}

void dlg_TripleHistogramTF::modalitiesChanged()
{
	m_histogramStack->updateModalities();
	if (m_histogramStack->modalitiesCount() >= 3) {
		modalitiesChanged(m_histogramStack->getModality(0), m_histogramStack->getModality(1), m_histogramStack->getModality(2));
	}
}

void dlg_TripleHistogramTF::modalitiesChanged(QSharedPointer<iAModality> modality1, QSharedPointer<iAModality> modality2, QSharedPointer<iAModality> modality3)
{
	QString name;

	m_triangleWidget->setModalities(modality1->GetImage(), modality2->GetImage(), modality3->GetImage());

	m_triangleWidget->update();
}