#include "PhysicsSystem.h"
#include "System/Scene/SceneSystem.h"
bool PhysicsSystem::Initialize()
{
    return false;
}

void PhysicsSystem::Update()
{
	auto& spaces = Scene.GetSpaces();
	std::for_each(spaces.begin(), spaces.end(),
		[](auto& it)
		{
			auto& rigidbodies = it->GetComponentsType<Rigidbody>();
			std::for_each(rigidbodies.begin(), rigidbodies.end(),
				[](auto& comp)
				{
					static_cast<Rigidbody*>(comp)->Update();
				});
		});
}
