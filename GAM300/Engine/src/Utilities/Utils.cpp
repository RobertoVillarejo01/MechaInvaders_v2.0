#include "AudioManager/Audio.h"
#include "Window/Window.h"
#include "resourcemanager/Resourcemanager.h"
#include "Collisions/CollisionSystem.h"
#include "Graphics/RenderManager/RenderManager.h"
#include "FrameRateController/FrameRateController.h"
#include "Serializer/Factory.h"
#include "Physics/PhysicsSystem/PhysicsSystem.h"
#include "GameStateManager/GameStateManager.h"
#include "LogicSystem/LogicSystem.h"
#include "Networking\Networking.h"
#include "Utils.h"

#ifdef EDITOR
#include "../Editor/src/Editor.h"
#endif // EDITOR


int SystemInit(int width, int height)
{
	// Create the window for the game
	WindowMgr.CreateWindow_("Networking Shooter v0.001 - Pre pre pre pre Alpha", { width, height}, false);

	// Initialize the various systems
	Scene.Load();
	ResourceMgr.InitializeImporters();
	CollisionManager.Initialize();
	RenderMgr.Initialize();
	gAudioMgr.Initialize();
	FRC.Initialize();
	factory.Initialize();
	physicsSystem.Initialize();
	NetworkingMrg.Initialize();

#ifdef EDITOR
	//ImGuiMgr.Initialize();
	EditorMgr.Initialize();
#endif

	GSM.Initialize("LobbyLevel");

	return 0;
}

int SystemUpdate()
{
	// Update the various systems
	Scene.Update();
	CollisionManager.Update();
	gAudioMgr.Update();
	physicsSystem.Update();
	NetworkingMrg.Update();
	LogicMgr.Update();
	//missing graphics?
	//missing logic??

	return 0;
}

bool GameLoop()
{
	while (GSM.IsRunning())
	{
		//check if wants to quit
		if (GSM.GetCurrentLevel() == QUIT_GAME)
			return false;

		GSM.SetCurrLevel(GSM.GetCurrentLevel());

		GSM.Load();

		GSM.Init();

#ifdef EDITOR

		while (GSM.GetCurrentLevel() == GSM.GetNextLevel())
		{
			bool quit = false;

			FRC.Update();
			InputManager.StartFrame();
			InputManager.HandleEnvents(&quit);
			RenderMgr.ClearBuffer();
			EditorMgr.StartFrame();

			if (quit)
			{
				GSM.SetCurrLevel(QUIT_GAME);
				break;
			}

			//if(!Menu)
			SystemUpdate();
			RenderMgr.RenderEditor();
			EditorMgr.MyEditor();
			EditorMgr.Render();
			//else
			//{
			//MenuManager update
			//}

			//swapping buffers
			SDL_GL_SwapWindow(WindowMgr.GetHandle());

		}

#else

		while (GSM.GetCurrentLevel() == GSM.GetNextLevel())
		{
			bool quit = false;

			FRC.Update();
			InputManager.StartFrame();
			InputManager.HandleEnvents(&quit);
			RenderMgr.ClearBuffer();

			if (quit)
			{
				GSM.SetCurrLevel(QUIT_GAME);
				break;
			}

			//if(!Menu)
			SystemUpdate();
			RenderMgr.RenderScene();
			//else
			//{
			//MenuManager update
			//}

			//swapping buffers
			SDL_GL_SwapWindow(WindowMgr.GetHandle());

		}

#endif // !EDITOR

		GSM.Free();

		GSM.Unload();

		if (GSM.GetNextLevel() == RESTART)
			GSM.SetCurrLevel(GSM.GetNextLevel());

		if(GSM.GetCurrentLevel() != QUIT_GAME)
			GSM.SetCurrLevel(GSM.GetNextLevel());

	}

	return false;
}

