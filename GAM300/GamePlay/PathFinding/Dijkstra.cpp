#include "Dijkstra.h"
#include <algorithm>
#include <iterator>

DijkstraGraph::DijkstraGraph() : startNode(0u),endNode(0u),mSize(0u)
{
}

DijkstraGraph::DijkstraGraph(unsigned size) : startNode(0u), endNode(0u), mSize(size)
{
}

void DijkstraGraph::Create(unsigned size)
{
	mCostTable.clear();
	DijkstraTable.clear();
	unsigned max = static_cast<unsigned>(-1); //infinite value

	for (unsigned i = 0; i < size; i++)
	{
		//the index on the vector is the same as the id on the node
		std::vector<AdjacencyInfo> info(size); //empty node to push
		for (unsigned j = 0u; j < info.size(); j++)
		{
			info[j].id = j + 1;

			if (i == j) info[j].weight = 0u;
			else info[j].weight = -1;
		}
		mCostTable.push_back(info); // put the node on the table;
		
		DijkstraInfo dInfo; //node for the dijkstra algorithm table
		dInfo.cost = max; //initialize the cost to infinty
		dInfo.path.clear(); //just in case clear the path to reach there
		DijkstraTable.push_back(dInfo); //as before, the index is the same as the id
		mPositionArray.push_back({ 0.0f,0.0f,0.0f });
	}

	startNode = 1u;
	endNode = 1u;
}

void DijkstraGraph::AddEdge(unsigned source, unsigned destination, unsigned weight)
{
	AdjacencyInfo newInfo; //create the 
	newInfo.id = destination; //set the id of the destination
	newInfo.weight = weight; //distance, change to float TODO

	//function to compare 2 adjacency infos
	auto lessWeight = [newInfo](AdjacencyInfo adj)
					  {
						return newInfo.weight < adj.weight;
					  };

	//finds the correct spot in the table to put the edge
	auto pos = std::find_if(mCostTable[source - 1].begin(), mCostTable[source - 1].end(), lessWeight);

	mCostTable[source - 1u].insert(pos, newInfo); //put it in the correct place
}

void DijkstraGraph::AddUEdge(unsigned node1, unsigned node2, unsigned weight)
{
	AddEdge(node1, node2, weight);
	AddEdge(node2, node1, weight);
}

void DijkstraGraph::InitialzieCosts(unsigned size)
{
	unsigned max = -1;
	for (unsigned i = 0u; i < size; i++)
	{
		for (unsigned j = 0u; j < size; j++)
		{
			AddUEdge(i + 1, j + 1, max);
		}
	}

	return;
}

void DijkstraGraph::InitializeCostsDistance()
{
	for (unsigned i = 0u; i < mCostTable.size(); i++)
	{
		auto& adJacentNodes = mCostTable[i];
		glm::vec3 pos1 = mPositionArray[i];

		for (unsigned j = 0u; j < adJacentNodes.size(); j++)
		{
			//if (i == j) {
			//	mCostTable[i][j] = 0.0f;
			//	continue;
			//}
			//
			//glm::vec3 pos2 = mPositionArray[j];
			//float Distance = glm::distance(pos1, pos2);
		}
	}
}

std::vector<DijkstraInfo> DijkstraGraph::Dijkstra(unsigned start_node)
{
	ClearDijkstraTable();
	if (start_node - 1 >= DijkstraTable.size())
		return DijkstraTable;
	DijkstraTable[start_node - 1].cost = 0;
	DijkstraTable[start_node - 1].path.push_back(start_node);
	for (unsigned i = 0u; i < mCostTable[start_node - 1].size(); i++)
	{
		unsigned id = mCostTable[start_node - 1][i].id; //get node
		unsigned cost = mCostTable[start_node - 1][i].weight; //get cost to that node 

		if ((i + 1u) == start_node) continue; //doin't ùsh again the same

		DijkstraTable[id - 1].cost = cost; //initialize cost
		if (cost <= -1)
		{
			DijkstraTable[id - 1].path.push_back(start_node);
			DijkstraTable[id - 1].path.push_back(id);
		}

		AdjacencyInfo pushInfo;
		pushInfo.id = id;
		pushInfo.weight = cost;
		
		
		pQ.push(pushInfo); //put the info on priority queue
	}

	while (!pQ.empty())
	{
		AdjacencyInfo vertex = pQ.top(); //get unvisited vertex with smallest distance from start vertex
		pQ.pop();
		unsigned idV = vertex.id; //get index
		unsigned costV = DijkstraTable[idV - 1].cost; //get cost from start vertex
		for (unsigned i = 0; i < mCostTable[idV - 1].size(); i++) //go through all neighbours of the vertex
		{
			AdjacencyInfo& adj = mCostTable[idV - 1][i]; //id of the adjacent vertex
			unsigned Totalcost = costV + adj.weight; //total cost of adjacent vertex
			if (adj.weight >= -1 || costV >= -1) 
				Totalcost = -1;
			DijkstraInfo& info = DijkstraTable[adj.id - 1];

			if (Totalcost < info.cost) //update table
			{
				info.cost = Totalcost;
				info.path = DijkstraTable[idV - 1].path;
				info.path.push_back(adj.id);

				//push again the ad
				AdjacencyInfo newAdj;
				newAdj.id = adj.id;
				newAdj.weight = Totalcost;
				newAdj.position = adj.position;
				pQ.push(newAdj);
			}
		}
	}

	return DijkstraTable; //just reutrn
}

std::vector<DijkstraInfo>& DijkstraGraph::getDijkstraTable()
{
	return DijkstraTable;
}

void DijkstraGraph::ClearDijkstraTable()
{
	for (auto& it : DijkstraTable)
	{
		it.path.clear();
		it.cost = -1;
	}
}

void DijkstraGraph::InsertNode()
{
	DijkstraTable.clear();
	unsigned newSize = (unsigned)mPositionArray.size() + 1u;
	mSize = newSize;
	unsigned max = -1;

	//recreate dijkstra table
	for (unsigned i = 0u; i < newSize; i++)
	{
		DijkstraInfo dInfo; //node for the dijkstra algorithm table
		dInfo.cost = max; //initialize the cost to infinty
		dInfo.path.clear(); //just in case clear the path to reach there
		DijkstraTable.push_back(dInfo); //as before, the index is the same as the id
	}

	//put 1 positions more
	mPositionArray.push_back({ 0.0f,0.0f,0.0f });

	//put on the table the new node
	std::vector<AdjacencyInfo> newAdj(newSize);

	//put values for new node
	for (unsigned i = 0u; i < newAdj.size(); i++)
	{
		newAdj[i].id = i + 1u;
		newAdj[i].weight = -1;

		if (i == newSize - 1u) newAdj[i].weight = 0u;
	}

	mCostTable.push_back(newAdj);

	//update the rest of the table putting the new value
	for (unsigned i = 0u; i < mCostTable.size() - 1u; i++)
	{
		AdjacencyInfo theInfo;
		theInfo.id = newSize;
		theInfo.weight = -1;
		mCostTable[i].push_back(theInfo);
	}

	return;
}

void DijkstraGraph::LinkNodes(unsigned src, unsigned dst)
{
	glm::vec3 SrcPos = mPositionArray[src];
	glm::vec3 DstPos = mPositionArray[dst];

	float dist = glm::distance(SrcPos, DstPos);

	mCostTable[src][dst].weight = (unsigned)dist;
	mCostTable[dst][src].weight = (unsigned)dist;
}

std::vector<DijkstraInfo>& DijkstraGraph::shortestPaths()
{
	return DijkstraTable;
}

CostTable DijkstraGraph::GetInitalTable() const
{
	return mCostTable;
}


CostTable& DijkstraGraph::GetCurrentTable()
{
	return mCostTable;
}

std::vector<glm::vec3>& DijkstraGraph::GetPosArray()
{
	return mPositionArray;
}
