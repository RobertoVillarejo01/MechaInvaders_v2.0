#pragma once
#include <unordered_map>
#include "../json.hpp"
#include "glm/glm.hpp"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR


class ISerializable
{
public:
	virtual nlohmann::json& operator<<(nlohmann::json& j) const { return j; }
	virtual void            operator>>(nlohmann::json& j) {}
};

class Property : public ISerializable
{
public:
	//template <typename T = int>
	//T& get() { return T(); }
};

using PropertyMap = std::unordered_map<std::string, Property*>;

using json = nlohmann::json;

// basic types - write
inline json& operator<<(json& j, const int& val) { j = val; return j; }
inline json& operator<<(json& j, const unsigned& val) { j = val; return j; }
inline json& operator<<(json& j, const float& val) { j = val; return j; }
inline json& operator<<(json& j, const double& val) { j = val; return j; }
inline json& operator<<(json& j, const unsigned short& val) { j = val; return j; }
inline json& operator<<(json& j, const bool& val) { j = val; return j; }
inline json& operator<<(json& j, const std::string& val) { j = val.c_str(); return j; }
inline json& operator<<(json& j, const glm::ivec2& val) { j["x"] << val.x; j["y"] << val.y; return j; }
inline json& operator<<(json& j, const glm::vec2& val) { j["x"] << val.x; j["y"] << val.y; return j; }
inline json& operator<<(json& j, const glm::vec3& val) { j["x"] << val.x; j["y"] << val.y; j["z"] << val.z; return j; }
inline json& operator<<(json& j, const glm::vec4& val) { j["x"] << val.x; j["y"] << val.y; j["z"] << val.z; j["w"] << val.w; return j; }


// basic types - read
inline int& operator>>(const json& j, int& val) { val = j; return val; }
inline unsigned& operator>>(const json& j, unsigned& val) { val = j; return val; }
inline float& operator>>(const json& j, float& val) { val = j; return val; }
inline double& operator>>(const json& j, double& val) { val = j; return val; }
inline unsigned short& operator>>(const json& j, unsigned short& val) { val = j; return val; }
inline bool& operator>>(const json& j, bool& val) { val = j; return val; }
inline std::string& operator>>(const json& j, std::string& val) { val = j.get<std::string>(); return val; }
inline glm::ivec2& operator>>(const json& j, glm::ivec2& val) { j["x"] >> val.x; j["y"] >> val.y; return val; }
inline glm::vec2& operator>>(const json& j, glm::vec2& val) { j["x"] >> val.x; j["y"] >> val.y; return val; }
inline glm::vec3& operator>>(const json& j, glm::vec3& val) { j["x"] >> val.x; j["y"] >> val.y; j["z"] >> val.z; return val; }
inline glm::vec4& operator>>(const json& j, glm::vec4& val) { j["x"] >> val.x; j["y"] >> val.y; j["z"] >> val.z; j["w"] >> val.w; return val; }

#ifdef EDITOR
bool Edit(int& variable, std::string& name);
bool Edit(float& variable, std::string& name);
bool Edit(double& variable, std::string& name);
bool Edit(bool& variable, std::string& name);
bool Edit(std::string& variable, std::string& name);
bool Edit(glm::vec3& variable, std::string& name);
bool Edit(glm::vec4& variable, std::string& name);
#endif // EDITOR

template <typename T>
struct PropertyEX : public Property
{
public:
	PropertyEX(const char* _name, PropertyMap& _properties);
	PropertyEX(const char* _name, const T& _data, PropertyMap& _properties);

	inline nlohmann::json& operator<<(nlohmann::json& j) const override;
	inline void             operator>>(nlohmann::json& j) override;

	T& get();

	operator T() const { return mData; }
	T& operator=(const T& rhs) { mData = rhs; return mData; }

private:
	T mData;
};

#define PROP(_type, _name)              PropertyEX<_type> _name{ #_name, mProperties }
#define PROP_VAL(_type, _name, _value)  PropertyEX<_type> _name{ #_name, _value, mProperties }


/*******************************  Property <T>  ******************************/

template<typename T>
inline PropertyEX<T>::PropertyEX(const char* _name, PropertyMap& _properties)
{
	_properties[_name] = this;
}

template<typename T>
inline PropertyEX<T>::PropertyEX(const char* _name, const T& _data, PropertyMap& _properties)
{
	_properties[_name] = this;
	mData = _data;
}

template<typename T>
inline nlohmann::json& PropertyEX<T>::operator<<(nlohmann::json& j) const
{
	j << mData;
	return j;
}

template<typename T>
inline void PropertyEX<T>::operator>>(nlohmann::json& j)
{
	j >> mData;
}

template<typename T>
inline T& PropertyEX<T>::get() { return mData; }

class AutoSerialize : public ISerializable
{
public:
	inline nlohmann::json& operator<<(nlohmann::json& j) const override;
	inline void operator>>(nlohmann::json& j) override;

	friend inline void operator<<(nlohmann::json& j, const AutoSerialize& _rhs);
	friend inline void operator>>(nlohmann::json& j, AutoSerialize& _rhs);

#ifdef EDITOR
	//void Edit();
#endif

protected:
	PropertyMap mProperties;
};


/******************************  Auto Serialize  *****************************/

inline nlohmann::json& AutoSerialize::operator<<(nlohmann::json& j) const
{
	for (auto& prop : mProperties)
		prop.second->operator<<(j[prop.first]);

	return j;
}
inline void AutoSerialize::operator>>(nlohmann::json& j)
{
	for (auto& prop : mProperties)
		if (j.find(prop.first) != j.end())
			prop.second->operator>>(j[prop.first]);
}

inline void operator<<(nlohmann::json& j, const AutoSerialize& _rhs)
{
	_rhs.operator<<(j);
}
inline void operator>>(nlohmann::json& j, AutoSerialize& _rhs)
{
	_rhs.operator>>(j);
}
