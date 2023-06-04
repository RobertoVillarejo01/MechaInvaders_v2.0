#pragma once
#include <vector>
#include <string>
#include "Utilities/Singleton.h"
#include "Graphics/Renderable/Renderable.h"



class ILogic;
class MenuComp;
class SoundEmitter;
using FnPtr = void (*)();

class MenuManager
{
	MAKE_SINGLETON(MenuManager)

public:
	void CheckInMenu();
	void Update();

	bool InMenu() const;
	void SetInMenu(bool set);
	void SetActiveMenu(MenuComp* menu) { mActiveMenu = menu; }
	void Initialize();
	void Reset() {
		mActiveMenu = nullptr;

		// If any of the pointers saved are deleted or change locations, they should 
		// be cleared here

	}
	MenuComp* GetActiveMenu() { return mActiveMenu; }

	std::map<std::string, FnPtr>	mFunctions;
	std::map<std::string, float*>	mSliders;
	std::map<std::string, bool*>	mCheckboxes;

	float	mSliderTest = 0.5f;
	bool	mCheckboxTest = false;

	void SetInLobby(bool set) { mInLobby = set; }
	bool InLobby() const { return mInLobby; }

private:
	bool		mInMenu = false;
	MenuComp*	mActiveMenu = nullptr;

	bool		mInLobby = true;
};

#define MenuMgr (MenuManager::Instance())


class MenuComp : public IRenderable
{
	struct Box {

		virtual void React(float t = 0.0f) = 0;
#ifdef EDITOR
		virtual void Edit() = 0;
#endif
		virtual void Render(Transform const& tr) = 0;

		std::string		mItemName = "";
		unsigned		mTextIdx = 0;

		glm::vec3		mOffset = { 0.0, 0.0, 0.05 };
		glm::vec2		mScale = { 0.4f, 0.12f };
	};

	struct Slider : public Box {			// Reacts to Left, Right and mouse
		void React(float t = 0.0f) override;
#ifdef EDITOR
		void Edit() override;
#endif
		void Render(Transform const& tr) override;

		float* mProgress = nullptr;
	};

	struct CheckBox : public Box {			// Reacts to mouse and Space / Enter
		void React(float t = 0.0f) override;
#ifdef EDITOR
		void Edit() override;
#endif
		void Render(Transform const& tr) override;

		bool mbUseFnPtrs = true;			// Use Activate/Deactivate
		
		bool* mStatePtr = nullptr;			// The actual state
		FnPtr mActivate = nullptr;			// Activate/Deactivate functions may be needed
		FnPtr mDeactivate = nullptr;


		std::string mActivateName;
		std::string mDeactivateName;
	};

	struct Button : public Box {
		void React(float t = 0.0f) override;
#ifdef EDITOR
		void Edit() override;
#endif
		void Render(Transform const& tr) override;

		FnPtr mFunction = nullptr;

	};
		SoundEmitter* emitter = nullptr;

public:
	void Initialize();
	void Update();
	void Render() override;
	void Shutdown() override;
	void Reset();
	IComp* Clone();

#ifdef EDITOR
	bool Edit() override;
#endif
	void ToJson(nlohmann::json& j) const override;
	void FromJson(nlohmann::json& j) override;

	std::vector<Box*>	mBoxes;
	unsigned			mCurrBox = 0;
	bool				mFirstMenu = false;
	static bool			mLevelShouldStartPaused;
};
