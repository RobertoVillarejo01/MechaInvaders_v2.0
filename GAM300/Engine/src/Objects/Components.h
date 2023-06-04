#pragma once

#include "System/Base.h"
#include "GameObject.h"

// ----------------------------------------------------------------------------
// \class	IComp 
// \brief	Base component class, meant to be used as an interface.
class IComp : public IBase, public AutoSerialize
{
//	friend class GameObject;

public:
	virtual ~IComp() = default;

	// State Methods
	bool IsEnabled();
	void SetEnabled(bool enabled);

	GameObject* GetOwner(void) const { return mOwner; }

	// Overridable methods - each component should implement their own versions of those
	virtual void Initialize();		// Called when the owner object is about to be initalized.
	virtual void Update();			// Called by the system at each update.
	virtual void Shutdown();		// Called by the owner object when destroyed.

#ifdef EDITOR

	// Edit function to be override by child class, for ImGui
	virtual bool Edit();
#endif

	//virtual void Dragging() {}
	//virtual void Selected() {}

	//Clone function for deep copys in case is necesary
	virtual IComp * Clone() = 0;

	//From/To Json function to be override by child class,
	//in order to save and load from file
	friend void operator<<(nlohmann::json& j, const IComp& _rhs);
	friend void operator>>(nlohmann::json& j, IComp& _rhs);

	nlohmann::json& operator<<(nlohmann::json& j) const override;
	void            operator>>(nlohmann::json& j);

	//virtual void operator=(IComp &) {};

	//friend comparision operator to be able to check if two components are the same
	//friend bool operator==(IComp const& lhs, IComp const& rhs) {
	//	return lhs.equal_to(rhs);
	//}

	bool are_the_same_type(IComp const& lhs);
	//virtual void operator=(IComp & _comp);

	

protected:
	virtual void ToJson(nlohmann::json& j) const;
	virtual void FromJson(nlohmann::json& j);
	//virtual bool equal_to(IComp const& other) const = 0;


public:
	GameObject* mOwner = nullptr;
	bool				mbEnabled = true;
};




// ----------------------------------------------------------------------------