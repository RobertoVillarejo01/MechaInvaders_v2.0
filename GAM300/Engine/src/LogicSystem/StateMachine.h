#pragma once

#include <vector>
#include <queue>
#include <iostream>

#include <glm/glm.hpp>
#include "Objects/GameObject.h"
#include "LogicSystem.h"
#include "System/Scene/SceneSystem.h"


const bool gbDebugging = true;

template <typename Derived>
class SMComponent : public ILogic
{
  using SM_Function = void (Derived::*)();
  struct SM_State
  {
    SM_State();
    SM_State(SM_Function _init, SM_Function _update, SM_Function _shutdown, std::string _state_name = std::string("unnamed"));

    SM_Function mInitialize;
    SM_Function mUpdate;
    SM_Function mShutdown;
	std::string mName;

	bool operator!()
	{
		if (mInitialize == nullptr && mUpdate == nullptr && mShutdown == nullptr)
			return true;
		else
			return false;
	}
  };

  struct StateMachine
  {
	void Initialize(Derived* Owner);
    void Update(Derived* Owner);
    void Shutdown(Derived* Owner);

    SM_State              mPreviousState;
    SM_State              mCurrentState;
    SM_State              mNextState;

	SM_State			  mMainState;

	//All the states are stored here
    std::vector<SM_State> mStates;
	//A queue that stores the name and the pointer to the state in the call order
	std::queue<std::pair<SM_Function, std::string>> WS_Queue;
    
	bool                  mbGoNext = false;

	SM_Function GetCurrentStateFn() { return mCurrentState; }
  
	void RegulateQueueSize() { if (WS_Queue.size() >= MAX_QUEUE_SIZE) WS_Queue.pop(); }
  };

  static const size_t MAX_BRAIN_SIZE = 8;
  static const size_t MAX_QUEUE_SIZE = 15;

public:
  size_t GetBrainSize() const;
  void GetAllCurrentStatesNames();

  void ChangeState(const std::string& _name, size_t _pos = 0);
  void ReInitState(const std::string& _name, size_t _pos = 0);

  bool QueueActivated = false;

  IComp* Clone() { return Scene.CreateComp<SMComponent>(Scene.GetSpace("MainArea"), this); }
private:
  void InternalInitialize() final;
  void InternalUpdate() final;
  void InternalShutdown() override;
  virtual void EnterUpdate() {}
  void StateMachineUpdate();
  virtual void EndUpdate() {}

  void ShowDebugInfo();

  std::vector<StateMachine>                 mBrain;

protected:
  /*! Function expected to only be called once, on initialization with the amount SMEnum::TOTAL 
  (which should be the last parameter of an enum with the names of the state machines. Check
  ErremenatariBrain (StateMachineTest file) for reference on this) */
  void SetBrainSize(size_t _size);

  void AddState(SM_Function _init, SM_Function _update,
    SM_Function _shutdown, std::string _state_name = std::string("unnamed"), size_t _pos = 0);

  void SetMainState(std::string _name = std::string("unnamed"), size_t _pos = 0);

  virtual void StateMachineInitialize() {}
  virtual void StateMachineShutDown() {}
};

template<typename Derived>
inline size_t SMComponent<Derived>::GetBrainSize() const { return mBrain.size(); }

template <typename T>
void SMComponent<T>::InternalInitialize()
{
	StateMachineInitialize();

	using Index = decltype(mBrain.size());
	for (Index i = 0; i < mBrain.size(); ++i)
		mBrain[i].Initialize(static_cast<T*>(this));
}

template <typename T>
void SMComponent<T>::InternalUpdate()
{
  if (gbDebugging) ShowDebugInfo();

  EnterUpdate();
  StateMachineUpdate();
  EndUpdate();
}

template<typename Derived>
inline void SMComponent<Derived>::InternalShutdown()
{
	StateMachineShutDown();

  using Index = decltype(mBrain.size());
  for (Index i = 0; i < mBrain.size(); ++i)
    mBrain[i].Shutdown(static_cast<Derived*>(this));
}

template <typename T>
void SMComponent<T>::StateMachineUpdate()
{
  using Index = decltype(mBrain.size());
  for (Index i = 0; i < mBrain.size(); ++i)
    mBrain[i].Update(static_cast<T*>(this));
}

template <typename T>
void SMComponent<T>::GetAllCurrentStatesNames()
{
	for (int i = 0; i < mBrain.size(); i++)
	{
		mBrain[i].GetCurrentStateName();
	}
}

template <typename T>
void SMComponent<T>::ShowDebugInfo() {}

template<typename Derived>
inline void SMComponent<Derived>::AddState(SM_Function _init, SM_Function _update, SM_Function _shutdown, std::string _state_name, size_t _pos)
{
  if (_pos >= mBrain.size() || _pos < 0)
  {
    if (gbDebugging)
      std::cerr << mOwner->GetName() << " .Trying to change non-existent StateMachine (" << _pos << ")" << std::endl;
    return;
  }

  mBrain[_pos].mStates.push_back(SM_State(_init, _update, _shutdown, _state_name));
}

template<typename Derived>
inline void SMComponent<Derived>::SetMainState(std::string _name, size_t _pos)
{
  if (_pos >= mBrain.size() || _pos < 0)
  {
    if (gbDebugging)
      std::cerr << mOwner->GetName() << " .Trying to change non-existent StateMachine (" << _pos << ")" << std::endl;
    return;
  }

  for (int i = 0; i < mBrain[_pos].mStates.size(); i++)
  {
	  if (_name == mBrain[_pos].mStates[i].mName)
	  {
		  mBrain[_pos].mMainState = mBrain[_pos].mStates[i];
		  return;
	  }
  }

  if (gbDebugging)
	  std::cerr << mOwner->GetName() << ". StateMachine(" << _pos << "): " << "Trying to change a non-existent State (" << _name << ")" << std::endl;
}

template<typename Derived>
inline void SMComponent<Derived>::ChangeState(const std::string& _name, size_t _pos)
{
	if (_pos >= mBrain.size() || _pos < 0)
	{
		if (gbDebugging)
			std::cerr << mOwner->GetName() << " .Trying to change non-existent StateMachine (" << _pos << ")" << std::endl;
		return;
	}

	if (mBrain[_pos].mCurrentState.mName == _name)return;

	for (int i = 0; i < mBrain[_pos].mStates.size(); i++)
	{
		if (_name == mBrain[_pos].mStates[i].mName)
		{
			mBrain[_pos].mNextState = mBrain[_pos].mStates[i];
			mBrain[_pos].mbGoNext = true;
			return;
		}
	}

	if (gbDebugging)
		std::cerr << mOwner->GetName() << ". StateMachine(" << _pos << "): " << "Trying to change a non-existent State (" << _name << ")" << std::endl;
}

template<typename Derived>
inline void SMComponent<Derived>::ReInitState(const std::string& _name, size_t _pos)
{
	if (_pos >= mBrain.size() || _pos < 0)
	{
		if (gbDebugging)
			std::cerr << mOwner->GetName() << " .Trying to change non-existent StateMachine (" << _pos << ")" << std::endl;
		return;
	}

	for (int i = 0; i < mBrain[_pos].mStates.size(); i++)
	{
		if (_name == mBrain[_pos].mStates[i].mName)
		{
			mBrain[_pos].mbGoNext = true;
			return;
		}
	}

	if (gbDebugging)
		std::cerr << mOwner->GetName() << ". StateMachine(" << _pos << "): " << "Trying to Reinit a non-existent State (" << _name << ")" << std::endl;
}

template<typename Derived>
inline void SMComponent<Derived>::SetBrainSize(size_t _size)
{
  if (_size > MAX_BRAIN_SIZE) return;
  else mBrain.resize(_size);
}

template<typename Derived>
inline SMComponent<Derived>::SM_State::SM_State() :
	mInitialize{ nullptr }, mUpdate{ nullptr }, mShutdown{ nullptr }, mName{ std::string("unnamed") } {}

template<typename Derived>
inline SMComponent<Derived>::SM_State::SM_State(SM_Function _init, SM_Function _update, SM_Function _shutdown, std::string _state_name) :
	mInitialize{ _init }, mUpdate{ _update }, mShutdown{ _shutdown }, mName{ _state_name } {}

template<typename Derived>
inline void SMComponent<Derived>::StateMachine::Initialize(Derived* Owner)
{
	if (mStates.empty())
		return;
	if (!mMainState)
		//Set main state by default
		mMainState = mStates[0];

	if (mMainState.mInitialize)
	{
		(Owner->*mMainState.mInitialize)();
		WS_Queue.push({ mMainState.mInitialize, std::string(mMainState.mName + ": Initialize") });
	}
	mCurrentState = mMainState;
}

template<typename Derived>
inline void SMComponent<Derived>::StateMachine::Update(Derived* Owner)
{
  /* It has been requested to change the state, and the state should be stored in "mNextState" */
  if (mbGoNext)
  {
    if (mCurrentState.mShutdown)
	{
		(Owner->*mCurrentState.mShutdown)();
		if (Owner->QueueActivated && WS_Queue.back().first != mCurrentState.mShutdown)
		{
			WS_Queue.push({ mCurrentState.mShutdown, std::string(mCurrentState.mName + ": Shut Down") });
			RegulateQueueSize();
		}
	}
	if (mNextState.mInitialize)
	{
		(Owner->*mNextState.mInitialize)();
		mbGoNext = false;
		if (Owner->QueueActivated && WS_Queue.back().first != mNextState.mInitialize)
		{

			WS_Queue.push( { mNextState.mInitialize, std::string(mNextState.mName + ": Initialize") } );
			RegulateQueueSize();
		}
	}
    mPreviousState = mCurrentState;
    mCurrentState  = mNextState;
  }

  /* Even if the state has just been changed it must be Updated too */
  if (mCurrentState.mUpdate)
  {
	  (Owner->*mCurrentState.mUpdate)();
	  if (Owner->QueueActivated && WS_Queue.back().first != mCurrentState.mUpdate)
	  {
		  WS_Queue.push({ mCurrentState.mUpdate, std::string(mCurrentState.mName + ": Update") });
		  RegulateQueueSize();
	  }
  }
}

template<typename Derived>
inline void SMComponent<Derived>::StateMachine::Shutdown(Derived* Owner)
{
  /* Even if the state has just been changed it must be Updated too */
  if (mCurrentState.mShutdown)
    (Owner->*mCurrentState.mShutdown)();
}
