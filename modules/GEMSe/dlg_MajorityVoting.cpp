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
#include "pch.h"
#include "dlg_MajorityVoting.h"

#include "dlg_GEMSe.h"
#include "iAColorTheme.h"
#include "iAConsole.h"
#include "iADockWidgetWrapper.h"
#include "iAImageTreeNode.h"
#include "iAIOProvider.h"
#include "iALookupTable.h"
#include "iAQSplom.h"
#include "iASingleResult.h"
#include "mdichild.h"

#include "ParametrizableLabelVotingImageFilter.h"
#include "FilteringLabelOverlapMeasuresImageFilter.h"

#include <itkLabelStatisticsImageFilter.h>

#include <vtkAxis.h>
#include <vtkChartXY.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkFloatArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPlot.h>
#include <vtkPlotLine.h>
#include <vtkTable.h>
#include <QVTKWidget2.h>

#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>

dlg_MajorityVoting::dlg_MajorityVoting(MdiChild* mdiChild, dlg_GEMSe* dlgGEMSe, int labelCount, QString const & folder) :
	m_mdiChild(mdiChild),
	m_dlgGEMSe(dlgGEMSe),
	m_labelCount(labelCount),
	m_chartDiceVsUndec(vtkSmartPointer<vtkChartXY>::New()),
	m_chartValueVsDice(vtkSmartPointer<vtkChartXY>::New()),
	m_chartValueVsUndec(vtkSmartPointer<vtkChartXY>::New()),
	m_folder(folder)
{
	QString defaultTheme("Brewer Set3 (max. 12)");
	m_colorTheme = iAColorThemeManager::GetInstance().GetTheme(defaultTheme);

	auto vtkWidget = new QVTKWidget2();
	auto contextView = vtkSmartPointer<vtkContextView>::New();
	contextView->SetRenderWindow(vtkWidget->GetRenderWindow());
	m_chartDiceVsUndec->SetSelectionMode(vtkContextScene::SELECTION_NONE);
	auto xAxis1 = m_chartDiceVsUndec->GetAxis(vtkAxis::BOTTOM);
	auto yAxis1 = m_chartDiceVsUndec->GetAxis(vtkAxis::LEFT);
	xAxis1->SetTitle("Undecided Pixels");
	xAxis1->SetLogScale(false);
	yAxis1->SetTitle("Mean Dice");
	yAxis1->SetLogScale(false);
	contextView->GetScene()->AddItem(m_chartDiceVsUndec);
	iADockWidgetWrapper * w(new iADockWidgetWrapper(vtkWidget, "Mean Dice vs. Undecided", "ChartDiceVsUndec"));
	mdiChild->splitDockWidget(this, w, Qt::Vertical);

	auto vtkWidget2 = new QVTKWidget2();
	auto contextView2 = vtkSmartPointer<vtkContextView>::New();
	contextView2->SetRenderWindow(vtkWidget2->GetRenderWindow());
	m_chartValueVsDice->SetSelectionMode(vtkContextScene::SELECTION_NONE);
	auto xAxis2 = m_chartValueVsDice->GetAxis(vtkAxis::BOTTOM);
	auto yAxis2 = m_chartValueVsDice->GetAxis(vtkAxis::LEFT);
	xAxis2->SetTitle("Value");
	xAxis2->SetLogScale(false);
	yAxis2->SetTitle("Mean Dice");
	yAxis2->SetLogScale(false);
	contextView2->GetScene()->AddItem(m_chartValueVsDice);
	iADockWidgetWrapper * w2(new iADockWidgetWrapper(vtkWidget2, "Value vs. Dice", "ChartValueVsDice"));
	mdiChild->splitDockWidget(this, w2, Qt::Vertical);

	auto vtkWidget3 = new QVTKWidget2();
	auto contextView3 = vtkSmartPointer<vtkContextView>::New();
	contextView3->SetRenderWindow(vtkWidget3->GetRenderWindow());
	m_chartValueVsUndec->SetSelectionMode(vtkContextScene::SELECTION_NONE);
	auto xAxis3 = m_chartValueVsUndec->GetAxis(vtkAxis::BOTTOM);
	auto yAxis3 = m_chartValueVsUndec->GetAxis(vtkAxis::LEFT);
	xAxis3->SetTitle("Value");
	xAxis3->SetLogScale(false);
	yAxis3->SetTitle("Undecided Pixels");
	yAxis3->SetLogScale(false);
	contextView3->GetScene()->AddItem(m_chartValueVsUndec);
	iADockWidgetWrapper * w3(new iADockWidgetWrapper(vtkWidget3, "Value vs. Undecided", "ChartValueVsUndec"));
	mdiChild->splitDockWidget(this, w3, Qt::Vertical);

	QSharedPointer<iAImageTreeNode> root = dlgGEMSe->GetRoot();
	int ensembleSize = root->GetClusterSize();
	slMinRatio->setMaximum(ensembleSize*100);
	slLabelVoters->setMaximum(ensembleSize);

	connect(pbSample, SIGNAL(clicked()), this, SLOT(Sample()));
	connect(pbMinAbsPercent_Plot, SIGNAL(clicked()), this, SLOT(MinAbsPlot()));
	connect(pbMinDiffPercent_Plot, SIGNAL(clicked()), this, SLOT(MinDiffPlot()));
	connect(pbMinRatio_Plot, SIGNAL(clicked()), this, SLOT(RatioPlot()));
	connect(pbMaxPixelEntropy_Plot, SIGNAL(clicked()), this, SLOT(MaxPixelEntropyPlot()));
	connect(pbClusterUncertaintyDice, SIGNAL(clicked()), this, SLOT(ClusterUncertaintyDice()));
	connect(pbStore, SIGNAL(clicked()), this, SLOT(StoreResult()));
	connect(pbStoreConfig, SIGNAL(clicked()), this, SLOT(StoreConfig()));
	connect(slAbsMinPercent, SIGNAL(valueChanged(int)), this, SLOT(AbsMinPercentSlider(int)));
	connect(slMinDiffPercent, SIGNAL(valueChanged(int)), this, SLOT(MinDiffPercentSlider(int)));
	connect(slMinRatio, SIGNAL(valueChanged(int)), this, SLOT(MinRatioSlider(int)));
	connect(slMaxPixelEntropy, SIGNAL(valueChanged(int)), this, SLOT(MaxPixelEntropySlider(int)));
	connect(slLabelVoters, SIGNAL(valueChanged(int)), this, SLOT(LabelVoters(int)));
}

vtkSmartPointer<vtkTable> CreateVTKTable(int rowCount, QVector<QString> const & columnNames)
{
	auto result = vtkSmartPointer<vtkTable>::New();
	for (int i = 0; i < columnNames.size(); ++i)
	{
		vtkSmartPointer<vtkFloatArray> arr(vtkSmartPointer<vtkFloatArray>::New());
		arr->SetName(columnNames[i].toStdString().c_str());
		result->AddColumn(arr);
	}
	result->SetNumberOfRows(rowCount);
	return result;
}

void dlg_MajorityVoting::SetGroundTruthImage(LabelImagePointer groundTruthImage)
{
	m_groundTruthImage = groundTruthImage;
}

#include "iAAttributes.h"

void dlg_MajorityVoting::ClusterUncertaintyDice()
{
	if (!m_groundTruthImage)
	{
		QMessageBox::warning(this, "GEMSe", "Please load a reference image first!");
		return;
	}
	// gather Avg. Uncertainty vs. Accuracy, make it entry in row 1!
	int diceIdx = m_dlgGEMSe->GetMeasureStartID(); // TODO: replace hard-coded values with search in attributes!
	int avgUncIdx = diceIdx - 2;

	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	QSharedPointer<iAImageTreeNode> node = m_dlgGEMSe->GetSelectedCluster();

	QVector<QString> columns;
	columns.push_back("Average Uncertainty");
	columns.push_back("Mean Dice");

	auto table = CreateVTKTable(m_selection.size(), columns);

	for (int i = 0; i < m_selection.size(); ++i)
	{
		double unc = m_selection[i]->GetAttribute(avgUncIdx);
		double dice = m_selection[i]->GetAttribute(diceIdx);
		table->SetValue(i, 0, unc);
		table->SetValue(i, 1, dice);
	}
	AddResult(table, "Cluster (id=" + QString::number(node->GetID()) + ") Avg. Unc. vs. Mean Dice");
}


typedef ParametrizableLabelVotingImageFilter<LabelImageType> LabelVotingType;

LabelVotingType::Pointer GetLabelVotingFilter(
	QVector<QSharedPointer<iASingleResult> > selection,
	double minAbsPercentage, double minDiffPercentage, double minRatio, double maxPixelEntropy,
	int labelVoters, int weightType, int labelCount)
{
	LabelVotingType::Pointer labelVotingFilter;
	labelVotingFilter = LabelVotingType::New();
	labelVotingFilter->SetAbsoluteMinimumPercentage(minAbsPercentage);
	labelVotingFilter->SetMinimumDifferencePercentage(minDiffPercentage);
	labelVotingFilter->SetMinimumRatio(minRatio);
	labelVotingFilter->SetMaxPixelEntropy(maxPixelEntropy);
	labelVotingFilter->SetWeightType(static_cast<WeightType>(weightType));
	if (labelVoters > 0)
	{
		labelVoters = std::min(selection.size(), labelVoters);
		std::set<std::pair<int, int> > inputLabelVotersSet;

		for (int l = 0; l<labelCount; ++l)
		{
			std::vector<std::pair<int, double> > memberDice;
			for (int m = 0; m < selection.size(); ++m)
			{
				int attributeID = selection[m]->GetAttributes()->Find(QString("Dice %1").arg(l));
				if (attributeID == -1)
				{
					DEBUG_LOG(QString("Attribute 'Dice %1' not found, aborting!").arg(l));
					return LabelVotingType::Pointer();
				}
				memberDice.push_back(std::make_pair(m, selection[m]->GetAttribute(attributeID)));
			}
			// sort in descending order by metric
			sort(memberDice.begin(), memberDice.end(),
				[](const std::pair<int, double> a, const std::pair<int, double> b)
			{
				return a.second > b.second; // > because we want to order descending
			}
			);
			for (int m = 0; m < labelVoters; ++m)
			{
				inputLabelVotersSet.insert(std::make_pair(l, memberDice[m].first));
			}
		}
		labelVotingFilter->SetInputLabelVotersSet(inputLabelVotersSet);
	}

	if (weightType == LabelBased)
	{
		std::map<std::pair<int, int>, double> inputLabelWeightMap;

		for (int l = 0; l < labelCount; ++l)
		{
			for (int m = 0; m < selection.size(); ++m)
			{
				int attributeID = selection[m]->GetAttributes()->Find(QString("Dice %1").arg(l));
				if (attributeID == -1)
				{
					DEBUG_LOG(QString("Attribute 'Dice %1' not found, aborting!").arg(l));
					return LabelVotingType::Pointer();
				}
				double labelDice = selection[m]->GetAttribute(attributeID);
				inputLabelWeightMap.insert(
					std::make_pair(std::make_pair(l, m), labelDice));
			}
		}
		labelVotingFilter->SetInputLabelWeightMap(inputLabelWeightMap);
	}

	for (unsigned int i = 0; i < static_cast<unsigned int>(selection.size()); ++i)
	{
		LabelImageType* lblImg = dynamic_cast<LabelImageType*>(selection[i]->GetLabelledImage().GetPointer());
		labelVotingFilter->SetInput(i, lblImg);
		if (maxPixelEntropy >= 0 || weightType == Certainty || weightType == FBGSBGDiff)
		{
			typedef LabelVotingType::DoubleImg::Pointer DblImgPtr;
			std::vector<DblImgPtr> probImgs;
			for (int l = 0; l < labelCount; ++l)
			{
				iAITKIO::ImagePointer p = selection[i]->GetProbabilityImg(l);
				DblImgPtr dp = dynamic_cast<typename LabelVotingType::DoubleImg*>(p.GetPointer());
				probImgs.push_back(dp);
			}
			labelVotingFilter->SetProbabilityImages(i, probImgs);
		}
	}
	labelVotingFilter->Update();
	return labelVotingFilter;
}

iAITKIO::ImagePointer GetMajorityVotingImage(QVector<QSharedPointer<iASingleResult> > selection,
	double minAbsPercentage, double minDiffPercentage, double minRatio, double maxPixelEntropy,
	int labelVoters, int weightType, int labelCount)
{
	auto labelVotingFilter = GetLabelVotingFilter(
		selection, minAbsPercentage, minDiffPercentage, minRatio, maxPixelEntropy, labelVoters, weightType, labelCount);
	if (!labelVotingFilter)
		return iAITKIO::ImagePointer();
	LabelImagePointer labelResult = labelVotingFilter->GetOutput();
	iAITKIO::ImagePointer result = dynamic_cast<iAITKIO::ImageBaseType *>(labelResult.GetPointer());
	return result;
}

iAITKIO::ImagePointer GetMajorityVotingNumbers(QVector<QSharedPointer<iASingleResult> > selection,
	double minAbsPercentage, double minDiffPercentage, double minRatio, double maxPixelEntropy,
	int labelVoters, int weightType, int labelCount, int mode)
{
	auto labelVotingFilter = GetLabelVotingFilter(
		selection, minAbsPercentage, minDiffPercentage, minRatio, maxPixelEntropy, labelVoters, weightType, labelCount);
	if (!labelVotingFilter)
		return iAITKIO::ImagePointer();
	typename LabelVotingType::DoubleImg::Pointer labelResult = labelVotingFilter->GetNumbers(mode);
	iAITKIO::ImagePointer result = dynamic_cast<iAITKIO::ImageBaseType *>(labelResult.GetPointer());
	return result;
}


void dlg_MajorityVoting::AbsMinPercentSlider(int)
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minAbs = static_cast<double>(slAbsMinPercent->value()) / slAbsMinPercent->maximum();
	lbValue->setText("Value: Abs. Min % = "+QString::number(minAbs * 100, 'f', 2) + " %");
	UpdateWeightPlot();
	m_lastMVResult = GetMajorityVotingImage(m_selection, minAbs, -1, -1, -1, -1, GetWeightType(), m_labelCount);
	m_dlgGEMSe->AddMajorityVotingImage(m_lastMVResult);
}

void dlg_MajorityVoting::MinDiffPercentSlider(int)
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minDiff = static_cast<double>(slMinDiffPercent->value()) / slMinDiffPercent->maximum();
	lbValue->setText("Value: Min. Diff. % = "+QString::number(minDiff * 100, 'f', 2) + " %");
	UpdateWeightPlot();
	m_lastMVResult = GetMajorityVotingImage(m_selection, -1, minDiff, -1, -1, -1, GetWeightType(), m_labelCount);
	m_dlgGEMSe->AddMajorityVotingImage(m_lastMVResult);
}

void dlg_MajorityVoting::MinRatioSlider(int)
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minRatio = static_cast<double>(slMinRatio->value()) / 100;
	lbValue->setText("Value: Ratio = "+QString::number(minRatio, 'f', 2));
	UpdateWeightPlot();
	m_lastMVResult = GetMajorityVotingImage(m_selection, -1, -1, minRatio, -1, -1, GetWeightType(), m_labelCount);
	m_dlgGEMSe->AddMajorityVotingImage(m_lastMVResult);
}

void dlg_MajorityVoting::MaxPixelEntropySlider(int)
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double maxPixelEntropy = static_cast<double>(slMaxPixelEntropy->value()) / slMaxPixelEntropy->maximum();
	lbValue->setText("Value: Max. Pixel Ent. = " + QString::number(maxPixelEntropy*100, 'f', 2)+" %");
	UpdateWeightPlot();
	m_lastMVResult = GetMajorityVotingImage(m_selection, -1, -1, -1, maxPixelEntropy, -1, GetWeightType(), m_labelCount);
	m_dlgGEMSe->AddMajorityVotingImage(m_lastMVResult);
}

void dlg_MajorityVoting::LabelVoters(int)
{
	if (!m_groundTruthImage)
	{
		DEBUG_LOG("Please load a reference image first!");
		return;
	}
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	int labelVoters = slLabelVoters->value();
	lbValue->setText("Value: Label Voters = " + QString::number(labelVoters));
	UpdateWeightPlot();
	m_lastMVResult = GetMajorityVotingImage(m_selection, -1, -1, -1, -1, labelVoters, GetWeightType(), m_labelCount);
	m_dlgGEMSe->AddMajorityVotingImage(m_lastMVResult);
}

void dlg_MajorityVoting::MinAbsPlot()
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minAbs = static_cast<double>(slAbsMinPercent->value()) / slAbsMinPercent->maximum();
	m_lastMVResult = GetMajorityVotingNumbers(m_selection, minAbs, -1, -1, -1, -1, GetWeightType(), m_labelCount, AbsolutePercentage);
	m_dlgGEMSe->AddMajorityVotingNumbers(m_lastMVResult);
}

void dlg_MajorityVoting::MinDiffPlot()
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minDiff = static_cast<double>(slAbsMinPercent->value()) / slAbsMinPercent->maximum();
	m_lastMVResult = GetMajorityVotingNumbers(m_selection, -1, minDiff, -1, -1, -1, GetWeightType(), m_labelCount, DiffPercentage);
	m_dlgGEMSe->AddMajorityVotingNumbers(m_lastMVResult);
}

void dlg_MajorityVoting::RatioPlot()
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double minRatio = static_cast<double>(slAbsMinPercent->value()) / slAbsMinPercent->maximum();
	m_lastMVResult = GetMajorityVotingNumbers(m_selection, -1, -1, minRatio, -1, -1, GetWeightType(), m_labelCount, Ratio);
	m_dlgGEMSe->AddMajorityVotingNumbers(m_lastMVResult);
}


void dlg_MajorityVoting::MaxPixelEntropyPlot()
{
	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);
	double maxPixelEntropy = static_cast<double>(slMaxPixelEntropy->value()) / slMaxPixelEntropy->maximum();
	m_lastMVResult = GetMajorityVotingNumbers(m_selection, -1, -1, -1, maxPixelEntropy, -1, GetWeightType(), m_labelCount, PixelEntropy);
	m_dlgGEMSe->AddMajorityVotingNumbers(m_lastMVResult);
}

void dlg_MajorityVoting::StoreResult()
{
	iAITKIO::ScalarPixelType pixelType = itk::ImageIOBase::INT;
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Store Last Majority Voting Result"),
		m_folder,
		iAIOProvider::GetSupportedSaveFormats()
	);
	iAITKIO::writeFile(fileName, m_lastMVResult, pixelType);
}

void dlg_MajorityVoting::StoreConfig()
{

}

// Where to put temporary output

// currently chosen option:
//  * detail view
//      + prominent display
//      + close to current analysis
//      - lots to rewrite, as it expects node with linked values
//      - volatile - will be gone after next cluster / example image selection

// Options / Design considerations:
//  * integrate into clustering
//      + image is then part of rest of analysis
//      ~ follow-up decision required:
//          - consider MJ separate algorithm?
//          - how to preserve creation "parameters"?
//		- have to re-run whole clustering? or integrate it somehow faked?
//	* intermediate step: add as separate result (e.g. in favorite view)
//      // (chance to include in clustering after renewed clustering)
//  * separate list of  majority-voted results
//		- separate from
//  * new dock widget in same mdichild
//		+ closer to current analysis than separate mdi child
//		- lots of new implementation required
//		- no clear benefit - if each
//  * new window (mdichild?)
//      - detached from current design
//      + completely independent of other implementation (should continue working if anything else changes)
//      - completely independent of other implementation (not integrated into current analysis)
//      +/- theoretically easier to do/practically probably also not little work to make it happen


vtkIdType AddPlot(int plotType,
	vtkSmartPointer<vtkChartXY> chart,
	vtkSmartPointer<vtkTable> table,
	int col1, int col2,
	QColor const & color)
{
	vtkSmartPointer<vtkPlot> plot;
	switch (plotType)
	{
		default: // intentional fall-through
		case vtkChart::POINTS: plot = vtkSmartPointer<vtkPlotPoints>::New(); break;
		case vtkChart::LINE: plot = vtkSmartPointer<vtkPlotLine>::New(); break;
	}
	plot->SetColor(
		static_cast<unsigned char>(color.red()),
		static_cast<unsigned char>(color.green()),
		static_cast<unsigned char>(color.blue()),
		static_cast<unsigned char>(color.alpha())
	);
	plot->SetTooltipLabelFormat("%x, %l: %y");
	plot->SetWidth(1.0);
	plot->SetInputData(table, col1, col2);
	vtkIdType plotID = chart->AddPlot(plot);
	return plotID;
}

void dlg_MajorityVoting::AddResult(vtkSmartPointer<vtkTable> table, QString const & title)
{
	int idx = twSampleResults->rowCount();
	twSampleResults->setRowCount(idx + 1);
	QCheckBox * checkBox = new QCheckBox;
	//if (i == 3) checkBox->setChecked(true);
	twSampleResults->setCellWidget(idx, 0, checkBox);
	connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(CheckBoxStateChanged(int)));
	twSampleResults->setItem(idx, 1, new QTableWidgetItem(title));
	m_checkBoxResultIDMap.insert(checkBox, idx);
	if (m_results.size() != idx)
	{
		DEBUG_LOG("Results vector and table are out of sync!");
		return;
	}
	m_results.push_back(table);
}

int dlg_MajorityVoting::GetWeightType()
{
	if (rbWeightLabelDice->isChecked())  return LabelBased;
	if (rbWeightCertainty->isChecked())  return Certainty;
	if (rbWeightDiffFBGSBG->isChecked()) return FBGSBGDiff;
	else return Equal;
}

QString GetWeightName(int weightType)
{
	switch (weightType)
	{
		case LabelBased: return "LabelDice";
		case Certainty:  return "Certainty";
		case FBGSBGDiff: return "FBG-SBG";
		default:         return "Equal";
	}
}


void dlg_MajorityVoting::UpdateWeightPlot()
{
	lbWeight->setText("Weight " + GetWeightName(GetWeightType()));
}

void dlg_MajorityVoting::Sample()
{
	if (!m_groundTruthImage)
	{
		QMessageBox::warning(this, "GEMSe", "Please load a reference image first!");
		return;
	}

	QVector<QString> columnNames;
	columnNames.push_back("Value");
	columnNames.push_back("Undecided Pixels");
	columnNames.push_back("Mean Dice");

	const int SampleCount = sbSampleCount->value();
	const int ResultCount = 5;
	const int UndecidedLabel = m_labelCount;

	vtkSmartPointer<vtkTable> tables[ResultCount];
	QString titles[ResultCount] =
	{
		QString("Min. Absolute Percentage"),
		QString("Min. Percentage Difference"),
		QString("Ratio"),
		QString("Max. Pixel Uncertainty"),
		QString("Max. Label Voters")
	};
	for (int r = 0; r < ResultCount; ++r)
	{
		tables[r] = CreateVTKTable(SampleCount, columnNames);
	}

	QVector<QSharedPointer<iASingleResult> > m_selection;
	m_dlgGEMSe->GetSelection(m_selection);

	double absPercMin = 1.0 / m_labelCount;
	double absPercMax = 1;

	double ratioMin = 1;
	double ratioMax = m_selection.size();

	double labelVoterMin = 1;
	double labelVoterMax = m_selection.size();
	DEBUG_LOG(QString("Majority Voting evaluation for a selection of %1 images").arg(m_selection.size()));

	typedef fhw::FilteringLabelOverlapMeasuresImageFilter<LabelImageType> DiceFilter;
	typedef itk::LabelStatisticsImageFilter<LabelImageType, LabelImageType> StatFilter;

	auto region = m_groundTruthImage->GetLargestPossibleRegion();
	auto size = region.GetSize();
	double pixelCount = size[0] * size[1] * size[2];

	for (int i = 0; i < SampleCount; ++i)
	{
		// calculate current value:
		double norm = mapToNorm(0, SampleCount, i);

		double value[ResultCount] = {
			mapNormTo(absPercMin, absPercMax, norm),		// minimum absolute percentage
			norm,											// minimum relative percentage
			mapNormTo(ratioMin, ratioMax, norm),			// ratio
			norm,											// maximum pixel uncertainty
			mapNormTo(labelVoterMin, labelVoterMax, norm)
		};

		// calculate majority voting using these values:
		iAITKIO::ImagePointer result[ResultCount];

		result[0] = GetMajorityVotingImage(m_selection, value[0], -1, -1, -1, -1, GetWeightType(), m_labelCount);
		result[1] = GetMajorityVotingImage(m_selection, -1, value[1], -1, -1, -1, GetWeightType(), m_labelCount);
		result[2] = GetMajorityVotingImage(m_selection, -1, -1, value[2], -1, -1, GetWeightType(), m_labelCount);
		result[3] = GetMajorityVotingImage(m_selection, -1, -1, -1, value[3], -1, GetWeightType(), m_labelCount);
		result[4] = GetMajorityVotingImage(m_selection, -1, -1, -1, -1, value[4], GetWeightType(), m_labelCount);

		//QString out(QString("absPerc=%1, relPerc=%2, ratio=%3, pixelUnc=%4\t").arg(absPerc).arg(relPerc).arg(ratio).arg(pixelUnc));
		// calculate dice coefficient and percentage of undetermined pixels
		// (percentage of voxels with label = difference marker = max. label + 1)
		for (int r = 0; r < ResultCount; ++r)
		{
			LabelImageType* labelImg = dynamic_cast<LabelImageType*>(result[r].GetPointer());

			auto diceFilter = DiceFilter::New();
			diceFilter->SetSourceImage(m_groundTruthImage);
			diceFilter->SetTargetImage(labelImg);
			diceFilter->SetIgnoredLabel(UndecidedLabel);
			diceFilter->Update();
			auto statFilter = StatFilter::New();
			statFilter->SetInput(labelImg);
			statFilter->SetLabelInput(labelImg);
			statFilter->Update();

			double meanDice = diceFilter->GetMeanOverlap();

			double undefinedPerc =
				statFilter->HasLabel(UndecidedLabel)
				? static_cast<double>(statFilter->GetCount(UndecidedLabel)) / pixelCount
				: 0;
			//out += QString("%1 %2\t").arg(meanDice).arg(undefinedPerc);

			// add values to table
			tables[r]->SetValue(i, 0, value[r]);
			tables[r]->SetValue(i, 1, undefinedPerc);
			tables[r]->SetValue(i, 2, meanDice);
		}
		//DEBUG_LOG(QString::number(i) + ": " + out);
	}

	QString ids;
	for (int s = 0; s < m_selection.size(); ++s)
	{
		ids += QString::number(m_selection[s]->GetDatasetID()) + "-" + QString::number(m_selection[s]->GetID());
		if (s < m_selection.size() - 1)
		{
			ids += ", ";
		}
	}

	int startIdx = twSampleResults->rowCount();
	for (int i = 0; i < ResultCount; ++i)
	{
		AddResult(tables[i], "Sampling(w="+ GetWeightName(GetWeightType())+ ",value=" + titles[i] + ",ids=" + ids);
	}

}

void dlg_MajorityVoting::CheckBoxStateChanged(int state)
{
	QCheckBox* sender = dynamic_cast<QCheckBox*>(QObject::sender());
	int id = m_checkBoxResultIDMap[sender];
	if (state == Qt::Checked)
	{
		static int colorCnt = 0;
		int colorIdx = (colorCnt++) % 12;
		QColor plotColor = m_colorTheme->GetColor(colorIdx);

		QVector<vtkIdType> plots;
		if (m_results[id]->GetNumberOfColumns() == 3)
		{
			vtkIdType plot1 = AddPlot(vtkChart::POINTS, m_chartDiceVsUndec, m_results[id], 1, 2, plotColor);
			vtkIdType plot2 = AddPlot(vtkChart::LINE, m_chartValueVsDice, m_results[id], 0, 2, plotColor);
			vtkIdType plot3 = AddPlot(vtkChart::LINE, m_chartValueVsUndec, m_results[id], 0, 1, plotColor);
			plots.push_back(plot1);
			plots.push_back(plot2);
			plots.push_back(plot3);
		}
		else
		{
			vtkIdType plotID = AddPlot(vtkChart::POINTS, m_chartValueVsDice, m_results[id], 0, 1, plotColor);
			plots.push_back(plotID);
		}
		m_plotMap.insert(id, plots);
		twSampleResults->item(id, 1)->setBackgroundColor(plotColor);
	}
	else
	{
		twSampleResults->item(id, 1)->setBackgroundColor(Qt::white);
		QVector<vtkIdType> plots = m_plotMap[id];
		if (m_results[id]->GetNumberOfColumns() == 3)
		{
			m_chartDiceVsUndec->RemovePlot(plots[0]);
			m_chartValueVsDice->RemovePlot(plots[1]);
			m_chartValueVsUndec->RemovePlot(plots[2]);
		}
		else
		{
			m_chartValueVsDice->RemovePlot(plots[0]);
		}
		m_plotMap.remove(id);
	}
}