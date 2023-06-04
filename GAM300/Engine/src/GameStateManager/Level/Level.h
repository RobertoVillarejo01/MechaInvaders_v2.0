#pragma once
#include <vector>
#include <string>

class Space;

class Level
{
public:
	Level(std::string name = "Daniel");
	~Level();
	void AddSpace(std::string name = "newSpace", bool visible = true);

private:
	std::vector<Space*> mSpaces;
	std::string mName;
};
