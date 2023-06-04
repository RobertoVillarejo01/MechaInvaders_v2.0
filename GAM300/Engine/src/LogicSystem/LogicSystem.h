#pragma once
#include "Objects/Components.h"
#include "Utilities/Singleton.h"
#include "Events/EventDispatcher.h"

struct Collider;
class ILogic : public IComp, public Listener
{
public:
  void Initialize();
  void Update();
  void Shutdown();

  void SetActive(bool _mode);
  bool IsActive() const;

  virtual IComp* Clone() = 0;

  virtual void OnCollisionStarted(const Collider* coll) {}
  virtual void OnCollisionPersisted(const Collider* coll) {}
  virtual void OnCollisionEnded(const Collider* coll) {}
  void handle_event(const Event& ev) {}

protected:
	void subscribe_collision_event();

private:
  virtual void InternalInitialize() {}
  virtual void InternalUpdate() {}
  virtual void InternalShutdown() {}

  bool          mbAlive = true;
  bool          mbActive = true;
};

class LogicManager
{
	MAKE_SINGLETON(LogicManager)

public:
	void Initialize();
	void Update();
	void ShutDown();
	~LogicManager() {}

private:
};

#define LogicMgr (LogicManager::Instance())
