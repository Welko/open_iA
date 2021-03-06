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
#include "dlg_trackingGraph.h"

#include <iAVtkGraphDrawer.h>
#include <iAVtkWidget.h>

#include <vtkContextActor.h>
#include <vtkContextInteractorStyle.h>
#include <vtkContextScene.h>
#include <vtkContextTransform.h>
#include <vtkGraphItem.h>
#include <vtkMutableDirectedGraph.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

const int MAX_ITERATIONS		= 24;
const double BACKGROUND[3]		= {1, 1, 1};

dlg_trackingGraph::dlg_trackingGraph(QWidget *parent) : QDockWidget(parent)
{
	setupUi(this);

	// create graph
	m_graph = vtkSmartPointer<vtkMutableDirectedGraph>::New();

	m_graphItem = vtkSmartPointer<iATrackingGraphItem>::New();
	m_graphItem->SetGraph(m_graph);

	m_trans = vtkSmartPointer<vtkContextTransform>::New();
	m_trans->SetInteractive(true);
	m_trans->AddItem(m_graphItem);

	m_contextScene = vtkSmartPointer<vtkContextScene>::New();
	m_contextScene->AddItem(m_trans);

	m_actor = vtkSmartPointer<vtkContextActor>::New();
	m_actor->SetScene(m_contextScene);

	m_renderer = vtkSmartPointer<vtkRenderer>::New();
	m_renderer->SetBackground(BACKGROUND[0], BACKGROUND[1], BACKGROUND[2]);
	m_renderer->AddActor(m_actor);

	CREATE_OLDVTKWIDGET(graphWidget);
	this->horizontalLayout->addWidget(graphWidget);
	graphWidget->GetRenderWindow()->AddRenderer(m_renderer);

	m_interactorStyle = vtkSmartPointer<vtkContextInteractorStyle>::New();
	m_interactorStyle->SetScene(m_contextScene);

	m_interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	m_interactor->SetInteractorStyle(m_interactorStyle);
	m_interactor->SetRenderWindow(graphWidget->GetRenderWindow());

	graphWidget->GetRenderWindow()->Render();
}

void dlg_trackingGraph::updateGraph(vtkMutableDirectedGraph* g, int nunRanks, std::map<vtkIdType, int> nodesToLayers, std::map<int, std::map<vtkIdType, int>> graphToTableId)
{
	if(g->GetNumberOfVertices() < 1) return;

	this->m_graph = g;
	this->m_nodesToLayers = nodesToLayers;

	vtkNew<vtkPoints> points;	
	iAVtkGraphDrawer graphDrawer;
	//graphDrawer.setMaxIteration(MAX_ITERATIONS);
	graphDrawer.createLayout(points.GetPointer(), m_graph, graphWidget->GetRenderWindow()->GetSize(), nunRanks);
	m_graph->SetPoints(points.GetPointer());
	
	m_graphItem->SetGraph(m_graph);
	m_graphItem->Update();
	graphWidget->GetRenderWindow()->Render();
}
