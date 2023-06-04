#pragma once
#include <stack>
#include "Objects/GameObject.h"
#include "Utilities/Singleton.h"
class Undo
{
	MAKE_SINGLETON(Undo);
public:
	void StoreChange(GameObject* _go);
	void UndoChange();
	void Clear();

private:
	std::stack<GameObject*> mStack;
};

#define undo (Undo::Instance());

