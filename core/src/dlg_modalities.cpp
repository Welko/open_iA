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
#include "dlg_modalities.h"

#include "dlg_commoninput.h"
#include "dlg_modalityProperties.h"
#include "dlg_transfer.h"
#include "iAConsole.h"
#include "iAFast3DMagicLensWidget.h"
#include "iAModality.h"
#include "iAModalityList.h"
#include "iAModalityTransfer.h"
#include "iARenderer.h"
#include "iASlicer.h"
#include "iASlicerData.h"
#include "iAVolumeRenderer.h"
#include "io/iAIO.h"
#include "io/iAIOProvider.h"
#include "io/extension2id.h"
#include "mainwindow.h"

#include <QVTKInteractor.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>

#include <QFileDialog>
#include <QSettings>

#include <cassert>

dlg_modalities::dlg_modalities(iAFast3DMagicLensWidget* magicLensWidget,
	vtkRenderer* mainRenderer, int numBin) :

	modalities(new iAModalityList),
	m_magicLensWidget(magicLensWidget),
	m_mainRenderer(mainRenderer),
	m_showSlicers(false),
	m_plane1(nullptr),
	m_plane2(nullptr),
	m_plane3(nullptr)
{
	connect(pbAdd,    SIGNAL(clicked()), this, SLOT(AddClicked()));
	connect(pbRemove, SIGNAL(clicked()), this, SLOT(RemoveClicked()));
	connect(pbEdit,   SIGNAL(clicked()), this, SLOT(EditClicked()));
	connect(cbManualRegistration, SIGNAL(clicked()), this, SLOT(ManualRegistration()));
	connect(cbShowMagicLens, SIGNAL(clicked()), this, SLOT(MagicLens()));

	connect(lwModalities, SIGNAL(itemClicked(QListWidgetItem*)),
		this, SLOT(ListClicked(QListWidgetItem*)));

	connect(lwModalities, SIGNAL(itemChanged(QListWidgetItem*)),
		this, SLOT(ShowChecked(QListWidgetItem*)));

	connect(magicLensWidget, SIGNAL(MouseMoved()), this, SLOT(RendererMouseMoved()));
}

void dlg_modalities::SetModalities(QSharedPointer<iAModalityList> modList)
{
	modalities = modList;
	lwModalities->clear();
}

void dlg_modalities::SelectRow(int idx)
{
	lwModalities->setCurrentRow(idx);
}

QString GetCaption(iAModality const & mod)
{
	QFileInfo fi(mod.GetFileName());
	return mod.GetName()+" ("+fi.fileName()+")";
}

bool CanHaveMultipleChannels(QString const & fileName)
{
	return fileName.endsWith(iAIO::VolstackExtension) || fileName.endsWith(".oif");
}

void dlg_modalities::AddClicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load"),
		"",
		iAIOProvider::GetSupportedLoadFormats() + tr("Volume Stack (*.volstack);;"));
	if (fileName.isEmpty())
		return;

	const int DefaultRenderFlags = iAModality::MainRenderer;
	bool split = false;
	if (CanHaveMultipleChannels(fileName))
	{
		QStringList inList;
		inList << tr("$Split Channels");
		QList<QVariant> inPara;
		inPara << tr("%1").arg(true);
		QTextDocument * descr = new QTextDocument;
		descr->setHtml("Input file potentially has multiple channels. "
			"Should they be split into separate datasets, "
			"or kept as one dataset with multiple components ?");
		dlg_commoninput splitInput(this, "Seed File Format", inList, inPara, descr);
		if (splitInput.exec() != QDialog::Accepted)
		{
			DEBUG_LOG("Aborted by user.");
			return;
		}
		split = splitInput.getCheckValue(0);
	}
	ModalityCollection mods = iAModalityList::Load(fileName, "", -1, split, DefaultRenderFlags);
	for (auto mod : mods)
	{
		modalities->Add(mod);
	}
	emit ModalitiesChanged();
}

void dlg_modalities::MagicLens()
{
	if (cbShowMagicLens->isChecked())
	{
		m_magicLensWidget->magicLensOn();
	}
	else
	{
		m_magicLensWidget->magicLensOff();
	}
}

void dlg_modalities::InitDisplay(QSharedPointer<iAModality> mod)
{
	QSharedPointer<iAVolumeRenderer> renderer(new iAVolumeRenderer(mod->GetTransfer().data(), mod->GetImage()));
	mod->SetRenderer(renderer);
	if (mod->hasRenderFlag(iAModality::MainRenderer))
	{
		renderer->AddTo(m_mainRenderer);
	}
	if (mod->hasRenderFlag(iAModality::BoundingBox))
	{
		renderer->AddBoundingBoxTo(m_mainRenderer);
	}
	if (mod->hasRenderFlag(iAModality::MagicLens))
	{
		renderer->AddTo(m_magicLensWidget->getLensRenderer());
	}
}

void dlg_modalities::AddToList(QSharedPointer<iAModality> mod)
{
	QListWidgetItem* listItem = new QListWidgetItem(GetCaption(*mod));
	lwModalities->addItem(listItem);
	lwModalities->setCurrentItem(listItem);
	listItem->setFlags(listItem->flags() | Qt::ItemIsUserCheckable);
	listItem->setCheckState(Qt::Checked);
}

void dlg_modalities::AddListItem(QSharedPointer<iAModality> mod)
{
	AddToList(mod);
	EnableButtons();
}

void dlg_modalities::ModalityAdded(QSharedPointer<iAModality> mod)
{
	AddListItem(mod);
	InitDisplay(mod);
	emit ModalityAvailable(lwModalities->count()-1);
}

void dlg_modalities::InteractorModeSwitched(int newMode)
{
	cbManualRegistration->setChecked(newMode == 'a');
}

void dlg_modalities::EnableUI()
{
	pbAdd->setEnabled(true);
	cbManualRegistration->setEnabled(true);
	cbShowMagicLens->setEnabled(true);
}

void dlg_modalities::RemoveClicked()
{
	int idx = lwModalities->currentRow();
	if (idx < 0 || idx >= modalities->size())
	{
		DEBUG_LOG(QString("Index out of range (%1)").arg(idx));
		return;
	}
	QSharedPointer<iAVolumeRenderer> renderer = modalities->Get(idx)->GetRenderer();
	if (modalities->Get(idx)->hasRenderFlag(iAModality::MainRenderer) ||
		modalities->Get(idx)->hasRenderFlag(iAModality::MagicLens))
	{
		renderer->Remove();
	}
	if (modalities->Get(idx)->hasRenderFlag(iAModality::BoundingBox))
	{
		renderer->RemoveBoundingBox();
	}
	modalities->Remove(idx);
	delete lwModalities->takeItem(idx);
	EnableButtons();

	m_mainRenderer->GetRenderWindow()->Render();
	emit ModalitiesChanged();
}

void dlg_modalities::EditClicked()
{
	int idx = lwModalities->currentRow();
	if (idx < 0 || idx >= modalities->size())
	{
		DEBUG_LOG(QString("Index out of range (%1).").arg(idx));
		return;
	}
	int renderFlagsBefore = modalities->Get(idx)->RenderFlags();
	QSharedPointer<iAModality> editModality(modalities->Get(idx));
	if (!editModality->GetRenderer())
	{
		DEBUG_LOG(QString("Volume renderer not yet initialized, please wait..."));
		return;
	}
	dlg_modalityProperties prop(this, editModality);
	if (prop.exec() == QDialog::Rejected)
	{
		return;
	}
	QSharedPointer<iAVolumeRenderer> renderer = modalities->Get(idx)->GetRenderer();
	if (!renderer)
	{
		return;
	}
	if ((renderFlagsBefore & iAModality::MainRenderer) == iAModality::MainRenderer
		&& !editModality->hasRenderFlag(iAModality::MainRenderer))
	{
		renderer->Remove();
	}
	if ((renderFlagsBefore & iAModality::MainRenderer) == 0
		&& editModality->hasRenderFlag(iAModality::MainRenderer))
	{
		renderer->AddTo(m_mainRenderer);
	}
	if ((renderFlagsBefore & iAModality::BoundingBox) == iAModality::BoundingBox
		&& !editModality->hasRenderFlag(iAModality::BoundingBox))
	{
		renderer->RemoveBoundingBox();
	}
	if ((renderFlagsBefore & iAModality::BoundingBox) == 0
		&& editModality->hasRenderFlag(iAModality::BoundingBox))
	{
		renderer->AddBoundingBoxTo(m_mainRenderer);
	}
	if ((renderFlagsBefore & iAModality::MagicLens) == iAModality::MagicLens
		&& !editModality->hasRenderFlag(iAModality::MagicLens))
	{
		renderer->Remove();
	}
	if ((renderFlagsBefore & iAModality::MagicLens) == 0
		&& editModality->hasRenderFlag(iAModality::MagicLens))
	{
		renderer->AddTo(m_magicLensWidget->getLensRenderer());
	}
	lwModalities->item(idx)->setText(GetCaption(*editModality));
	emit ModalitiesChanged();
}

void dlg_modalities::EnableButtons()
{
	bool enable = modalities->size() > 0;
	pbEdit->setEnabled(enable);
	pbRemove->setEnabled(enable);
}

void dlg_modalities::ManualRegistration()
{
	vtkInteractorStyleSwitch* interactSwitch = dynamic_cast<vtkInteractorStyleSwitch*>(m_magicLensWidget->GetInteractor()->GetInteractorStyle());
	if (!interactSwitch)
	{
		return;
	}
	if (cbManualRegistration->isChecked())
	{
		interactSwitch->SetCurrentStyleToTrackballActor();
	}
	else
	{
		interactSwitch->SetCurrentStyleToTrackballCamera();
	}
}

void dlg_modalities::ListClicked(QListWidgetItem* item)
{
	int selectedRow = lwModalities->row( item );
	if (selectedRow < 0)
	{
		return;
	}
	QSharedPointer<iAModality> currentData = modalities->Get(selectedRow);
	QSharedPointer<iAModalityTransfer> modTransfer = currentData->GetTransfer();
	for (int i = 0; i<modalities->size(); ++i)
	{
		QSharedPointer<iAModality> mod = modalities->Get(i);
		if (!mod->GetRenderer())
		{
			DEBUG_LOG(QString("Renderer for modality %1 not yet created. Please try again later!").arg(i));
			continue;
		}
		mod->GetRenderer()->SetMovable(mod == currentData);
	}
	emit ModalitySelected(selectedRow);
}

void dlg_modalities::ShowChecked(QListWidgetItem* item)
{
	int i = lwModalities->row(item);
	QSharedPointer<iAVolumeRenderer> renderer = modalities->Get(i)->GetRenderer();
	if (!renderer)
	{
		return;
	}
	bool isChecked = item->checkState() == Qt::Checked;
	renderer->ShowVolume(isChecked);
	m_mainRenderer->GetRenderWindow()->Render();
}

QSharedPointer<iAModalityList const> dlg_modalities::GetModalities() const
{
	return modalities;
}

QSharedPointer<iAModalityList> dlg_modalities::GetModalities()
{
	return modalities;
}

int dlg_modalities::GetSelected() const
{
	return lwModalities->currentRow();
}

vtkSmartPointer<vtkColorTransferFunction> dlg_modalities::GetCTF(int modality)
{
	return modalities->Get(modality)->GetTransfer()->getColorFunction();
}

vtkSmartPointer<vtkPiecewiseFunction> dlg_modalities::GetOTF(int modality)
{
	return modalities->Get(modality)->GetTransfer()->getOpacityFunction();
}

void dlg_modalities::ChangeRenderSettings(iAVolumeSettings const & rs, const bool loadSavedVolumeSettings)
{
	for (int i = 0; i < modalities->size(); ++i)
	{
		QSharedPointer<iAVolumeRenderer> renderer = modalities->Get(i)->GetRenderer();
		if (!renderer)
		{
			DEBUG_LOG("ChangeRenderSettings: No Renderer set!");
			return;
		}
		//load volume settings from file otherwise use default rs
		//check if a volume setting is saved for a modality
		//set saved status to false after loading
		if (loadSavedVolumeSettings  &&
			modalities->Get(i)->getVolSettingsSavedStatus())
		{
			renderer->ApplySettings(modalities->Get(i)->getVolumeSettings());
			modalities->Get(i)->setVolSettingsSavedStatusFalse();
		}
		//use default settings
		else
		{
			renderer->ApplySettings(rs);
		}
	}
}

void dlg_modalities::RendererMouseMoved()
{
	for (int i = 0; i < modalities->size(); ++i)
	{
		if (!modalities->Get(i)->GetRenderer())
		{
			return;
		}
		modalities->Get(i)->GetRenderer()->UpdateBoundingBox();
	}
}

void dlg_modalities::ShowSlicers(bool enabled)
{
	m_showSlicers = enabled;
	for (int i = 0; i < modalities->size(); ++i)
	{
		QSharedPointer<iAVolumeRenderer> renderer = modalities->Get(i)->GetRenderer();
		if (!renderer)
		{
			DEBUG_LOG("ShowSlicePlanes: No Renderer set!");
			return;
		}
		if (enabled)
		{
			renderer->SetCuttingPlanes(m_plane1, m_plane2, m_plane3);
		}
		else
		{
			renderer->RemoveCuttingPlanes();
		}
	}
}

void dlg_modalities::SetSlicePlanes(vtkPlane* plane1, vtkPlane* plane2, vtkPlane* plane3)
{
	m_plane1 = plane1;
	m_plane2 = plane2;
	m_plane3 = plane3;
}

void dlg_modalities::AddModality(vtkSmartPointer<vtkImageData> img, QString const & name)
{
	QSharedPointer<iAModality> newModality(new iAModality(name, "", -1, img, iAModality::MainRenderer));
	modalities->Add(newModality);
}

void dlg_modalities::SetFileName(int modality, QString const & fileName)
{
	modalities->Get(modality)->SetFileName(fileName);
	lwModalities->item(modality)->setText(GetCaption(*modalities->Get(modality).data()));
}
