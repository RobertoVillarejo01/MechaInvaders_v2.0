#pragma once
#include <vector>
#include "Utilities/Singleton.h"
#include "Physics/Rigidbody/Rigidbody.h"

class PhysicsSystem
{
	MAKE_SINGLETON(PhysicsSystem);

public:
	bool Initialize();
	void Update();

private:

};

#define physicsSystem (PhysicsSystem::Instance())