#include "Serializer/Factory.h"
#include "AudioManager/Audio.h"
#include "Window/Window.h"
#include "resourcemanager/Resourcemanager.h"
#include "Physics/Physics.h"
#include "Graphics/Graphics.h"
#include "Graphics/TextRendering/TextRender.h"
#include "LogicSystem/LogicSystem.h"
#include "Utilities/Utils.h"
#include "MenuManager/MenuManager.h"
#include "GameStateManager.h"
#include "Networking\Networking.h"
#include "../GamePlay/src/WaveSystem/WaveSystem.h"
#include "../GamePlay/src/TaskSystem/TaskSystem.h"

#ifdef EDITOR
#include "../Editor/src/Editor.h"
#endif // EDITOR

namespace {
	static bool gbPrintRenderTime = false;
}

void GameStateManager::Free()
{
#ifdef EDITOR
	EditorMgr.OnLevelChanged();
#endif

	MenuMgr.Reset();
	WaveSys.Shutdown();
	TaskSys.Shutdown();
	gAudioMgr.StopAll();
}

void GameStateManager::Unload()
{
	Scene.ShutDown();
	gAudioMgr.StopAll();
}

nlohmann::json& GameStateManager::operator<<(nlohmann::json& j) const
{
	return j;
}

void GameStateManager::operator>>(nlohmann::json& j)
{

}

void GameStateManager::Initialize(const SystemConfig& cfg)
{
	serializer.GetLevels(mLevels);
	ResourceMgr.LoadFromTXT("./../Resources/to_load.TXT");
	
	if (std::find(mLevels.begin(), mLevels.end(), cfg.mStartingLevel) != mLevels.end())
	{
		mCurrLevel = cfg.mStartingLevel;
		mNextLevel = cfg.mStartingLevel;
	}
	else
	{
		std::cerr << "Could not open level: " << cfg.mStartingLevel << std::endl;
		mCurrLevel = mLevels[0];
		mNextLevel = mLevels[0];
	}

	mIsRunning = true;
	mConfig = cfg;
}

void GameStateManager::Load()
{
	serializer.LoadLevel(mCurrLevel.c_str());
}

void GameStateManager::Init()
{
	Scene.Initialize();
	RenderMgr.Initialize();
}

int GameStateManager::SystemInit(const SystemConfig& cfg)
{
	// Create the window for the game
	mConfig = cfg;
	bool check = WindowMgr.CreateWindow_(cfg.mWindowTitle, { cfg.mWindowSize.x, cfg.mWindowSize.y }, cfg.mbFullScreen);

	assert(check != false);

	// Initialize the various systems
	serializer.Initialize();
	Scene.Load();
	ResourceMgr.InitializeImporters();
	CollisionManager.Initialize();
	RenderMgr.Load();
	gAudioMgr.Initialize();
	FRC.Initialize();
	factory.Initialize();
	physicsSystem.Initialize();
	TranslationMgr.FromJson();
	MenuMgr.Initialize();
	
#ifdef EDITOR
	EditorMgr.Load();
	EditorMgr.Initialize();
#endif

	GSM.Initialize(cfg);

	/* set this to true for testing server/client.
	 * If false will work as singleplayer and won't ask for input */
	NetworkingMrg.Initialize(true);


	return 0;
}

void GameStateManager::SystemUpdate()
{
	// Update the various systems
	//issue, updating the camera even thogh in pause, maybe make each system update the components related to it?
	//so graphics mgr updates renderables, or at least the cameras to avoid this issue
	Scene.FreeDestroyedObjects();

	//if (KeyTriggered(Key::N))
	//	ResourceMgr.HotReload();
	//if (KeyTriggered(Key::M))
	//	RenderMgr.ReloadShaders();
	//if (KeyTriggered(Key::K))
	//	mConfig.mbDbgLines = !mConfig.mbDbgLines;
	//if (KeyTriggered(Key::F2))
	//	WindowMgr.ToggleFullScreen();
	//if (KeyTriggered(Key::F3))
	//	TranslationMgr.ChangeLanguage(TranslationManager::Languages::ENGLISH);
	//if (KeyTriggered(Key::F4))
	//	TranslationMgr.ChangeLanguage(TranslationManager::Languages::SPANISH);
	//
	//if (KeyTriggered(Key::F6))
	//	gbPrintRenderTime = !gbPrintRenderTime;

	if (mConfig.mbCheats)
	{
		//insert the cheat codes here
	}



#ifdef EDITOR

	if (KeyTriggered(Key::F5))
		EditorMgr.SetPause(false);

	if (KeyTriggered(Key::Esc))
	{
		GSM.WS_initialized = false;
		EditorMgr.SetPause(true);
		MenuMgr.Reset();
	}

	//WS_initialized = false;
	if (!EditorMgr.mbInEditor)
	{
		if (!WS_initialized)
		{
			WaveSys.Initialize();
			TaskSys.Initialize();
			RenderMgr.Initialize();
			WS_initialized = true;
		}
		MenuMgr.CheckInMenu();
		gAudioMgr.Update();

		if (MenuMgr.InMenu())
			MenuMgr.Update();

		NetworkingMrg.Update();
		physicsSystem.Update();
		CollisionManager.Update();
		LogicMgr.Update();
		if (WS_initialized)
		{
			WaveSys.Update();
			TaskSys.Update();
		}

	}
	else
	{
		EditorMgr.Update();
	}
#else //!Editor
	if (!WS_initialized)
	{
		WaveSys.Initialize();
		TaskSys.Initialize();
		WS_initialized = true;
	}

	MenuMgr.CheckInMenu();

	if (MenuMgr.InMenu())
		MenuMgr.Update();

	NetworkingMrg.Update();
	physicsSystem.Update();
	CollisionManager.Update();
	LogicMgr.Update();
	if (WS_initialized)
	{
		WaveSys.Update();
		TaskSys.Update();
	}

	gAudioMgr.Update();
#endif

	Scene.Update();

}

void GameStateManager::StartFrame(bool* quit)
{
	FRC.Update();
	InputManager.StartFrame();
	InputManager.HandleEnvents(quit);
	RenderMgr.ClearBuffer();

#ifdef EDITOR
	EditorMgr.StartFrame();
#endif // EDITOR

}

void GameStateManager::EndFrame()
{

#ifdef EDITOR
	EditorMgr.EndFrame();
#endif // EDITOR

	//send networking packets

}

#include <GL/glew.h>
#include <GL/GL.h>

void GameStateManager::GameLoop()
{
	while (mIsRunning)
	{
		Load();
		Init();

		while (mIsRunning && mCurrLevel == mNextLevel)
		{

			bool quit = false;
			StartFrame(&quit);

			if (quit)
			{
				mIsRunning =  false;
				break;
			}

			SystemUpdate();

			if (KeyDown(Key::A) && KeyDown(Key::R)) {
				GSM.SetNextLevel("FirstPlayable");
			}

			if (gbPrintRenderTime) {
				glGenQueries(4, RenderMgr.queryID);
				glQueryCounter(RenderMgr.queryID[0], GL_TIMESTAMP);
			}

#ifdef EDITOR // EDITOR

			if (EditorMgr.mbInEditor)
			{
				RenderMgr.RenderEditor();
				EditorMgr.Render();
			}
			else
				RenderMgr.RenderScene();

#else // !EDITOR

			RenderMgr.RenderScene();
#endif
			if (gbPrintRenderTime) {
				glQueryCounter(RenderMgr.queryID[1], GL_TIMESTAMP);
			}

			if(mConfig.mbDbgLines)
				Debug::DebugRenderer::Instance().Render();

			EndFrame();
			//WindowMgr.SetTemporalName(std::to_string(FRC.GetFrameRate()) + " " +  WindowMgr.GetName());

			//swapping buffers
			CheckGL_Error();
			SDL_GL_SwapWindow(WindowMgr.GetHandle());

			if (gbPrintRenderTime) {
				GLuint64 startTime, stopTime;
				glGetQueryObjectui64v(RenderMgr.queryID[0], GL_QUERY_RESULT, &startTime);
				glGetQueryObjectui64v(RenderMgr.queryID[1], GL_QUERY_RESULT, &stopTime);
				printf("Time spent on the GPU: %f ms\n", (stopTime - startTime) / 1000000.0);
			}
			//
			//glGetQueryObjectui64v(RenderMgr.queryID[2], GL_QUERY_RESULT, &startTime);
			//glGetQueryObjectui64v(RenderMgr.queryID[3], GL_QUERY_RESULT, &stopTime);
			//printf("Time spent on the Shadow Computing: %f ms\n", (stopTime - startTime) / 1000000.0);
		}

		Free();
		WS_initialized = false;

		if (mNextLevel == RESTART)
		{
			WS_initialized = false;
			mNextLevel = mCurrLevel;
			continue;
		}

		if (mNextLevel != QUIT_GAME)
		{
			WS_initialized = false;
			mCurrLevel = mNextLevel;
		}
		Unload();
	}
}

void GameStateManager::SystemShutDown()
{

#ifdef EDITOR
	EditorMgr.Unload();
	ImGuiMgr.CleanUp();
#endif

	// Resources at the manager are done automagically through resource manager 
	// destructor

	//does audio manager free resources also? because i believe it loads them separately
	serializer.ShutDown();
	Scene.ShutDown();
	NetworkingMrg.ShutDown();
	MemoryMgr.ShutDown();
	LogicMgr.ShutDown();
	gAudioMgr.Shutdown();
	WaveSys.Shutdown();
	TaskSys.Shutdown();
	TranslationMgr.ToJson();
}

std::vector<std::string>& GameStateManager::GetLevels() { return mLevels; }

std::string GameStateManager::GetCurrentLevel() const { return mCurrLevel; }

std::string GameStateManager::GetLevel(int level) const
{
	if(level <= mLevels.size())
		return mLevels[level];

	return std::string{};
}

void GameStateManager::SetNextLevel(std::string level) { mNextLevel = level; }

void SystemConfig::Load()
{
	std::ifstream inFile(mSavePath.c_str());
	
	assert(inFile.good());
	
	json j;
	
	if (inFile.good() && inFile.is_open()) {
		inFile >> j;
		operator>>(j);
		inFile.close();
	}
}

void SystemConfig::Save()
{
	json j;
	operator<<(j);

	std::ofstream outFile(mSavePath.c_str());
	if (outFile.good() && outFile.is_open()) {
		outFile << std::setw(4) << j;
		outFile.close();
	}
}

nlohmann::json& SystemConfig::operator<<(nlohmann::json& j) const
{
	j["System Configuration"]["Resolution"] << mWindowSize;
	j["System Configuration"]["FullScreen"] << mbFullScreen;
	j["System Configuration"]["DebugLines"] << mbDbgLines;
	j["System Configuration"]["Cheats"] << mbCheats;
	j["System Configuration"]["VSYNC"] << mbVsync;
	j["System Configuration"]["Staring Level"] << mStartingLevel;
	j["System Configuration"]["Game Level"] << mGameLevel;
	j["System Configuration"]["Title"] << mWindowTitle;
	j["System Configuration"]["LocalHost"] << mbLocalHost;
	j["System Configuration"]["IP"] << mbIP;
	j["System Configuration"]["Port"] << mbPort;
	j["System Configuration"]["Sensitivity"] << mSensitivity;
	
	return j;
}

void SystemConfig::operator>>(nlohmann::json& j)
{
	j["System Configuration"]["Cheats"] >> mbCheats;
	j["System Configuration"]["DebugLines"] >> mbDbgLines;
	j["System Configuration"]["FullScreen"] >> mbFullScreen;
	j["System Configuration"]["Game Level"] >> mGameLevel;
	j["System Configuration"]["Resolution"] >> mWindowSize;
	j["System Configuration"]["Title"] >> mWindowTitle;
	j["System Configuration"]["Staring Level"] >> mStartingLevel;
	j["System Configuration"]["VSYNC"] >> mbVsync;

	if (j["System Configuration"].find("LocalHost") != j["System Configuration"].end())
	{
		j["System Configuration"]["LocalHost"] >> mbLocalHost;
		j["System Configuration"]["IP"] >> mbIP;
		j["System Configuration"]["Port"] >> mbPort;
		j["System Configuration"]["Sensitivity"] >> mSensitivity;
	}
	else
	{
		std::cerr << "Could not find LocalHost in System config file (Resources//config.json)" << std::endl;
	}
}

SystemConfig& SystemConfig::operator=(const SystemConfig& rhs)
{
	mbDbgLines = rhs.mbDbgLines;
	mbFullScreen = rhs.mbFullScreen;
	mbCheats = rhs.mbCheats;
	mbVsync = rhs.mbVsync;
	mStartingLevel = rhs.mStartingLevel;
	mGameLevel = rhs.mGameLevel;
	mWindowSize = rhs.mWindowSize;
	mWindowTitle = rhs.mWindowTitle;
	mSensitivity = rhs.mSensitivity;

	mbLocalHost = rhs.mbLocalHost;
	mbIP = rhs.mbIP;
	mbPort = rhs.mbPort;

	return *this;
}
