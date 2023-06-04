#pragma once
#include "Serializer/Properties/Property.h"
#include "Utilities/Singleton.h"

struct SystemConfig : public ISerializable
{
	glm::ivec2 mWindowSize{};
	bool mbFullScreen = false;
	bool mbVsync = false;
	bool mbDbgLines = false;
	bool mbCheats = false;
	float mSensitivity = 1.0f;
	std::string mStartingLevel;
	std::string mGameLevel;
	std::string mWindowTitle;
	const std::string mSavePath = "./../Resources/config.json";

	bool mbLocalHost = false;
	std::string mbIP;
	unsigned short mbPort = 0u;

	void Load();
	void Save();

	nlohmann::json& operator<<(nlohmann::json& j) const;
	void            operator>>(nlohmann::json& j);

	SystemConfig& operator=(const SystemConfig& rhs);

};

class GameStateManager : public AutoSerialize
{

	MAKE_SINGLETON(GameStateManager)

public:

	void Initialize(const SystemConfig& cfg);
	int SystemInit(const SystemConfig& cfg);

	void Load();
	void Init();

	void StartFrame(bool* quit);
	void GameLoop();
	void EndFrame();

	void Free();
	void Unload();

	void SystemUpdate();
	
	void SystemShutDown();

	std::vector<std::string>& GetLevels();
	std::string GetCurrentLevel() const;
	std::string GetLevel(int level_id) const;
	void SetNextLevel(std::string level);

	void TerminateExecution() { mIsRunning = false; }

	nlohmann::json& operator<<(nlohmann::json& j) const;
	void            operator>>(nlohmann::json& j);

	bool WS_initialized = false;
	SystemConfig mConfig;
private:

	std::string mCurrLevel{};
	std::string mNextLevel{};
	std::string mPrevLevel{};

	bool mIsRunning = false;
	std::vector<std::string> mLevels;
};

#define GSM (GameStateManager::Instance())