#pragma once
#include <glm/glm.hpp> //for position vector
#include <vector>
#include <map>
#include <queue>

//Struct with the final path 
struct DijkstraInfo
{
	unsigned cost = 0u; //cost of the path 
	std::vector<unsigned> path; //nodes of final path
};


//TODO, CHANGE WEIGHT TO FLOAT (WEIGHT WILL BE DISTANCE)
struct AdjacencyInfo
{
	unsigned id; //name of the node
	unsigned weight; //weight of the node
	glm::vec3 position;
};

//This list will be the matrix with all the costs
typedef std::vector<std::vector<AdjacencyInfo>> CostTable;

class DijkstraGraph
{
public:

	//Creation methods
	DijkstraGraph();
	DijkstraGraph(unsigned size);
	void Create(unsigned size);

	///Adds 2 nodes to the table with the connections info
	void AddEdge(unsigned source, unsigned destination, unsigned weight);

	///adds 2 nodes but bidirectional
	void AddUEdge(unsigned node1, unsigned node2, unsigned weight);

	///add all edges with cost
	void InitialzieCosts(unsigned size);

	//put costs with distances
	void InitializeCostsDistance();

	///Gives shortest distance to all possible paths 
	std::vector<DijkstraInfo> Dijkstra(unsigned start_node);

	///gets the table with current djkstra values
	std::vector<DijkstraInfo>& getDijkstraTable();

	//clears the table before doing algorithm
	void ClearDijkstraTable();

	///gives the shortest paths
	std::vector<DijkstraInfo>& shortestPaths();
	
	///get's the adjacency list 
	CostTable GetInitalTable() const;

	///gets the aabbs of the rooms


	//get adjacecny
	CostTable& GetCurrentTable();

	///inserts a new node
	void InsertNode();
	
	void LinkNodes(unsigned src, unsigned dst);

	//get positions
	std::vector<glm::vec3>& GetPosArray();

	unsigned startNode; //node to start algorithm from
	unsigned endNode; //node that we want to go t
	unsigned mSize;

private:

	//For the priority queue to get the one with less weight
	class CompareQueue
	{
	public:
		bool operator() (const AdjacencyInfo& left, const AdjacencyInfo& right)
		{
			return left.weight > right.weight;
		}
	};

	mutable CostTable mCostTable; //table edge costs
	mutable std::vector<DijkstraInfo> DijkstraTable; //table with values of the algorithm resut
	mutable std::priority_queue<AdjacencyInfo, std::vector<AdjacencyInfo>, CompareQueue> pQ; //priority queue with unvisited nodes and their current values
	
	std::vector<glm::vec3> mPositionArray; //positions of nodes
	std::vector<glm::vec3> mRoomScales; //aabbs representing rooms

};