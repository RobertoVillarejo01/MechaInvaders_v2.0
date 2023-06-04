#pragma once
#include "PageTable.h"
#include <map>
#include "Utilities/Singleton.h"
#include "System/TypeInfo/TypeInfo.h"
#include "pool.hh"


class IComp;
class GameObject;
class Space;

const unsigned Components_Default_Size	= 50u;
const unsigned Space_Default_Size		= 5u;
const unsigned GameObject_Default_Size	= 50u;

class MemoryManager
{
	MAKE_SINGLETON(MemoryManager)

public:
	void Initialize();
	void ShutDown();
	~MemoryManager();

	template <typename T>
	T* CreateEntity(T* entity = nullptr);

	template <typename T>
	void DestroyEntity(T* object);

#ifdef EDITOR
	bool use_custom_memory = true;
	int  Total_allocations = 0;
	int  Total_deallocations = 0;

	void ShowStats();
#endif // EDITOR

private:
	std::map<TypeInfo, IStorage_*>		memory;
};

#define MemoryMgr (MemoryManager::Instance())

template <typename T>
T* MemoryManager::CreateEntity(T* entity)
{
#ifdef EDITOR
	Total_allocations++;
	if (!use_custom_memory)
		return new T();
#endif // EDITOR

	TypeInfo type = typeid(T);
	IStorage_* storage = memory[type];

	if (storage == nullptr)
	{
 		storage = new Storage<T, mPageTables::get<T::type_id>::size>();
		memory[type] = storage;
	}

	if(entity)
		return new (storage->allocate()) T(*entity);
	return new (storage->allocate()) T();
}

template <typename T>
void MemoryManager::DestroyEntity(T* object)
{
	if (object == nullptr) return;

#ifdef EDITOR
	Total_allocations--;
	if (!use_custom_memory)
	{
		delete object;
		return;
	}
#endif // EDITOR

	TypeInfo type = typeid(*object);
	//object->~T();
	memory[type]->deallocate(object);
	object = nullptr;
}
