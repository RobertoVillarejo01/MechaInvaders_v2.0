#pragma once

#include <string>
#include "Utilities/Types.h"

enum class TypeIds { IBase, renderable, StaticCollider, DynamicCollider, RigidBody, GameObject, Space };

class IBase
{
public:
	virtual ~IBase() {}

public:
	const char * GetName() { return mName.data(); }
	void         SetName(const char * name) { mName = name; }

	u32					GetUID() { return mUID; }
	const u32	  GetUID() const { return mUID; } ;

protected:
	std::string mName;	
	u32					mUID;		

	static u32	mLastUID;	// Unique number ID
	
	IBase() {
		mUID = ++mLastUID;
	} // only accessible from child classes. can't construct an IBase explicitly.

public:
	static const unsigned type_id = static_cast<const unsigned>(TypeIds::IBase);
};

