#pragma once
#include <glm/glm.hpp>
#include "LogicSystem/Logic.h"
#include "Dijkstra.h"


class NavMesh : public ILogic
{
public:
	void Initialize() override;
	void Update() override;
	void Shutdown() override;

#ifdef EDITOR
	virtual bool Edit();
	virtual bool ConnectNodes();
	void DeleteNode(unsigned nodeID);
	virtual bool ShowNodes();
	virtual bool RunAlgEdit();
	virtual bool EditColumns();
	virtual bool ShowPathFollowing();
	unsigned	 GetNodeInMouse();
#endif // EDITOR

	IComp* Clone();

	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);
	void LoadFromTXT(std::string path);
	void SaveToTXT(std::string path);
	std::vector<glm::vec3> GetClosetPath(unsigned startNode, unsigned endNode, std::vector<unsigned> * pathToFill = nullptr, unsigned * TotalCost = nullptr);

	bool showPath;
	unsigned srcLine; //for connecting 2 nodes through editor
	unsigned dstLine; //for connecting 2 nodes through editor
	unsigned toDelete;
	unsigned mGraphSize;
	DijkstraGraph mGraph; //graph with all the nodes
	std::vector<unsigned> mDemoPath;
	glm::vec3 pointFollowing;
};


