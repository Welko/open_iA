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

#include "open_iA_Core_export.h"
#include "graph.h"
#include "graphstat.h"

#include <vector>

class open_iA_Core_API GraphDrawer
{
public:
							GraphDrawer();
							~GraphDrawer();

	void					setNumberOfIterations(int n);
	void					setNumRanks();
	void					setGraph(Graph* g);
	void					start();

private:
	typedef std::vector<std::vector<Graph::idType>> OrderType;

	void					initialOrder(OrderType& order);
	void					addVerticesToOrder(Graph::idType headerVert, OrderType& order);

	void					wmedian(OrderType& order, bool forwardTraversal);
	float					medianValue(Graph::idType vert, OrderType& order, bool forwardTraversal);
	std::vector<int>		getAdjacentPositions(Graph::idType vert, OrderType& order, bool forwardTraversal);
	void					transpose(OrderType& order, bool forwardTraversal);
	int						numberOfCrossing(OrderType& order);
	int						numberOfCrossing(OrderType& order, int rank, int pos1, int pos2, bool forwardTraversal);


	int						m_maxIteration;
	OrderType				m_order;
	Graph*					m_graph;
	GraphStat				m_graphStat;
	int						m_ranks;
};

