#pragma once
#include "Objects/Components.h"
#include "System/Memory/MemoryMgr.h"

class ILogic;
class Space : public IBase, public AutoSerialize
{
public:
	Space(const char* name = "MainArea") : space_name{ name } {};
	~Space() { ShutDown(); }

	void ShutDown();
	void Initialize();
	void Update();
	void FreeDestroyedObjects();

	//OBJECT FEATURES
	void AddObject(GameObject* object);
	GameObject* CreateObject(GameObject* object = nullptr);
	void DestroyObject(GameObject* object);
	void RemoveObject(GameObject* object);
	const std::vector<GameObject*>& GetAliveObjects();

	//OBJECT GETTORS
	GameObject* FindObject(std::string name);
	GameObject* FindObject(int id);
	std::vector<GameObject*> FindObjects(std::string name);
	std::vector<GameObject*> FindObjects(Tags tag);

	//COMPONENTS FEATURES
	void AddComp(IComp* comp);
	void DestroyComp(IComp* comp);
	void RemoveComp(IComp* comp);

	template <typename T>
	T* CreateComp(T* comp = nullptr);
	template <typename T>
	std::vector<IComp*>& GetComponentsType();
	template <typename T>
	std::vector<T*> GetLogicComps();
	
#ifdef EDITOR
	void Edit();
#endif // EDITOR


	//
	void SetSpaceName(const char* name) { space_name = name; }
	std::string GetSpaceName()			{ return space_name; }
	void SetVisibility(bool visible)	{ mbVisible = visible; }
	bool IsVisible() { return mbVisible; }

	nlohmann::json& operator<<(nlohmann::json& j) const override;
	void            operator>>(nlohmann::json& j);

private:
	std::map<TypeInfo, std::vector<IComp*>> alive_components;
	std::vector<IComp*> dead_components;

	std::vector<GameObject*> alive_objects;
	std::vector<GameObject*> dead_objects;

	std::string space_name = "MainArea";
	bool mbVisible = true;
};

template <typename T>
T* Space::CreateComp(T* comp)
{
	/// CALL THE MEMORY MANAGER TO ALLOCATE THE COMPONENT
	T* new_comp = MemoryMgr.CreateEntity<T>(comp);

	AddComp(new_comp);
	return new_comp;
}

template <typename T>
std::vector<IComp*>& Space::GetComponentsType()
{
	TypeInfo type{ typeid(T) };
	return alive_components[type];
}

template <typename T>
std::vector<T*> Space::GetLogicComps()
{
	std::vector<T*> result;
	auto& logic_comps = GetComponentsType<ILogic>();
	std::for_each(logic_comps.begin(), logic_comps.end(),
		[&](IComp* it)
		{
			if (dynamic_cast<T*>(it))
				result.push_back(it);
		});
	return result;
}