// ----------------------------------------------------------------------------
// Project: GAM200
// File:	Components.cpp
// Purpose:	IComp API
//
// Copyright DigiPen Institute of Technology
// ----------------------------------------------------------------------------
#include "Components.h"
#include "Objects/GameObject.h"
#include "System/TypeInfo/TypeInfo.h"

/* Global variable for IBase */
u32	IBase::mLastUID = 0;

void IComp::Initialize() {}
void IComp::Update() {}
void IComp::Shutdown() {}

// ----------------------------------------------------------------------------
// Return wheter the component is enabled or not.
// ----------------------------------------------------------------------------
bool IComp::IsEnabled()
{
	return mbEnabled;
}

// ----------------------------------------------------------------------------
// Set the component to enabled or disabled.
// ----------------------------------------------------------------------------
void IComp::SetEnabled(bool enabled)
{
	mbEnabled = enabled;
}

#ifdef EDITOR
// ----------------------------------------------------------------------------
// Edit function for ImGui editing
// ----------------------------------------------------------------------------
bool IComp::Edit() { return false; }

#endif

/*void IComp::operator=(IComp & _comp) {
	mbEnabled = _comp.mbEnabled;
	mUID = _comp.mUID;
}*/



nlohmann::json& IComp::operator<<(nlohmann::json& j) const
{
	ToJson(j);

	return AutoSerialize::operator<<(j);
}

void IComp::operator>>(nlohmann::json& j)
{
	FromJson(j);

	AutoSerialize::operator>>(j);
}

bool IComp::are_the_same_type(IComp const& lhs)
{
	if (typeid(lhs) == typeid(*this))
		return true;
	return false;
}

void IComp::ToJson(nlohmann::json& j) const { }

void IComp::FromJson(nlohmann::json& j) { }

void operator<<(nlohmann::json& j, const IComp& _rhs)
{
	j["Type"] << TypeInfo(typeid(_rhs)).get_name();

	j["Enabled"] << _rhs.mbEnabled;

	_rhs.operator<<(j);
}

void operator>>(nlohmann::json& j, IComp& _rhs)
{
	j["Enabled"] >> _rhs.mbEnabled;

	_rhs.operator>>(j);
}
