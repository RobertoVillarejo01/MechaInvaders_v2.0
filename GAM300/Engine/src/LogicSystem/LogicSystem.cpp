#include "LogicSystem.h"
#include "System/Scene/SceneSystem.h"
#include "Collisions/Collider.h"
#include "../Editor/src/Editor.h"

void ILogic::Initialize()
{
#ifdef EDITOR
	if(!EditorMgr.mbInEditor)
		InternalInitialize();
#else
	InternalInitialize();
#endif

}

void ILogic::Update()
{
	if (!mbAlive || !mbActive) return;
	InternalUpdate();
}

void ILogic::Shutdown()
{
#ifdef EDITOR
	if (!EditorMgr.mbFromEditor)
		InternalShutdown();
#else
	InternalShutdown();
#endif
}

void ILogic::SetActive(bool _mode)
{
	mbActive = _mode;
}

bool ILogic::IsActive() const
{
	return mbActive;
}

void ILogic::subscribe_collision_event()
{
	EventDisp.subscribe_collison(this, mOwner->GetUID());
}

void LogicManager::Initialize()
{

}

void LogicManager::Update()
{
	auto& spaces = Scene.GetSpaces();
	std::for_each(spaces.begin(), spaces.end(),
		[](auto& it)
		{
			auto& logics = it->GetComponentsType<ILogic>();
			for (int i = 0; i < logics.size(); i++)
			{
				static_cast<ILogic*>(logics[i])->Update();
			}
		});
}

void LogicManager::ShutDown()
{

}
