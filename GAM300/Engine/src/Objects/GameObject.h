#pragma once
#include <algorithm>
#include <glm/glm.hpp>

#include "System/Base.h"
#include "Geometry/Transform.h"


enum class Tags
{
	None,
	GameObjects,
	Player,
	Enemy,
	EnemyBullet,
	Spawner,
	SpawnPoint,
	Task,
	Ramp,
	Door,
	VendingMachine,
	Icon
};

class IComp;
class Space;
class MemoryManager;

template <typename T, std::size_t count>
struct Block;

class GameObject : public IBase, public ISerializable
{
	public:
		virtual ~GameObject();

		// State Methods
		virtual void SetEnabled(bool enabled);	// Call Set Enabled on all components
		virtual void Initialize();				// Calls initialize on all components
		virtual void Update();
		virtual void Shutdown();
		
		// --------------------------------------------------------------------
		//----------------------JSON-------------------------------------------
		friend bool operator<<(nlohmann::json& j, const GameObject& _rhs);
		friend void operator>>(nlohmann::json& j, GameObject& _rhs);

#pragma region// COMPONENT MANAGEMENT
		// Getters
		void InitComp();
		size_t GetCompCount() const;

		//get the component by type
		template <typename T>
		T* GetComponentType()const;

		//get the component by type
		template <typename T>
		std::vector<T*> GetComponentsType()const;

		// Add/Remove by address
		IComp* AddComp(IComp* pComp);
		void RemoveComp(IComp* pComp);
#pragma endregion

		// Remove the object
		void Destroy();

		Space*		GetSpace()  const { return mSpace; }
		GameObject* GetParent() const { return mParent; }

		GameObject* CreateChild(GameObject* _obj = nullptr);
		void        DestroyChild(GameObject* _obj);
		void		ChangeSpace(Space* space);

		void		SetVisibility(bool _mbVisible);

		// debug only!!
		std::vector<IComp*>&		      GetComps() { return mComps; }
		const std::vector<IComp*>&		  GetComps() const { return mComps; }
		GameObject*					      FindChild(std::string child_name);
		const std::vector<GameObject*>&   GetChilds()const { return mChilds; }

		GameObject* mParent = nullptr;

	protected:
		bool mbEnabled = true;
		Space* mSpace = nullptr;

		std::vector<IComp*> mComps;
		std::vector<GameObject*> mChilds;
	public: 
#ifdef EDITOR
		// (Variables of transform comp) (Should be made properties)
		glm::vec3 prev_rotation = {};
		bool edit_child = false;
		bool attaching = false;
		//Used for undoing transformations
		bool mGuizmoIsUsed = false;
		bool mGuiIsUsed = false;
		bool storeChange = false;
		unsigned mChangesDone = 0;

		GameObject* selected_child = nullptr;

		void Edit();
		void EditCompList();
		void EditTransform();
		void EditAddComp();
#endif
		Transform mTransform = {};
		Tags mTag = Tags::None;

		bool dead = false;
		bool IsEditing = false;
		bool mbUseParentDirection = false;
		bool mbUseParentPosition = true;
	private:
		GameObject();
		GameObject(GameObject* object);
		const GameObject* operator=(GameObject* _go);

		friend Space;
		friend MemoryManager;
		template <typename T, std::size_t count>
		friend struct Block;
};


template <typename T>
T* GameObject::GetComponentType()const
{
	//create a pointer as the same type as the one provided
	T* Type = nullptr;
	//iterate throw all the components
	for (unsigned i = 0; i < mComps.size(); i++)
	{
		//cast the component to the one provided
		Type = dynamic_cast<T*>(mComps[i]);
		//if it exists returns it
		if (Type)
			return Type;
	}
	//otherwise return null
	return nullptr;
}

template <typename T>
std::vector<T*> GameObject::GetComponentsType()const
{
	// resultant vector
	std::vector<T*> result;
	//create a pointer as the same type as the one provided
	T* Type = nullptr;
	//iterate throw all the components
	for (unsigned i = 0; i < mComps.size(); i++)
	{
		//cast the component to the one provided
		Type = dynamic_cast<T*>(mComps[i]);
		//if it exists returns it
		if (Type)
			result.push_back(Type);
	}
	//otherwise return null
	return result;
}