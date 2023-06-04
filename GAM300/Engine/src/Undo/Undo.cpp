#include "Undo.h"
#include "Serializer/Factory.h"
#include <fstream>
static std::string stackPath = "../Resources/temp/";
using json = nlohmann::json;

#ifdef EDITOR
void Undo::StoreChange(GameObject* _go)
{
	if (!_go) return;
	std::string path = stackPath;
	path += "Undo_";
	path += _go->GetName();
	path += "_";
	path += std::to_string(_go->mChangesDone);
	path += ".json";
	json j;
	j << _go->mTransform;
	serializer.Write(path, j);
	mStack.push(_go);
}

void Undo::UndoChange()
{
	if (mStack.empty()) return;
	json j;

	GameObject* last = mStack.top();
	std::string path = stackPath;
	path += "Undo_";
	path += last->GetName();
	path += "_";
	path += std::to_string(last->mChangesDone);
	path += ".json";
	std::ifstream inFile(path.c_str());
	Transform prev;
	if (inFile.good() && inFile.is_open()) 
	{
		inFile >> j;
		inFile.close();
		j >> prev;
		remove(path.c_str());
	}
	last->mTransform = prev;
	last->mChangesDone -= 1;
	mStack.pop();
}

void Undo::Clear()
{
	while (!mStack.empty()) mStack.pop();
}
#endif